/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
	HAL_RCCEx_SelectLSEMode(RCC_LSE_HIGHDRIVE_MODE);
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * Установка калибровачных значений
 * secs - необходимое значение смещения за день в секундах
 */
void set1dCalibSec(float secs)
{
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
	if (secs == 0.f)
	{
		RTC->CALR = 0;
	}else if (secs > 0.f)
	{
		uint32_t APres = hrtc.Instance->PRER >> 16;
		uint32_t SPres = hrtc.Instance->PRER & 0x7fff;
		uint32_t step = ((APres + 1U) * (SPres + 1U) * 60U * 60U * 24U / 1048576U);
		float res = secs / (float)step * (float)((APres + 1U) * (SPres + 1U)) + 0.5f;
		uint32_t calm = (uint32_t)res;
		if (calm > 512) calm = 512;
		calm = 512 - calm;
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
		RTC->CALR = RTC_SMOOTHCALIB_PERIOD_32SEC | RTC_SMOOTHCALIB_PLUSPULSES_SET | calm;
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	}else
	{
		uint32_t APres = hrtc.Instance->PRER >> 16;
		uint32_t SPres = hrtc.Instance->PRER & 0x7fff;
		uint32_t step = ((APres + 1U) * (SPres + 1U) * 60U * 60U * 24U / 1048576U);
		float res = -1.f * secs / (float)step * (float)((APres + 1U) * (SPres + 1U)) + 0.5f;
		uint32_t calm = (uint32_t)res;
		if (calm > 0x1ff) calm = 0x1ff;
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
		RTC->CALR = RTC_SMOOTHCALIB_PERIOD_32SEC | RTC_SMOOTHCALIB_PLUSPULSES_RESET | calm;
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	}
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
}

/**
 * Калибровочное смещение времени за день
 */
float get1dCalibSec()
{
	float res = 0;
	uint32_t APres = hrtc.Instance->PRER >> 16;
	uint32_t SPres = hrtc.Instance->PRER & 0x7fff;
	uint32_t step = ((APres + 1U) * (SPres + 1U) * 60U * 60U * 24U / 1048576U);
	uint32_t calm = RTC->CALR & 0x1FF;
	if (RTC->CALR & (1 << 15))
	{
		res = (float)(512U - calm) / (float)((APres + 1U) * (SPres + 1U)) * (float)step;
	}else
	{
		res = -1.f * (float)calm / (float)((APres + 1U) * (SPres + 1U)) * (float)step;
	}
	return res;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
