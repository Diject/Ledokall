
#include <usbd_audio_if.h>
#include "main.h"

#include "tim.h"
#include "buttons.h"
#include "Device.h"
#include "WavePlayer.h"
#include "AudioProcessing.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == Button0_Pin)
	{
		if (HAL_GPIO_ReadPin(Button0_GPIO_Port, Button0_Pin))
		{
			button_pressedCallback(0);
		}else
		{
			button_releasedCallback(0);
		}
	}else if (GPIO_Pin == Button1_Pin)
	{
		if (HAL_GPIO_ReadPin(Button1_GPIO_Port, Button1_Pin))
		{
			button_pressedCallback(1);
		}else
		{
			button_releasedCallback(1);
		}
	}else if (GPIO_Pin == Button2_Pin)
	{
		if (HAL_GPIO_ReadPin(Button2_GPIO_Port, Button2_Pin))
		{
			button_pressedCallback(2);
		}else
		{
			button_releasedCallback(2);
		}
	}else if (GPIO_Pin == USBC_Vbus_Pin) // USB C Vbus
	{
		EXTI->RTSR &= ~USBC_Vbus_Pin;
		EXTI->FTSR &= ~USBC_Vbus_Pin;
		device.USBInput = USB_INPUT_UNDEFINED;
		if (HAL_GPIO_ReadPin(USBC_Vbus_GPIO_Port, USBC_Vbus_Pin))
		{
			device.voltageInput |= VOLTAGE_INPUT_USBC;
			if (device.USBInput == USB_INPUT_NONE) device.USBInput = USB_INPUT_UNDEFINED;
		}else
		{
			device.voltageInput &= ~VOLTAGE_INPUT_USBC;
			if (device.USBInput == USB_INPUT_TYPEC) device.USBInput = USB_INPUT_UNDEFINED;
		}
	}else if (GPIO_Pin == MUSB_ID_Pin) // uUSB ID
	{
		EXTI->RTSR &= ~MUSB_ID_Pin;
		EXTI->FTSR &= ~MUSB_ID_Pin;
		if (HAL_GPIO_ReadPin(MUSB_ID_GPIO_Port, MUSB_ID_Pin))
		{
			device.voltageInput |= VOLTAGE_INPUT_MICROUSB;
			if (device.USBInput == USB_INPUT_NONE) device.USBInput = USB_INPUT_UNDEFINED;
		}else
		{
			device.voltageInput &= ~VOLTAGE_INPUT_MICROUSB;
			if (device.USBInput == USB_INPUT_MICRO) device.USBInput = USB_INPUT_UNDEFINED;
		}
	}else if (GPIO_Pin == SDIO_CD_Pin) // SD Card CD
	{
		if (HAL_GPIO_ReadPin(SDIO_CD_GPIO_Port, SDIO_CD_Pin))
		{
			HAL_GPIO_WritePin(SDCard_Pow_GPIO_Port, SDCard_Pow_Pin, 0);
			device.sdcard.sdcardConnection = SDCARD_UNCONNECTED;
			device.sdcard.sdcardInitialization = SDCARD_INITIALIZATION_OFF;
		}else
		{
			device.sdcard.sdcardConnection = SDCARD_CONNECTED;
			device.sdcard.sdcardInitialization = SDCARD_INITIALIZATION_START;
		}
	}
//	else if (GPIO_Pin == SP_OUTL_P_Pin) // Audio connector
//	{
//
//	}
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) wavePlayer_FullTransferCallback();
	else if (device.audio.USBAudioTransmission == AUDIOTRANSMISSION_RUNS) TransferComplete_CallBack_FS();
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) wavePlayer_HalfTransferCallback();
	else if (device.audio.USBAudioTransmission == AUDIOTRANSMISSION_RUNS) HalfTransfer_CallBack_FS();
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM12)
	{
		NN_Processing();
	}else if (htim->Instance == TIM11)
	{
		if (device.audioFFTProcessing == AUDIOFFTPROCESSING_ENABLE) audioProcessing_Run();
	}else if (htim->Instance == TIM10)
	{
		VisualizationModelUpdate();
	}
}










