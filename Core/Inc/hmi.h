#include "main.h"

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
// void HMI_
