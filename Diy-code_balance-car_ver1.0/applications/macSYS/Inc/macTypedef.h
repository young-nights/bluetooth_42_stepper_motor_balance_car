/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-08     Administrator       the first version
 */
#ifndef APPLICATIONS_MACSYS_INC_MACTYPEDEF_H_

#define APPLICATIONS_MACSYS_INC_MACTYPEDEF_H_

#include <applications/macSYS/Inc/macSYS.h>

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */




// 以下为移植时必须需要的结构体等的初始化---------------------------------------------------------------------------------------------------------
extern uint8_t usartID;
typedef struct {
    rt_uint8_t  kprintf_cnt;                     // 用于打印序列
    rt_uint8_t  Log_cnt;                         // 用于日志打印序列
    rt_uint8_t  Log_Uart_cnt;                    // 用于串口接受指令的日志序列打印
    //------------------------------------------------------------
    /*! 按键相关参数 */
    rt_uint8_t  KeyPowerCnt;                     // 开关电源按下次数


    //------------------------------------------------------------
    /*! 小车pid控制相关参数 */
    rt_uint32_t  motor1_speed;                      // 电机1PWM控制参数
    rt_uint32_t  motor2_speed;                      // 电机2WM控制参数

}RecordStruct;
extern RecordStruct Record;



typedef struct {
    rt_uint32_t log_cnt;
    rt_uint8_t  kprintf_cnt;
    rt_uint8_t  update;                 // Remote upgrade flag
    rt_uint8_t  OldMode;                // Aging mode flag
    rt_uint8_t  AT_REC_Mode;            // AT receive mode
    rt_uint8_t  Protocol_Mode;          // Protocol communication flag
    rt_uint8_t  FirstActivate;          // First startup flag

    //------------------------------------------------------------
    /*! Work status parameters */
    rt_uint8_t  WorkStatus;             // Work status (0: stopped  1: running)
    rt_uint8_t  Left_Direction;         // Left turn flag (0: backward  1: forward)
    rt_uint8_t  Right_Direction;        // Right turn flag (0: backward  1: forward)

    //------------------------------------------------------------
    /*! Car control flags */
    rt_uint8_t  forward;                // Move forward
    rt_uint8_t  back;                   // Move backward
    rt_uint8_t  left;                   // Turn left
    rt_uint8_t  right;                  // Turn right
    rt_uint8_t  stop;                   // Stop (0: start  1: stop)

    //------------------------------------------------------------
    /*! Thread-safe protection */
    struct rt_mutex flag_mutex;         // Mutex for flag thread safety

}FlagStruct;
extern FlagStruct Flag;





typedef struct {

    rt_uint8_t  if_start_master_cali_process;       // 是否开启总校准流程            （0： 不开启       1：开启）
    rt_uint8_t  if_finish_master_cali_process;      // 是否完成总校准流程            （0： 未完成       1：已完成）
    rt_uint8_t  if_start_gyro_cali_process;         // 是否开启陀螺仪静态校准    （0： 不开启       1：开启）
    rt_uint8_t  if_finish_gyro_cali_process;        // 是否完成陀螺仪静态校准    （0： 未完成       1：已完成）
    rt_uint8_t  if_start_x_axis_positive_process;   // 是否开启x轴正方向校准      （0： 不开启       1：开启）
    rt_uint8_t  if_finih_x_axis_negetive_process;   // 是否完成x轴正方向校准      （0： 未完成       1：已完成）
    rt_uint8_t  if_start_y_axis_positive_process;   // 是否开启y轴正方向校准      （0： 不开启       1：开启）
    rt_uint8_t  if_finih_y_axis_negetive_process;   // 是否完成y轴正方向校准      （0： 未完成       1：已完成）
    rt_uint8_t  if_start_z_axis_positive_process;   // 是否开启z轴正方向校准      （0： 不开启       1：开启）
    rt_uint8_t  if_finih_z_axis_negetive_process;   // 是否完成z轴正方向校准      （0： 未完成       1：已完成）

}mpu6xxxStruct;
extern mpu6xxxStruct mpu6xxxParameter;



/**
  * @brief  枚举类型,指令解码步骤
  * @param  None
  */
typedef enum
{
    Decode_Step_0 = 0,
    Decode_Step_1,
    Decode_Step_2,
    Decode_Step_3,
    Decode_Step_4,
    Decode_Step_5,
    Decode_Step_6,
    Decode_Step_7
}DecodeStep_StructType;






//---------------------------------------------------------------------------------------------------------

/**
  * @brief  枚举类型,指令码
  * @param  None
  */
typedef enum
{
    Order_SEND_CAR_IS_READY_CMD = 0,
    Order_SEND_CAR_IS_NOT_FINISHED_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_GYRO_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_X_AXIS_POSITIVE_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_X_AXIS_NEGETIVE_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_Y_AXIS_POSITIVE_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_Y_AXIS_NEGETIVE_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_Z_AXIS_POSITIVE_CALI_CMD,
    Order_SEND_CAR_IS_FINISHED_Z_AXIS_NEGETIVE_CALI_CMD,
}Android_Order_StructType;




// 欧拉角数据结构体
typedef struct {
    float pitch;
    float roll;
    float yaw;
}EulerAngles;
extern EulerAngles carEulerAngles;


/**
  * @brief  枚举类型
  * @param  None
  */
typedef enum
{
    nothing = 0,
    x_axis_interchangeable_y_axis,

}transform_coordinates_StructType;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APPLICATIONS_MACSYS_INC_MACTYPEDEF_H_ */
