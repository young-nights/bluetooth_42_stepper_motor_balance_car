/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-07     teati       the first version
 */
#ifndef APPLICATIONS_MACAPP_INC_RTT_MOTOR_PID_H_
#define APPLICATIONS_MACAPP_INC_RTT_MOTOR_PID_H_
#include "macSYS.h"



#define USE_MY_ARITHMETIC   1


#if USE_MY_ARITHMETIC
#define TARGET_MAX  FLT_MAX     // 默认最大限幅值
#define OUT_MAX     FLT_MAX     // 默认最大限幅值

typedef struct _PID_T mac_pid_t;
#define PID_TYPE float





typedef union
{
    float kd_lpf;    // 不完全微分系数
    PID_TYPE kd_pre; // 微分先行系数
} kd_u;



typedef struct PID_Parameter
{
    kd_u kd_;

    PID_TYPE dt;    // 时间周期
    PID_TYPE kp;    // 比例系数
    PID_TYPE ki;    // 积分系数
    PID_TYPE kd;    // 微分系数

    // 和俯仰角相关的参数-----------------------------------------------
    PID_TYPE pitch_target_limit;      // 俯仰角的目标值限幅
    PID_TYPE pitch_bias_limit;        // 俯仰角的误差值限幅
    PID_TYPE pitch_target;            // 俯仰角目标值
    /*! 当误差的绝对值小于这个阈值时,积分项才会被激活,进行累积；
     *! 而当误差超过这个阈值时,积分项会被暂停,防止积分项过度累积 */
    PID_TYPE pitch_bias_for_integral; // 启用积分项的误差阈值   --  用于积分分离

    // 和小车线速度相关的参数-----------------------------------------------
    PID_TYPE speed_target_limit;        // 速度限幅
    PID_TYPE speed_bias_limit;          // 速度误差限幅
    PID_TYPE speed_target;              // 速度目标值
    PID_TYPE speed_integral_limit;      // 速度积分限幅
    PID_TYPE speed_bias_for_integral;   // 启用积分项的误差阈值   --  用于积分分离



    PID_TYPE out_limit;              // 输出限幅

} PID_Parameter;




typedef struct PID_Process
{
    // 和角度相关的参数-------------------------------------------------
    PID_TYPE pitch_bias;            // 俯仰角偏差
    PID_TYPE pitch_present;         // 俯仰角当前值
    PID_TYPE pitch_integral_bias;   // 俯仰角积分误差

    // 和角速度相关的参数-----------------------------------------------
    PID_TYPE ygyro_bias;                  // 角速度偏差
    PID_TYPE ygyro_present;               // 角速度的当前值
    PID_TYPE ygyro_last_bias;             // 上次角速度偏差
    PID_TYPE ygyro_lastlast_bias;         // 上上次角速度偏差
    PID_TYPE ygyro_different_bias;        // y轴角速度微分偏差
    PID_TYPE ygyro_lpf_different_bias;    // 上次经过低通滤波器的y轴角速度偏差



    // 和速度环相关的参数-----------------------------------------------
    PID_TYPE speed_bias;                  // 速度偏差
    PID_TYPE speed_last_bias;             // 速度上一次的偏差
    PID_TYPE speed_lastlast_bias;         // 速度上上一次的偏差
    PID_TYPE speed_integral_bias;         // 速度偏差积分积累
    PID_TYPE speed_present;               // 当前速度


    PID_TYPE out;                   // 此节点pid输出

} PID_Process;



/* 3-axis data structure - uint: m/s² */
struct mpu6xxx_3axis_m_per_s
{
    float x;
    float y;
    float z;
};



struct _PID_T
{
    PID_Parameter speedParam;
    PID_Parameter verticalParam;
    PID_Process   speedProcess;
    PID_Process   verticalProcess;
};

#endif







int balance_Car_Thread_Init(void);







#endif /* APPLICATIONS_MACAPP_INC_RTT_MOTOR_PID_H_ */
