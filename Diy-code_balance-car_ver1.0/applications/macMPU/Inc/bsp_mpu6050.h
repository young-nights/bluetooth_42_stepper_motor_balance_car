/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-06     teati       the first version
 */
#ifndef APPLICATIONS_MACBSP_INC_BSP_MPU6500_H_
#define APPLICATIONS_MACBSP_INC_BSP_MPU6500_H_
#include "macSYS.h"


extern struct mpu6xxx_device *mpu6050_dev;

int BSP_MPU6050_Init(void);


#endif /* APPLICATIONS_MACBSP_INC_BSP_MPU6500_H_ */
