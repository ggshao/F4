#include "oledStatus.h"

OLEDStatus oledStatus;

typedef enum {
    DISPLAY,
    CURRENT,
    MODE,
	SAVE,
} Page;


void OLEDStatus_Init(OLEDStatus *sm){
    sm->page = 0;
	sm->up_cnt = 0;
	sm->down_cnt = 0;
	sm->left_cnt = 0;
	sm->right_cnt = 0;
	sm->press_flag = 0;
	sm->delay_time = 500;
	sm->save_flag = 0;
}

void OLEDStatus_Display(){
	if(oledStatus.left_cnt > 0 || oledStatus.right_cnt > 0){
		oledStatus.press_flag = 1;
		OLED_Clear();
		HAL_Delay(50);
		OLED_Refresh();
		HAL_Delay(50);
	}
	else{
		oledStatus.press_flag = 0;
	}

	if(oledStatus.press_flag == 0){
		if(oledStatus.page == DISPLAY){
			sprintf(stateMach.oled_message, "I   VOUT   VIN \n");
			OLED_ShowString(8,0,(uint8_t *)stateMach.oled_message,16,1);
			sprintf(stateMach.oled_message, "%+2.1f,%2.0f,%2.0f \n",stateMach.current,stateMach.v_out,stateMach.v_in);
			OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
			sprintf(stateMach.oled_message, "s:%s e:%s \n",status_show[stateMach.currentState],error_show[stateMach.error]);
			OLED_ShowString(8,32,(uint8_t *)stateMach.oled_message,16,1);
			OLED_Refresh();
		}
		else if(oledStatus.page == CURRENT){
			sprintf(stateMach.oled_message, "CURRENT MAX:   \n");
			OLED_ShowString(8,0,(uint8_t *)stateMach.oled_message,16,1);
			sprintf(stateMach.oled_message, "%2.1f          \n",stateMach.maxcurrent);
			OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
			OLED_Refresh();
		}
		else if(oledStatus.page == MODE){
			sprintf(stateMach.oled_message, "MODE SETTING:   \n");
			OLED_ShowString(8,0,(uint8_t *)stateMach.oled_message,16,1);
			if(stateMach.start_mode == REMOTE_CONTROL){
				sprintf(stateMach.oled_message, "REMOTE_CONTROL\n");
			}
			else if(stateMach.start_mode == AUTO_START){
				sprintf(stateMach.oled_message, "AUTO_START    \n");
			}
			OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
			OLED_Refresh();
		}
		else if(oledStatus.page == SAVE){
			sprintf(stateMach.oled_message, "UP TO SAVE   \n");
			OLED_ShowString(8,0,(uint8_t *)stateMach.oled_message,16,1);
			if(oledStatus.save_flag == 0){
				sprintf(stateMach.oled_message, "NO_SAVE        \n");
				OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
				OLED_Refresh();
			}
			else{
				sprintf(stateMach.oled_message, "PLEASEWAIT      \n");
				OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
				OLED_Refresh();
				Flash_WriteStateMachine(&stateMach);
				oledStatus.save_flag = 0;
			}
		}

	}
//	sprintf(stateMach.oled_message, "%d,%d,%d \n",oledStatus.press_flag,oledStatus.left_cnt,oledStatus.page);
//	OLED_ShowString(8,16,(uint8_t *)stateMach.oled_message,16,1);
//	OLED_Refresh();
	
}

void OLEDStatus_debounce_cnt_increase(){
	if(HAL_GPIO_ReadPin(UPGPIO, UPPIN) == GPIO_PIN_RESET){
		oledStatus.up_cnt++;
	}
	else if(HAL_GPIO_ReadPin(UPGPIO, UPPIN) == GPIO_PIN_SET)
	{
		oledStatus.up_cnt = 0;
	}
	
	if(HAL_GPIO_ReadPin(DOWNGPIO, DOWNPIN) == GPIO_PIN_RESET){
		oledStatus.down_cnt++;
	}
	else if(HAL_GPIO_ReadPin(DOWNGPIO, DOWNPIN) == GPIO_PIN_SET)
	{
		oledStatus.down_cnt = 0;
	}

	if(HAL_GPIO_ReadPin(LEFTGPIO, LEFTPIN) == GPIO_PIN_RESET){
		oledStatus.left_cnt++;
	}
	else if(HAL_GPIO_ReadPin(LEFTGPIO, LEFTPIN) == GPIO_PIN_SET)
	{
		oledStatus.left_cnt = 0;
	}

	if(HAL_GPIO_ReadPin(RIGHTGPIO, RIGHTPIN) == GPIO_PIN_RESET){
		oledStatus.right_cnt++;
	}
	else if(HAL_GPIO_ReadPin(RIGHTGPIO, RIGHTPIN) == GPIO_PIN_SET)
	{
		oledStatus.right_cnt = 0;
	}

//    OLEDStatus_UP();
//    OLEDStatus_DOWN();
//    OLEDStatus_LEFT();
//    OLEDStatus_RIGHT();

}

//void OLEDStatus_UP(){
//	if(oledStatus.up_cnt == 20 || (oledStatus.up_cnt>oledStatus.delay_time&&oledStatus.up_cnt%50==0)){
//		switch(oledStatus.page){
//			case DISPLAY:
//				if(stateMach.currentState == WAITTING && stateMach.v_in>3){
//					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_SET);
//					stateMach.currentState = PRELOAD;
//					stateMach.counter = 0;
//					stateMach.remote_lock = 1;
//				}
//				break;
//			case CURRENT:
//				stateMach.maxcurrent += 1;
//				if(stateMach.maxcurrent > 100){
//					stateMach.maxcurrent = 100;
//				}
//				break;
//			case MODE:
//				stateMach.start_mode += 1;
//				if(stateMach.start_mode > 1){
//					stateMach.start_mode = 0;
//				}
//				break;
//			case SAVE:
//				oledStatus.save_flag = 1;
//				break;
//		}
//	}
//}
//
//void OLEDStatus_DOWN(){
//	if(oledStatus.down_cnt == 20 || (oledStatus.down_cnt>oledStatus.delay_time&&oledStatus.down_cnt%50==0)){
//		switch(oledStatus.page){
//			case DISPLAY:
//				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,GPIO_PIN_RESET);
//				stateMach.error = NO_ERROR;
//				stateMach.currentState = WAITTING;
//				stateMach.counter = 0;
//				break;
//			case CURRENT:
//				stateMach.maxcurrent -= 1;
//				if(stateMach.maxcurrent < 0){
//					stateMach.maxcurrent = 0;
//				}
//				break;
//			case MODE:
//				stateMach.start_mode -= 1;
//				if(stateMach.start_mode < 0){
//					stateMach.start_mode = 1;
//				}
//				break;
//		}
//	}
//
//}

void OLEDStatus_LEFT(){
	if(oledStatus.left_cnt == 20){
		oledStatus.page--;
		if(oledStatus.page < 0){
			oledStatus.page = 3;
		}
	}
}

void OLEDStatus_RIGHT(){
	if(oledStatus.right_cnt == 20){
		oledStatus.page++;
		if(oledStatus.page > 3){
			oledStatus.page = 0;
		}
	}
}
