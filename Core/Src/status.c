#include "status.h"
//#include "FlashDrv.h"
char status_show[4][20] = {
    "WAIT",
    "PREL",
    "OPEN",
    "ERROR",
};  //四种状态

char error_show[4][20] = {
    "NOERR",
    "OC",
    "P_S",
    "P_C",
}; //四种错误类型：无错误、过流、短路、开路

StateMachine stateMach;
uint8_t message[30];
void StateMachine_Init(StateMachine *sm) {    //各种初始化：状态、计数器、电流、电压、PWM、风扇、蓝牙、oled
	StateMachine temp; //存储FLASH中读取到的状态机信息
	Flash_ReadStateMachine(&temp);
	sm->currentState = WAITTING; // 根据你的状态机定义设置初始状态，目前为等待
	sm->error =NO_ERROR;
	sm->counter = 0;
	sm->alive_counter = 0;
	sm->bt_counter = 0;
    sm->I_counter = 0;
	sm->short_cnt = 10; // 根据你的需求设置初始值 短路保护
	sm->cap_cnt = 130;   // 根据你的需求设置初始值 电容充电
	sm->main_power_cnt = 2000;   
//	sm->cap_cnt = 10;
    sm->wait_stable_cnt = 0;
	sm->auto_start_cnt = 0;
	sm->fans_cnt = 0;
	sm->current = 0.0f;
    sm->maxcurrent = temp.maxcurrent;
    if(sm->maxcurrent<0 || sm->maxcurrent>100 || isnan(sm->maxcurrent))
    sm->maxcurrent = 100;
	sm->v_out = 0.0f;
	sm->v_in = 0.0f;
	sm->duty_temp_1 = 0.0f; //占空比
	sm->duty_temp_2 = 0.0f;

	memset(sm->fans_speed, 0, sizeof(sm->fans_speed));
	memset(sm->fans_toggle, 0, sizeof(sm->fans_toggle));
	memset(sm->fans_prev_level, 0, sizeof(sm->fans_prev_level));
	memset(sm->fans_current_level, 0, sizeof(sm->fans_current_level));

	memset(sm->bt_message, 0, sizeof(sm->bt_message));
	memset(sm->oled_message, 0, sizeof(sm->oled_message));
	sm->error = NO_ERROR;
    sm->alpha = 0.05f; //滤波系数
    sm->start_mode = temp.start_mode;
    if(sm->start_mode<0 || sm->start_mode>1)
    	sm->start_mode = AUTO_START; //自动启动
    sm->remote_lock = 1;
}

void StateMachine_OPEN() {
	if(stateMach.currentState == WAITTING && stateMach.v_in>3){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,GPIO_PIN_SET);//自己加的
		stateMach.currentState = PRELOAD;
//		stateMach.currentState = OPEN;
		stateMach.counter = 0;
	}   //wait且vin大于3状态时PC0为高电平，改目前状态为PRELOAD,counter重置为0
}

void StateMachine_OFF(){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,GPIO_PIN_RESET);
	stateMach.error = NO_ERROR;
	stateMach.currentState = WAITTING;
	stateMach.counter = 0;
	// stateMach.remote_lock = 0; 定义关闭状态：PC0、PC1为低电平，无错误、等待状态、置0.
}

void StateMachine_alive_test() {
	stateMach.alive_counter++;
	if(stateMach.alive_counter == 500) {
		stateMach.alive_counter = 0;
		// HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7); 检测系统是否正常运行，判断时间为500ms
	}
}

void StateMachine_timer1_test() {
	stateMach.I_counter++;
    if(stateMach.I_counter == 500) {
        stateMach.I_counter = 0;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7); //电流计数器计算500后重新从0开始计数
    }
}

void StateMachine_messure(){
	stateMach.v_in = (double)AdcBuf[2]/4096*3.3/0.026;
	stateMach.v_out = (double)AdcBuf[1]/4096*3.3/0.026;
	stateMach.v_in_filtered = stateMach.v_in_filtered* 0.9999 + 0.0001*((double)AdcBuf[2]/4096*3.3)/0.026;
} //计算输入电压和输出电压

void StateMachine_BT_cnt_add(){
	if(stateMach.bt_counter < 100) //蓝牙计数器0到100计数，100时停止
		stateMach.bt_counter++;
	
}

const char* stateStrings[] = {
    "WAITTING",
    "PRELOAD",
    "OPEN",
    "STATE_ERROR"
};

const char* errorStrings[] = {
    "NO_ERROR",
    "OVERCURRENT",
    "PRELOAD_SHORT",  //预加载短路
    "PRELOAD_CAP"	//预加载电容
};

//void StateMachine_BT_sent(){
//	if(stateMach.bt_counter == 100) {
//		stateMach.bt_counter = 0;
//		// 临时缓冲区，用于存储 fans_speed 数组的字符串
//		char fans_speed_str[100] = {0};
//		char temp[16];
//		// 将 fans_speed 数组的内容拼接到 fans_speed_str
//		for (int i = 0; i < 8; i++) { //8个风扇
//			snprintf(temp, sizeof(temp), "sp%d:%d ", i + 1, stateMach.fans_speed[i]); //将格式化后的字符串存储到 temp 中，格式为 "sp风扇编号:速度值 "，例如 "sp1:100 "。
//			strcat(fans_speed_str, temp); // 拼接到 fans_speed_str
//		}
//		sprintf(stateMach.bt_message, "i:%+2.1f vo:%2.1f vi:%2.1f s:%s e:%s MI:%.1f GET:%d pwm1:%.1f pwm2:%.1f\n %s     \n\n",
//		        stateMach.current,
//		        stateMach.v_out,
//		        stateMach.v_in,
//		        stateStrings[stateMach.currentState], // 将状态枚举转为字符串
//		        errorStrings[stateMach.error],       // 将错误枚举转为字符串
//		        stateMach.maxcurrent,
//				stateMach.bt_rx_buffer[0],       //蓝牙接收缓冲区的第一个字节
//				stateMach.duty_temp_1,
//				stateMach.duty_temp_2,
//				fans_speed_str
//				);
//		HAL_UART_Transmit_DMA(&huart1,(uint8_t *)stateMach.bt_message,sizeof(stateMach.bt_message)); //使用 DMA（直接内存访问）方式通过 UART1 发送 stateMach.bt_message 中的内容。DMA 发送可以提高数据传输的效率，减少 CPU 的负担。
//	}
//}

//void StateMachine_BT_receive(){
//	if(stateMach.bt_rx_buffer[0] != 0){
//		if(stateMach.bt_rx_buffer[0] == 'o'){
//			StateMachine_OPEN();
//		}
//		if(stateMach.bt_rx_buffer[0] == 's'){
//			StateMachine_OFF();
//		}
//		if(stateMach.bt_rx_buffer[0] == 'u'){
//			if(stateMach.maxcurrent<100)
//				stateMach.maxcurrent += 5;
//		}
//		if(stateMach.bt_rx_buffer[0] == 'd'){
//			if(stateMach.maxcurrent>1)
//				stateMach.maxcurrent -= 5;
//		}
//		if(stateMach.bt_rx_buffer[0] == 'b'){
//			Flash_WriteStateMachine(&stateMach);
//		} //调用FLASH函数存储状态机状态
//		if(stateMach.bt_rx_buffer[0] == 'f'){
//			stateMach.duty_temp_1 += 50;
//			stateMach.duty_temp_2 += 50;
//			if (stateMach.duty_temp_1 > 500) {
//				stateMach.duty_temp_1 = 0;
//				stateMach.duty_temp_2 = 0; //调整占空比
//			}
//		}
//		stateMach.bt_rx_buffer[0] = 0;
//	}
//}


void StateMachine_remote_check(){
	uint8_t remote = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
	if(remote == GPIO_PIN_SET && stateMach.currentState == WAITTING && stateMach.v_in>VIN_OPEN && stateMach.remote_lock==0){
		// HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_SET);
		// stateMach.currentState = PRELOAD;
		// stateMach.counter = 0;
		StateMachine_OPEN();//状态机打开
	}
	else if(remote == GPIO_PIN_RESET){
		StateMachine_OFF();
		stateMach.remote_lock = 0;//遥控器关闭则解锁遥控器开启
	}
}

void StateMachine_auto_start(){
	if(stateMach.currentState == WAITTING && stateMach.v_in>VIN_OPEN && stateMach.start_mode == AUTO_START && stateMach.auto_start_cnt<AUTO_START_TIME){
		stateMach.auto_start_cnt ++;
	} //充电充两秒
	if(stateMach.auto_start_cnt == AUTO_START_TIME){	//在系统稳定条件下，3秒钟后自启动
		StateMachine_OPEN();
		stateMach.auto_start_cnt = AUTO_START_TIME + 1;
	}
}

void StateMachine_vin_low_protect(){
	if(stateMach.v_in < VIN_OPEN){
		StateMachine_OFF();
		stateMach.auto_start_cnt = 0;
		stateMach.remote_lock = 1;
	}
}

void StateMachine_preload_protect(){
	if(stateMach.currentState == PRELOAD){// 处理预加载状态,控制预充和主开关
        stateMach.counter ++;
        if ((stateMach.v_out < (stateMach.v_in * 0.1)) && (stateMach.counter == stateMach.short_cnt)) {//检查输出电压是否不足输入电压的10%
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,GPIO_PIN_RESET); 
			stateMach.currentState = STATE_ERROR;
			stateMach.error = PRELOAD_SHORT;
		}
		if ((stateMach.v_out < (stateMach.v_in * 0.85)) && (stateMach.counter == stateMach.cap_cnt)) {//电容充满电后检查输出电压是否不足输入电压的85%
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,GPIO_PIN_RESET);
			stateMach.currentState = STATE_ERROR;
            stateMach.error = PRELOAD_CAP; //报错充电失败
		}
		else if (stateMach.v_out >= (stateMach.v_in * 0.85) && stateMach.counter >= stateMach.main_power_cnt) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
//            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);//自己加的
            stateMach.currentState = OPEN;
        }
    }
}


void StateMachine_overcurrent_protect() {
	float current_raw = ((double)AdcBuf[0] - 2048) / 4096 * 3.3 / 50 / 0.3 * 1000;
	stateMach.current = stateMach.alpha * current_raw + (1 - stateMach.alpha) * stateMach.current;//低通滤波
    if (fabs(stateMach.current) > stateMach.maxcurrent) { // 过流保护
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
        stateMach.currentState = STATE_ERROR;
        stateMach.error = OVERCURRENT;
        stateMach.counter = 0;
    }
}

GPIO_TypeDef* GPIOx[8] = {
    GPIOA, GPIOA, GPIOA, GPIOC, GPIOC, GPIOB, GPIOB, GPIOB
};
uint16_t GPIO_PIN[8] = {
    GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_4,
    GPIO_PIN_5, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_12
};
void StateMachine_fans_speed() {
	stateMach.fans_cnt ++;
	for (int i = 0; i < 8; i++) {
		stateMach.fans_current_level[i] = HAL_GPIO_ReadPin(GPIOx[i], GPIO_PIN[i]); // 替换为实际 IO 配置
		// 检测电平变化
		if (stateMach.fans_current_level[i] != stateMach.fans_prev_level[i]) {
			stateMach.fans_toggle[i]++;//如果电平变化，将对应的风扇切换计数器 fans_toggle[i] 增加 1。这个计数器用于记录风扇脉冲信号的次数，从而可以计算风扇的转速。
			stateMach.fans_prev_level[i] = stateMach.fans_current_level[i]; // 更新电平状态
		}
	}
	if(stateMach.fans_cnt >= 1000){
		stateMach.fans_cnt = 0; //每隔一秒计算一次风扇的速度
		memcpy(stateMach.fans_speed, stateMach.fans_toggle, sizeof(stateMach.fans_speed));//表示风扇的速度 速度=（脉冲计数 × 60) / (测量时间 × 每转脉冲数)
		memset(stateMach.fans_toggle, 0, sizeof(stateMach.fans_toggle));
		memset(stateMach.fans_prev_level, 0, sizeof(stateMach.fans_prev_level));
		memset(stateMach.fans_current_level, 0, sizeof(stateMach.fans_current_level));
	}
}

void StateMachine_wait_stable() {
    if(stateMach.wait_stable_cnt < 3000)
	    stateMach.wait_stable_cnt++;
}



// 写入 StateMachine 结构体到 Flash
void Flash_WriteStateMachine(StateMachine *sm) {
    uint8_t buffer[sizeof(StateMachine)];
    memcpy(buffer, sm, sizeof(StateMachine));
    FlashDrv_Write(ADDR_FLASH_SECTOR_6, buffer, sizeof(StateMachine));
}

// 从 Flash 读取 StateMachine 结构体
void Flash_ReadStateMachine(StateMachine *sm) {
    uint8_t buffer[sizeof(StateMachine)];//创建与结构体大小相同的缓冲区
    FlashDrv_read(ADDR_FLASH_SECTOR_6, buffer, sizeof(StateMachine));// 从Flash扇区6读取数据到缓冲区
    memcpy(sm, buffer, sizeof(StateMachine));// 将缓冲区数据复制回结构体
}


