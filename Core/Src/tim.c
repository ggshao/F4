/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"
#include "stm32f4xx_it.h"
#include "main.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
uint16_t RX3_TimeOut = 0;
uint16_t RX3_Cnt = 0;
#define TEMP_THRESHOLD 35.0 // 温度下阈值
#define PWM_MAX 500
#define TEMP_THRESHOLD_HIGH 55.0 //温度上阈值


float motor_temperature=0.0;
float temperature_raw=0.0;
float v1 = 0.0;
float v2 = 0.0;
float temp1=27.0;
float temp2=27.0;
float tmp1=0.0;
int duty1 =0;
int duty2 =0;
float a=0.0;
int w =0;
//float r_pt=0.0;


/* TIM1 init function */
void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 18;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}
/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 9-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 500;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 250;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 179;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}



void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
   {
   /* USER CODE BEGIN TIM3_MspInit 0 */

   /* USER CODE END TIM3_MspInit 0 */
     /* TIM3 clock enable */
     __HAL_RCC_TIM3_CLK_ENABLE();

     /* TIM3 interrupt Init */
     HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
     HAL_NVIC_EnableIRQ(TIM3_IRQn);
   /* USER CODE BEGIN TIM3_MspInit 1 */

   /* USER CODE END TIM3_MspInit 1 */
   }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspPostInit 0 */

  /* USER CODE END TIM2_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PB2     ------> TIM2_CH4
    PB10     ------> TIM2_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspPostInit 1 */

  /* USER CODE END TIM2_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
   {
   /* USER CODE BEGIN TIM3_MspDeInit 0 */

   /* USER CODE END TIM3_MspDeInit 0 */
     /* Peripheral clock disable */
     __HAL_RCC_TIM3_CLK_DISABLE();

     /* TIM3 interrupt Deinit */
     HAL_NVIC_DisableIRQ(TIM3_IRQn);
   /* USER CODE BEGIN TIM3_MspDeInit 1 */

   /* USER CODE END TIM3_MspDeInit 1 */
   }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


void Temp_PWM_Control(void)
{

//	 uint16_t temperature_raw=(AdcBuf[3]<<8) | AdcBuf[4];
//	 temperature_raw &= 0x0FFF;
//	 v1=(double)temperature_raw *3.3/4095.0;
//	 double r=(v1 * 1000.0) / (3.3 - v1);
//	 temp1=(r - 100.0) / 0.385;
	 temperature_raw = (AdcBuf[3]<<8 | AdcBuf[4]) * 2.0485/ 4096 / 2;
//	 temperature_raw = (AdcBuf[3]<<8 | AdcBuf[4]);
	 motor_temperature = motor_temperature * 0.7 + 0.3 *(((temperature_raw * 1000 / (3.3 - temperature_raw))) - 100)/0.385;
	 v1 = (double)AdcBuf[3] /4096*3.3;
	 v2 = (double)AdcBuf[4] /4096*3.3;
	 temp1=temp1* 0.9999+0.0001*(((v2*1000/(3.3-v2)))-100)/0.385;
	 temp2=temp2* 0.9999+0.0001*(((v1*1000/(3.3-v1)))-100)/0.385;
	 float temp = temp1 > temp2 ? temp1 : temp2;

	 if(temp< 33.0) {
	     duty1 = 0;
	     duty2 = 0;
	 }
	 else if(temp < TEMP_THRESHOLD) {
//		 duty1 = 0;
//		 duty2 = 0;
	 }
	 else if(temp < 42.0) {
	     duty1 = 250;
	     duty2 = 250;// 低速运行
	 }
	 else if(temp < 45.5) {
	     duty1 = 375;
	     duty2 = 375;// 中速运行
	 }
	 else if(temp < 54.0) {
	     duty1 = 499;
	     duty2 = 499;// 高速运行
	 }
	 else {
	     duty1 = 499;
	     duty2 = 499;// 全速运行
	 }

//	 if(temp1 < 33) {
//	     duty1 = 0;
//	 }
//	 else if(temp1 < TEMP_THRESHOLD) {
////	     duty2 = 125;
//	 }
//	 else if(temp1 < 42.0) {
//	     duty1 = 250;  // 低速运行
//	 }
//	 else if(temp1 < 45.5) {
//	     duty1 = 375;  // 中速运行
//	 }
//	 else if(temp1 < 54.0) {
//	     duty1 = 499;  // 高速运行
//	 }
//	 else {
//	     duty1 = 499;  // 全速运行
//	 }









}

//void initMovingAverage(void)
//{
//	adcSum = 0;
//	adcIndex = 0;
//	sampleCount = 0;
//	    for (int i = 0; i < WINDOW_SIZE; i++) {
//	        adcBuffer[i] = 0; // 缓冲区清零
//	    }
//
//}

//uint16_t movingAverage(uint16_t newAdcValue) {
//    // 更新总和：减去最旧值，加上新值
//    if (sampleCount >= WINDOW_SIZE) {
//        adcSum -= adcBuffer[adcIndex]; // 缓冲区已满，替换最旧值
//    } else {
//        sampleCount++; // 缓冲区未满，增加样本计数
//    }
//    adcSum += newAdcValue;          // 添加新值到总和
//    adcBuffer[adcIndex] = newAdcValue; // 存储新值到缓冲区
//    adcIndex = (adcIndex + 1) % WINDOW_SIZE; // 更新循环索引
//
//    // 计算并返回平均值
//    return (uint16_t)(adcSum / sampleCount);
//}



/* USER CODE END 1 */
