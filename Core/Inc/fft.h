#ifndef __FFT_H__
#define __FFT_H__

#include <stdint.h>
#include "arm_math.h"

#define FFT_LEN 1024
#define SAMPLE_NUM 1024
#define SAMPLE_RATE 500000.0f
#define PI 3.14159265358979f

extern uint16_t adc_raw[SAMPLE_NUM * 2];
extern float32_t fft_outputbuf[FFT_LEN];
extern float32_t fft_inputbuf[FFT_LEN * 2];

extern uint8_t adc_completed;

extern float32_t fft_dc_offset;
extern float32_t fft_main_value_A;
extern uint32_t  fft_main_index_A;
extern float32_t fft_main_value_B;
extern uint32_t  fft_main_index_B;
extern DDS_Waveform waveform_A;
extern DDS_Waveform waveform_B;


void  FFT_StartADC(ADC_HandleTypeDef *hadc);
void  FFT_PrepareData(uint16_t * adc_raw);
void  FFT_Process(void);
float FFT_GetPhase(uint32_t index);
float FFT_GetBaseFrequency(uint32_t index);
float FFT_CorrectFrequency(float phase1, float phase2, float base_freq);

void FFT_Reset(uint16_t index);
void FFT_Restore(uint16_t index);

void FFT_FindHarmonicValues(void);
void FFT_JudgeWaveform(void);

#endif
