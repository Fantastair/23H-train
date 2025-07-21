#ifndef __HMI_H__
#define __HMI_H__


#include "stm32f4xx_hal.h"
#include "dds.h"

extern char orderBuffer[64];
extern uint8_t receive_byte[32];

void HMI_Init(void);

void HMI_SendOrder(int length);
void HMI_SendData(uint8_t *data, int length);
void HMI_SendDebug(int length);

int HMI_AddString(const char *str, int startIndex);
int HMI_AddInt(int value, int startIndex);
int HMI_AddDouble(double value, int startIndex, int precision);
int HMI_AddHex(uint32_t value, int startIndex);
int HMI_AddBin(uint32_t value, int startIndex);

int Hmi_Pow(int base, int exp);

void HMI_UpdateFreq(void);
void HMI_UpdateFFT(void);
void HMI_UpdateADC(void);
void HMI_DrawWaveform(float freq_a, float freq_b, DDS_Waveform waveform_a, DDS_Waveform waveform_b, float value_a, float value_b);
void HMI_ShowFreqA(float freq, float freq_);
void HMI_ShowFreqB(float freq);

#endif
