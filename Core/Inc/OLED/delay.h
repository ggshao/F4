#ifndef __DELAY_H
#define __DELAY_H 			   

#include "stm32f4xx.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif





























