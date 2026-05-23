/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "macSYS.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_TIM3_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BEEP_Pin GPIO_PIN_3
#define BEEP_GPIO_Port GPIOC
#define DRV1_VOL_REF_Pin GPIO_PIN_0
#define DRV1_VOL_REF_GPIO_Port GPIOA
#define DRV2_VOL_REF_Pin GPIO_PIN_1
#define DRV2_VOL_REF_GPIO_Port GPIOA
#define DRV2_HOME_Pin GPIO_PIN_4
#define DRV2_HOME_GPIO_Port GPIOA
#define DRV2_NRESET_Pin GPIO_PIN_5
#define DRV2_NRESET_GPIO_Port GPIOA
#define DRV2_FAULT_Pin GPIO_PIN_6
#define DRV2_FAULT_GPIO_Port GPIOA
#define DRV2_DIR_Pin GPIO_PIN_4
#define DRV2_DIR_GPIO_Port GPIOC
#define DRV2_ENABLE_Pin GPIO_PIN_5
#define DRV2_ENABLE_GPIO_Port GPIOC
#define DRV1_NEBALE_Pin GPIO_PIN_2
#define DRV1_NEBALE_GPIO_Port GPIOB
#define DRV1_DIR_Pin GPIO_PIN_10
#define DRV1_DIR_GPIO_Port GPIOB
#define DRV1_NRESET_Pin GPIO_PIN_12
#define DRV1_NRESET_GPIO_Port GPIOB
#define DRV1_HOME_Pin GPIO_PIN_13
#define DRV1_HOME_GPIO_Port GPIOB
#define DRV1_FAULT_Pin GPIO_PIN_14
#define DRV1_FAULT_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOB


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
