/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-06     teati       the first version
 */
#include <macMath/Inc/bsp_math.h>


extern mac_pid_t carPID;

/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{
    int temp;

    if(a<0)
        temp=-a;
    else
        temp=a;

    return temp;
}






/**
  * @brief  线性化输出
  * @param  mode:线性化模式选择
  * @retval
  */
int32_t linearized_output(mathMode mode,float Values)
{
    static float verticalOut_linear_Out;
    static float Output;
    if(mode == verticalOut_linear_mode){
        if(Values < 0){ /* 向后运行 */
            Flag.Right_Direction = 0;
            Flag.Left_Direction = 0;
        }
        else{ /* 向前运行 */
            Flag.Right_Direction = 1;
            Flag.Left_Direction = 1;
        }
        ALL_Motor_Direction_Config(auto_direcation);
        verticalOut_linear_Out = fabsf(Values);
        if(verticalOut_linear_Out < carPID.verticalParam.out_limit){
            Output = carPID.verticalParam.out_limit - verticalOut_linear_Out;
            if(Output < 900){
                Output = 900;
            }
        }
    }
    return Output;
}






/*
 *  滑动滤波器算法
 *  xn            输入的计算值
 *  MVF_LENGTH    滑动滤波的长度
 */
Filter Speed_Move_Filter;
Filter Angle_Move_Filter;
//初始化滤波器
void Init_filter(Filter *f) {
    for (int i = 0; i < MVF_LENGTH; i++) {
        f->data[i] = 0; //将数据清零
    }
    f->index = 0; //将索引置零
    f->sum = 0; //将和置零
}

//滤波函数，输入新数据，返回滤波后的数据
float moving_average_filtre(Filter *f, float x) {
    f->sum -= f->data[f->index]; //减去最旧的数据
    f->data[f->index] = x; //存储新数据
    f->sum += x; //加上新数据
    f->index = (f->index + 1) % MVF_LENGTH; //更新索引，循环使用数组
    return f->sum / MVF_LENGTH; //返回平均值
}






//-------------------------------------------------------------------------------------------------------------------------
/*! 一阶低通滤波器 */
/**
  * @brief  一阶低通滤波器
  * @param  In      : 当前输入的信号值
  *         LastOut : 上一次滤波后的输出值
  *         a       : 滤波系数，决定了滤波器的截止频率和响应速度（取值范围0~1）
  * @retval 约束值
  */
float FOLowPassFilter(float In, float LastOut, float a)
{
    return a * In + (1 - a) * LastOut;
}







//-------------------------------------------------------------------------------------------------------------------------
/*! 卡尔曼滤波 */
KalmanFilter mykf = {
    .angle = 0,
    .bias = 0,
    .P = {{1, 0}, {0, 1}},
    .Q_angle = 0.01,    // 过程噪声（角度）
    .Q_bias = 0.003,     // 过程噪声（零偏）
    .R_measure = 0.03    // 观测噪声（加速度计）
};


/**
  * @brief  卡尔曼滤波器
  * @param  Gyro        : 陀螺仪测量的角速度[°/s]（绕y轴）
  *         Angles      : 加速度计计算的俯仰角
  *         dt          : 控制周期（单位：s）
  * @retval 滤波后的俯仰角
  */
float kalman_filter_for_pitch(KalmanFilter* kf, float Gyro, float Angles, float dt)
{

    //----- 1. 预测步骤 -----
    // 先验估计（角度预测）
    kf->angle += (Gyro - kf->bias) * dt;
    // 预测协方差矩阵
    kf->P[0][0] += dt * (dt * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    //----- 2. 更新步骤 -----
    // 计算俯仰角度误差
    float y = Angles - kf->angle;
    // 计算卡尔曼增益
    float S = kf->P[0][0] + kf->R_measure;
    float K[2];
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    // 更新状态估计
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;

    // 更新协方差矩阵
    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];
    kf->P[0][0] -= K[0] * P00_temp;
    kf->P[0][1] -= K[0] * P01_temp;
    kf->P[1][0] -= K[1] * P00_temp;
    kf->P[1][1] -= K[1] * P01_temp;

    return kf->angle;
}



