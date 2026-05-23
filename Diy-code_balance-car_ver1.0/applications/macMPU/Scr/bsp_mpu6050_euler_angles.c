/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-23     teati       the first version
 */
#include "bsp_mpu6050_euler_angles.h"





//以下是解算MPU6500的数据函数驱动--------------------------------------------------------------------------------------------------------
#define GRAVITY         9.80665f                // 标准重力加速度(m/s²)
#define RAD_TO_DEG      57.2957795f             // 弧度转角度的转换因子
#define SAMPLE_RATE     100                     // 采样频率(Hz)
#define DT              1.0f/SAMPLE_RATE        // 采样周期
#define ALPHA           0.96f                   /* Complementary filter coefficient
                                                     * Trade-off: higher = more gyro trust (smoother but drifts),
                                                     * lower = more accel trust (responsive but noisy).
                                                     * 0.96 provides better noise rejection than 0.99. */

struct mpu6xxx_3axes bsp_accel, bsp_gyro;






/**
  * @brief  x轴与y轴坐标转换
  * @param  accel,gyro的数据结构
  * @retval None
  */
static void transform_coordinates(struct mpu6xxx_3axes *accel, struct mpu6xxx_3axes *gyro, transform_coordinates_StructType mode)
{
    if(mode == nothing){
        return;
    }
    else if(mode == x_axis_interchangeable_y_axis){

        rt_int16_t empty;

        empty = accel->x;
        accel->x = accel->y;
        accel->y = empty;

        empty = gyro->x;
        gyro->x = gyro->y;
        gyro->y = empty;
    }
}




/**
  * @brief  利用加速度计的数据计算初始角度
  * @param  accel,gyro的三轴数据
  * @retval None
  */
static void get_accel_angles(struct mpu6xxx_3axes *accel, float *pitch, float *roll)
{
    /***
     *! 单位转换:由MPU6500得到的加速度三轴数据单位均是mg,因此需要将单位转换为m/s²
     *! 1g = 9.80665m/s²
     */
    float ax = accel->x * 0.001f * GRAVITY;
    float ay = accel->y * 0.001f * GRAVITY;
    float az = accel->z * 0.001f * GRAVITY;

    /***
     *! pitch       ：表示物体在垂直轴上的倾斜程度，单位为度数
     *! atan2f      ：反正切函数，返回两个参数的反正切值，范围在 −π 到 π 之间，单位为弧度
     *! RAD_TO_DEG  ：弧度转换为度数的转换因子
     *! 当 ax 为正时，−ax 为负，表示物体沿 x 轴正方向倾斜，俯仰角为负
     *! 当 ax 为负时，−ax 为正，表示物体沿 x 轴负方向倾斜，俯仰角为正
     */
    *pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * RAD_TO_DEG;

    /***
     *!    ay(表示沿y轴的加速度)
     *!    az(表示沿z轴的加速度)
     */
    *roll  = atan2f(ay, az) * RAD_TO_DEG;

}



/**
  * @brief  互补滤波姿态解算
  * @param  EulerAngles ：声明的欧拉角结构体
  * @retval None
  */
void calculate_euler_angles(struct mpu6xxx_device *dev, EulerAngles *angles)
{
    static float pitch = 0.0f, roll = 0.0f;
    float pitch_acc, roll_acc;

    /* Read sensor data with error checking */
    if(mpu6xxx_get_accel(dev, &bsp_accel) != RT_EOK ||
       mpu6xxx_get_gyro(dev, &bsp_gyro) != RT_EOK) {
        return; /* Skip this iteration if sensor read fails */
    }

    /* Coordinate transform (based on actual mounting orientation) */
    transform_coordinates(&bsp_accel, &bsp_gyro, nothing);

    /* Calculate angles from accelerometer */
    get_accel_angles(&bsp_accel, &pitch_acc, &roll_acc);

    // 陀螺仪积分（转换为deg/s）
    float gyro_x = bsp_gyro.x * 0.1f; // 原单位是deg/10s
    float gyro_y = bsp_gyro.y * 0.1f;

    pitch += gyro_y * DT; // 绕Y轴旋转对应pitch
    roll  += gyro_x * DT; // 绕X轴旋转对应roll

    // 互补滤波融合
    pitch = ALPHA * pitch + (1 - ALPHA) * pitch_acc;
    roll  = ALPHA * roll  + (1 - ALPHA) * roll_acc;

    // Yaw需要磁力计数据（此处未实现）
    angles->pitch = pitch;
    angles->roll  = roll;
    angles->yaw   = 0.0f; // 需磁力计支持
}








/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是平衡小车姿态解算线程的创建以及回调函数                                                                                                                                                                                                */
/*---------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  数据解码回调函数入口
  * @retval void
  */
EulerAngles carEulerAngles;
void car_euler_angles_thread_entry(void* parameter)
{
    while(1)
    {
        /* Validate sensor data: skip if out of range */
        if(fabsf(bsp_accel.x) > 16000 || fabsf(bsp_accel.y) > 16000 || fabsf(bsp_accel.z) > 16000) {
            rt_thread_mdelay(10);
            continue;
        }

        calculate_euler_angles(mpu6050_dev,&carEulerAngles);
        rt_thread_mdelay(1000/SAMPLE_RATE);
    }
}


/**
  * @brief  初始化数据解码函数
  * @retval int
  */
rt_thread_t car_euler_angles_Thread_Handle;
int car_euler_angles_Thread_Init(void)
{
    car_euler_angles_Thread_Handle = rt_thread_create("car_euler_angles_thread_entry", car_euler_angles_thread_entry, RT_NULL, 2048, 5, 20);
    if(car_euler_angles_Thread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. car_euler_angles_Thread_Handle is created!!\r\n",Record.kprintf_cnt++);
        rt_thread_startup(car_euler_angles_Thread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. car_euler_angles_Thread_Handle is not created!!\r\n",Record.kprintf_cnt++);
    }
    return RT_EOK;
}










