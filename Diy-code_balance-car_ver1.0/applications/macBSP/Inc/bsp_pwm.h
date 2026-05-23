/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-12     zphu       the first version
 */
#ifndef APPLICATIONS_MACBSP_INC_BSP_PWM_H_

#define APPLICATIONS_MACBSP_INC_BSP_PWM_H_
#include <applications/macSYS/Inc/macSYS.h>

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */





int PWM_Init(void);
void Motor_Left_Set(rt_uint16_t Period, rt_uint16_t Dutyfactor);
void Motor_Right_Set(rt_uint16_t Period, rt_uint16_t Dutyfactor);
rt_uint32_t Motor_Duty_Calculate(rt_uint32_t Period);

int Mortor_Left_expedite(void);
uint16_t Linear_Conversion(int moto);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* APPLICATIONS_MACBSP_INC_BSP_PWM_H_ */
