/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-11     RT-Thread    first version
 */

#include <rtthread.h>
#include "macSYS.h"

int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
#if 0 /* Reserved: ADC not currently used for balance car */
    MX_ADC1_Init();
#endif
    MX_TIM3_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */
    /*! Filter initialization */
    Init_filter(&Speed_Move_Filter);
    Init_filter(&Angle_Move_Filter);

    /*! Initialize flag mutex for thread-safe access */
    rt_mutex_init(&Flag.flag_mutex, "flag_mtx", RT_IPC_FLAG_FIFO);

    /*! RT-Thread thread initialization */
    uart2_decodeThread_Init();
    ledTimer_Init();
    beepTimer_Init();
    motorCheckTimer_Init();
    printf_show_Thread_Init();

    /*! MPU6050 initialization */
    BSP_MPU6050_Init();
    car_euler_angles_Thread_Init();

    /*! PID initialization */
    balance_Car_Thread_Init();

    /*! PWM initialization */
    PWM_Init();
    DRV1_Work_Mode_Set(drv_disen, clockwise, 6000, 3000);
    DRV2_Work_Mode_Set(drv_disen, anticlockwise, 6000, 3000);

    /*! Flash operation - use halfword write (STM32F1 does not support byte program) */
    Flag.FirstActivate = macNorFlash_Read_Byte(ADDR_FLASH_START_ADDR_DATA);
    if(Flag.FirstActivate != 0x66){
        BLE_Send_ENAT();
        rt_thread_mdelay(100);
        BLE_Send_LENA();
        rt_thread_mdelay(100);
        BLE_Send_REST();
        macNorFlash_Erase_Page(ADDR_FLASH_START_ADDR_DATA , 1);
        {
            uint16_t flag_val = 0x66;
            macNorFlash_Write_HalfWord(ADDR_FLASH_START_ADDR_DATA, flag_val);
        }
    }
    else{
        BLE_Send_EXAT();
        rt_thread_mdelay(100);
        Flag.AT_REC_Mode = 0;
        Flag.Protocol_Mode = 1;
    }


    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      /* USER CODE END WHILE */


        rt_thread_mdelay(1000);
      /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
