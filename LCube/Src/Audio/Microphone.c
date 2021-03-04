
#include "Microphone.h"
#include "arm_math.h"

uint16_t microphone_data[MICROPHONE_CAPTURE_DATA_SIZE];
int16_t microphone_audio[MICROPHONE_OUT_DATA_SIZE];
uint8_t amplifier_code = MICROPHONE_AMPLIFIER_DEFAULT_CODE;
uint32_t clipped_samples = 0;
uint32_t clipped_tick = 0;

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
	clipped_samples++;
	clipped_tick = get_device_counter();
}

void Microphone_Clipping_Check()
{
	if (clipped_samples >= MICROPHONE_CLIPPED_FRAMES_LIMIT)
	{
		if  (I2C_Ready_State() == HAL_I2C_STATE_READY)
		{
			if (amplifier_code > 1)
			{
				if (MCP40D17_SetResistanceCode_DMA(amplifier_code - 1) == HAL_OK) amplifier_code--;
			}
		}
	}else if (amplifier_code < MICROPHONE_AMPLIFIER_DEFAULT_CODE)
	{
		if ((get_device_counter() - clipped_tick) > MICROPHONE_UNCLIPPED_TIME_LIMIT)
		{
			if  (I2C_Ready_State() == HAL_I2C_STATE_READY)
			{
				if (MCP40D17_SetResistanceCode_DMA(amplifier_code + 1) == HAL_OK)
				{
					amplifier_code++;
					clipped_tick = get_device_counter();
				}
			}
		}
	}

	clipped_samples = 0;
}

/**
 * Converts ADC microphone data to signed values & filter them
 */
void prepare_mic_data(uint16_t *in, int16_t *out, uint32_t size)
{
	while (size-- != 0)
	{
		int32_t sum;
		int32_t res = (int32_t)*(in++) - MICROPHONE_CAPTURE_ADC_CENTER;
		sum = IIR_Calc(res);
		res = (int32_t)*(in++) - MICROPHONE_CAPTURE_ADC_CENTER;
		sum += IIR_Calc(res);
		sum /= 2;
		if (sum < -0x7fff) sum = -0x7fff;
		else if (sum > 0x7fff) sum = 0x7fff;
		*(out++) = (int16_t)sum;
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
//	Microphone_Clipping_Check();
	prepare_mic_data(&microphone_data[0], microphone_audio, MICROPHONE_OUT_DATA_SIZE);
	USBD_MICROPHONE_AddData(microphone_audio, MICROPHONE_OUT_DATA_SIZE);
	audioProcessing_AddData(microphone_audio, MICROPHONE_OUT_DATA_SIZE, AUDIO_INPUT_MICROPHONE);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	Microphone_Clipping_Check();
	prepare_mic_data(&microphone_data[MICROPHONE_CAPTURE_DATA_HALFSIZE], microphone_audio, MICROPHONE_OUT_DATA_SIZE);
	USBD_MICROPHONE_AddData(microphone_audio, MICROPHONE_OUT_DATA_SIZE);
	audioProcessing_AddData(microphone_audio, MICROPHONE_OUT_DATA_SIZE, AUDIO_INPUT_MICROPHONE);
}

inline int16_t* Microphone_Data()
{
	return microphone_audio;
}

/**
 * Start microphone audio capture
 */
void Microphone_Start_Capture()
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)microphone_data, MICROPHONE_CAPTURE_DATA_SIZE);
	HAL_TIM_Base_Start(&htim3);
}

/**
 * Stop microphone audio capture
 */
void Microphone_Stop_Capture()
{
	HAL_TIM_Base_Stop(&htim3);
	HAL_ADC_Stop_DMA(&hadc1);
}

/**
 * Microphone capture sys init
 */
void Microphone_Init()
{
	amplifier_code = MICROPHONE_AMPLIFIER_DEFAULT_CODE;
	MCP40D17_SetResistanceCode(amplifier_code);
}



