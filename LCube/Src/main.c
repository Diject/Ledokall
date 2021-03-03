/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "i2s.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sdio.h"
#include "LEDVisualization.h"
#include "AudioProcessing.h"
#include "Microphone.h"
#include "buttons.h"
#include "WavePlayer.h"
#include "NN.h"
#include "NN_Detection.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern ringBuffer_Int16 buffer;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define AUDIO_CLOCK_PLLI2SN_MAX 318U
#define AUDIO_CLOCK_PLLI2SN_MIN 316U

void audio_clock_increase()
{
	//�?зменяем множитель PLL для аудио.
	uint32_t clockMul = (RCC->PLLI2SCFGR & ~0xFFFF803F) >> 6;
	if (clockMul < AUDIO_CLOCK_PLLI2SN_MAX)
	{
		RCC->PLLI2SCFGR = (RCC->PLLI2SCFGR & 0xFFFF803F) | (AUDIO_CLOCK_PLLI2SN_MAX << 6);
	}

}

void audio_clock_decrease()
{
	//�?зменяем множитель PLL для аудио.
	uint32_t clockMul = (RCC->PLLI2SCFGR & ~0xFFFF803F) >> 6;
	if (clockMul > AUDIO_CLOCK_PLLI2SN_MIN)
	{
		RCC->PLLI2SCFGR = (RCC->PLLI2SCFGR & 0xFFFF803F) | (AUDIO_CLOCK_PLLI2SN_MIN << 6);
	}
}

void HAL_IncTick()
{

}

inline uint32_t HAL_GetTick()
{
	return get_device_counter();
}

/**
 * Определение рабочего usb
 */
void detect_usb_input()
{
	//По очереди включаем usb входы и проверяем на наличие sof-пакетов
	static uint32_t step = 0;
	static uint32_t timTick = 0xffff;
	if (device.USBInput != USB_INPUT_UNDEFINED)
	{
		//Проверка, работает ли текущее соединение
		if (((get_device_counter() - timTick) > 20000) && step == 0)
		{
			device.USBInput = USB_INPUT_NONE;
			USB_OTG_FS->GINTMSK |= 1 << 3; //Enable SOF Interrupt
			timTick = get_device_counter();
			step = 1;
		}else if (((get_device_counter() - timTick) > 5000) && step == 1)
		{
			if (device.USBInput == USB_INPUT_NONE)
			{
				device.USBInput = USB_INPUT_UNDEFINED;
			}
			step = 0;
			timTick = get_device_counter();
		}
	}else
	{
		//Определение рабочего usb
		if (step == 0)
		{
			HAL_GPIO_WritePin(USB_Select_GPIO_Port, USB_Select_Pin, 0);
			step = 1;
			timTick = get_device_counter();
		}else if (step == 1 && (get_device_counter() - timTick) > 10)
		{
			MX_USB_DEVICE_Init();
			step = 2;
			timTick = get_device_counter();
		}else if (step == 2 && (get_device_counter() - timTick) > 3000)
		{
			if (device.USBInput == USB_INPUT_UNDEFINED)
			{
				HAL_GPIO_WritePin(USB_Select_GPIO_Port, USB_Select_Pin, 1);
				step = 3;
			}else step = 0;
			timTick = get_device_counter();
			EXTI->RTSR |= USBC_Vbus_Pin;
			EXTI->FTSR |= USBC_Vbus_Pin;
			EXTI->RTSR |= MUSB_ID_Pin;
			EXTI->FTSR |= MUSB_ID_Pin;
		}else if (step == 3 && (get_device_counter() - timTick) > 10)
		{
			MX_USB_DEVICE_Init();
			step = 4;
			timTick = get_device_counter();
		}else if (step == 4 && (get_device_counter() - timTick) > 3000)
		{
			if (device.USBInput == USB_INPUT_UNDEFINED) device.USBInput = USB_INPUT_NONE;
			step = 0;
			timTick = get_device_counter();
		}
	}
//	HAL_Delay(2000);
//	HAL_GPIO_WritePin(USB_Select_GPIO_Port, USB_Select_Pin, 0);
//	HAL_Delay(1);
//	MX_USB_DEVICE_Init();
//	HAL_Delay(2000);
//	if (device.USBInput == USB_INPUT_UNDEFINED)
//	{
//		HAL_GPIO_WritePin(USB_Select_GPIO_Port, USB_Select_Pin, 1);
//		HAL_Delay(1);
//		MX_USB_DEVICE_Init();
//	}
//	HAL_Delay(2000);
//	if (device.USBInput == USB_INPUT_UNDEFINED) device.USBInput = USB_INPUT_NONE;
//	EXTI->RTSR |= USBC_Vbus_Pin;
//	EXTI->FTSR |= USBC_Vbus_Pin;
//	EXTI->RTSR |= MUSB_ID_Pin;
//	EXTI->FTSR |= MUSB_ID_Pin;
}

void buttons_control()
{
	uint8_t pcnt[3] = {0};
	uint16_t ptime[3] = {0};
	uint8_t pbtn = buttons_stateEx(pcnt, ptime); //Нажатые кнопки
	if ((pbtn == (BUTTON0 | BUTTON2)) && (ptime[0] > 0x0200) && (ptime[2] > 0x0200))
	{
		VisualizationStop(device.visualizationStatus ? DISABLE : ENABLE);
	}else if ((pbtn == BUTTON1) && (ptime[1] > 0x0300))
	{
		if (device.audio.muteVoice) NN_Action_UnMuteCommands(0);
		else NN_Action_MuteCommands(0);
	}else if ((pbtn) == BUTTON0)
	{
		uint32_t vis = device.visualization != 0 ? device.visualization - 1 : VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS;
		InitLEDVisualization(vis);
	}else if ((pbtn) == BUTTON2)
	{
		uint32_t vis = device.visualization >= VISUALIZATION_STATE_TIME_VIS ? VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS : device.visualization + 1;
		InitLEDVisualization(vis);
	}else if ((pbtn) == BUTTON1)
	{
		if (device.audio.muteAudio) NN_Action_UnMuteSound(0);
		else NN_Action_MuteSound(0);
	}
}

/**
 * Инициализация SD карты
 */
void sdcard_init()
{
	static uint32_t updateSDCardTick = 0xffff;
	if (device.sdcard.sdcardInitialization > 0)
	{
		if (device.sdcard.sdcardInitialization == SDCARD_INITIALIZATION_START)
		{
			HAL_GPIO_WritePin(SDCard_Pow_GPIO_Port, SDCard_Pow_Pin, 1);
			updateSDCardTick = get_device_counter();
			device.sdcard.sdcardInitialization = SDCARD_INITIALIZATION_POWERON;
		}else if (device.sdcard.sdcardInitialization == SDCARD_INITIALIZATION_POWERON)
		{
			if ((get_device_counter() - updateSDCardTick) > 2000)
			{
				MX_FATFS_Init();
				if (f_mount(&SDFatFS, (const TCHAR*)SDPath, 1) == FR_OK)
				{
					device.sdcard.sdcardInitialization = SDCARD_INITIALIZATION_MOUNTED;
				}
			}
		}
	}
}

/**
 * Обновление данных температуры/влажности
 */
void update_sensor_data()
{
	static uint32_t updateTempTick = 0xffffff;
	if ((get_device_counter() - updateTempTick) > 30000)
	{
		static uint32_t step = 0;
		if (step++ & 1) HDC1080_TriggerMeasurement();
		else HDC1080_ReadData_TriggerDMAReceive();
		updateTempTick = get_device_counter();
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  	SysTick->CTRL = 0; //disable systick
  	MX_TIM4_Init();
	MX_TIM5_Init();
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_Base_Start(&htim5); //start device counter
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C3_Init();
  MX_I2S1_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */
  //�?нициализация нейросети
  MX_SDIO_SD_Init();
  NN_Init();
  //�?нициализация sdcard
  if (!HAL_GPIO_ReadPin(SDIO_CD_GPIO_Port, SDIO_CD_Pin))
  {
    //Вкл питание
    HAL_GPIO_WritePin(SDCard_Pow_GPIO_Port, SDCard_Pow_Pin, 1);
  }

  HAL_GPIO_WritePin(GPIOB, 0x9249, 1);
  GPIOC->BSRR = 0b10000000 << 16;
  //Триггер измерения темп/вл
  HDC1080_TriggerMeasurement();
  HAL_Delay(1000);
  //�?нициализация системы определения голосовых команд
  NN_Detect_Init();
  //�?нициализация аудиообработки
  audioProcessing_Init();
  //�?нициализация
  InitDevice();
  //�?нициализация микрофона
  Microphone_Init();
  Microphone_Start_Capture();
  //Включение усилителя звука
  HAL_GPIO_WritePin(MAX_SD_GPIO_Port, MAX_SD_Pin, 1);

  //Определение USB
  detect_usb_input();


  //Отладка
//  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
//  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  update_sensor_data();

	  sdcard_init();

	  buttons_control();

	  if (device.USBInput == USB_INPUT_UNDEFINED)
		  detect_usb_input();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S_APB2|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_SDIO|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 317;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 11;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 7;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 8;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
  PeriphClkInitStruct.I2sApb2ClockSelection = RCC_I2SAPB2CLKSOURCE_PLLI2S;
  PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
