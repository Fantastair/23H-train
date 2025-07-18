#ifndef __FFT_H__
#define __FFT_H__

#include <stdint.h>

#define FFT_LEN 1024
#define SAMPLENUM 1024
#define PI 3.14159265358979f

// 谐波参数结构体
typedef struct{
  uint8_t type;
  int16_t fre;
}Wave;

extern uint16_t adc_value[];
extern Wave wave_val[2];
extern float fft_mag[FFT_LEN / 2];
extern uint8_t fft_flag;

void FFT_Start_ADC(void);

void FFT_Start(float voltage[], float fft_output[], float fft_mag[], uint16_t len, Wave wave_val[]);

void Transmit_adc_to_int16(int16_t geted_val[]);
double corr1000_200(int16_t *data, const int16_t *mask);
double process_frequency(int16_t *res, int f, int template_row) ;
double Get_Delta_Phase( double phase_diff[]);
double Get_Delta_Fre(double delta_phase[], double now_fre, double delay_time);

void FFT_Process(void);

#endif
