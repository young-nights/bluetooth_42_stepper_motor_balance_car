/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-06     Administrator       the first version
 */
#ifndef APPLICATIONS_MACAPP_INC_RTT_MOTOR_CTRL_H_
#define APPLICATIONS_MACAPP_INC_RTT_MOTOR_CTRL_H_
#include "macSYS.h"




/**
  * @brief  定义一个控制电机正反旋转的数据类型
  * @param  dir    : 0 -> clockwise        1 -> anticlockwise
  * @retval None
  */
typedef enum{
    anticlockwise = 0,
    clockwise,
    auto_direcation,
    forward,
    back,
}motorDir;



/**
  * @brief  定义一个控制电机使能模式的数据类型
  * @param
  * @retval None
  */
typedef enum{
    drv_en = 0,
    drv_disen,
}motorEnable;




/* nDECAY 引脚配置 */
typedef enum{
  slow_decay = 0,
  quick_decay,
  mixed_decay
}motorDECAY;


/* nRESET 引脚配置 */
typedef enum{
  drv_reset = 0,
  drv_set
}motorRESET;





typedef struct _CAR_T mac_car_t;

#define CAR_TYPE float

#define PAI                     3.14   // 圆周率
#define MOTOR_STEPPING_ANGLE    1.8    // 步距角
#define MOTOR_SUBDIVISION_NUM   16     // 细分数
#define WHEEL_DIAMETER          0.065  // 轮子的直径
typedef struct Car_Parameter
{

    CAR_TYPE Perimeter; // 小车轮胎的周长


    //小车左轮的相关参数--------------------------------------------------------------
    CAR_TYPE    Left_PRM;            // 小车左轮的转速
    CAR_TYPE    Left_Speed;          // 小车左轮的线性速度
    rt_uint16_t Left_Frequency_Hz;   // 控制小车左轮的PWM的频率

    //小车右轮的相关参数--------------------------------------------------------------
    CAR_TYPE    Right_PRM;           // 小车右轮的转速
    CAR_TYPE    Right_Speed;         // 小车右轮的线性速度
    rt_uint16_t Right_Frequency_Hz;  // 控制小车右轮的PWM的频率

} Car_Parameter;

struct _CAR_T
{
    Car_Parameter parameter;
};

void DRV1_Enable_Set(motorEnable status);
void DRV1_Direction_Config(motorDir direction);
void DRV1_nRESET_Config(motorRESET value);
void DRV1_Decay_Config(motorDECAY decay_val);
void DRV2_Enable_Set(motorEnable status);
void DRV2_Direction_Config(motorDir direction);
void DRV2_nRESET_Config(motorRESET value);
void DRV2_Decay_Config(motorDECAY decay_val);
void ALL_Motor_Direction_Config(uint8_t dir);
void DRV1_Work_Mode_Set(motorEnable status,motorDir direction,rt_uint32_t Period, rt_uint32_t Dutyfactor);
void DRV2_Work_Mode_Set(motorEnable status,motorDir direction,rt_uint32_t Period, rt_uint32_t Dutyfactor);
int motorCheckTimer_Init(void);

/* Car pick-up detection: returns 1 if tipped over (>45 deg) */
uint8_t If_Car_Was_Picked_Up(void);

#endif /* APPLICATIONS_MACAPP_INC_RTT_MOTOR_CTRL_H_ */
