#include <bsp_mpu6050.h>
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-06     teati       the first version
 */
#include "mpu6xxx.h"



/* Default configuration, please change according to the actual situation, support i2c and spi device name */
#define MPU6XXX_DEVICE_NAME  "i2c2"

/* Test function */
static int mpu6xxx_test()
{
    struct mpu6xxx_device *dev;
    struct mpu6xxx_3axes accel, gyro;
    int i;

    /* Initialize mpu6xxx, The parameter is RT_NULL, means auto probing for i2c*/
    dev = mpu6xxx_init(MPU6XXX_DEVICE_NAME, RT_NULL);

    if (dev == RT_NULL)
    {
        rt_kprintf("mpu6xxx init failed\n");
        return -1;
    }
    rt_kprintf("mpu6xxx init succeed\n");

    for (i = 0; i < 50; i++)
    {
        mpu6xxx_get_accel(dev, &accel);
        mpu6xxx_get_gyro(dev, &gyro);

        rt_kprintf("accel.x = %3d, accel.y = %3d, accel.z = %3d ", accel.x, accel.y, accel.z);
        rt_kprintf("gyro.x = %3d gyro.y = %3d, gyro.z = %3d\n", gyro.x, gyro.y, gyro.z);

        rt_thread_mdelay(100);
    }

    mpu6xxx_deinit(dev);

    return 0;
}
MSH_CMD_EXPORT(mpu6xxx_test,mpu6xxx_tes_cmd);



//以下是解算MPU6500的初始化函数驱动--------------------------------------------------------------------------------------------------------

#define MPU6050_DEVICE_NAME  "i2c2"
struct mpu6xxx_device *mpu6050_dev;






int BSP_MPU6050_Init(void)
{
    mpu6050_dev = mpu6xxx_init(MPU6XXX_DEVICE_NAME, RT_NULL);
    if (mpu6050_dev == RT_NULL)
    {
        rt_kprintf("mpu6050_dev init failed\n");
        return -1;
    }
    else{
        rt_kprintf("mpu6050_dev init succeed\n");
    }
    return RT_EOK;
}



