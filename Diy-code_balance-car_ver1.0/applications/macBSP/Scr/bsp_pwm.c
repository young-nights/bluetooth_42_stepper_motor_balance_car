#include <applications/macBSP/Inc/bsp_pwm.h>
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-12     zphu       the first version
 */


#define PWM3_CH3_GPIO_PIN_NUM  16      /* PWM输出引脚编号-PB0，查看驱动文件drv_gpio.c确定 */
#define PWM3_CH3_DEV_NAME      "pwm3"  /* 设备名称 */
#define PWM3_CH3_DEV_CHANNEL   3       /* PWM输出通道 */


#define PWM3_CH4_GPIO_PIN_NUM  17      /* PWM输出引脚编号-PB1，查看驱动文件drv_gpio.c确定 */
#define PWM3_CH4_DEV_NAME      "pwm3"  /* 设备名称 */
#define PWM3_CH4_DEV_CHANNEL   4       /* PWM输出通道 */

/**
  * @brief  Used to initialize PWM devices and channels
  * @retval None
  */
typedef struct {
    char pwm_dev1_name[16];             /* pwm设备名称 */
    int  pwm_dev1_channel;              /* 电机驱动的pwm输出通道 */
    int  pwm_dev1_out_pin;              /* 电机输出pwm的引脚编号 */

    char pwm_dev2_name[16];             /* pwm设备名称 */
    int  pwm_dev2_channel;              /* 电机驱动的pwm输出通道 */
    int  pwm_dev2_out_pin;              /* 电机输出pwm的引脚编号 */

    struct rt_device_pwm *pwm_dev;
}_pwm_init;



/**
  * @brief  TIM3_CH0 -- PB0 Initialization
  * @retval None
  */
static _pwm_init pwm_dev1   = {
        .pwm_dev1_name      =   PWM3_CH3_DEV_NAME,
        .pwm_dev1_channel   =   PWM3_CH3_DEV_CHANNEL,
        .pwm_dev1_out_pin   =   PWM3_CH3_GPIO_PIN_NUM,
};



/**
  * @brief  TIM3_CH1 -- PB1 Initialization
  * @retval None
  */
static _pwm_init pwm_dev2   = {
        .pwm_dev2_name      =   PWM3_CH4_DEV_NAME,
        .pwm_dev2_channel   =   PWM3_CH4_DEV_CHANNEL,
        .pwm_dev2_out_pin   =   PWM3_CH4_GPIO_PIN_NUM,
};


/**
  * @brief  PWM Device Initialization
  * @retval int
  */
int PWM_Init(void)
{

    /* 查找设备 */
    pwm_dev1.pwm_dev = (struct rt_device_pwm *)rt_device_find(pwm_dev1.pwm_dev1_name);
    if(pwm_dev1.pwm_dev != RT_NULL){
        rt_kprintf("PRINTF:%d. pwm1 device is created !! \r\n",Record.kprintf_cnt++);
        /* 初始化默认的周期和占空比
                  *   第三个参数是设置周期，    单位是ns，如果设置100*1000那么周期就是100us，取倒数得到频率为10Khz
                  *   第四个参数是设置占空比，单位是ns，如果设置100*1000那么占空比取50%，就需要设置为他的一半，即50*1000
                  *   注意：在CubeMX中设置的period本来是1000-1，也即设置的周期为1ms，经过此rt_pwm_set设置后会无效化，即重新设置了，增强了灵活性
                  *   此处，PWM设置的三要素，频率，周期，占空比就都可以使用该函数进行设置了
         */
        rt_pwm_set(pwm_dev1.pwm_dev, pwm_dev1.pwm_dev1_channel, 1000*1000,0*1000);
        rt_pwm_enable(pwm_dev1.pwm_dev, pwm_dev1.pwm_dev1_channel);

    }
    else {
        rt_kprintf("PRINTF:%d. pwm3_ch3 device created failed !! \r\n",Record.kprintf_cnt++);
        return RT_ERROR;
    }


    /* 查找设备*/
    pwm_dev2.pwm_dev = (struct rt_device_pwm *)rt_device_find(pwm_dev2.pwm_dev2_name);
    if(pwm_dev2.pwm_dev != RT_NULL){
        rt_kprintf("PRINTF:%d. pwm2 device is created !! \r\n",Record.kprintf_cnt++);
        rt_pwm_enable(pwm_dev2.pwm_dev, pwm_dev2.pwm_dev2_channel);
        rt_pwm_set(pwm_dev2.pwm_dev, pwm_dev2.pwm_dev2_channel, 1000*1000, 0*1000);
    }
    else {
        rt_kprintf("PRINTF:%d. pwm3_ch4 device created failed !! \r\n",Record.kprintf_cnt++);
        return RT_ERROR;
    }

    return RT_EOK;
}
//INIT_APP_EXPORT(PWM_Init);



/**
  * @brief  Left Motor (Motor1) PWM Output Ctrl - uses TIM3_CH3 (pwm_dev1)
  * @retval void
  */
void Motor_Left_Set(rt_uint16_t Period, rt_uint16_t Dutyfactor)
{
    rt_pwm_set(pwm_dev1.pwm_dev, pwm_dev1.pwm_dev1_channel, Period * 1000, Dutyfactor * 1000);
}


/**
  * @brief  Right Motor (Motor2) PWM Output Ctrl - uses TIM3_CH4 (pwm_dev2)
  * @retval void
  */
void Motor_Right_Set(rt_uint16_t Period, rt_uint16_t Dutyfactor)
{
    rt_pwm_set(pwm_dev2.pwm_dev, pwm_dev2.pwm_dev2_channel, Period * 1000, Dutyfactor * 1000);
}




/**
  * @brief  计算平衡车电机PWM占空比
  *
  * @param  传入的控制频率
  *
  * @retval 占空比
  */
rt_uint32_t Motor_Duty_Calculate(rt_uint32_t Period)
{
    rt_uint32_t Duty = 0;

    Duty = Period / 2;

    return Duty;
}








