#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>
#include "main.h"
#include <stdio.h>
#include <math.h>

typedef enum {
    WAITTING,
    PRELOAD,
    OPEN,
	STATE_ERROR,
} State;

typedef enum {
	NO_ERROR,
    OVERCURRENT,
	PRELOAD_SHORT,
	PRELOAD_CAP,
} ERR;

typedef enum {
	AUTO_START,
    REMOTE_CONTROL,
} StartMode;

extern char status_show[4][20];
extern char error_show[4][20];

typedef struct {
    State currentState;
    uint32_t counter;
    uint32_t alive_counter;
    uint32_t bt_counter;
    uint32_t I_counter;
    uint32_t short_cnt;
    uint32_t cap_cnt;
    uint32_t main_power_cnt;
    uint32_t wait_stable_cnt;
    uint32_t auto_start_cnt;
    uint32_t fans_cnt;
    float current;
    float maxcurrent;
    float v_out;
    float v_in;
    float v_in_filtered;

    float duty_temp_1;//PWM占空比
    float duty_temp_2;
    uint32_t fans_speed[8];
    uint32_t fans_toggle[8];
    uint8_t fans_prev_level[8];
    uint8_t fans_current_level[8];

    uint8_t bt_message[200];
    uint8_t bt_rx_buffer[1];

    uint8_t oled_message[30];
    ERR error;
    float alpha; // 低通滤波系数，范围在0到1之间，值越小滤波效果越强
    StartMode start_mode; //上电是否自启动
    uint8_t remote_lock;
} StateMachine;

void StateMachine_Init(StateMachine *sm);
void StateMachine_OPEN();
void StateMachine_OFF();
void StateMachine_alive_test();
void StateMachine_timer1_test();
void StateMachine_messure();
void StateMachine_BT_cnt_add();
void StateMachine_BT_sent();
void StateMachine_BT_receive();
void StateMachine_remote_check();
void StateMachine_auto_start();
void StateMachine_vin_low_protect();
void StateMachine_preload_protect();
void StateMachine_overcurrent_protect();
void StateMachine_fans_speed();
void StateMachine_wait_stable();
void Flash_WriteStateMachine(StateMachine *sm);
void Flash_ReadStateMachine(StateMachine *sm);
extern StateMachine stateMach;

#define VIN_OPEN 7
#define AUTO_START_TIME 2000
#endif // STATUS_H
