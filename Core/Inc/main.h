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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_A_Pin GPIO_PIN_5
#define ADC_A_GPIO_Port GPIOF
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOF
#define ADC_C_Pin GPIO_PIN_0
#define ADC_C_GPIO_Port GPIOA
#define SPI1_FSYNC_Pin GPIO_PIN_4
#define SPI1_FSYNC_GPIO_Port GPIOA
#define SPI1_CLK_Pin GPIO_PIN_5
#define SPI1_CLK_GPIO_Port GPIOA
#define SPI1_DATA_Pin GPIO_PIN_6
#define SPI1_DATA_GPIO_Port GPIOA
#define KEY1_Pin GPIO_PIN_8
#define KEY1_GPIO_Port GPIOB
#define KEY1_EXTI_IRQn EXTI9_5_IRQn
#define KEY2_Pin GPIO_PIN_9
#define KEY2_GPIO_Port GPIOB
#define KEY2_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
