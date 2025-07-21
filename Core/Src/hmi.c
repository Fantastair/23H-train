#include "stm32f4xx_hal.h"
#include "adc.h"
#include "usart.h"
#include "hmi.h"
#include "dds.h"
#include "fft.h"
#include "math.h"
#include "arm_math.h"

char orderBuffer[64];                       // 发送指令缓冲区
uint8_t *order = (uint8_t *)orderBuffer;    // 指令缓冲区指针
uint8_t receive_byte[32] = {0};             // 接收数据缓冲区
uint8_t receive_byte_index = 0;             // 接收数据索引


/**
 * @brief 初始化 HMI
 */
void HMI_Init(void)
{
    HAL_UART_Receive_IT(&huart1, receive_byte, 1);
    HAL_Delay(500);
    int index = 0;
    index = HMI_AddString("debug.t0.txt=\"\"", index);
    HMI_SendOrder(index);
    index = 0;
    index = HMI_AddString("--- Launched ---", index);
    HMI_SendDebug(index);
}


/**
 * @brief 发送指令到 HMI
 * @param length 指令的长度
 * @note 会自动在指令末尾添加三个 255 字节作为结束标志，确保 order 数组有足够的空间来存储这些字节。
 */
void HMI_SendOrder(int length)
{
    orderBuffer[length] = orderBuffer[length + 1] = orderBuffer[length + 2] = 255;
    HAL_UART_Transmit(&huart1, order, length + 3, HAL_MAX_DELAY);
}

/**
 * @brief 发送数据到 HMI
 * @param data 要发送的数据指针
 * @param length 数据的长度
 */
void HMI_SendData(uint8_t *data, int length)
{
    HAL_UART_Transmit(&huart1, data, length, HAL_MAX_DELAY);
}

uint8_t debugLines = 0;    // 调试信息行计数
uint8_t clear_debug_order[19] = "debug.t0.txt=\"\"\xff\xff\xff";
/**
 * @brief 发送调试信息到 HMI
 * @param length 指令的长度
 */
void HMI_SendDebug(int length)
{
    debugLines++;
    if (debugLines > 16)
    {
        debugLines = 1;
        HAL_UART_Transmit(&huart1, clear_debug_order, 18, HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)"debug.t0.txt+=\"", 15, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, order, length, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n\"\xff\xff\xff", 6, HAL_MAX_DELAY);
}

/**
 * @brief 将指令添加到缓冲区
 * @param str 要添加的字符串
 * @param startIndex 缓冲区的起始索引
 * @return 返回下一个可用的索引位置。
 */
int HMI_AddString(const char *str, int startIndex)
{
    int i = 0;
    while (str[i] != '\0')
    {
        orderBuffer[i + startIndex] = str[i];
        i++;
    }
    return startIndex + i;
}

/**
 * @brief 将整数添加到缓冲区
 * @param value 要添加的整数值
 * @param startIndex 缓冲区的起始索引
 * @return 返回下一个可用的索引位置。
 */
int HMI_AddInt(int value, int startIndex)
{
    int i = 0;
    if (value < 0)
    {
        orderBuffer[startIndex++] = '-';
        value = -value;
    }
    
    // 将整数转换为字符串
    if (value == 0)
    {
        orderBuffer[startIndex++] = '0';
        return startIndex;
    }

    int temp = value;
    while (temp > 0)
    {
        temp /= 10;
        i++;
    }

    for (int j = i - 1; j >= 0; j--)
    {
        orderBuffer[startIndex + j] = (value % 10) + '0';
        value /= 10;
    }
    
    return startIndex + i;
}

/**
 * @brief 将双精度浮点数添加到缓冲区
 * @param value 要添加的双精度浮点数值
 * @param startIndex 缓冲区的起始索引
 * @param precision 小数点后保留的位数
 * @return 返回下一个可用的索引位置。
 */
int HMI_AddDouble(double value, int startIndex, int precision)
{
    // 1. 处理负号并记录原始符号
    int isNegative = (value < 0);
    double absValue = fabs(value);

    // 2. 计算四舍五入因子
    double factor = Hmi_Pow(10.0, precision);
    
    // 3. 整体四舍五入（避免分离计算导致的进位错误）
    double rounded = round(absValue * factor) / factor;
    
    // 4. 重新分离整数和小数部分
    double integerPart;
    double fractionalPart = modf(rounded, &integerPart);
    
    // 5. 处理负号输出
    if (isNegative) {
        orderBuffer[startIndex++] = '-';
    }
    
    // 6. 输出整数部分
    startIndex = HMI_AddInt((int)integerPart, startIndex);
    
    // 7. 输出小数点
    orderBuffer[startIndex++] = '.';
    
    // 8. 处理小数部分（考虑四舍五入和补零）
    if (precision > 0) {
        // 获取精确的小数部分整数表示
        int fracInt = (int)round(fractionalPart * factor);
        
        // 处理进位产生的整数部分变化
        if (fracInt >= (int)factor) {
            fracInt = 0;
            // 注意：整数部分已在第4步通过modf处理进位
        }
        
        // 按精度位数补零
        int divisor = (int)factor / 10;
        for (int i = 0; i < precision; i++) {
            int digit = (divisor > 0) ? (fracInt / divisor) % 10 : 0;
            orderBuffer[startIndex++] = '0' + digit;
            divisor /= 10;
        }
    }
    return startIndex;
}

/**
 * @brief 将十六进制数添加到缓冲区
 * @param value 要添加的十六进制数值
 * @param startIndex 缓冲区的起始索引
 * @return 返回下一个可用的索引位置。
 */
int HMI_AddHex(uint32_t value, int startIndex)
{
    int i = 0;
    static char hexDigits[] = "0123456789ABCDEF";
    startIndex = HMI_AddString("0x", startIndex);

    // 处理0的情况
    if (value == 0)
    {
        orderBuffer[startIndex++] = '0';
        return startIndex;
    }

    // 计算十六进制数的长度
    uint32_t temp = value;
    while (temp > 0)
    {
        temp /= 16;
        i++;
    }

    // 将十六进制数转换为字符串
    for (int j = i - 1; j >= 0; j--)
    {
        orderBuffer[startIndex + j] = hexDigits[value % 16];
        value /= 16;
    }
    
    return startIndex + i;
}

/**
 * @brief 将二进制数添加到缓冲区
 * @param value 要添加的二进制数值
 * @param startIndex 缓冲区的起始索引
 * @return 返回下一个可用的索引位置。
 */
int HMI_AddBin(uint32_t value, int startIndex)
{
    int i = 0;
    startIndex = HMI_AddString("0b", startIndex);

    // 处理0的情况
    if (value == 0)
    {
        orderBuffer[startIndex++] = '0';
        return startIndex;
    }

    // 计算二进制数的长度
    uint32_t temp = value;
    while (temp > 0)
    {
        temp /= 2;
        i++;
    }

    // 将二进制数转换为字符串
    for (int j = i - 1; j >= 0; j--)
    {
        orderBuffer[startIndex + j] = (value % 2) + '0';
        value /= 2;
    }
    
    return startIndex + i;
}


/**
 * @brief 计算 base 的 exp 次幂
 * @param base 底数
 * @param exp 指数
 * @return 返回计算结果
 */
int Hmi_Pow(int base, int exp)
{
    int result = 1;
    for (int i = 0; i < exp; i++)
    {
        result *= base;
    }
    return result;
}


uint8_t state = 0;        // 接收状态
double freq_temp = 0;     // 接收频率数值缓存
uint8_t freq_time = 0;    // 接收频率数据次数

extern uint8_t fft_debug;
extern uint8_t process_state;

/**
 * @brief USART 接收完成回调函数
 * @param huart USART 句柄
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int index = 0;
    if (huart->Instance == USART1)
    {
        if (state == 0)    // 正常接受数据
        {
            // 处理接收到的数据
            switch (receive_byte[receive_byte_index++])
            {
            case 0x01:    // 关闭DDS
                DDS_SetWaveform(NONE_WAVEFORM);
                break;
            case 0x02:    // 正弦波
                DDS_SetWaveform(SINE_WAVEFORM);
                break;
            case 0x03:    // 三角波
                DDS_SetWaveform(TRIANGLE_WAVEFORM);
                break;
            case 0x04:    // 方波
                DDS_SetWaveform(SQUARE_WAVEFORM);
                break;
            case 0x05:    // 设置频率
                state = 1;
                freq_temp = 0;
                freq_time = 0;
                break;
            case 0x06:    // 设置相位
                break;
            case 0x07:    // 清空调试信息
                debugLines = 0;
                break;
            case 0x08:    // FFT_调试
                fft_debug = 1;
                index = 0;
                index = HMI_AddString("--- FFT Debug ---", index);
                HMI_SendDebug(index);
                FFT_StartADC(&hadc1);
                break;
            case 0x09:    // 开始信号分离
                fft_debug = 0;
                process_state = 1;
                index = 0;
                index = HMI_AddString("--- Start Separation ---", index);
                HMI_SendDebug(index);
                index = 0;
                index = HMI_AddString("main.t0.txt=\"正在分析信号 C\"", index);
                HMI_SendOrder(index);
                index = 0;
                index = HMI_AddString("main.t1.txt=\"\"", index);
                HMI_SendOrder(index);
                index = 0;
                index = HMI_AddString("main.t2.txt=\"\"", index);
                HMI_SendOrder(index);
                FFT_StartADC(&hadc1);
                break;
            default:
                // 无效命令或未处理的命令
                break;
            }
        }
        else if (state == 1)    // 接受频率数据
        {
            freq_time++;
            freq_temp += (receive_byte[receive_byte_index++] << (8 * (freq_time - 1)));
            if (freq_time == 4)
            {
                DDS_SetFreq(freq_temp / 10000);
                HMI_UpdateFreq();
                state = 0;
                {
                    int index = 0;
                    index = HMI_AddString("Display set DDS_FREQ: ", index);
                    index = HMI_AddDouble(freq_temp / 10000, index, 4);
                    index = HMI_AddString(" Hz", index);
                    HMI_SendDebug(index);
                }
            }
        }
        if (receive_byte_index >= sizeof(receive_byte))
        {
            receive_byte_index = 0;
        }

        HAL_UART_Receive_IT(huart, receive_byte + receive_byte_index, 1);
    }
}


/**
 * @brief 更新 HMI 上的频率显示
 * @note 该函数将当前 DDS 频率以特定格式发送到 HMI，以便在界面上显示。
 */
void HMI_UpdateFreq(void)
{
    int index = 0;
    index = HMI_AddString("dds.x0.val=", index);
    index = HMI_AddInt(DDS_GetFreq() * 10000, index);
    HMI_SendOrder(index);
}

float fft_curve_temp[460];
/**
 * @brief 更新 HMI 上的 FFT 频谱图显示
 */
void HMI_UpdateFFT(void)
{
  float max_value = 0.0f;
 
  for (int i = 459; i > -1; i--)
  {
    int mag_index = 512 * i / 460;
    float mag = fft_outputbuf[mag_index];
    fft_curve_temp[i] = mag;
    if (mag > max_value) { max_value = mag; }
  }

  int index = 0;
  for (int i = 459; i > -1; i--)
  {
    index = 0;
    index = HMI_AddString("add s0.id,0,", index);
    index = HMI_AddInt((uint8_t)(255 * fft_curve_temp[i] / max_value), index);
    HMI_SendOrder(index);
  }
}

uint16_t adc_value_temp[460];
/**
 * @brief 更新 HMI 上的 ADC 数据显示
 */
void HMI_UpdateADC(void)
{
    uint16_t adc_max = 0, adc_min = 4095;
    for (int i = 459; i > -1; i--)
    {
        int value_index = SAMPLE_NUM * i / 460;
        adc_value_temp[i] = adc_raw[value_index];
        if (adc_value_temp[i] > adc_max) { adc_max = adc_value_temp[i]; }
        if (adc_value_temp[i] < adc_min) { adc_min = adc_value_temp[i]; }
    }
    for (int i = 459; i > -1; i--)
    {
        int index = 0;
        index = HMI_AddString("add s0.id,1,", index);
        index = HMI_AddInt((adc_value_temp[i] - adc_min) * 255 / (adc_max - adc_min), index);
        HMI_SendOrder(index);
    }
}

float wave_data_tempA[256];
float wave_data_tempB[256];
// uint8_t wave_data_temp[256];
/**
 * @brief 绘制波形到 HMI
 * @param freq_a 波形 A 的频率
 * @param freq_b 波形 B 的频率
 * @param waveform_a 波形 A 的类型
 * @param waveform_b 波形 B 的类型
 * @param value_a 波形 A 的幅度
 * @param value_b 波形 B 的幅度
 */
void HMI_DrawWaveform(float freq_a, float freq_b, DDS_Waveform waveform_a, DDS_Waveform waveform_b, float value_a, float value_b)
{
    const float period_a = 1.0f / freq_a;
    for (int i = 0; i < 256; i++)
    {
        float phase = (float)i / 256.0f;
        float sample;
        switch(waveform_a)
        {
        case TRIANGLE_WAVEFORM:
            sample = (phase < 0.5f) ? (2.0f * phase) : (2.0f * (1.0f - phase));
            break;
        default:
            sample = 0.5f * (1.0f + sinf(2 * PI * phase));
        }
        wave_data_tempA[i] = sample;
    }
    float value_scale = value_b / value_a;
    for (int i = 0; i < 256; i++)
    {
        float t = period_a * ((float)i / 256.0f);
        float phase_b = fmodf(t * freq_b, 1.0f);
        float sample;
        switch(waveform_b)
        {
        case TRIANGLE_WAVEFORM:
            sample = (phase_b < 0.5f) ? (2.0f * phase_b) : (2.0f * (1.0f - phase_b));
            break;
        default:
            sample = 0.5f * (1.0f + sinf(2 * PI * phase_b));
        }
        wave_data_tempB[i] = sample * value_scale;
    }
    float offset_a = 0;
    float offset_b = 0;
    arm_mean_f32(wave_data_tempA, 256, &offset_a);
    arm_mean_f32(wave_data_tempB, 256, &offset_b);
    offset_a = 0.5f - offset_a;
    offset_b = 0.5f - offset_b;
    for (int i = 0; i < 256; i++)
    {
        int index = 0;
        index = HMI_AddString("add s0.id,0,", index);
        index = HMI_AddInt((uint8_t)(64.0f * (wave_data_tempA[i] + wave_data_tempB[i] + offset_a + offset_b) + 127), index);
        HMI_SendOrder(index);
        HAL_Delay(1);
    }
    for (int i = 0; i < 256; i++)
    {
        int index = 0;
        index = HMI_AddString("add s0.id,1,", index);
        index = HMI_AddInt((uint8_t)(64.0f * (wave_data_tempA[i] + offset_a) + 63), index);
        HMI_SendOrder(index);
        HAL_Delay(1);
    }
    for (int i = 0; i < 256; i++)
    {
        int index = 0;
        index = HMI_AddString("add s0.id,2,", index);
        index = HMI_AddInt((uint8_t)(64.0f * (wave_data_tempB[i] + offset_b)), index);
        HMI_SendOrder(index);
        HAL_Delay(1);
    }
}

/**
 * @brief 显示频率 A 的值
 * @param freq A 的频率值
 */
void HMI_ShowFreqA(float freq, float freq_)
{
    int index = 0;
    index = HMI_AddString("main.t1.txt=\"A  ", index);
    index = HMI_AddDouble(freq, index, 4);
    index = HMI_AddString(" Hz\r\nA' ", index);
    index = HMI_AddDouble(freq_, index, 4);
    index = HMI_AddString(" Hz\"", index);
    HMI_SendOrder(index);
}

/**
 * @brief 显示频率 B 的值
 * @param freq B 的频率值
 */
void HMI_ShowFreqB(float freq)
{
    int index = 0;
    index = HMI_AddString("main.t2.txt=\"B  ", index);
    index = HMI_AddDouble(freq, index, 4);
    index = HMI_AddString(" Hz\"", index);
    HMI_SendOrder(index);
}
