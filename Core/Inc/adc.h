/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#define FFT_LEN 1024
#define SAMPLENUM 1024
#define ADC_mv  0.80586
#define Rasing_Voltage_mv 1650
#define Input_Voltage_RMS_mv 1060.66017
#define R 6040.0
#define Frequency 10000.0f
#define PI 3.14159265358979f

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;
  
/* USER CODE BEGIN Private defines */

void ADC_DMA_Start(void);
//void Transform_ADC_to_Voltage(float *buffer);
void Ger_Fact_Voltage(float *buffer1,float *buffer2);
void Get_RMS_Voltage(float *buffer1 ,double *voltage_RMS);
void Get_Equivalent_Resistance(double *equivalent_resistance,double *voltage_RMS);
void GET_LCR_Value (char *type ,double *equivalent_resistance,double *value);
//void Transform_Into_FFT(float *INPUT, float *Fact_Voltage);
void Arry_To_Arry(float *buffer1,float *buffer2,uint32_t num,float times);

/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

