/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "status.h"
#include "ES1642.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim3;

//uint8_t RS485_rx_buf[RS485_REC_LEN]; /*接收缓冲,最大RS485_REC_LEN个字节*/
//uint8_t RS485_rx_cnt = 0;
/* USER CODE BEGIN EV */
uint8_t RX_data[128];
//uint16_t RX3_Cnt = 0;
//uint16_t RX3_TimeOut = 0;
#define RX3_TIMEOUT_SET 5 // 10ms超时
//extern UART_HandleTypeDef huart3;

uint8_t src_addr[6];
uint8_t user_data[4];
uint16_t user_data_len;
int result = 0;
int i;
uint8_t ES1642_frame[32];
uint8_t frame_length;
uint8_t chargeState = 0;
uint16_t connect_timeout = 0;
// int tim3_cnt=0;





/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  StateMachine_wait_stable();
  StateMachine_messure();
  StateMachine_alive_test();
  StateMachine_vin_low_protect(); //先测试是否稳定，再做后续的操作
  Temp_PWM_Control();
  // StateMachine_overcurrent_protect();
//  Temp_PWM_Control();
  if(stateMach.wait_stable_cnt>1000){
    StateMachine_auto_start();
    StateMachine_BT_cnt_add();
    StateMachine_remote_check();
    StateMachine_preload_protect();
    OLEDStatus_debounce_cnt_increase();
  }
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
  */
void TIM1_UP_TIM10_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */
  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream2 global interrupt.
  */
void DMA2_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream2_IRQn 0 */

  /* USER CODE END DMA2_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA2_Stream2_IRQn 1 */

  /* USER CODE END DMA2_Stream2_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream4 global interrupt.
  */
void DMA2_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream4_IRQn 0 */

  /* USER CODE END DMA2_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN 4_IRQn 1 */

  /* USER CODE END DMA2_Stream4_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream7 global interrupt.
  */
void DMA2_Stream7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream7_IRQn 0 */

  /* USER CODE END DMA2_Stream7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  /* USER CODE BEGIN DMA2_Stream7_IRQn 1 */

  /* USER CODE END DMA2_Stream7_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //定时器回调函数
{
//    static	int tim3_cnt=0;
    if(TIM1 == htim->Instance)
    {
      StateMachine_timer1_test();
     StateMachine_overcurrent_protect();
      StateMachine_fans_speed();
    }
    else if(htim->Instance == TIM3)
    { // 假设用TIM3做1ms定时
//    	tim3_cnt++;
//    	if(tim3_cnt == 500)
//    	{
//    		tim3_cnt = 0;
//    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
//    	}
//    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        if (RX3_TimeOut > 0) {
            if (--RX3_TimeOut == 0) {
                if (RX3_Cnt > 0) {
//                	for (i = 0; i < RX3_Cnt; i++)
//                	    HAL_UART_Transmit(&huart3, &RX_data[i], 1, HAL_MAX_DELAY);
                        result = parse_received_data_frame(
                        RX_data,
                        src_addr,
                        user_data,
                        sizeof(user_data),
                        &user_data_len);
                    if (result == 0) {
                        if (user_data[0] == 0xDD && uart_tx_enabled == 1) {
                            for (i = 0; i < frame_length; i++)
                            HAL_UART_Transmit(&huart3, &ES1642_frame[i], 1, HAL_MAX_DELAY);
                            chargeState = 1;
                            connect_timeout = 1000;
                        }
                    }
                    RX3_Cnt = 0;
                    HAL_UART_Receive_IT(&huart3, &RX_data[RX3_Cnt], 1);

                }
            }
        }

        // 其他定时任务...&& uart_tx_enabled == 1
    }
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim->Instance == TIM3) { // 假设用TIM3做1ms定时
//        if (RX3_TimeOut > 0) {
//            if (--RX3_TimeOut == 0) {
//                if (RX3_Cnt > 0) {
//                    result = parse_received_data_frame(
//                        RX_data,
//                        src_addr,
//                        user_data,
//                        sizeof(user_data),
//                        &user_data_len);
//                    if (result == 0) {
//                        if (user_data[0] == 0xDD) {
//                            for (i = 0; i < frame_length; i++)
//                                HAL_UART_Transmit(&huart3, &ES1642_frame[i], 1, HAL_MAX_DELAY);
//                            chargeState = 1;
//                            connect_timeout = 1000;
//                        }
//                    }
//                    RX3_Cnt = 0;
//                }
//            }
//        }
//        // 其他定时任务...

//    }
//}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ //串口回调函数，接收完一个字节后继续下一个
	if(huart->Instance == USART1){

	HAL_UART_Receive_IT(&huart1,(uint8_t *)stateMach.bt_rx_buffer, 1);
	}

	else if (huart->Instance == USART3) {
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);


	    // 重新启动接收中断
		//RS485_rx_cnt++;
		RX3_Cnt++;
		RX3_TimeOut = RX3_TIMEOUT_SET;
		HAL_UART_Receive_IT(&huart3, &RX_data[RX3_Cnt], 1);


	}
}


/* USER CODE END 1 */
