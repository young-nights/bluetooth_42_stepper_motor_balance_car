/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-06     teati       the first version
 */
#ifndef APPLICATIONS_MACMATH_INC_BSP_MATH_H_
#define APPLICATIONS_MACMATH_INC_BSP_MATH_H_

#include "macSYS.h"


//滤波器长度
#define         MVF_LENGTH                  4

typedef enum{
    verticalOut_linear_mode = 0,
}mathMode;



//定义滤波器结构体
typedef struct {
    float data[MVF_LENGTH]; //存储数据的数组
    int index; //当前数据的索引
    float sum; //当前数据的和
} Filter;
extern Filter Speed_Move_Filter;
extern Filter Angle_Move_Filter;


typedef struct {
    float angle;      // 俯仰角θ
    float bias;       // 陀螺仪零偏
    float P[2][2];    // 协方差矩阵
    float Q_angle;    // Qθ
    float Q_bias;     // Q_bias
    float R_measure;  // R
} KalmanFilter;




int myabs(int a);
void  Init_filter(Filter *f);
float moving_average_filtre(Filter *f, float x);
float FOLowPassFilter(float In, float LastOut, float a);
int32_t linearized_output(mathMode mode,float Values);
float kalman_filter_for_pitch(KalmanFilter* kf, float Gyro, float Angles, float dt);


#endif /* APPLICATIONS_MACMATH_INC_BSP_MATH_H_ */
