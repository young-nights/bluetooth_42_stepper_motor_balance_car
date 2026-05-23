#include <rtt_uart2_Decode.h>
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-14     teati       the first version
 */


#if USE_UART2_DMA_MODE

/* 定义串口设备节点 */
rt_device_t  serial2;
/* 定义串口设备的名称 */
#define USART2_NAME "uart2"
/* 重定向串口配置结构体 */
struct serial_configure usart2Config = RT_SERIAL_CONFIG_DEFAULT;
/* 定义消息队列控制块（句柄） */
rt_mq_t usart2_QueueHandle = RT_NULL;

/* UART2 TX mutex for exclusive serial access */
static struct rt_mutex uart2_tx_mutex;




/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是解码共用的函数                                                                                                                                                                                                                                             */
/*---------------------------------------------------------------------------------------------------------------*/
/*****************************************************************************
* 功能:       CRC校验码计算（采用Crc16Modbus标准多项式）
* 说明:       校验步骤：
            1、 设置 CRC 寄存器， 并给其赋值 FFFF(hex)
            2、 将数据的第一个 8-bit 字符与 16 位 CRC 寄存器的低 8 位进行异或， 并把结果 存入 CRC 寄存器
            3、 CRC 寄存器向右移一位， MSB 补零， 移出并检查 LSB
            4、 如果 LSB 为 0， 重复第三步； 若 LSB 为 1， CRC 寄存器与多项式码相异或
            5、 重复第 3 与第 4 步直到 8 次移位全部完成。 此时一个 8-bit 数据处理完毕
            6、 重复第 2 至第 5 步直到所有数据全部处理完成
            7、 最终 CRC 寄存器的内容即为 CRC 值
            8、 CRC(16 位)多项式为 X16+X15+X2+1， 其对应校验二进制位列为 1 1000 0000 0000 0101
*****************************************************************************/
uint16_t CrcCalc_Crc16Modbus(uint8_t *dat,uint8_t len)
{
    uint16_t    CRC_index = 0xffff;
    uint16_t    buffer;
    volatile    uint8_t i = 0, j = 0;
    for(i = 0; i < len; i++){
        buffer = dat[i];                            // 把数据取出来放在缓冲区
        CRC_index ^= buffer;
        for(j = 0; j < 8; j++){
            if(CRC_index & 0x0001){
                CRC_index >>= 1;
                CRC_index ^= 0xa001;
            }else{
                CRC_index >>= 1;
            }
        }
    }
    return CRC_index;
}



/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是串口4初始化的创建以及回调函数                                                                                                                                                          */
/*---------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  串口4的DMA回调函数
  * @retval int
  */
rt_err_t Usart2_DMA_RX_Callback(rt_device_t dev, rt_size_t size)
{
    MessageQueue usart2Message;

    usart2Message.device_t = dev;
    usart2Message.size = size;


    rt_mq_send(usart2_QueueHandle, &usart2Message, sizeof(usart2Message));

    return RT_EOK;
}



/**
  * @brief  串口4初始化函数
  * @retval int
  */
int USART2_Init(void)
{
    static rt_size_t sendNum = 0;

    serial2 = rt_device_find(USART2_NAME);
    if(serial2 != RT_NULL){
        rt_kprintf("PRINTF:%d. Usart2 Device node created succeed! \r\n",Record.kprintf_cnt++);
        usart2Config.baud_rate = BAUD_RATE_115200;
        usart2Config.bufsz = 1024;
    }
    else {
        rt_kprintf("PRINTF:%d. Usart2 Device node created Failed!! \r\n",Record.kprintf_cnt++);
        return RT_ERROR;
    }
    rt_device_control(serial2, RT_DEVICE_CTRL_CONFIG, &usart2Config);
    rt_device_open(serial2, RT_DEVICE_FLAG_DMA_RX);
    rt_device_set_rx_indicate(serial2, Usart2_DMA_RX_Callback);

    sendNum = rt_device_write(serial2, RT_NULL, "usart2 is opened!\r\n", 19);
    rt_kprintf("PRINTF:%d. The usart2 test send size : %d\r\n",Record.kprintf_cnt++,sendNum);

    return RT_EOK;
}




/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是数据解析线程的创建以及回调函数                                                                                                                                                         */
/*---------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  数据解码回调函数入口
  * @retval void
  */
void uart2_decode_thread_entry(void* parameter)
{
    MessageQueue        usart2Message;
    xUsart_Structure    usart2Buffer;
    uint8_t decodeStatus = 0;
    uint8_t usartID = 2;
    while(1)
    {

        rt_memset(&usart2Message, 0, sizeof(usart2Message));
        rt_memset(usart2Buffer.rxBuffer, 0, sizeof(usart2Buffer.rxBuffer));

        if(rt_mq_recv(usart2_QueueHandle, &usart2Message, sizeof(usart2Message), RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_device_read(usart2Message.device_t, RT_NULL, &usart2Buffer.rxBuffer, usart2Message.size);
            decodeStatus = USART2_Portocol_Get_Command(usart2Message.device_t,usart2Buffer.rxBuffer,usart2Message.size,usartID);
            if(decodeStatus == 1){
                LED_Blink(LED_Name_1, 1, 0, 0);
            }
        }
    }
}


/**
  * @brief  初始化数据解码函数
  * @retval int
  */
rt_thread_t uart2_decodeThread_Handle;
int uart2_decodeThread_Init(void)
{
    uart2_decodeThread_Handle = rt_thread_create("uart2_decode_thread_entry", uart2_decode_thread_entry, RT_NULL, 2048, 6, 500);
    if(uart2_decodeThread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. uart2 decodeThread is created!!\r\n",Record.kprintf_cnt++);
        /* 创建串口4通讯需要的消息队列 */
        usart2_QueueHandle = rt_mq_create("usart2_Message", 256, 30, RT_IPC_FLAG_FIFO);

        if(usart2_QueueHandle != RT_NULL){
            rt_kprintf("PRINTF:%d. usart2_QueueHandle is created!\r\n",Record.kprintf_cnt++);
        }

        /* 串口4初始化 */
        USART2_Init();

        /* 启动解码线程，必须在消息队列创建成功后开启 */
        rt_thread_startup(uart2_decodeThread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. decodeThread is not created!!\r\n",Record.kprintf_cnt++);
    }
    return RT_EOK;
}





static int USART2_Portocol_Get_Command_Again(rt_device_t dev,const uint8_t *cmdBuf ,uint16_t cmdLength)
{
    uint8_t Again_Decode_Step = 0;
    uint8_t Again_CMD_Length = 0;
    uint8_t Again_CMD_DataCnt = 0;
    uint8_t Again_CMD_buffer[30] = {0};
    uint8_t Again_i = 0;
    uint8_t again_len = 0;
    uint8_t  Again_CRC16_H,Again_CRC16_L = 0;
    uint16_t Again_CRC16_Value = 0;

    /* 如果未处理的数据长度小于指令长度 则不可能有完整的指令 */
    if(cmdLength < CMD_MINI_LENGTH)
    {
        return CMD_ERROR;
    }

    /* 然后可以进行正常的数据解析流程 */
    /*--------------------------------*/
    /*****************                第一步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_0)
    {
        if(*cmdBuf != 0x55)
        {
            /* 然后跳出此次解析，直接进入下一次解析 */
            return CMD_ERROR;
        }
        else
        {
            /* 正确的话数据解析第一步完成，进行变量赋值 */
            Again_Decode_Step = Decode_Step_1;
        }
    }
    /*****************                第二步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_1)
    {
        if(*(cmdBuf + Decode_Step_1) != 0xAA)
        {
            /* 包头的第二个数据不是0xAA -- 给步骤变量赋0值，重新回到0x55判断 */
            Again_Decode_Step = Decode_Step_0;
            /* 然后直接跳出本次解析，重新进入 */
            return CMD_ERROR;
        }
        else
        {
            /* 正确的话数据解析第二步完成，进行变量赋值 */
            Again_Decode_Step = Decode_Step_2;
        }
    }
    /*****************                第三步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_2)
    {
        /* 获取指令长度数据（除指令包头的2个字节，长度1字节以及CRC校验的2字节以外的长度） */
        Again_CMD_Length = *(cmdBuf + Decode_Step_2);
        Again_CMD_DataCnt = 0;
        Again_CMD_buffer[Again_CMD_DataCnt] = Again_CMD_Length;
        Again_CMD_DataCnt++;
        Again_Decode_Step = Decode_Step_3;
    }
    /*****************                第四步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_3)
    {
        /* 对比设备码ID高位进行判断 */
        if(*(cmdBuf + Decode_Step_3) != DEVICE_ID_H){
            Again_Decode_Step = Decode_Step_0;
            Again_CMD_DataCnt = 0;
            return CMD_ERROR;
        }
        else{
            Again_CMD_buffer[Again_CMD_DataCnt] = *(cmdBuf + Decode_Step_3);
            Again_CMD_DataCnt++;
            Again_Decode_Step = Decode_Step_4;
        }
    }

    /*****************                第五步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_4)
    {
        /* 对比设备码ID低位进行判断 */
        if(*(cmdBuf + Decode_Step_4) != DEVICE_ID_L){
            Again_Decode_Step = Decode_Step_0;
            Again_CMD_DataCnt = 0;
            return CMD_ERROR;
        }
        else{
            Again_CMD_buffer[Again_CMD_DataCnt] = *(cmdBuf + Decode_Step_4);
            Again_CMD_DataCnt++;
            Again_Decode_Step = Decode_Step_5;
        }
    }
    /*****************                第六步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_5)
    {
        /* 接收数据 */
        for(Again_i = 0; Again_CMD_DataCnt < (Again_CMD_Length + 1); Again_CMD_DataCnt++,Again_i++){
            Again_CMD_buffer[Again_CMD_DataCnt] = *(cmdBuf + Decode_Step_5 + Again_i);
        }

        Again_CRC16_H = *(cmdBuf + Decode_Step_5 + Again_i);
        Again_Decode_Step = Decode_Step_6;
        Again_i++;
    }
    /*****************                第七步数据解析               ****************************/
    if(Again_Decode_Step == Decode_Step_6)
    {
        Again_CRC16_L = *(cmdBuf + Decode_Step_5 + Again_i);
        Again_Decode_Step = Decode_Step_0;
        Again_CRC16_Value = CrcCalc_Crc16Modbus(Again_CMD_buffer, Again_CMD_Length + 1);
        if(((Again_CRC16_H << 8) | Again_CRC16_L) == Again_CRC16_Value)
        {
            Protocol_Operation_USART2(dev,Again_CMD_buffer);
            /* 进行二次解析 */
            if(*(cmdBuf + Decode_Step_5 + Again_i + 1) == 0x55){
                again_len = cmdLength - (Decode_Step_5 + Again_i) - 1;
                rt_kprintf("PRINTF:%d. cmdLength = %d, again_len = %d, FirstLen = %d\r\n",Record.kprintf_cnt++,cmdLength,again_len,(Decode_Step_5 + Again_i + 1));
                USART2_Portocol_Get_Command_Again(dev,&(*(cmdBuf + Decode_Step_5 + Again_i + 1)),again_len);
            }
#if USART2_REC_CMD_PRINTF
            // -------------------------------------------------------------------------------------
            rt_kprintf("LOG:%d. Android send to USART2 :: 55 aa ",Record.Log_Uart_cnt++);
            for(uint8_t i = 0; i < Again_CMD_Length + 1;i++){
                if(Again_CMD_buffer[i] > 0x0F){
                    rt_kprintf("%x ",Again_CMD_buffer[i]);
                }
                else if(Again_CMD_buffer[i] <= 0x0F){
                    rt_kprintf("0%x ",Again_CMD_buffer[i]);
                }
            }
            rt_kprintf("%x %x",Again_CRC16_H,Again_CRC16_L);
            rt_kprintf("\r\n");
            //-------------------------------------------------------------------------------------
#endif
        return CMD_TRUE;
        }
    }
}





/**
 * @brief 尝试获取一条指令
 * @param command 指令存放指针
 * @return 获取的指令长度
 * @retval 0 没有获取到指令
 */
static uint8_t  Decode_Step = 0;
static uint8_t  CMD_Length = 0;
static uint8_t  CMD_buffer[64] = {0};
static uint8_t  CMD_DataCnt = 0;
static uint8_t  CRC16_H,CRC16_L = 0;
static uint16_t CRC16_Value = 0;
/* Timeout mechanism: reset state machine if no complete frame within 100ms */
static rt_tick_t last_byte_tick = 0;
uint8_t USART2_Portocol_Get_Command(rt_device_t dev, const uint8_t *cmdBuf, const uint16_t cmdLength, uint8_t USART_ID)
{
    uint8_t i = 0;
    uint8_t again_len = 0;
    uint16_t recLen = cmdLength;
    /* If unprocessed data length is less than minimum command length, no complete command possible */
    if(cmdLength < CMD_MINI_LENGTH)
    {
        return CMD_ERROR;
    }

    /* Timeout check: reset state machine if >100ms since last byte */
    rt_tick_t now = rt_tick_get();
    if(Decode_Step != Decode_Step_0 && (now - last_byte_tick) > rt_tick_from_millisecond(100)) {
        Decode_Step = Decode_Step_0;
        CMD_DataCnt = 0;
        CMD_Length = 0;
    }
    last_byte_tick = now;

    /* 然后可以进行正常的数据解析流程 */
    /*--------------------------------*/
    /*****************                第一步数据解析               ****************************/
    if(Decode_Step == Decode_Step_0)
    {
        if(*cmdBuf != 0x55)
        {
            /* 然后跳出此次指令解析，直接进入下一次循环 */
            return CMD_ERROR;
        }
        else
        {
            /* 正确的话数据解析第一步完成，进行变量赋值 */
            Decode_Step = Decode_Step_1;
        }
    }
    /*****************                第二步数据解析               ****************************/
    if(Decode_Step == Decode_Step_1)
    {
        if(*(cmdBuf + Decode_Step_1) != 0xAA)
        {
            /* 包头的第二个数据不是0xAA -- 给步骤变量赋0值，重新回到0x55判断 */
            Decode_Step = Decode_Step_0;
            /* 然后直接跳出本次解析，重新进入 */
            return CMD_ERROR;
        }
        else
        {
            /* 正确的话数据解析第二步完成，进行变量赋值 */
            Decode_Step = Decode_Step_2;
        }
    }
    /*****************                第三步数据解析               ****************************/
    if(Decode_Step == Decode_Step_2)
    {
        /* 获取指令长度数据（除指令包头的2个字节，长度1字节以及CRC校验的2字节以外的长度） */
        CMD_Length = *(cmdBuf + Decode_Step_2);
        /* Buffer overflow protection */
        if(CMD_Length > CMD_MAX_LENGTH || CMD_Length == 0) {
            Decode_Step = Decode_Step_0;
            CMD_DataCnt = 0;
            return CMD_ERROR;
        }
        CMD_DataCnt = 0;
        CMD_buffer[CMD_DataCnt] = CMD_Length;
        CMD_DataCnt++;
        Decode_Step = Decode_Step_3;
    }
    /*****************                第四步数据解析               ****************************/
    if(Decode_Step == Decode_Step_3)
    {
        /* 对比设备码ID高位进行判断 */
        if(*(cmdBuf + Decode_Step_3) != DEVICE_ID_H)
        {
            Decode_Step = Decode_Step_0;
            CMD_DataCnt = 0;
            return CMD_ERROR;
        }
        else
        {
            CMD_buffer[CMD_DataCnt] = *(cmdBuf + Decode_Step_3);
            CMD_DataCnt++;
            Decode_Step = Decode_Step_4;
        }
    }
    /*****************                第五步数据解析               ****************************/
    if(Decode_Step == Decode_Step_4)
    {
        /* 对比设备码ID低位进行判断 */
        if(*(cmdBuf + Decode_Step_4) != DEVICE_ID_L)
        {
            Decode_Step = Decode_Step_0;
            CMD_DataCnt = 0;
            return CMD_ERROR;
        }
        else
        {
            CMD_buffer[CMD_DataCnt] = *(cmdBuf + Decode_Step_4);
            CMD_DataCnt++;
            Decode_Step = Decode_Step_5;
        }
    }
    /*****************                第六步数据解析               ****************************/
    if(Decode_Step == Decode_Step_5)
    {
        /* 接收数据 */
        for(i = 0; CMD_DataCnt < (CMD_Length + 1); CMD_DataCnt++,i++)
        {
            CMD_buffer[CMD_DataCnt] = *(cmdBuf + Decode_Step_5 + i);
        }

        CRC16_H = *(cmdBuf + Decode_Step_5 + i);
        Decode_Step = Decode_Step_6;
        i++;
    }
    /*****************                第七步数据解析               ****************************/
    if(Decode_Step == Decode_Step_6)
    {
        CRC16_L = *(cmdBuf + Decode_Step_5 + i);
        Decode_Step = Decode_Step_0;
        CRC16_Value = CrcCalc_Crc16Modbus(CMD_buffer, CMD_Length + 1);
        if(((CRC16_H << 8) | CRC16_L) == CRC16_Value)
        {
            if(USART_ID == 2){
                Protocol_Operation_USART2(dev,CMD_buffer);
                /* 进行二次解析 */
                if(*(cmdBuf + Decode_Step_5 + i + 1) == 0x55){
                    again_len = recLen - (Decode_Step_5 + i) - 1;
                    rt_kprintf("PRINTF:%d. USART2 cmdLength = %d, again_len = %d, FirstLen = %d\r\n",Record.kprintf_cnt++,recLen,again_len,(Decode_Step_5 + i + 1));
                    USART2_Portocol_Get_Command_Again(dev,&(*(cmdBuf + Decode_Step_5 + i + 1)),again_len);
                }
                else{
#if 0
                    rt_kprintf("PRINTF:%d. One Analytic completion\r\n",Record.kprintf_cnt++);
#endif
                }

#if USART2_REC_CMD_PRINTF
                // 用于打印接收的指令信息---------------------------------------------------------------------
                rt_kprintf("LOG:%d. Hand send to USART2 :: 55 aa ",Record.Log_Uart_cnt++);
                for(uint8_t i = 0; i < CMD_Length + 1;i++){
                    if(CMD_buffer[i] > 0x0F){
                        rt_kprintf("%x ",CMD_buffer[i]);
                    }
                    else if(CMD_buffer[i] <= 0x0F){
                        rt_kprintf("0%x ",CMD_buffer[i]);
                    }
                }
                rt_kprintf("%x %x",CRC16_H,CRC16_L);
                rt_kprintf("\r\n");
                //------------------------------------------------------------------------------------------
#endif
            }
            return CMD_TRUE;
        }
    }
}




/**
 * @brief   Send command from STM32 to host via UART2
 * @param   DataLen    Data length
 *          CmdType    Command type
 *          CmdStatus  Command status
 *          DataBuf    Data buffer pointer
 * @retval  None
 */
void USART2_Send_Command_to_Principal(uint8_t DataLen, uint8_t CmdType, uint8_t CmdStatus, uint8_t* DataBuf)
{
    uint8_t SendDat[30] = {0};
    uint16_t Empty_CRC = 0;
    rt_device_t dev = RT_NULL;

    SendDat[0] = FRAME_HEAD1;
    SendDat[1] = FRAME_HEAD2;
    SendDat[2] = 4 + DataLen;
    SendDat[3] = DEVICE_ID_H;
    SendDat[4] = DEVICE_ID_L;
    SendDat[5] = CmdType;
    SendDat[6] = CmdStatus;
    for(uint8_t i = 0; i < DataLen; i++)
    {
        SendDat[7+i] = DataBuf[i];
    }
    Empty_CRC = CrcCalc_Crc16Modbus(SendDat + 2, 5 + DataLen);
    SendDat[6 + DataLen + 1] = (uint8_t)(Empty_CRC >> 8);
    SendDat[6 + DataLen + 2] = (uint8_t)(Empty_CRC);
    dev = rt_device_find("uart2");
    /* Use mutex instead of critical section for UART TX protection */
    rt_mutex_take(&uart2_tx_mutex, RT_WAITING_FOREVER);
    rt_device_write(dev, RT_NULL, SendDat, 9+DataLen);
#if USART2_SEND_CMD_INFO_PRINTF
    rt_kprintf("LOG:%d. USART2 send to Hand :: 55 aa ",Record.Log_Uart_cnt++);
    for(uint8_t i = 0; i<9+DataLen; i++){
        if(SendDat[i] <= 0x0F){
            rt_kprintf("0%x ",SendDat[i]);
        }
        else{
            rt_kprintf("%x ",SendDat[i]);
        }
    }
    rt_kprintf("\r\n");
#endif
    rt_mutex_release(&uart2_tx_mutex);
}











/**
 * @brief   解析串口4数据域指令，执行响应的函数(小手柄的通讯执行函数)
 * @param   CmdBuf      数据域存放的指针
 *          USART_ID    串口号
 * @retval  None
 */
void Protocol_Operation_USART2(rt_device_t dev,uint8_t* CmdBuf)
{
    /*以 06 00 61 31 02 01 01 数据域指令为例*/
    /*长度 + 设备ID_H + 设备ID_L + 指令类型 + 指令状态 + 实际指令宏 + 指令数据 */
    switch(*(CmdBuf + 3))
    {

        /*********************************************************************************/
        /*    控制指令码 0x31  */
        /*********************************************************************************/
        case FRAME_TYPE_SET:
        {
            switch(*(CmdBuf + 5))
            {
                // APP发送广播指令---------------------------------------------------------------------------------------------------------------
                case FRAME_BOARDCAST_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send broadcast Command\r\n");
                }break;

                // APP send forward command
                case FRAME_SET_CAR_FORWARD_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send forward Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.forward = 1;
                    Flag.back = 0;
                    Flag.left = 0;
                    Flag.right = 0;
                    rt_mutex_release(&Flag.flag_mutex);
                }break;

                // APP send back command
                case FRAME_SET_CAR_BACK_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send back Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.forward = 0;
                    Flag.back = 1;
                    Flag.left = 0;
                    Flag.right = 0;
                    rt_mutex_release(&Flag.flag_mutex);
                }break;

                // APP send left turn command
                case FRAME_SET_CAR_LEFT_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send left Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.forward = 0;
                    Flag.back = 0;
                    Flag.left = 1;
                    Flag.right = 0;
                    rt_mutex_release(&Flag.flag_mutex);
                }break;

                // APP send right turn command
                case FRAME_SET_CAR_RIGHT_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send right Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.forward = 0;
                    Flag.back = 0;
                    Flag.left = 0;
                    Flag.right = 1;
                    rt_mutex_release(&Flag.flag_mutex);
                }break;

                // APP send vertical command
                case FRAME_SET_CAR_VERTICAL_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send start vertical Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.WorkStatus = *(CmdBuf + 6);
                    if(Flag.WorkStatus == 1){
                        LED_On(LED_Name_1);
                        Flag.forward = 0;
                        Flag.back = 0;
                        Flag.left = 0;
                        Flag.right = 0;
                    }
                    rt_mutex_release(&Flag.flag_mutex);
                }break;

                // APP send stop command
                case FRAME_SET_CAR_STOP_OR_ON_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP send stop Command\r\n");
                    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
                    Flag.WorkStatus = *(CmdBuf + 6);
                    if(Flag.WorkStatus == 0){
                        LED_Off(LED_Name_2);
                        Flag.stop = 1;
                        Flag.forward = 0;
                        Flag.back = 0;
                        Flag.left = 0;
                        Flag.right = 0;
                    }
                    else{
                        LED_On(LED_Name_2);
                        Flag.stop = 0;
                    }
                    rt_mutex_release(&Flag.flag_mutex);
                }break;










                // APP发送六面校准总指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_CMD:
                {
                    mpu6xxxParameter.if_start_master_cali_process = 1;
                }break;

                // APP发送六面校准-X正方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PX_CMD:
                {


                }break;

                // APP发送六面校准-X负方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NX_CMD:
                {


                }break;

                // APP发送六面校准-Y正方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PY_CMD:
                {


                }break;

                // APP发送六面校准-Y负方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NY_CMD:
                {


                }break;

                // APP发送六面校准-Z正方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PZ_CMD:
                {


                }break;

                // APP发送六面校准-Z负方向-指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NZ_CMD:
                {


                }break;

                // APP发送陀螺仪静态校准指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_GYRO_CALIBRATION_CMD:
                {
                    mpu6xxxParameter.if_start_gyro_cali_process = 1;
                }break;

                // 设置机械中值校准指令---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_MECHANICALl_MEDIAN_Value_CMD:
                {

                }break;

                default:    break;
            }
        }break;


        default:    break;
    }
}







/**
 * @brief   STM32通过串口4向上位机发送指令
 * @param   order   指令码
 * @retval  None
 */
void USART2_Order_to_Principal(uint8_t order)
{
    uint8_t emptyBuf[20] = {0};
    switch(order)
    {
        // 小车准备就绪，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 05 00 01 32 66 A2 64 0A
        case Order_SEND_CAR_IS_READY_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_ACT_CAR_IS_READY_CMD;
            USART2_Send_Command_to_Principal(1, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;


        // 未完成校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 00 C7 DF
        case Order_SEND_CAR_IS_NOT_FINISHED_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 0;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 完成陀螺仪静态校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 01 07 1E
        case Order_SEND_CAR_IS_FINISHED_GYRO_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 1;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成x轴正-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 02 06 5E
        case Order_SEND_CAR_IS_FINISHED_X_AXIS_POSITIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 2;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成x轴负-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 03 C6 9F
        case Order_SEND_CAR_IS_FINISHED_X_AXIS_NEGETIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 3;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成y轴正-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 04 04 DE
        case Order_SEND_CAR_IS_FINISHED_Y_AXIS_POSITIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 4;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成y轴负-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 05 C4 1F
        case Order_SEND_CAR_IS_FINISHED_Y_AXIS_NEGETIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 5;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成z轴正-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 06 C5 5F
        case Order_SEND_CAR_IS_FINISHED_Z_AXIS_POSITIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 6;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;

        // 已完成z轴负-向下校准，主动上报该指令给APP------------------------------------------------------------------------------------------------
        // 指令码： 55 AA 06 00 73 31 02 1F 07 05 9E
        case Order_SEND_CAR_IS_FINISHED_Z_AXIS_NEGETIVE_CALI_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            emptyBuf[0] = FRAME_SET_CAR_FINISHED_CALIBRATE_CMD;
            emptyBuf[1] = 7;
            USART2_Send_Command_to_Principal(2, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;


        default: break;
    }
}

#endif
