/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-19     Administrator       the first version
 */
#ifndef APPLICATIONS_MACMPU_INC_BSP_MPU6050_CALIBRATION_H_
#define APPLICATIONS_MACMPU_INC_BSP_MPU6500_CALIBRATION_H_
#include "macSYS.h"


/**
  * @brief  枚举类型
  * @param  None
  */
typedef enum
{
    MPU6xxx_Cali_Positive_X = 1,
    MPU6xxx_Cali_Negative_X,
    MPU6xxx_Cali_Positive_Y,
    MPU6xxx_Cali_Negative_Y,
    MPU6xxx_Cali_Positive_Z,
    MPU6xxx_Cali_Negative_Z,
}MPU6xxx_Cali_StructType;






#endif /* APPLICATIONS_MACMPU_INC_BSP_MPU6050_CALIBRATION_H_ */
