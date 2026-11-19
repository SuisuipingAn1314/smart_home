#ifndef _SHT3X_H_
#define _SHT3X_H_


#include <stdint.h>

#define SHT3X_ADDR 0x44  // 定义从机地址



void SHT3X_Init(void);
uint8_t SHT3X_Get(float* temp, float* humi);



#endif
