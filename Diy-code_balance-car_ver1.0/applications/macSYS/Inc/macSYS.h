/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-30     Administrator       the first version
 */
#ifndef APPLICATIONS_MACSYS_INC_MACSYS_H_

#define APPLICATIONS_MACSYS_INC_MACSYS_H_



/* RTT实时操作系统的头文件 */
#include <rtthread.h>
#include <drv_common.h>
#include <board.h>
#include <rtdevice.h>
#include <rthw.h>


/* 该头文件包含了所有CubeMX自动生成的初始化引脚 */
#include "main.h"

/* macBSP中的头文件 */
#include "bsp_led.h"
#include "bsp_pwm.h"
#include "bsp_beep.h"

/* macAPP中的头文件 */
#include "rtt_uart2_Decode_Poll_Pattern.h"
#include "rtt_uart2_Decode.h"
#include "rtt_motor_ctrl.h"
#include "rtt_motor_pid.h"
#include "rtt_printf_show.h"

/* macSYS中的头文件 */
#include "macTypedef.h"
#include "macFlash.h"

/* macMPU中的头文件 */
#include "bsp_mpu6050_euler_angles.h"
#include "bsp_mpu6050.h"

/* 标准库头文件 */
#include "string.h"
#include "stdio.h"
#include "float.h"
#include "math.h"
#include "stdlib.h"

/* Packages头文件 */
#include "mpu6xxx.h"
#include "mpu6xxx_reg.h"


/* macMath头文件 */
#include "bsp_math.h"



#endif /* APPLICATIONS_MACSYS_INC_MACSYS_H_ */
