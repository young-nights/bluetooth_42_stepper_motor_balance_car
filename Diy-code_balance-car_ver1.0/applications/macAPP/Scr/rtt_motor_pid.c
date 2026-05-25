/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-07     teati       the first version
 */
#include "rtt_motor_pid.h"


#if USE_MY_ARITHMETIC

mac_pid_t carPID;


/**
  * @brief  数值约束函数
  * @param  val : 目标数值
  *         min : 最小对比值
  *         max : 最大对比值
  * @retval 约束值
  */
static inline float constrain(float val, float min, float max)
{
    return (val < min) ? min : ((val > max) ? max : val);
}




// 直立环相关函数------------------------------------------------------------------------------------

/**
 * @description: 将pid参数复位成原始状态
 * @param {apid_t*} pid 实例句柄
 * @return {*}
 */
void PID_Vertical_Paramter_DeInit(mac_pid_t *pid)
{
    memset(pid,0,sizeof(mac_pid_t));

    pid->verticalParam.dt = 0;
    pid->verticalParam.kp = 0;
    pid->verticalParam.ki = 0;
    pid->verticalParam.kd = 0;

    // 俯仰角相关参数初始化-------------------------------------------------------------
    pid->verticalParam.pitch_target_limit = TARGET_MAX;
    pid->verticalParam.pitch_bias_for_integral = -1;
    pid->verticalParam.pitch_bias_limit = -1;
    pid->verticalParam.pitch_target = 0;

}


/**
 * @description: 初始化pid,配置必要参数
 * @param {apid_t*} pid 实例句柄
 * @param {ALL_PID_Mode} mode 增量式或位置式
 * @param {PID_TYPE} kp pid参数
 * @param {PID_TYPE} ki pid参数
 * @param {PID_TYPE} kd pid参数
 * @return {*}
 */
void PID_Vertical_Parameter_Init(mac_pid_t *pid)
{
    PID_Vertical_Paramter_DeInit(pid);

    pid->verticalParam.kp = 15.0f;
    pid->verticalParam.ki = 0;
    pid->verticalParam.kd = 4.0f;  /* Increased from 0.1 to 4.0: 40× greater damping to suppress high-frequency oscillation */
    /* 设置输出限幅 */
    pid->verticalParam.out_limit = 1300;
    /* 姿态更新周期 */
    pid->verticalParam.dt = 0.005;
}




/**
  * @brief  直立环控制器(PD控制器)
  *         Kp*Ek+Kd*Ek_D
  *
  * @param  Angle : 真实角度
  *         Gyro  : 真实角速度
  *         Median: 机械中值（期望角度）
  *
  * @retval 直立环输出
  */
//#define Mechanical_Median_Value 0.66    //直立环的机械中值(平衡角度)
#define Mechanical_Median_Value 0.36    //直立环的机械中值(平衡角度)
PID_TYPE Vertical_Ring_Controller(mac_pid_t *pid, PID_TYPE Angle, PID_TYPE Gyro, PID_TYPE Median)
{
    //================ 角度误差计算 ===================================
    pid->verticalParam.pitch_target = Median;
    pid->verticalProcess.pitch_present = Angle;
    pid->verticalProcess.pitch_bias = pid->verticalProcess.pitch_present - pid->verticalParam.pitch_target;
    pid->verticalProcess.pitch_bias = moving_average_filtre(&Angle_Move_Filter,pid->verticalProcess.pitch_bias);
    //================ 角速度误差计算 ===================================
    /* 单位转换：MPU6050输出的单位是deg/10s,需要转换为deg/s */
    pid->verticalProcess.ygyro_present = Gyro * 0.1f;
    pid->verticalProcess.ygyro_bias = pid->verticalProcess.ygyro_present - 0;

    //================ 直立环输出计算（PD控制器） ===================================
    pid->verticalProcess.out = pid->verticalParam.kp * pid->verticalProcess.pitch_bias +
                               pid->verticalParam.kd * pid->verticalProcess.ygyro_bias;


    /* 输出限幅 */
    pid->verticalProcess.out = constrain(pid->verticalProcess.out, -pid->verticalParam.out_limit, pid->verticalParam.out_limit);

    return pid->verticalProcess.out;
}















// 速度环相关函数------------------------------------------------------------------------------------
/**
 * @description: 将pid参数复位成原始状态
 * @param {apid_t*} pid 实例句柄
 * @return {*}
 */
void PID_Speed_Paramter_DeInit(mac_pid_t *pid)
{
    pid->speedParam.dt = 0;
    pid->speedParam.kp = 0;
    pid->speedParam.ki = 0;
    pid->speedParam.kd = 0;

    // 速度相关参数初始化-------------------------------------------------------------
    pid->speedParam.speed_target_limit = TARGET_MAX;
    pid->speedParam.speed_bias_for_integral = -1;
    pid->speedParam.speed_bias_limit = -1;
    pid->speedParam.speed_target = 0;

}


/**
 * @description: 初始化pid,配置必要参数
 * @param {apid_t*} pid 实例句柄
 * @param {ALL_PID_Mode} mode 增量式或位置式
 * @param {PID_TYPE} kp pid参数
 * @param {PID_TYPE} ki pid参数
 * @param {PID_TYPE} kd pid参数
 * @return {*}
 */
void PID_Speed_Parameter_Init(mac_pid_t *pid)
{
    PID_Speed_Paramter_DeInit(pid);

    pid->speedParam.dt = 0.005;
    pid->speedParam.kp = 0.0f;   /* Disabled (was 10.0): speed estimation from accelerometer integration is too noisy for reliable PI control */
    pid->speedParam.ki = 0.0f;   /* Disabled (was 0.1): re-enable after encoder-based speed feedback is implemented */
    pid->speedParam.kd = 0;


    pid->speedParam.speed_bias_for_integral = 3000;
    pid->speedParam.speed_integral_limit = 3000;
    pid->speedParam.speed_target = 0;
    pid->speedParam.out_limit = 6000;
}


/**
  * @brief  Linear velocity estimation using MPU6xxx accelerometer
  *         v = ∫(ax − g⋅sinθ)dt
  *
  * @param  ax       : Accelerometer X-axis output (forward direction, m/s²)
  *         theta(θ) : Pitch angle
  *         g        : Gravity acceleration (9.8m/s²)
  *
  * @retval Estimated linear velocity (m/s)
  */
PID_TYPE Estimate_Speed(float ax, EulerAngles *theta)
{
    static float velocity = 0.0f;
    /* Control period (5ms) */
    static float dt = 0.005f;

    /* Remove gravity component (theta in radians) */
    float horizontal_accel = ax - 9.80665f * sinf(theta->pitch);

    /* First-order low-pass filter */
    static float filter_accel = 0.0f;
    filter_accel = FOLowPassFilter(horizontal_accel, filter_accel, 0.8);

    /* Acceleration dead zone to suppress noise */
    if(fabsf(filter_accel) < ACCEL_DEAD_ZONE) filter_accel = 0.0f;

    /* Integrate to get velocity (high-pass filter to suppress drift) */
    velocity += filter_accel * dt;
    velocity *= 0.995f; /* High-pass filter coefficient */

    /* Velocity dead zone to suppress static drift */
    if(fabsf(velocity) < VELOCITY_DEAD_ZONE) velocity = 0.0f;

    return velocity;
}



/**
 * @description: 内部使用,位置式积分分离
 * @param {apid_t*} pid 实例句柄
 * @param {parameter.kd_lpf} *
 * @return {*}
 */
void i_Speed_Position_Separation(mac_pid_t *pid)
{
    /* 积分分离 -- 速度偏差大于积分阈值,就不进行积分积累 */
    if(pid->speedProcess.speed_bias > pid->speedParam.speed_bias_for_integral ||
       pid->speedProcess.speed_bias < -pid->speedParam.speed_bias_for_integral){
        return;
    }
    /* 积分积累 */
    pid->speedProcess.speed_integral_bias = pid->speedProcess.speed_bias * pid->speedParam.dt;
}



/**
  * @brief  Speed loop controller (PI controller)
  *         Kp*Ek + Ki*ΣEk*dt
  *
  * @param  {mac_pid_t*} pid  Instance handle
  *         {curSpeed} Current speed
  *
  * @retval Speed loop output
  */
PID_TYPE Speed_Loop_Controller(mac_pid_t *pid, PID_TYPE curSpeed)
{
    static float filterSpeed = 0.0f;
    static float carSpeed = 0.0f;
    static float SportTarget = SPEED_TARGET_VALUE;
    static float Movement = 0.0f;

    /* Read Flag with mutex protection */
    rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
    uint8_t local_forward = Flag.forward;
    uint8_t local_back = Flag.back;
    rt_mutex_release(&Flag.flag_mutex);

    /* Position offset based on direction flags */
    if(local_forward == 1){
        Movement = SportTarget;
    }
    else if(local_back == 1){
        Movement = -SportTarget;
    }
    else{
        Movement = 0;
    }

    /* Speed PI controller */
    filterSpeed = moving_average_filtre(&Speed_Move_Filter, curSpeed);
    carSpeed = FOLowPassFilter(carSpeed, filterSpeed, 0.7);
    /* Speed error = target - actual (closed-loop) */
    pid->speedProcess.speed_bias = (Movement - carSpeed);
    /* Position integral separation */
    i_Speed_Position_Separation(pid);
    /* Integral accumulation for movement control */
    pid->speedProcess.speed_integral_bias += Movement;
    /* Integral limit */
    pid->speedProcess.speed_integral_bias = constrain(pid->speedProcess.speed_integral_bias,
            -pid->speedParam.speed_integral_limit, pid->speedParam.speed_integral_limit);
    /* PI calculation */
    PID_TYPE output = pid->speedParam.kp * pid->speedProcess.speed_bias +
                      pid->speedParam.ki * pid->speedProcess.speed_integral_bias;
    /* Output limit */
    output = constrain(output, - pid->speedParam.out_limit, pid->speedParam.out_limit);

    pid->speedProcess.out = output;

    return pid->speedProcess.out;
}







/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是平衡小车姿态平衡PID线程的创建以及回调函数                                                                                                                                                                                         */
/*---------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  Balance car PID control thread entry
  * @retval void
  */
#define VERTIACL_PRONTF 0  /* Default: off. Set to 1 to enable vertical ring Vofa+ output */
#define SPEED_PRONTF    0  /* Default: off. Set to 1 to enable speed ring Vofa+ output */
#define GRAVITY         9.80665f                /* Standard gravity (m/s²) */
#define TURN_BIAS_VAL   200                     /* Differential steering bias */
extern struct mpu6xxx_3axes bsp_accel, bsp_gyro;
void balance_car_thread_entry(void* parameter)
{
    struct mpu6xxx_3axis_m_per_s accel_m_per_s;
    int32_t finalOut = 0;
    int32_t finalDuty = 0;
    int32_t leftDuty = 0, rightDuty = 0;

    float speedOut = 0.0f;
    float verticalOut = 0.0f;
    static rt_tick_t next_tick = 0;

    PID_Vertical_Parameter_Init(&carPID);
    PID_Speed_Parameter_Init(&carPID);

    /* Initialize absolute timing */
    next_tick = rt_tick_get() + rt_tick_from_millisecond(5);

    while(1)
    {
        /* Pick-up detection: disable motors if car tipped over */
        if(If_Car_Was_Picked_Up()) {
            DRV1_Enable_Set(drv_disen);
            DRV2_Enable_Set(drv_disen);
            rt_thread_delay_until(&next_tick, rt_tick_from_millisecond(5));
            continue;
        }

        DRV1_Enable_Set(drv_en);
        DRV2_Enable_Set(drv_en);

        /* Accelerometer unit conversion: mg -> m/s² */
        accel_m_per_s.x = bsp_accel.x * 0.001f * GRAVITY;
        accel_m_per_s.y = bsp_accel.y * 0.001f * GRAVITY;
        accel_m_per_s.z = bsp_accel.z * 0.001f * GRAVITY;

        /* Zero-division protection: verify Dt > 0 before PID calculation */
        if(carPID.verticalParam.dt <= 0.0f || carPID.speedParam.dt <= 0.0f) {
            rt_thread_delay_until(&next_tick, rt_tick_from_millisecond(5));
            continue;
        }

        /* Vertical ring (PD controller) */
        verticalOut = Vertical_Ring_Controller(&carPID, carEulerAngles.pitch, (float)bsp_gyro.y, Mechanical_Median_Value);
        verticalOut = linearized_output(verticalOut_linear_mode, verticalOut);
#if VERTIACL_PRONTF
        rt_kprintf("<any>:%f,%f,%f,%f,%f\n", carPID.verticalProcess.pitch_present, carPID.verticalProcess.pitch_bias,
                carPID.verticalParam.pitch_target, verticalOut, carPID.verticalProcess.ygyro_bias);
#endif

        /* Speed ring (PI controller) */
        carPID.speedProcess.speed_present = Estimate_Speed(accel_m_per_s.x, &carEulerAngles);
        speedOut = Speed_Loop_Controller(&carPID, carPID.speedProcess.speed_present);
#if SPEED_PRONTF
        rt_kprintf("<any>:%f,%f,%f,%f,%f\n", carPID.speedProcess.speed_present, carPID.speedProcess.speed_bias,
                carPID.speedParam.speed_target, speedOut, carPID.speedProcess.speed_integral_bias);
#endif

        /* Output control with differential steering */
        finalOut = (int32_t)(verticalOut + speedOut);
        finalDuty = Motor_Duty_Calculate((rt_uint32_t)finalOut);

        /* Read turn flags with mutex protection */
        {
            uint8_t local_left, local_right;
            rt_mutex_take(&Flag.flag_mutex, RT_WAITING_FOREVER);
            local_left = Flag.left;
            local_right = Flag.right;
            rt_mutex_release(&Flag.flag_mutex);

            if(local_left) {
                /* Left turn: left wheel slows down, right wheel speeds up */
                leftDuty = finalOut - TURN_BIAS_VAL;
                rightDuty = finalOut + TURN_BIAS_VAL;
            } else if(local_right) {
                /* Right turn: left wheel speeds up, right wheel slows down */
                leftDuty = finalOut + TURN_BIAS_VAL;
                rightDuty = finalOut - TURN_BIAS_VAL;
            } else {
                leftDuty = finalOut;
                rightDuty = finalOut;
            }
        }

        Motor_Left_Set(leftDuty, finalDuty);
        Motor_Right_Set(rightDuty, finalDuty);

        /* Absolute timing: maintain strict 5ms control period */
        rt_thread_delay_until(&next_tick, rt_tick_from_millisecond(5));
    }
}


/**
  * @brief  初始化数据解码函数
  * @retval int
  */
rt_thread_t balance_car_Thread_Handle;
int balance_Car_Thread_Init(void)
{
    balance_car_Thread_Handle = rt_thread_create("balance_car_thread_entry", balance_car_thread_entry, RT_NULL, 4096, 20, 500);
    if(balance_car_Thread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. balance_car_Thread_Handle is created!!\r\n",Record.kprintf_cnt++);
        /* 启动解码线程，必须在消息队列创建成功后开启 */
        rt_thread_startup(balance_car_Thread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. balance_car_Thread_Handle is not created!!\r\n",Record.kprintf_cnt++);
    }
    return RT_EOK;
}

#endif /* USE_MY_ARITHMETIC */

















