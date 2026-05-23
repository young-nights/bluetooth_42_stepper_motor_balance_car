/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-15     teati       the first version
 */
#ifndef APPLICATIONS_MACAPP_INC_RTT_UART2_DECODE_POLL_PATTERN_H_
#define APPLICATIONS_MACAPP_INC_RTT_UART2_DECODE_POLL_PATTERN_H_
#include "macSYS.h"

#define USE_UART2_POLL_PATTERN_DECODE 1
#if USE_UART2_POLL_PATTERN_DECODE

#ifdef BSP_USING_UART2
extern rt_device_t  serial2;
#define USART2_SEND_CMD_INFO_PRINTF     1       // 串口2发送指令信息打印
#define USART2_REC_CMD_PRINTF           1       // 串口2接收指令信息打印
#define USART2_REC_TEST_PRINTF          0       // 串口2接收指令信息打印（测试用）
#endif







//---------------------------------------------------------------------------------------------------------------------------
/* 函数进行解析指令后的返回宏 */
#define CMD_ERROR   0
#define CMD_TRUE    1
/* 主板设备码以及版本号 */
#define DEVICE_ID_H     0x00
#define DEVICE_ID_L     0x73
#define DEVICE_VERSION  0x01  // Version 0.1


/* 创建串口接收与发送缓冲区结构体参数 */
#define MAX_DATA_LENGTH 512
typedef struct{
    uint8_t rxBuffer[MAX_DATA_LENGTH];  // 循环队列缓冲区
    volatile rt_uint16_t rx_index;      // 数据索引
    volatile rt_uint16_t head;          // 队列头指针（读位置）
    volatile rt_uint16_t tail;          // 队列尾指针（写位置）
    rt_mutex_t lock;                    // 互斥锁
}xUsart_Structure;
extern xUsart_Structure Uart2Buf;





//---------------------------------------------------------------------------------------------------------------------------
#define AT_TERMINATOR        '\n'


/* 新增AT响应状态枚举 */
typedef enum {
    AT_RESP_NONE = 0,
    AT_RESP_OK,
    AT_RESP_ERROR,
    AT_RESP_TIMEOUT,
    AT_RESP_CUSTOM
} AT_Response_Type;


/* 新增AT响应解析结构体 */
#define AT_RESPONSE_MAX_LEN 1024
typedef struct {
    uint8_t buffer[AT_RESPONSE_MAX_LEN];
    uint16_t index;
    AT_Response_Type resp_type;
} AT_Response_Parser;




/* 解析指令数据的指令类型以及状态宏 */
//------------------------------------------------------------------------
#define       FRAME_HEAD1                                        (0x55)      // 帧头1
#define       FRAME_HEAD2                                        (0xAA)      // 帧头2
#define       FRAME_TYPE_SET                                     (0x31)      // 帧类型:动作命令
#define       FRAME_TYPE_ACT                                     (0x32)      // 帧类型:参数设置
#define       FRAME_TYPE_GET                                     (0x33)      // 帧类型:参数获取
#define       FRAME_TYPE_POST                                    (0x66)      // 帧类型:主动上报
#define       FRAME_STATE_ASK                                    (0x02)      // 帧状态:上位请求
#define       FRAME_STATE_ACK                                    (0x01)      // 帧状态:下位应答
#define       FRAME_STATE_ERR                                    (0x00)      // 帧状态:校验出错


/* 解析指令数据的实际操作指令宏 */
// 串口4链路相关的指令宏-----------------------------------------------------------------------------
/*! APP发送设置指令 */
#define       FRAME_SET_CAR_FORWARD_CMD                          (0x10)      // 设置：控制小车向前
#define       FRAME_SET_CAR_BACK_CMD                             (0x11)      // 设置：控制小车向后
#define       FRAME_SET_CAR_LEFT_CMD                             (0x12)      // 设置：控制小车左转
#define       FRAME_SET_CAR_RIGHT_CMD                            (0x13)      // 设置：控制小车右转
#define       FRAME_SET_CAR_VERTICAL_CMD                         (0x14)      // 设置：小车启动指令（直立指令）
#define       FRAME_SET_CAR_STOP_OR_ON_CMD                       (0x15)      // 设置：小车停止指令（躺倒指令）
#define       FRAME_SET_CAR_SPEED_SET_CMD                        (0x16)      // 设置：小车的目标速度

/*! APP发送与小车数据校准相关的指令 */
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_CMD            (0x17)      // 设置：六面校准总指令
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PX_CMD         (0x18)      // 设置：六面校准-X正方向-朝下
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NX_CMD         (0x19)      // 设置：六面校准-X负方向-朝下
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PY_CMD         (0x1A)      // 设置：六面校准-Y正方向-朝下
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NY_CMD         (0x1B)      // 设置：六面校准-Y负方向-朝下
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_PZ_CMD         (0x1C)      // 设置：六面校准-Z正方向-朝下
#define       FRAME_SET_CAR_SIX_SIDED_CALIBRATION_NZ_CMD         (0x1D)      // 设置：六面校准-Z负方向-朝下
#define       FRAME_SET_CAR_GYRO_CALIBRATION_CMD                 (0x1E)      // 设置：陀螺仪静态校准
#define       FRAME_SET_CAR_FINISHED_CALIBRATE_CMD               (0x1F)      // 设置：每个校准完后由此发送校准完的提示给APP用于界面弹窗提示
#define       FRAME_SET_CAR_RUNNING_MODE_CMD                     (0x20)      // 设置：小车的行进模式，此模式下只要单独点击就可以进行一直直行或者后退
#define       FRAME_SET_CAR_MECHANICALl_MEDIAN_Value_CMD         (0x21)      // 设置：小车的机械中值校准指令



/*! APP发送获取指令 */
#define       FRAME_GET_CAR_SPEED_CMD                            (0xA0)      // 获取：小车的行进线速度
#define       FRAME_GET_CAR_EULER_ANGLES_CMD                     (0xA1)      // 获取：小车的欧拉角数据（三轴）
/*! 小车单向发送指令 */
#define       FRAME_ACT_CAR_IS_READY_CMD                         (0xA2)      // 发送：小车的准备就绪指令


// 广播指令---------------------------------------------------------------------------------------
#define      FRAME_BOARDCAST_CMD                                 (0x00)      // 广播指令（用于测试）





uint16_t CrcCalc_Crc16Modbus(uint8_t *dat,uint8_t len);
int uart2_decodeThread_Init(void);
uint8_t USART2_Portocol_Get_Command(rt_device_t dev, uint8_t USART_ID);
void USART2_Send_Command_to_Principal(uint8_t DataLen, uint8_t CmdType, uint8_t CmdStatus, uint8_t* DataBuf);
void Protocol_Operation_USART2(rt_device_t dev,uint8_t* CmdBuf);
void USART2_Order_to_Principal(uint8_t order);

void BLE_Send_ENAT(void);
void BLE_Send_EXAT(void);
void BLE_Send_LEON(void);
void BLE_Send_LEOF(void);
void BLE_Send_SPNA(void);
void BLE_Send_LENA(void);
void BLE_Send_REST(void);
void Parse_AT_Response(void);
AT_Response_Type AT_Wait_Response(rt_uint32_t timeout);

#endif /* USE_UART2_POLL_PATTERN_DECODE */

#endif /* APPLICATIONS_MACAPP_INC_RTT_UART2_DECODE_POLL_PATTERN_H_ */
