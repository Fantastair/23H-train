#include "main.h"
#include "key.h"
#include "dds.h"
#include "hmi.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY1_Pin)
    {
        if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            // 按键按下
            DDS_SetFreqWord(DDS_GetFreqWord() + 1); // 增加频率
            HMI_UpdateFreq();
            int index = 0;
            index = HMI_AddString("DDS_FREQ_WORD+1: ", index);
            index = HMI_AddInt(DDS_GetFreqWord(), index);
            HMI_SendDebug(index);
        }
        else
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            // 按键松开
        }
    }
    else if (GPIO_Pin == KEY2_Pin)
    {
        if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
        {
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
            // 按键按下
            DDS_SetFreqWord(DDS_GetFreqWord() - 1); // 减少频率
            HMI_UpdateFreq();
            int index = 0;
            index = HMI_AddString("DDS_FREQ_WORD-1: ", index);
            index = HMI_AddInt(DDS_GetFreqWord(), index);
            HMI_SendDebug(index);
        }
        else
        {
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
            // 按键松开
        }
    }
}
