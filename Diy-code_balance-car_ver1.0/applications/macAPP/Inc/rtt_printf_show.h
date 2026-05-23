/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-08     Administrator       the first version
 */
#ifndef APPLICATIONS_MACAPP_INC_RTT_PRINTF_SHOW_H_
#define APPLICATIONS_MACAPP_INC_RTT_PRINTF_SHOW_H_
#include "macSYS.h"



// 以下为移植时必须需要的结构体等的初始化---------------------------------------------------------------------------------------------------------
extern uint8_t usartID;
typedef struct {
    rt_uint8_t  verticalOut;                           // 直立环输出
    rt_uint8_t  speedOut;                              // 速度环输出
    rt_uint8_t  vofa;                                  // 输出到上位机

}printfStruct;
extern printfStruct mylog;

int printf_show_Thread_Init(void);


#endif /* APPLICATIONS_MACAPP_INC_RTT_PRINTF_SHOW_H_ */
