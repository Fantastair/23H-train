/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "key.h"
#include "hmi.h"
#include "dds.h"
#include "fft.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t fft_debug = 0;        // FFT 调试标志
uint8_t process_state = 0;    // 处理状态
float freqA_temp = 0.0f;
float freqA_temp_ = 0.0f;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HMI_Init();
  DDS_Init();

  while (1)
  {
    if (adc_completed)
    {
      if (fft_debug)    // FFT 调试，显示 adc 数据和 fft 频谱
      {
        fft_debug = 0;
        FFT_PrepareData(adc_raw);
        FFT_Process();
        HMI_UpdateADC();
        HMI_UpdateFFT();
        int index = 0;
        index = HMI_AddString("frequency: ", index);
        index = HMI_AddDouble(FFT_GetBaseFrequency(fft_main_index_B), index, 4);
        index = HMI_AddString(" Hz", index);
        HMI_SendDebug(index);
        index = 0;
        index = HMI_AddString("phase: ", index);
        index = HMI_AddDouble(FFT_GetPhase(fft_main_index_B), index, 4);
        HMI_SendDebug(index);
      }
      else
      {
        int index = 0;
        switch (process_state)
        {
        case 1:    // 重建信号
          FFT_PrepareData(adc_raw);
          FFT_Process();
          FFT_FindHarmonicValues();
          FFT_JudgeWaveform();

          float phase_A1 = FFT_GetPhase(fft_main_index_A);
          float phase_B1 = FFT_GetPhase(fft_main_index_B);
          FFT_PrepareData(adc_raw + SAMPLE_NUM);
          FFT_Process();
          float phase_A2 = FFT_GetPhase(fft_main_index_A);
          float phase_B2 = FFT_GetPhase(fft_main_index_B);
          float freq_A = FFT_CorrectFrequency(phase_A1, phase_A2, FFT_GetBaseFrequency(fft_main_index_A));
          float freq_B = FFT_CorrectFrequency(phase_B1, phase_B2, FFT_GetBaseFrequency(fft_main_index_B));
          DDS_SetWaveform(waveform_A);
          DDS_SetFreq(freq_A);

          HMI_DrawWaveform(freq_A, freq_B, waveform_A, waveform_B, fft_main_value_A, fft_main_value_B);

          index = 0;
          index = HMI_AddString("main.t0.txt=\"分析完成\r\n正在重建信号 C\"", index);
          HMI_SendOrder(index);

          HMI_ShowFreqA(freq_A, freq_A);
          HMI_ShowFreqB(freq_B);

          FFT_StartADC(&hadc1);
          process_state = 2;
          break;
        case 2:    // 微调频率
          if (adc_completed == 1)
          {
            FFT_PrepareData(adc_raw);
            FFT_Process();
            float phase_A1 = FFT_GetPhase(fft_main_index_A);
            FFT_PrepareData(adc_raw + SAMPLE_NUM);
            FFT_Process();
            float phase_A2 = FFT_GetPhase(fft_main_index_A);
            freqA_temp = FFT_CorrectFrequency(phase_A1, phase_A2, FFT_GetBaseFrequency(fft_main_index_A));

            HMI_ShowFreqA(freqA_temp, freqA_temp_);

            FFT_StartADC(&hadc3);
          }
          else if (adc_completed == 2)
          {
            FFT_PrepareData(adc_raw);
            FFT_Process();
            float phase_A1_ = FFT_GetPhase(fft_main_index_A);
            FFT_PrepareData(adc_raw + SAMPLE_NUM);
            FFT_Process();
            float phase_A2_ = FFT_GetPhase(fft_main_index_A);
            freqA_temp_ = FFT_CorrectFrequency(phase_A1_, phase_A2_, FFT_GetBaseFrequency(fft_main_index_A));

            if (freqA_temp_ < freqA_temp) { DDS_SetFreqWord(DDS_GetFreqWord() + 1); }
            else if (freqA_temp_ > freqA_temp) { DDS_SetFreqWord(DDS_GetFreqWord() - 1); }

            HMI_ShowFreqA(freqA_temp, freqA_temp_);

            FFT_StartADC(&hadc1);
          }
          break;
        default:
          break;
        }
      }
      adc_completed = 0;
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
