#ifndef __DDS_H__
#define __DDS_H__


typedef enum     // 波形类型枚举
{
    SINE_WAVEFORM = 0,    // 正弦波
    SQUARE_WAVEFORM,      // 方波
    TRIANGLE_WAVEFORM,    // 三角波
    NONE_WAVEFORM         // 无效波形
} DDS_Waveform;

void DDS_Init(void);

void DDS_SetWaveform(DDS_Waveform waveform);
void DDS_SetFreq(double frequency);
void DDS_SetFreqWord(uint32_t freq);

uint32_t DDS_GetFreqWord(void);
double DDS_GetFreq(void);

void DDS_SetSystemClock(double clock);

#endif
