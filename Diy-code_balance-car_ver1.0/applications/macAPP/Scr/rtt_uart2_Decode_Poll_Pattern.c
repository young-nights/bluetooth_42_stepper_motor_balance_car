/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-15     teati       the first version
 */
#include "rtt_uart2_Decode_Poll_Pattern.h"
#include "rtt_pid_autotune.h"



/* 定义串口设备节点 */
rt_device_t  serial2;
/* 定义串口设备的名称 */
#define USART2_NAME "uart2"
/* 重定向串口配置结构体 */
struct serial_configure usart2Config = RT_SERIAL_CONFIG_DEFAULT;
/* 创建一个信号量用于接收单字节 */
rt_sem_t usart2_rec_sem = RT_NULL;
/* 创建串口2的缓冲区数组结构体 */
xUsart_Structure Uart2Buf;
/* 创建AT回显解析器结构体 */
AT_Response_Parser at_parser;


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
/* 以下是串口1初始化的创建以及回调函数                                                                                                                                                          */
/*---------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  串口2的中断回调函数
  * @retval rt_err_t
  */
rt_err_t Usart2_RX_Callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(usart2_rec_sem);
    return RT_EOK;
}


/**
  * @brief  串口2初始化函数
  * @retval int
  */
int USART2_Init(void)
{
    static rt_size_t sendNum = 0;

    // 创建动态信号量
    usart2_rec_sem = rt_sem_create("dynamic_sem2", 0, RT_IPC_FLAG_FIFO);
    if (usart2_rec_sem == RT_NULL){
        rt_kprintf("PRINTF:%d. create dynamic semaphore failed.\n",Record.kprintf_cnt++);
        return -1;
    }
    else{
        rt_kprintf("PRINTF:%d. create done. dynamic semaphore value = 0.\n",Record.kprintf_cnt++);
    }


    serial2 = rt_device_find(USART2_NAME);
    if(serial2 != RT_NULL){
        rt_kprintf("PRINTF:%d. Usart2 Device node created succeed! \r\n",Record.kprintf_cnt++);
        usart2Config.baud_rate = BAUD_RATE_115200;
        usart2Config.bufsz = 1024;
    }
    else {
        rt_kprintf("PRINTF:%d. Usart2 Device node created Failed! \r\n",Record.kprintf_cnt++);
        return RT_ERROR;
    }

    rt_device_control(serial2, RT_DEVICE_CTRL_CONFIG, &usart2Config);
    rt_device_open(serial2, RT_DEVICE_OFLAG_RDONLY | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(serial2, Usart2_RX_Callback);

    /* 初始化循环队列 */
    Uart2Buf.head = 0;
    Uart2Buf.tail = 0;
    Uart2Buf.lock = rt_mutex_create("uart2_buf_lock", RT_IPC_FLAG_FIFO);


    sendNum = rt_device_write(serial2, RT_NULL, "usart2 is opened!\r\n", 19);
    rt_kprintf("PRINTF:%d. The usart2 test send size : %d\r\n\n",Record.kprintf_cnt++,sendNum);

    return RT_EOK;
}






/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是数据解析线程的创建以及回调函数                                                                                                                                                         */
/*---------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  数据解码回调函数入口
  * @retval void
  */
void uart2_thread_entry(void* parameter)
{
    char recDat = 0;
    rt_size_t sizeValue = 0;
    uint8_t decodeStatus = 0;
    uint8_t usartID = 2;
    while(1)
    {
        sizeValue = rt_device_read(serial2, RT_NULL, &recDat, 1);
        if(sizeValue == 1){
            rt_sem_take(usart2_rec_sem, RT_WAITING_FOREVER);
            if(Flag.AT_REC_Mode == 1){
                /* 将数据存入AT解析器的缓冲区 */
                if(at_parser.index < AT_RESPONSE_MAX_LEN - 1){ /*! 如果数据指向索引位置小于缓冲数组的大小 */
                    at_parser.buffer[at_parser.index++] = recDat;
                    /* 检测结束符 */
                    if(recDat == AT_TERMINATOR){
                        at_parser.buffer[at_parser.index] = '\0';  /*! 添加字符串结束符 */
                        /* 触发解析器 */
                        Parse_AT_Response();
                        /* 重置缓冲区 */
                        at_parser.index = 0;
                    }
                }
                else{   /*! 缓冲区溢出 */
                    at_parser.index = 0;
                    rt_kprintf("AT Buffer Overflow!\n");
                }
            }


            /* 蓝牙通讯解码函数 */
            if(Flag.Protocol_Mode == 1){
                /* 加锁保护队列操作 */
                rt_mutex_take(Uart2Buf.lock, RT_WAITING_FOREVER);
                /* 计算下一个尾指针位置 */
                uint16_t next_tail = (Uart2Buf.tail + 1) % MAX_DATA_LENGTH;
                /* 队列未满 */
                if(next_tail != Uart2Buf.head) {
                    Uart2Buf.rxBuffer[Uart2Buf.tail] = recDat;
                    Uart2Buf.tail = next_tail;
                }
                else {
                    rt_kprintf("UART2 Queue Full! Data Lost: 0x%02X\n", recDat);
                }
                /* 释放互斥锁 */
                rt_mutex_release(Uart2Buf.lock);
                /* 触发协议解析 */
                decodeStatus = USART2_Portocol_Get_Command(serial2,usartID);
                if(decodeStatus == CMD_TRUE) {
                    BEEP_Blink(1, 0, 0);
                }
            }
        }
        rt_thread_mdelay(10);
    }
}





/**
  * @brief  初始化数据解码函数
  * @retval int
  */
rt_thread_t uart2_decodeThread_Handle;
int uart2_decodeThread_Init(void)
{
    uart2_decodeThread_Handle = rt_thread_create("uart2_thread_entry", uart2_thread_entry, RT_NULL, 1024, 10, 200);
    if(uart2_decodeThread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. uart2 Thread is created!!\r\n",Record.kprintf_cnt++);
        USART2_Init();
        rt_thread_startup(uart2_decodeThread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. Thread is not created!!\r\n",Record.kprintf_cnt++);
    }

    return RT_EOK;
}


/*******************************************************************************************************************************/
/*-----------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************************************************************/



#define AT_ENAT                 "AT+ENAT\r\n"
#define AT_EXAT                 "AT+EXAT\r\n"
#define AT_LEON                 "AT+LEON\r\n"
#define AT_LEOF                 "AT+LEOF\r\n"
#define AT_SPON                 "AT+SPON\r\n"
#define AT_SPOF                 "AT+SPOF\r\n"
#define AT_SPNA                 "AT+SPNABalanceCar\r\n"     // SPP蓝牙名称：BalanceCar
#define AT_LENA                 "AT+LENABalanceCar\r\n"     // BLE蓝牙名称：BalanceCar
#define AT_SPAD                 "AT+SPAD0123456789AC\r\n"   // 设置SPP地址：0123456789AC
#define AT_LEAD                 "AT+LEAD234567890ACD\r\n"   // 设置BLE地址：234567890ACD
#define AT_SPNC                 "AT+SPNC\r\n"
#define AT_LENC                 "AT+LENC\r\n"
#define AT_BAUD0                "AT+BAUD0\r\n"
#define AT_BAUD1                "AT+BAUD1\r\n"
#define AT_BAUD2                "AT+BAUD2\r\n"
#define AT_BAUD3                "AT+BAUD3\r\n"
#define AT_BAUD4                "AT+BAUD4\r\n"
#define AT_BAUD5                "AT+BAUD5\r\n"
#define AT_BAUD6                "AT+BAUD6\r\n"
#define AT_REST                 "AT+REST\r\n"
#define AT_RDEF                 "AT+RDEF\r\n"



/*! 进入命令模式 */
void BLE_Send_ENAT(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_ENAT, strlen(AT_ENAT));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}


/*! 进入数据模式 */
void BLE_Send_EXAT(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_EXAT, strlen(AT_EXAT));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}



/*! 开启BLE广播 */
void BLE_Send_LEON(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_LEON, strlen(AT_LEON));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}


/*! 关闭BLE广播 */
void BLE_Send_LEOF(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_LEOF, strlen(AT_LEOF));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}




/*! 修改SPP名称 -- BalanceCar */
void BLE_Send_SPNA(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_SPNA, strlen(AT_SPNA));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}




/*! 修改BLE名称 -- BalanceCar */
void BLE_Send_LENA(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_LENA, strlen(AT_LENA));
    AT_Response_Type resp = AT_Wait_Response(1000);
    if(resp == AT_RESP_TIMEOUT){
        rt_kprintf("AT Command Timeout!\n");
    }
}


/*! 蓝牙模块复位 */
void BLE_Send_REST(void)
{
    Flag.AT_REC_Mode = 1;
    Flag.Protocol_Mode = 0;
    rt_device_write(serial2, RT_NULL, AT_REST, strlen(AT_REST));
    rt_kprintf("AT+REST!\n");
}



/**
 * @brief   AT指令发送到蓝牙后等待蓝牙响应解析器
 *
 *   ——————————————————————————————————————————————————————————————————————————————————————————————————————————
 *     指令说明                           |     指令                |        回应             |     参数说明
 *   ——————————————————————————————————————————————————————————————————————————————————————————————————————————
 *     进入命令模式                            AT+ENAT             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     进入数据模式                            AT+EXAT             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     开启BLE广播                              AT+LEON             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     关闭BLE广播                              AT+LEOF             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     开启SPP广播                              AT+SPON             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     关闭SPP广播                              AT+SPOF             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     修改SPP名称                              AT+SPNA             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     修改BLE名称                              AT+BLNA             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置SPP地址                              AT+SPAD             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置BLE地址                              AT+LEAD             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     断开SPP连接                              AT+SPNC             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     断开BLE连接                              AT+LENC             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率9600            AT+BAUD0            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率19200           AT+BAUD1            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率38400           AT+BAUD2            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率57600           AT+BAUD3            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率115200          AT+BAUD4            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率230400          AT+BAUD5            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率256000          AT+BAUD6            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率460800          AT+BAUD7            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率921600          AT+BAUD8            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置波特率1000000         AT+BAUD9            OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置UUID Service          AT+UIDS             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     设置UUID Write Write      AT+UIDW             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *     Without Response
 *
 *     修改UUID Notify           AT+UIDN             OK\r\n          需要AT指令复位后生效，设置命令会掉电保存
 *
 *     广播内容加MAC地址                   AT+SADV             OK\r\n          广播增加 0X07 0XFF XXXXXXXXXXXX(X是MAC地址)
 *
 *     广播内容恢复出场内容              AT+CADV             OK\r\n          广播恢复默认内容
 *
 *     广播内容加用户自定义              AT+UADV             OK\r\n          广播增加 0Xxx 0XFF XXXXXXXXXXXX(X是用户自定定义内容)
 *                                                                   总广播长度不超过31
 *
 *     设置PA9(LED)输出电平             AT+OUTL             OK\r\n          蓝牙不连接输出高电平，蓝牙连接输出低电平（默认）
 *
 *     设置PA9(LED)输出电平             AT+OUTH             OK\r\n          蓝牙不连接输出低电平，蓝牙连接输出高电平
 *
 *     设置PA9(LED)输出电平             AT+OUTZ             OK\r\n          蓝牙不连接输出1HZ频率电平，蓝牙连接输出低电平
 *
 *     蓝牙模块复位                             AT+RESET            无                            进行复位，使命令生效，让系统重新上电工作
 *
 *     恢复出厂设置                             AT+RDEF             无                            恢复出厂设置会进行以下动作：
 *                                                                   1、BLE名称为: XLBLE
 *                                                                   2、SPP名称为XLBT
 *                                                                   3、关闭SPP广播（产品用不到SPP功能）
 *                                                                   4、BLE MAC地址为FUID的9到14个字节
 *                                                                   5、SPP MAC地址为FUID的9到14个字节
 *                                                                   6、波特率为115200
 *                                                                   7、UUID Server：0xFFE0
 *                                                                   8、 UUID Write Without Response ,Write: 0xFFE1
 *                                                                   9、 UUID Notify: 0xFFE2
 *                                                                   10、恢复默认广播内容
 *                                                                   11、恢复默认PA9(LED)输出电平
 *                                                                   12、模块复位
 * @param   void
 * @return  NULL
 */
void Parse_AT_Response(void)
{
    /* 基础响应检测 */
    if(strstr((char*)at_parser.buffer, "OK\r\n")) {
        at_parser.resp_type = AT_RESP_OK;
        rt_kprintf("AT Response: OK\n");
    }
    else if(strstr((char*)at_parser.buffer, "ERROR\r\n")) {
        at_parser.resp_type = AT_RESP_ERROR;
        rt_kprintf("AT Response: ERROR\n");
    }
    else {
        /* 自定义响应处理 */
        at_parser.resp_type = AT_RESP_CUSTOM;
        rt_kprintf("Custom Response: %s\n", at_parser.buffer);
    }
}





/**
 * @brief   AT指令等待响应函数（超时机制）
 * @param   void
 * @return  NULL
 */
AT_Response_Type AT_Wait_Response(rt_uint32_t timeout)
{
    rt_tick_t start_tick = rt_tick_get();
    at_parser.resp_type = AT_RESP_NONE;

    while((rt_tick_get() - start_tick) < timeout) {
        if(at_parser.resp_type != AT_RESP_NONE) {
            return at_parser.resp_type;
        }
        rt_thread_mdelay(10);
    }
    return AT_RESP_TIMEOUT;
}





/*******************************************************************************************************************************/
/*-----------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************************************************************/

/**
 * @brief 尝试获取一条指令
 * @param command 指令存放指针
 * @return 获取的指令长度
 * @retval 0 没有获取到指令
 */
uint8_t USART2_Portocol_Get_Command(rt_device_t dev, uint8_t USART_ID)
{
    uint8_t decode_result = CMD_ERROR;
    static uint8_t  Decode_Step = 0;
    static uint8_t  CMD_Length = 0;
    static uint8_t  CMD_buffer[64] = {0};
    static uint8_t  CMD_DataCnt = 0;
    static uint8_t  CRC16_H,CRC16_L = 0;
    static uint16_t CRC16_Value = 0;



    while(Uart2Buf.head != Uart2Buf.tail)
    {

        uint8_t DecodeDat = Uart2Buf.rxBuffer[Uart2Buf.head];
        Uart2Buf.head = (Uart2Buf.head + 1) % MAX_DATA_LENGTH;

        /* 然后可以进行正常的数据解析流程 */
        /*--------------------------------*/
        switch(Decode_Step)
        {
            /*****************                第一步数据解析               ****************************/
            case Decode_Step_0:
            {
                if(DecodeDat == 0x55){
                    Decode_Step = Decode_Step_1;
                }
            }break;

            /*****************                第二步数据解析               ****************************/
            case Decode_Step_1:
            {
                if(DecodeDat == 0xAA) {
                    Decode_Step = Decode_Step_2;
                }
                else {
                    Decode_Step = Decode_Step_0;
                }
            }break;

            /*****************                第三步数据解析               ****************************/
            case Decode_Step_2:
            {
                /* Get command length (excluding 2-byte header, 1-byte length, and 2-byte CRC) */
                CMD_Length = DecodeDat;
                /* Buffer overflow protection: validate length */
                if(CMD_Length > CMD_MAX_LENGTH || CMD_Length == 0) {
                    Decode_Step = Decode_Step_0;
                    CMD_DataCnt = 0;
                    rt_kprintf("CMD Length Error: %d\r\n", CMD_Length);
                    return CMD_ERROR;
                }
                CMD_buffer[CMD_DataCnt++] = CMD_Length;
                Decode_Step = Decode_Step_3;
            }break;

            /*****************                第四步数据解析               ****************************/
            case Decode_Step_3:
            {
                if(DecodeDat == DEVICE_ID_H){
                    CMD_buffer[CMD_DataCnt++] = DecodeDat;
                    Decode_Step = Decode_Step_4;
                }
                else{
                    Decode_Step = Decode_Step_0;
                    CMD_DataCnt = 0;
                }
            }break;

            /*****************                第五步数据解析               ****************************/
            case Decode_Step_4:
            {
                if(DecodeDat == DEVICE_ID_L){
                    CMD_buffer[CMD_DataCnt++] = DecodeDat;
                    Decode_Step = Decode_Step_5;
                }
                else{
                    Decode_Step = Decode_Step_0;
                    CMD_DataCnt = 0;
                }
            }break;

            /*****************                第六步数据解析               ****************************/
            case Decode_Step_5:
            {
                CMD_buffer[CMD_DataCnt++] = DecodeDat;
                if(CMD_DataCnt >= (CMD_Length + 1)){
                    Decode_Step = Decode_Step_6;
#if USART2_REC_TEST_PRINTF
                    for(uint8_t d = 0; d < CMD_Length + 1;d++){
                        rt_kprintf("%x ",CMD_buffer[d]);
                    }
#endif
                }
            }break;

            /*****************                第七步数据解析               ****************************/
            case Decode_Step_6:
            {
                CRC16_H = DecodeDat;
#if USART2_REC_TEST_PRINTF
                rt_kprintf("%x ",CRC16_H);
#endif
                Decode_Step = Decode_Step_7;
            }break;

            /*****************                第八步数据解析               ****************************/
            case Decode_Step_7:
            {
                CRC16_L = DecodeDat;
#if USART2_REC_TEST_PRINTF
                rt_kprintf("%x ",CRC16_L);
#endif
                Decode_Step = Decode_Step_0;
            }break;

            default:{
                Decode_Step = Decode_Step_0;
            }break;
        }



        /* CRC校验处理 */
        if((Decode_Step == Decode_Step_0) && (CRC16_H != 0))
        {
            CRC16_Value = CrcCalc_Crc16Modbus(CMD_buffer, CMD_Length + 1);

            if(((CRC16_H << 8) | CRC16_L) == CRC16_Value)
            {
                if(USART_ID == 2)
                {
                    Protocol_Operation_USART2(dev,CMD_buffer);
                    #if USART2_REC_CMD_PRINTF
                        rt_kprintf("Valid   Command: ");
                        for(uint8_t i = 0; i < CMD_Length + 5; i++) {
                            rt_kprintf("%02X ", Uart2Buf.rxBuffer[(Uart2Buf.head - (CMD_Length + 5) + i) % MAX_DATA_LENGTH]);
                        }
                        rt_kprintf("\n");
                    #endif
                        decode_result = CMD_TRUE;
                }
            }
            else
            {
                rt_kprintf("InValid Command: ");
                for(uint8_t i = 0; i < CMD_Length + 5; i++) {
                    rt_kprintf("%02X ", Uart2Buf.rxBuffer[(Uart2Buf.head - (CMD_Length + 5) + i) % MAX_DATA_LENGTH]);
                }
                rt_kprintf("\n");
                rt_kprintf("CRC Error: Calc=0x%04X , Recv=0x%02X%02X\n",CRC16_Value, CRC16_H, CRC16_L);
            }
            /* 重置校验相关变量 */
            CRC16_H = 0;
            CRC16_L = 0;
            CMD_DataCnt = 0;
        }
    }
    return decode_result;
}








static struct rt_mutex uart2_tx_mutex;

/**
 * @brief   STM32通过串口2发送指令到上位机进行应答
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

                // APP发送前进指令---------------------------------------------------------------------------------------------------------------
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

                // APP发送后退指令---------------------------------------------------------------------------------------------------------------
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

                // APP发送左转指令---------------------------------------------------------------------------------------------------------------
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

                // APP发送右转指令---------------------------------------------------------------------------------------------------------------
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

                // APP发送直立指令---------------------------------------------------------------------------------------------------------------
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

                // APP发送停止指令---------------------------------------------------------------------------------------------------------------
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

                // PID自适应整定-启动---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_AUTOTUNE_START_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP start autotune Command\r\n");
                    autotune_start();
                }break;

                // PID自适应整定-中止---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_AUTOTUNE_STOP_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP stop autotune Command\r\n");
                    autotune_stop();
                }break;

                // 从Flash加载已保存PID参数---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_AUTOTUNE_LOAD_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP load autotune params Command\r\n");
                    autotune_load_from_flash();
                }break;

                // 恢复默认PID参数---------------------------------------------------------------------------------------------------------------
                case FRAME_SET_CAR_AUTOTUNE_RESET_CMD:
                {
                    rt_kprintf("PRINTF:%d. APP reset autotune params Command\r\n");
                    autotune_load_default();
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

        // PID自适应整定状态上报------------------------------------------------------------------------------------------------
        case Order_SEND_CAR_AUTOTUNE_STATUS_CMD:
        {
            rt_memset(emptyBuf, 0, sizeof(emptyBuf));
            memcpy(emptyBuf, &tune.best_kp, 4);
            memcpy(emptyBuf + 4, &tune.best_kd, 4);
            USART2_Send_Command_to_Principal(8, FRAME_TYPE_ACT, FRAME_TYPE_POST, emptyBuf);
        }break;


        default: break;
    }
}







