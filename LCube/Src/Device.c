
#include "Device.h"

#include "AudioProcessing.h"

deviceState device = {0}; //Состояние устройства

uint16_t rowRegArray_r[LED_DMA_BUFFER_HALFSIZE] = {0};
uint16_t rowRegArray_b[LED_DMA_BUFFER_HALFSIZE] = {0};
uint8_t floorRegArray_r[LED_DMA_BUFFER_HALFSIZE] = {0};
uint8_t floorRegArray_b[LED_DMA_BUFFER_HALFSIZE] = {0};
uint16_t counterRegArray[LED_MODEL_STEP_SIZE * LED_MODEL_FRAME_SIZE] = {0};
LEDDataModel LEDModel = {0}; //Модель для всех анимаций

uint32_t pause_timeout_tick = 0;
uint32_t pause_timeout_interval = 0;

void InitLEDModel()
{
	LED_GFX_InitModel_2Color(&LEDModel, rowRegArray_r, rowRegArray_b, floorRegArray_r, floorRegArray_b, counterRegArray, LED_DMA_BUFFER_HALFSIZE_STEPS);
}

/**
 * Начальная инициализация
 */
void InitDevice()
{
	//Определение usb входа
	if (HAL_GPIO_ReadPin(USBC_Vbus_GPIO_Port, USBC_Vbus_Pin))
	{
		device.voltageInput |= VOLTAGE_INPUT_USBC;
	}
	if (HAL_GPIO_ReadPin(MUSB_ID_GPIO_Port, MUSB_ID_Pin))
	{
		device.voltageInput |= VOLTAGE_INPUT_MICROUSB;
	}

	if (HAL_GPIO_ReadPin(USBC_Vbus_GPIO_Port, USBC_Vbus_Pin))
	{
		device.voltageInput |= VOLTAGE_INPUT_USBC;
	}else device.voltageInput &= ~VOLTAGE_INPUT_USBC;

	if (HAL_GPIO_ReadPin(MUSB_ID_GPIO_Port, MUSB_ID_Pin))
	{
		device.voltageInput |= VOLTAGE_INPUT_MICROUSB;
	}else device.voltageInput &= ~VOLTAGE_INPUT_MICROUSB;

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

	//Загрузка настроек
	 HAL_PWR_EnableBkUpAccess();

	 device.audio.muteAudio = HAL_RTCEx_BKUPRead(&hrtc, DEVICE_SAVE_AUDIO_MUTE_ID);
	 device.audio.muteVoice = HAL_RTCEx_BKUPRead(&hrtc, DEVICE_SAVE_MESSAGE_MUTE_ID);

	 uint32_t dummy = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
	 if (dummy == 0)
	 {
		 device.audio.messageVolume = 0.15f;
		 HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_MESSAGE_VOLUME_ID, *(uint32_t *)&device.audio.messageVolume);
	 }else
	 {
		 device.audio.messageVolume = *(float *)&dummy;
	 }

	//Инициализация управления светодиодами
	LED_Stop();
	LED_Init();
	LED_Start();
	LED_GFX_Init();
	device.visualizationStatus = ENABLE;

	InitLEDModel();
	LED_LoadModel(&LEDModel);

	dummy = HAL_RTCEx_BKUPRead(&hrtc, DEVICE_SAVE_VISUALIZATION_ID);
	if (dummy == 0)
	{
		InitLEDVisualization(VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS);
		Device_Save_Visualization();
	}else
	{
		InitLEDVisualization(dummy);
	}

	device.neuralNetworkStatus = NEURAL_NETWORK_ENABLE;
}

void DeinitLEDVisualization(visualizationType vis)
{
	LED_GFX_ClearModel(&LEDModel);
	switch (vis)
	{
	case VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS:
		audioVisualizationDeinit();
		audioProcessing_ClearBuffer();
		break;

	case VISUALIZATION_STATE_USB_AUDIO_VIS:
		audioVisualizationDeinit();
		audioProcessing_ClearBuffer();
		break;

	case VISUALIZATION_STATE_TIME_VIS:
		timeVisualizationDeinit();
		break;

	case VISUALIZATION_STATE_TEMPERATURE_VIS:
		break;

	case VISUALIZATION_STATE_HUMIDITY_VIS:
		break;

	case VISUALIZATION_STATE_CUSTOM_VIS:
		break;

	default:
		break;
	}
}

void InitLEDVisualization(visualizationType newVis)
{
	if (newVis != device.visualization)
	{
		DeinitLEDVisualization(device.visualization);
		switch (newVis)
		{
		case VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS:
			device.audio.frequency = 16000;
			device.audio.audioInput = AUDIO_INPUT_MICROPHONE;
			break;

		case VISUALIZATION_STATE_USB_AUDIO_VIS:
			device.audio.frequency = 44100;
			device.audio.audioInput = AUDIO_INPUT_USB;
			break;

		case VISUALIZATION_STATE_TIME_VIS:
			break;

		case VISUALIZATION_STATE_TEMPERATURE_VIS:
			break;

		case VISUALIZATION_STATE_HUMIDITY_VIS:
			break;

		case VISUALIZATION_STATE_CUSTOM_VIS:
			break;

		default:
			break;
		}
	}
	device.visualization = newVis;
	Device_Save_Visualization();
}



void VisualizationModelUpdate()
{
	if (*(uint32_t *)&LEDModel.loaded == MODEL_FULLYLOADED)
	{
		if ((get_device_counter() - pause_timeout_tick) > pause_timeout_interval)
		{
			pause_timeout_interval = 0;
			switch (device.visualization)
			{
			case VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS:
				audioVisualizationUpdate();
				break;

			case VISUALIZATION_STATE_USB_AUDIO_VIS:
				audioVisualizationUpdate();
				break;

			case VISUALIZATION_STATE_TIME_VIS:
				timeVisualizationUpdate();
				break;

			case VISUALIZATION_STATE_TEMPERATURE_VIS:
				temperatureVisualizationUpdate();
				break;

			case VISUALIZATION_STATE_HUMIDITY_VIS:
				humidityVisualizationUpdate();
				break;

			case VISUALIZATION_STATE_CUSTOM_VIS:
				break;

			default:
				break;
			}
			*(uint32_t *)&LEDModel.loaded = 0;
		}
	}
}


/**
 * Остановить обновление анимации модели на timeout_ms сек.
 */
void VisualizationPause(uint32_t timeout_ms)
{
	pause_timeout_tick = get_device_counter();
	pause_timeout_interval = timeout_ms;
}

/**
 * Включить/Отключить светодиоды
 */
void VisualizationStop(FunctionalState state)
{
	device.visualizationStatus = state;
	if (state)
	{
		LED_Start();
	}else
	{
		LED_Pause();
	}
}

void Device_Save_Mute_State()
{
	HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_AUDIO_MUTE_ID, device.audio.muteAudio);
	HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_MESSAGE_MUTE_ID, device.audio.muteVoice);
	HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_MICROPHOME_MUTE_ID, device.audio.muteMicrophoneOutput);
}

void Device_Save_Message_Volume()
{
	float val = device.audio.messageVolume;
	HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_MESSAGE_VOLUME_ID, *(uint32_t *)&val);
}

void Device_Save_Visualization()
{
	HAL_RTCEx_BKUPWrite(&hrtc, DEVICE_SAVE_VISUALIZATION_ID, device.visualization);
}


//***************************************************************
//NN
//Команды нейросети
//***************************************************************

void NN_Action_ShowTimeScreen(uint32_t nn_command)
{
	InitLEDVisualization(VISUALIZATION_STATE_TIME_VIS);
}

void NN_Action_ShowMicSpectrogramScreen(uint32_t nn_command)
{
	if (device.visualization != VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS)
	{
		InitLEDVisualization(VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS);
	}else
	{
		InitLEDVisualization(VISUALIZATION_STATE_USB_AUDIO_VIS);
	}
}

void NN_Action_ShowTemperatureScreen(uint32_t nn_command)
{
	InitLEDVisualization(VISUALIZATION_STATE_TEMPERATURE_VIS);
}

void NN_Action_ShowHumidityScreen(uint32_t nn_command)
{
	InitLEDVisualization(VISUALIZATION_STATE_HUMIDITY_VIS);
}

void NN_Action_SayTime(uint32_t nn_command)
{
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	hrtc.Instance->DR;
	wavePlayer_PlayTime(time.Hours, time.Minutes);
}

void NN_Action_SayTemperature(uint32_t nn_command)
{
	float temp = 0;
	float hum = 0;
	HDC1080_ReadData_FromLastDMATransfer(&temp, &hum);
	wavePlayer_PlayTemperature(temp);
}

void NN_Action_SayHumidity(uint32_t nn_command)
{
	float temp = 0;
	float hum = 0;
	HDC1080_ReadData_FromLastDMATransfer(&temp, &hum);
	wavePlayer_PlayHumidity(hum);
}

void NN_Action_MuteSound(uint32_t nn_command)
{
	device.audio.muteAudio = ENABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
//	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_UnMuteSound(uint32_t nn_command)
{
	device.audio.muteAudio = DISABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
//	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_MuteMicrophone(uint32_t nn_command)
{
	device.audio.muteMicrophoneOutput = ENABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
//	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_UnMuteMicrophone(uint32_t nn_command)
{
	device.audio.muteMicrophoneOutput = DISABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
//	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[17], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_MuteCommands(uint32_t nn_command)
{
	device.audio.muteVoice = ENABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
//	LED_GFX_DrawSymbol(&LEDSymbols[13], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[18], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[18], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_UnMuteCommands(uint32_t nn_command)
{
	device.audio.muteVoice = DISABLE;
	Device_Save_Mute_State();
	VisualizationPause(2000);
	LED_GFX_ClearModel(&LEDModel);
//	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_RED, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[14], &LEDModel, COLOR_BLUE, 1, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[18], &LEDModel, COLOR_RED, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	LED_GFX_DrawSymbol(&LEDSymbols[18], &LEDModel, COLOR_BLUE, 2, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
}

void NN_Action_Joke(uint32_t nn_command)
{
	wavePlayer_PlayJoke();
}






