#include "main.h"
#include "dds.h"
#include "hmi.h"

double AD9833_SYSTEM_CLOCK = 25000000.0;
uint32_t DDS_FREQ_WORD = 0;

/**
 * @brief  写入数据到DDS
 * @param  data 要写入的数据
 */
void DDS_WriteData(uint16_t data)
{
    // int index = 0;
    // index = HMI_AddString("t0.txt+=\"", index);
    // index = HMI_AddHex(data, index);
    // index = HMI_AddString(" ", index);
    // index = HMI_AddBin(data, index);
    // index = HMI_AddString("\r\n\"", index);
    // HMI_SendOrder(index);

    uint8_t i;
    HAL_GPIO_WritePin(SPI1_CLK_GPIO_Port, SPI1_CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPI1_FSYNC_GPIO_Port, SPI1_FSYNC_Pin, GPIO_PIN_RESET);
    for (i = 0; i < 16; i++)
    {
        if (data & 0x8000)
        {
            HAL_GPIO_WritePin(SPI1_DATA_GPIO_Port, SPI1_DATA_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(SPI1_DATA_GPIO_Port, SPI1_DATA_Pin, GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(SPI1_CLK_GPIO_Port, SPI1_CLK_Pin, GPIO_PIN_RESET);
        data <<= 1;
        HAL_GPIO_WritePin(SPI1_CLK_GPIO_Port, SPI1_CLK_Pin, GPIO_PIN_SET);
    }
    HAL_GPIO_WritePin(SPI1_FSYNC_GPIO_Port, SPI1_FSYNC_Pin, GPIO_PIN_SET);
}

/**
 * @brief  初始化DDS
 */
void DDS_Init(void)
{ 
  DDS_SetWaveform(SINE_WAVEFORM);
  DDS_SetSystemClock(1000000);
  DDS_SetFreq(1000);
  HMI_UpdateFreq();
}

/**
 * @brief  设置DDS波形类型
 * @param  waveform 波形类型枚举
 */
void DDS_SetWaveform(enum DDS_Waveform waveform)
{
    int index = 0;
    index = HMI_AddString("debug.t0.txt+=\"set DDS_WAVEFORM: ", index);
    switch (waveform)
    {
    case SINE_WAVEFORM:
        // 设置为正弦波
        DDS_WriteData(0x2000);
        index = HMI_AddString("SINE_WAVEFORM\r\n\"", index);
        break;
    case SQUARE_WAVEFORM:
        // 设置为方波
        DDS_WriteData(0x2028);
        index = HMI_AddString("SQUARE_WAVEFORM\r\n\"", index);
        break;
    case TRIANGLE_WAVEFORM:
        // 设置为三角波
        DDS_WriteData(0x2002);
        index = HMI_AddString("TRIANGLE_WAVEFORM\r\n\"", index);
        break;
    default:
        // 无效的波形类型
        DDS_WriteData(0x00C0);
        index = HMI_AddString("NONE_WAVEFORM\r\n\"", index);
        break;
    }
    HMI_SendDebug(index);
}


/**
 * @brief 设置 DDS 输出频率
 * @param frequency 频率值
 * @return 返回设置的频率字
 */
void DDS_SetFreq(double frequency)
{
    DDS_FREQ_WORD = (uint32_t)(268435456.0 / AD9833_SYSTEM_CLOCK * frequency);
    DDS_SetFreqWord(DDS_FREQ_WORD);
}


/**
 * @brief 设置 DDS 输出频率（使用字频率）
 * @param freq 频率值
 * @return 返回设置的频率值
 */
void DDS_SetFreqWord(uint32_t freq)
{
    uint16_t lsb_14bits, msb_14bits;
    DDS_FREQ_WORD = freq;

    lsb_14bits = (uint16_t)(freq & 0x3FFF);
    msb_14bits = (uint16_t)((freq >> 14) & 0x3FFF);

    lsb_14bits |= 0x4000;
    msb_14bits |= 0x4000;

    DDS_WriteData(lsb_14bits);
    DDS_WriteData(msb_14bits);
}

/**
 * @brief 获取当前设置的 DDS 频率字
 * @return 返回当前的 DDS 频率字
 */
uint32_t DDS_GetFreqWord(void)
{
    return DDS_FREQ_WORD;
}

/**
 * @brief 获取当前设置的 DDS 输出频率
 * @return 返回当前的 DDS 输出频率
 */
double DDS_GetFreq(void)
{
    return DDS_FREQ_WORD * AD9833_SYSTEM_CLOCK / 268435456.0;
}

/**
 * @brief 设置 DDS 系统时钟
 * @param clock 时钟频率
 */
void DDS_SetSystemClock(double clock)
{
    AD9833_SYSTEM_CLOCK = clock;
}
