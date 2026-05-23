#include <bsp_mpu6050_calibration.h>
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-19     Administrator       the first version
 */



//以下是MPU6500数据进行静态校准函数驱动--------------------------------------------------------------------------------------------------------
// 校准数据结构体
typedef struct {
    int16_t accel_min[3];  // 各轴最小值
    int16_t accel_max[3];  // 各轴最大值
    int16_t gyro_offset[3];// 陀螺仪偏移
} mpu6xxx_calib_data;


mpu6xxx_calib_data mpu6050_cali;


/* 陀螺仪校准函数（需静止放置） */
extern rt_err_t mpu6xxx_get_gyro_raw(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *gyro);
rt_err_t bsp_mpu6xxx_calibrate_gyro(struct mpu6xxx_device *dev, mpu6xxx_calib_data *calib)
{
    struct mpu6xxx_3axes gyro;
    int32_t sum[3] = {0};
    const uint16_t samples = 200;

    // 采集数据
    for(int i=0; i<samples; i++) {
        mpu6xxx_get_gyro_raw(dev, &gyro);
        sum[0] += gyro.x;
        sum[1] += gyro.y;
        sum[2] += gyro.z;
        rt_thread_mdelay(10);
    }

    // 计算平均值
    calib->gyro_offset[0] = sum[0] / samples;
    calib->gyro_offset[1] = sum[1] / samples;
    calib->gyro_offset[2] = sum[2] / samples;

    // 写入偏移寄存器
    struct mpu6xxx_3axes offset = {
        calib->gyro_offset[0],
        calib->gyro_offset[1],
        calib->gyro_offset[2]
    };

    return mpu6xxx_set_gyro_offset(dev, &offset);
}






/* 加速度计六面校准函数 */
extern rt_err_t mpu6xxx_get_accel_raw(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *accel);
rt_err_t mpu6xxx_calibrate_accel_6side(struct mpu6xxx_device *dev, mpu6xxx_calib_data *calib, rt_uint8_t sideValue)
{
    struct mpu6xxx_3axes accel;
    int16_t min[3], max[3];
    static uint8_t caliCnt = 0;


    // 初始化极值
    for(int i=0; i<3; i++) {
        min[i] = 32767;
        max[i] = -32768;
    }

    // 提示用户旋转设备
    rt_kprintf("开始六面校准，请依次将各面对准下方保持稳定...\n");

    //--------------------------------------------------------------------------X轴正方向朝下
    if(sideValue == MPU6xxx_Cali_Positive_X){   /*! 采集50组X轴方向朝下的三轴数据,然后更新当X轴正方向朝下时的极值数据 */
        for(int i=0; i<50; i++){
            mpu6xxx_get_accel_raw(dev, &accel);
            for(int axis=0; axis<3; axis++) {   /*! 更新每个轴的极值数据 */
                int16_t val = ((int16_t*)&accel)[axis];
                if(val < min[axis]) min[axis] = val;
                if(val > max[axis]) max[axis] = val;
            }
            rt_thread_mdelay(10);
            if(i == 49 ){                       /*! 向APP发送X轴正方向数据采集完毕的信息 */
                caliCnt++;
            }
        }
    }
    else if(sideValue == MPU6xxx_Cali_Negative_X){
        for(int i = 0; i < 50; i++){
            mpu6xxx_get_accel_raw(dev, &accel);
            for(int axis = 0; axis < 3; axis++){
                int16_t val = ((uint16_t*)&accel)[axis];
                if(val < min[axis]) min[axis] = val;
                if(val > max[axis]) max[axis] = val;
            }
            rt_thread_mdelay(10);
            if(i == 49 ){                       /*! 向APP发送X轴负方向数据采集完毕的信息 */
                caliCnt++;
            }
        }
    }



    if(caliCnt == 6){
        // 保存极值
        memcpy(calib->accel_min, min, sizeof(min));
        memcpy(calib->accel_max, max, sizeof(max));

        // 计算偏移和比例因子
        int16_t offset[3] = { 0 };
        for(int i=0; i<3; i++) {
            // 理想情况下 (max + min)/2 = offset
            offset[i] = (max[i] + min[i]) / 2;
        }

        // 应用校准参数
        struct mpu6xxx_3axes accel_offset = {
            offset[0],
            offset[1],
            offset[2]
        };
        return mpu6xxx_set_accel_offset(dev, &accel_offset);
    }

}


//------------------------------------------------------------------------------------------------------


/**
  * @brief  MPU6XXX校准回调线程函数入口
  * @retval void
  */
void mpu6xxx_cali_thread_entry(void* parameter)
{

    while(1)
    {
        if(mpu6xxxParameter.if_start_master_cali_process == 1){
            /* Calibration process: wait for external trigger, then execute */
        }
        else{
            /* Idle: suspend self to save CPU (wake on calibration command) */
        }


        /* Check if gyro static calibration is requested */
        if(mpu6xxxParameter.if_start_gyro_cali_process == 1){
            rt_err_t if_finish_cali = bsp_mpu6xxx_calibrate_gyro(mpu6050_dev,&mpu6050_cali);
            if(if_finish_cali == RT_EOK){
                mpu6xxxParameter.if_start_gyro_cali_process = 0;
                rt_kprintf("Now is finished gyro static calibration.\r\n");
                USART2_Order_to_Principal(Order_SEND_CAR_IS_FINISHED_GYRO_CALI_CMD);
            }
            else{
                rt_kprintf("Now is not finished gyro static calibration.\r\n");
                USART2_Order_to_Principal(Order_SEND_CAR_IS_NOT_FINISHED_CALI_CMD);
            }
        }

        /* Suspend self when no calibration is active to save CPU cycles */
        if(mpu6xxxParameter.if_start_master_cali_process == 0 &&
           mpu6xxxParameter.if_start_gyro_cali_process == 0) {
            rt_thread_suspend(rt_thread_self());
        }

        rt_thread_mdelay(100);
    }
}




/**
  * @brief  初始化MPU6XXX的校准线程函数
  * @retval int
  */
rt_thread_t mpu6xxx_calibrate_Thread_Handle;
int bsp_mpu6xxx_calibrate_Thread_Init(void)
{
    mpu6xxx_calibrate_Thread_Handle = rt_thread_create("mpu6xxx_cali_thread_entry", mpu6xxx_cali_thread_entry, RT_NULL, 2048, 6, 500);
    if(mpu6xxx_calibrate_Thread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. mpu6xxx calibrate thread is created!!\r\n",Record.kprintf_cnt++);
        rt_thread_startup(mpu6xxx_calibrate_Thread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. mpu6xxx calibrate thread is not created!!\r\n",Record.kprintf_cnt++);
    }
    return RT_EOK;
}







