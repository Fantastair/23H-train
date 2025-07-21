#include "stm32f4xx_hal.h"

#include "tim.h"
#include "adc.h"
#include "dds.h"
#include "hmi.h"
#include "fft.h"

#include "math.h"
#include "arm_math.h"
#include "arm_const_structs.h"

__attribute__((section(".ccmram"))) float32_t fft_adc_buffer[SAMPLE_NUM] = { 0.0f };    // ADC 浮点数据缓冲区
__attribute__((section(".ccmram"))) float32_t fft_inputbuf[FFT_LEN * 2] = { 0.0f };     // FFT 输入缓冲区
__attribute__((section(".ccmram"))) float32_t fft_outputbuf[FFT_LEN] = { 0.0f };        // FFT 输出缓冲区

uint8_t adc_completed = 0;                   // ADC 转换完成标志，0: 未完成，1: ADC1完成，2: ADC3完成
float32_t fft_reset_temp[9] = { 0.0f };      // FFT 置零缓冲区
uint16_t adc_raw[SAMPLE_NUM * 2] = { 0 };    // ADC 原始数据
float32_t fft_dc_offset = 0.0f;              // 直流偏移量

float32_t fft_main_value_A = 0.0f;           // A主峰值
uint32_t  fft_main_index_A = 0;              // A主峰索引
float32_t fft_main_value_B = 0.0f;           // B主峰值
uint32_t  fft_main_index_B = 0;              // B主峰索引
uint32_t  fft_3_harmonic_index_A = 0;        // A三次谐波索引
float32_t fft_3_harmonic_value_A = 0.0f;     // A三次谐波值
uint32_t  fft_5_harmonic_index_A = 0;        // A五次谐波索引
float32_t fft_5_harmonic_value_A = 0.0f;     // A五次谐波值
uint32_t  fft_3_harmonic_index_B = 0;        // B三次谐波索引
float32_t fft_3_harmonic_value_B = 0.0f;     // B三次谐波值
uint32_t  fft_5_harmonic_index_B = 0;        // B五次谐波索引
float32_t fft_5_harmonic_value_B = 0.0f;     // B五次谐波值

DDS_Waveform waveform_A;    // A波形类型
DDS_Waveform waveform_B;    // B波形类型

/**
 * @brief 启动 ADC 采集
 * @param hadc ADC 句柄
 */
void FFT_StartADC(ADC_HandleTypeDef *hadc)
{
    HAL_ADC_Start_DMA(hadc, (uint32_t *)adc_raw, SAMPLE_NUM * 2);
    HAL_TIM_Base_Start(&htim3);
}

/**
 * @brief ADC 转换完成回调函数
 * @param hadc ADC 句柄
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_completed = 1;    // ADC1 转换完成，对应信号 C
    }
    else if (hadc->Instance == ADC3)
    {
        adc_completed = 2;    // ADC3 转换完成，对应信号 A'
    }
    HAL_TIM_Base_Stop(&htim3);
}

/**
 * @brief 准备 FFT 数据
 * @note 将 ADC 原始数据转换为浮点数格式，准备进行 FFT 计算
 */
void FFT_PrepareData(uint16_t * adc_raw)
{
    for (uint16_t i = 0; i < SAMPLE_NUM; i++)
    {
        fft_adc_buffer[i] = (float32_t)(adc_raw[i]);
    }
}

/**
 * @brief 处理 FFT 计算
 * @param adc_raw ADC 原始数据
 * @return 返回 FFT 相位
 */
void FFT_Process(void)
{
    arm_mean_f32(fft_adc_buffer, SAMPLE_NUM, &fft_dc_offset);

    for (uint16_t i = 0; i < SAMPLE_NUM; i++)
    {
        fft_inputbuf[2 * i] = fft_adc_buffer[i] - fft_dc_offset;
        fft_inputbuf[2 * i + 1] = 0.0f;
    }

    arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
    arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LEN);
    arm_max_f32(fft_outputbuf, FFT_LEN / 2, &fft_main_value_A, &fft_main_index_A);
    FFT_Reset(fft_main_index_A);
    arm_max_f32(fft_outputbuf, FFT_LEN / 2, &fft_main_value_B, &fft_main_index_B);
    FFT_Restore(fft_main_index_A);
    if (fft_main_index_A > fft_main_index_B)
    {
        float32_t value_temp = fft_main_value_A;
        uint32_t index_temp = fft_main_index_A;
        fft_main_value_B = value_temp;
        fft_main_value_A = fft_main_value_B;
        fft_main_index_B = index_temp;
        fft_main_index_A = fft_main_index_B;
    }
}

/**
 * @brief 获取 FFT 相位
 * @param index FFT 输出缓冲区索引
 * @return 返回 FFT 相位
 */
float FFT_GetPhase(uint32_t index)
{
    float real = fft_inputbuf[2 * index];
    float imag = fft_inputbuf[2 * index + 1];
    return atan2f(imag, real);
}

/**
 * @brief 获取 FFT 频率
 * @return 返回 FFT 频率
 */
float FFT_GetBaseFrequency(uint32_t index)
{
    return (float)index * SAMPLE_RATE / (float)SAMPLE_NUM;
}

float correct_K = 1.0f / (2 * PI) * (SAMPLE_RATE / (float)SAMPLE_NUM);    // 修正系数
/**
 * @brief 修正 FFT 频率
 * @param phase1 第一个相位
 * @param phase2 第二个相位
 * @param base_freq 基础频率
 * @return 返回修正后的 FFT 频率
 */
float FFT_CorrectFrequency(float phase1, float phase2, float base_freq)
{
    float raw_phase_diff = phase2 - phase1;
    if (raw_phase_diff > PI) { raw_phase_diff -= 2 * PI; }
    else if (raw_phase_diff < -PI) { raw_phase_diff += 2 * PI; }
    float freq = base_freq + raw_phase_diff * correct_K;
    return freq;
}

/**
 * @brief 将 FFT 输出缓冲区的指定索引区域置零
 * @param index 要重置的索引位置
 */
void FFT_Reset(uint16_t index)
{
    for (int16_t i = index - 4; i <= index + 4; i++)
    {
        if (i >= 0 && i <= FFT_LEN - 1)
        {
            fft_reset_temp[i - index + 4] = fft_outputbuf[i];
            fft_outputbuf[i] = 0.0f;
        }
    }
}

/**
 * @brief 将 FFT 输出缓冲区的指定索引区域恢复为之前的值
 * @param index 要恢复的索引位置
 */
void FFT_Restore(uint16_t index)
{
    for (int16_t i = index - 4; i <= index + 4; i++)
    {
        if (i >= 0 && i <= FFT_LEN - 1)
        {
            fft_outputbuf[i] = fft_reset_temp[i - index + 4];
        }
    }
}

/**
 * @brief 查找谐波分量
 */
void FFT_FindHarmonicValues(void)
{
    arm_max_f32(fft_outputbuf + fft_main_index_A * 3 - 3, 7, &fft_3_harmonic_value_A, &fft_3_harmonic_index_A);
    fft_3_harmonic_index_A += fft_main_index_A * 3 - 3;
    arm_max_f32(fft_outputbuf + fft_main_index_A * 5 - 3, 7, &fft_5_harmonic_value_A, &fft_5_harmonic_index_A);
    fft_5_harmonic_index_A += fft_main_index_A * 5 - 3;
    arm_max_f32(fft_outputbuf + fft_main_index_B * 3 - 3, 7, &fft_3_harmonic_value_B, &fft_3_harmonic_index_B);
    fft_3_harmonic_index_B += fft_main_index_B * 3 - 3;
    arm_max_f32(fft_outputbuf + fft_main_index_B * 5 - 3, 7, &fft_5_harmonic_value_B, &fft_5_harmonic_index_B);
    fft_5_harmonic_index_B += fft_main_index_B * 5 - 3;
}


/**
 * @brief 判断波形类型
 */
void FFT_JudgeWaveform(void)
{
    float freq_ratio = FFT_GetBaseFrequency(fft_main_index_B) / FFT_GetBaseFrequency(fft_main_index_A);    
    if (freq_ratio > 4.0f || freq_ratio < 2.0f)    // 如果频率比在2-4倍之外，认为没有叠加
    {
        // 如果3次谐波存在且幅度比小于15，判断为三角波
        if (fft_3_harmonic_index_A != 0 && fft_outputbuf[fft_main_index_A] / fft_outputbuf[fft_3_harmonic_index_A] < 15.0f) { waveform_A = TRIANGLE_WAVEFORM; }
        else { waveform_A = SINE_WAVEFORM; }
        if (fft_3_harmonic_index_B != 0 && fft_outputbuf[fft_main_index_B] / fft_outputbuf[fft_3_harmonic_index_B] < 15.0f) { waveform_B = TRIANGLE_WAVEFORM; }
        else { waveform_B = SINE_WAVEFORM; }
    }
    else     // 可能有频率叠加
    {
        // 处理较低频率分量
        if (fft_3_harmonic_index_A != 0 && fft_5_harmonic_index_A != 0 && 
            fft_outputbuf[fft_main_index_A] / fft_outputbuf[fft_3_harmonic_index_A] > 50.0f &&
            fft_outputbuf[fft_3_harmonic_index_A] / fft_outputbuf[fft_5_harmonic_index_A] > 2.0f &&
            fft_outputbuf[fft_main_index_A] / fft_outputbuf[fft_5_harmonic_index_A] < 100.0f) { waveform_A = SINE_WAVEFORM; }
        else { waveform_A = TRIANGLE_WAVEFORM; }
        // 处理较高频率分量
        if (fft_3_harmonic_index_B != 0 && fft_outputbuf[fft_main_index_B] - fft_outputbuf[fft_3_harmonic_index_B] < 500)
        {
            if (fft_5_harmonic_index_B != 0 && fft_outputbuf[fft_main_index_B] / fft_outputbuf[fft_5_harmonic_index_B] < 40.0f) { waveform_B = TRIANGLE_WAVEFORM; }
            else { waveform_B = SINE_WAVEFORM; }
        }
        else
        {
            if (fft_3_harmonic_index_B != 0 && fft_outputbuf[fft_main_index_B] / fft_outputbuf[fft_3_harmonic_index_B] < 15.0f) { waveform_B = TRIANGLE_WAVEFORM; }
            else { waveform_B = SINE_WAVEFORM; }
        }
    }
}
