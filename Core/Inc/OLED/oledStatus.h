#ifndef OLEDSTATUS_H
#define OLEDSTATUS_H

#include <stdint.h>
#include "main.h"

typedef struct {
    int page;//oled页
    uint32_t up_cnt;
    uint32_t down_cnt;
    uint32_t left_cnt;
    uint32_t right_cnt;
    uint8_t press_flag;
    int delay_time;
    uint8_t save_flag;
} OLEDStatus;

void OLEDStatus_Init(OLEDStatus *sm);
void OLEDStatus_Display();
void OLEDStatus_debounce_cnt_increase();
void OLEDStatus_UP();
void OLEDStatus_DOWN();
void OLEDStatus_LEFT();
void OLEDStatus_RIGHT();

extern OLEDStatus oledStatus;

#define UPGPIO GPIOC
#define DOWNGPIO GPIOC
#define LEFTGPIO GPIOA
#define RIGHTGPIO GPIOC

#define UPPIN GPIO_PIN_10
#define DOWNPIN GPIO_PIN_12
#define LEFTPIN GPIO_PIN_15
#define RIGHTPIN GPIO_PIN_11

#endif // STATUS_H
