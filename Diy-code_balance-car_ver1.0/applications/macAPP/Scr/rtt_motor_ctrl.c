/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-06     Administrator       the first version
 */
#include "rtt_motor_ctrl.h"



/***
 *
 * Drv1（电机1）：平衡车左轮
 * Drv2（电机2）：平衡车右轮
 *!电机细分模式   : 16细分 - 一个脉冲走 1.8°/16 = 0.1125° - 一圈是 3200 个脉冲
 *!小车轮胎       : 直径65mm  周长204mm
 */

//----------------------------------------------------------------------------------------------------------------------
/*! 以下是“电机1-DRV1-右轮”相关的函数 */
//----------------------------------------------------------------------------------------------------------------------


/**
  * @brief  控制电机使能
  * @param  nENABL
  * @retval None
  */
void DRV1_Enable_Set(motorEnable status)
{
    if(status == drv_en){
        HAL_GPIO_WritePin(DRV1_NEBALE_GPIO_Port,DRV1_NEBALE_Pin,GPIO_PIN_RESET);
    }
    else if(status == drv_disen){
        HAL_GPIO_WritePin(DRV1_NEBALE_GPIO_Port,DRV1_NEBALE_Pin,GPIO_PIN_SET);
    }
}



/**
  * @brief  电机1的运动方向
  * @param  direction
  * @retval None
  */
void DRV1_Direction_Config(motorDir direction)
{
    if(direction == anticlockwise){
        HAL_GPIO_WritePin(DRV1_DIR_GPIO_Port,DRV1_DIR_Pin,GPIO_PIN_RESET);
    }
    else if(direction == clockwise){
        HAL_GPIO_WritePin(DRV1_DIR_GPIO_Port,DRV1_DIR_Pin,GPIO_PIN_SET);
    }
}


/**
  * @brief  电机1的启动与复位控制
  * @param  value
  * @retval None
  */
void DRV1_nRESET_Config(motorRESET value)
{
    if(value == drv_reset){
        HAL_GPIO_WritePin(DRV1_NRESET_GPIO_Port, DRV1_NRESET_Pin, GPIO_PIN_RESET);
    }
    else if(value == drv_set){
        HAL_GPIO_WritePin(DRV1_NRESET_GPIO_Port, DRV1_NRESET_Pin, GPIO_PIN_SET);
    }
}




/**
  * @brief  Control motor current decay mode
  *         NOTE: PB11 is multiplexed as both nFAULT input and DECAY output.
  *         This design limitation means nFAULT monitoring is unavailable
  *         when DECAY is actively driven.
  * @param  decay
  * @retval None
  */
#define DRV1_DECAY_Pin GPIO_PIN_11
#define DRV1_DECAY_GPIO_Port GPIOB
void DRV1_Decay_Config(motorDECAY decay_val)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(decay_val == slow_decay){
        /*Configure GPIO pins : DRV1_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV1_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(DRV1_DECAY_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DRV1_DECAY_GPIO_Port,DRV1_DECAY_Pin,GPIO_PIN_RESET);
    }
    else if(decay_val == quick_decay){
        /*Configure GPIO pins : DRV1_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV1_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(DRV1_DECAY_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DRV1_DECAY_GPIO_Port,DRV1_DECAY_Pin,GPIO_PIN_SET);
    }
    else if(decay_val == mixed_decay){
        /*Configure GPIO pin : DRV1_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV1_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(DRV1_DECAY_GPIO_Port, &GPIO_InitStruct);
    }
}



















//----------------------------------------------------------------------------------------------------------------------
/*! 以下是“电机2-DRV2-左轮”相关的函数 */
//----------------------------------------------------------------------------------------------------------------------


/**
  * @brief  控制电机使能
  * @param  nENABL
  * @retval None
  */
void DRV2_Enable_Set(motorEnable status)
{
    if(status == drv_en){
        HAL_GPIO_WritePin(DRV2_ENABLE_GPIO_Port,DRV2_ENABLE_Pin,GPIO_PIN_RESET);
    }
    else if(status == drv_disen){
        HAL_GPIO_WritePin(DRV2_ENABLE_GPIO_Port,DRV2_ENABLE_Pin,GPIO_PIN_SET);
    }
}




/**
  * @brief  电机2的运动方向
  * @param  direction
  * @retval None
  */
void DRV2_Direction_Config(motorDir direction)
{
    if(direction == anticlockwise){
        HAL_GPIO_WritePin(DRV2_DIR_GPIO_Port,DRV2_DIR_Pin,GPIO_PIN_RESET);
    }
    else if(direction == clockwise){
        HAL_GPIO_WritePin(DRV2_DIR_GPIO_Port,DRV2_DIR_Pin,GPIO_PIN_SET);
    }
}


/**
  * @brief  电机2的启动与复位控制
  * @param  value
  * @retval None
  */
void DRV2_nRESET_Config(motorRESET value)
{
    if(value == drv_reset){
        HAL_GPIO_WritePin(DRV2_NRESET_GPIO_Port, DRV2_NRESET_Pin, GPIO_PIN_RESET);
    }
    else if(value == drv_set){
        HAL_GPIO_WritePin(DRV2_NRESET_GPIO_Port, DRV2_NRESET_Pin, GPIO_PIN_SET);
    }
}



/**
  * @brief  控制电机电流的衰减模式
  * @param  decay
  * @retval None
  */
#define DRV2_DECAY_Pin GPIO_PIN_7
#define DRV2_DECAY_GPIO_Port GPIOA
void DRV2_Decay_Config(motorDECAY decay_val)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    if(decay_val == slow_decay){
        /*Configure GPIO pins : DRV2_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV2_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DRV2_DECAY_GPIO_Port,DRV2_DECAY_Pin,GPIO_PIN_RESET);
    }
    else if(decay_val == quick_decay){
        /*Configure GPIO pins : DRV2_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV2_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DRV2_DECAY_GPIO_Port,DRV2_DECAY_Pin,GPIO_PIN_SET);
    }
    else if(decay_val == mixed_decay){
        /*Configure GPIO pin : DRV2_DECAY_Pin */
        GPIO_InitStruct.Pin = DRV2_DECAY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(DRV2_DECAY_GPIO_Port, &GPIO_InitStruct);
    }
}

















//----------------------------------------------------------------------------------------------------------------------
/*! 以下是两个电机的总体控制相关的函数 */
//----------------------------------------------------------------------------------------------------------------------

/**
  * @brief  根据小车倾倒情况同时控制小车的转向情况
  * @param  void
  * @retval None
  */
void ALL_Motor_Direction_Config(uint8_t dir)
{

    /* 向前运行  */
    if(Flag.Left_Direction == 1 && Flag.Right_Direction == 1 && dir == auto_direcation){
        DRV1_Direction_Config(anticlockwise);
        DRV2_Direction_Config(clockwise);
    }

    /* 向后运行 */
    if(Flag.Left_Direction == 0 && Flag.Right_Direction == 0 && dir == auto_direcation){
        DRV1_Direction_Config(clockwise);
        DRV2_Direction_Config(anticlockwise);
    }
}






/**
  * @brief  DRV1 Work Mode Set
  * @param  motorDir    : 0 -> clockwise        1 -> anticlockwise
  *         Period      : The output period of a waveform.  The value input is in milliseconds
  *         Dutyfactor  : The duty cycle of a waveform.     This value does not exceed the value of the period
  * @retval None
  */
void DRV1_Work_Mode_Set(motorEnable status,motorDir direction,rt_uint32_t Period, rt_uint32_t Dutyfactor)
{
    /* 禁用芯片驱动复位 */
    DRV1_nRESET_Config(drv_set);

    /* 驱动电流衰减模式 */
    DRV1_Decay_Config(mixed_decay);

    /* 设置电机的使能与非使能状态 */
    DRV1_Enable_Set(status);

    /* 设置电机的旋转方向 */
    DRV1_Direction_Config(direction);

    /* 设置电机转动的PWM */
    Motor_Left_Set(Period,Dutyfactor);
}



/**
  * @brief  DRV2 Work Mode Set
  * @param  motorDir    : 0 -> clockwise        1 -> anticlockwise
  *         Period      : The output period of a waveform.  The value input is in milliseconds
  *         Dutyfactor  : The duty cycle of a waveform.     This value does not exceed the value of the period
  * @retval None
  */
void DRV2_Work_Mode_Set(motorEnable status,motorDir direction,rt_uint32_t Period, rt_uint32_t Dutyfactor)
{

    /* 禁用芯片驱动复位 */
    DRV2_nRESET_Config(drv_set);

    /* 驱动电流衰减模式 */
    DRV2_Decay_Config(mixed_decay);

    /* 设置电机的使能与非使能状态 */
    DRV2_Enable_Set(status);

    /* 设置电机的旋转方向 */
    DRV2_Direction_Config(direction);

    /* 设置电机转动的PWM */
    Motor_Right_Set(Period,Dutyfactor);
}





/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是电机运行状态检测线程的创建以及回调函数                                                                            */
/*---------------------------------------------------------------------------------------------------------------*/

#define DRV1_nFAULT_GPIO_Read()         HAL_GPIO_ReadPin(DRV1_FAULT_GPIO_Port,DRV1_FAULT_Pin)
#define DRV2_nFAULT_GPIO_Read()         HAL_GPIO_ReadPin(DRV2_FAULT_GPIO_Port,DRV2_FAULT_Pin)

/**
  * @brief  beepTimer Callback Function
  * @retval void
  */
static void motorCheckTimer_callback(void* parameter)
{
    if(DRV1_nFAULT_GPIO_Read() == 0){
        rt_kprintf("LOG:%d. DRV1 Hard Fault\r\n",Record.Log_cnt++);
    }

    if(DRV2_nFAULT_GPIO_Read() == 0){
        rt_kprintf("LOG:%d. DRV2 Hard Fault\r\n",Record.Log_cnt++);
    }
}



/**
  * @brief  beepTimer initialize
  * @retval int
  */
int motorCheckTimer_Init(void)
{
    static rt_timer_t   motorCheckTimer = RT_NULL;
    /* 创建beep的软件定时器线程 */
    motorCheckTimer = rt_timer_create("motorCheckTimer_callback", motorCheckTimer_callback, RT_NULL, 100,
            RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);

    /* 启动beep软件定时器 */
    if(motorCheckTimer != RT_NULL )
    {
        rt_kprintf("PRINTF:%d. Motor Status Check initialize succeed!\r\n",Record.kprintf_cnt++);

        rt_timer_start(motorCheckTimer);
    }

    return RT_EOK;
}










//----------------------------------------------------------------------------------------------------------------------
/*! 以下是和电机控制速度相关的函数   */



/**
  * @brief  平衡车左轮转速--理论计算值（电机1）
  *
  *                     360°
  *         每转步数 = ———————————— * 细分数
  *                    步距角(°)
  *
  *                  脉冲频率(Hz)
  *         PRM   = ————————————— * 60
  *                    每转步数
  *
  * @param  NULL
  *
  * @retval 左轮转速
  */
CAR_TYPE Left_Speed_Calculate(mac_car_t *car)
{
    car->parameter.Perimeter = (float)(PAI * WHEEL_DIAMETER);
    /* 步数/每转 */
    CAR_TYPE steps_per_rev = (360.0f / MOTOR_STEPPING_ANGLE) * MOTOR_SUBDIVISION_NUM;
    /* 转数/分钟 */
    car->parameter.Left_PRM = (car->parameter.Right_Frequency_Hz * 60.0f)/steps_per_rev;
    /* m/s */
    car->parameter.Left_Speed = car->parameter.Left_PRM * car->parameter.Perimeter / 60.0f;

    return car->parameter.Left_Speed;
}




/**
  * @brief  平衡车右轮转速--理论计算值（电机2）
  *
  *                     360°
  *         每转步数 = ———————————— * 细分数
  *                    步距角(°)
  *
  *                  脉冲频率(Hz)
  *         PRM   = ————————————— * 60
  *                    每转步数
  *
  * @param  NULL
  *
  * @retval 右轮转速
  */
CAR_TYPE Right_Speed_Calculate(mac_car_t *car)
{
    car->parameter.Perimeter = (float)(PAI * WHEEL_DIAMETER);

    /* 步数/每转 */
    CAR_TYPE steps_per_rev = (360.0f / MOTOR_STEPPING_ANGLE) * MOTOR_SUBDIVISION_NUM;
    /* 转数/分钟 */
    car->parameter.Right_PRM = (car->parameter.Right_Frequency_Hz * 60.0f)/steps_per_rev;
    /* m/s */
    car->parameter.Right_Speed = car->parameter.Right_PRM * car->parameter.Perimeter / 60.0f;

    return car->parameter.Right_Speed;
}



/**
  * @brief  Car pick-up detection: checks if the car has been tipped over or picked up
  * @param  void
  * @retval 1 if car is picked up/tipped, 0 otherwise
  */
static inline uint8_t If_Car_Was_Picked_Up(void)
{
    extern EulerAngles carEulerAngles;
    /* Check if pitch or roll angle exceeds threshold (car tipped over or picked up) */
    if(fabsf(carEulerAngles.pitch) > 45.0f || fabsf(carEulerAngles.roll) > 45.0f) {
        return 1;  /* Car tipped over or picked up */
    }
    return 0;
}


int If_Car_Was_Picked_Up_Orig(float Acceleration,float Angle,int encoder_left,int encoder_right)
{
    uint8_t stepNum = 0;
    uint8_t status = 0;

    if(stepNum == 0){

    }

    return status;
}


