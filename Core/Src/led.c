#include "led.h"
#include "gpio.h"

/**
 * @brief  打开LED1
 */
void LED1_On(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  关闭LED1
 */
void LED1_Off(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

/**
 * @brief  打开LED2
 */
void LED2_On(void)
{
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  关闭LED2
 */
void LED2_Off(void)
{
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}
