/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-08     Administrator       the first version
 */
#include "rtt_printf_show.h"


printfStruct mylog;

extern mac_pid_t carPID;

/*---------------------------------------------------------------------------------------------------------------*/
/* 以下是数值打印线程的创建以及回调函数                                                                                                                                                                                         */
/*---------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  数据打印回调函数入口
  * @retval void
  */
extern int finalOut;
void printf_show_thread_entry(void* parameter)
{

    mylog.verticalOut = 0;
    mylog.speedOut = 0;
    mylog.vofa = 1;

    while(1)
    {

        /* 直立环输出 */
        if(mylog.verticalOut){
            rt_kprintf("verticalOut = %f \r\n",carPID.verticalProcess.out);
        }
        /* 速度环输出 */
        if(mylog.speedOut){
            rt_kprintf("speedOut = %f \r\n",carPID.speedProcess.out);
        }

        /* vofa+ */
        if(mylog.vofa){

        }

        rt_thread_mdelay(rt_tick_from_millisecond(500));
    }
}


/**
  * @brief  初始化数据解码函数
  * @retval int
  */
rt_thread_t printf_show_Thread_Handle;
int printf_show_Thread_Init(void)
{
    printf_show_Thread_Handle = rt_thread_create("printf_show_thread_entry", printf_show_thread_entry, RT_NULL, 2048, 29, 500);
    if(printf_show_Thread_Handle != RT_NULL){
        rt_kprintf("PRINTF:%d. printf_show_Thread_Handle is created!!\r\n",Record.kprintf_cnt++);
        /* 启动解码线程，必须在消息队列创建成功后开启 */
        rt_thread_startup(printf_show_Thread_Handle);
    }
    else {
        rt_kprintf("PRINTF:%d. printf_show_Thread_Handle is not created!!\r\n",Record.kprintf_cnt++);
    }
    return RT_EOK;
}











