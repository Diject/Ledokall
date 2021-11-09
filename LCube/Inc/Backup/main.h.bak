/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef union
{
	float fl;
	uint8_t u8[4];
	uint16_t u16[2];
	uint32_t u32;
} utype32;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Button2_Pin GPIO_PIN_1
#define Button2_GPIO_Port GPIOA
#define Button2_EXTI_IRQn EXTI1_IRQn
#define Button1_Pin GPIO_PIN_2
#define Button1_GPIO_Port GPIOA
#define Button1_EXTI_IRQn EXTI2_IRQn
#define Button0_Pin GPIO_PIN_3
#define Button0_GPIO_Port GPIOA
#define Button0_EXTI_IRQn EXTI3_IRQn
#define MAX_SD_Pin GPIO_PIN_6
#define MAX_SD_GPIO_Port GPIOA
#define USBC_Vbus_Pin GPIO_PIN_9
#define USBC_Vbus_GPIO_Port GPIOA
#define USBC_Vbus_EXTI_IRQn EXTI9_5_IRQn
#define MUSB_ID_Pin GPIO_PIN_10
#define MUSB_ID_GPIO_Port GPIOA
#define MUSB_ID_EXTI_IRQn EXTI15_10_IRQn
#define USB_Select_Pin GPIO_PIN_15
#define USB_Select_GPIO_Port GPIOA
#define SDCard_Pow_Pin GPIO_PIN_10
#define SDCard_Pow_GPIO_Port GPIOC
#define SDIO_CD_Pin GPIO_PIN_11
#define SDIO_CD_GPIO_Port GPIOC
#define SDIO_CD_EXTI_IRQn EXTI15_10_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
