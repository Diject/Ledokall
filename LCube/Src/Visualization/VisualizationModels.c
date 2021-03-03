
#include "Device.h"
#include "stdlib.h"
#include "math.h"
#include "AudioProcessing.h"

#include "rtc.h"
#include "I2C_Devices.h"

#define TIMEVISUALIZATION_TICKS_FOR_STEP 20U //Шагов изменения яркости
#define TIMEVISUALIZATION_TICKS (TIMEVISUALIZATION_TICKS_FOR_STEP + 4U) //Всего шагов
#define TIMEVISUALIZATION_TICKS_DIFFERENCE (TIMEVISUALIZATION_TICKS - TIMEVISUALIZATION_TICKS_FOR_STEP)
float timeVisualization_lastBrightness[4] = {0};
uint32_t timeVisualization_step = 0;
//Таблица значений яркостей для анимации
const float timeVisualization_brightnessTable[TIMEVISUALIZATION_TICKS] = {
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 0.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 1.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 2.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 3.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 4.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 5.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 6.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 7.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 8.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 9.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 10.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 11.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 12.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 13.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 14.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 15.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 16.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 17.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 18.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 19.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 20.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 21.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 22.f,
		(float)LED_MAX_BRIGHTNESS / (float)(TIMEVISUALIZATION_TICKS - 1) * 23.f,
};
/**
 * Обновление экрана времени
 */
void timeVisualizationUpdate()
{
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	hrtc.Instance->DR;
	LED_GFX_ClearModel(&LEDModel);

	//magic
	if (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP)
	{
		timeVisualization_lastBrightness[0] = LED_GFX_DrawSymbol(&LEDSymbols[time.Hours / 10], &LEDModel, COLOR_RED, 0, timeVisualization_lastBrightness[0],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 1.f);
	}else if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 2))
	{
		timeVisualization_lastBrightness[0] = LED_GFX_DrawSymbol(&LEDSymbols[time.Hours / 10], &LEDModel, COLOR_RED, 0, timeVisualization_lastBrightness[0],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 1.f);
	}else
	{
		LED_GFX_DrawSymbol(&LEDSymbols[time.Hours / 10], &LEDModel, COLOR_RED, 0, timeVisualization_lastBrightness[0], timeVisualization_lastBrightness[0], 0.f, 1.f);
	}

	if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 2) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 3))
	{
		timeVisualization_lastBrightness[1] = LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_RED, 1, timeVisualization_lastBrightness[1],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 0.66f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_BLUE, 1, timeVisualization_lastBrightness[1],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 0.33f);
	}else if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 3) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 4))
	{
		timeVisualization_lastBrightness[1] = LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_RED, 1, timeVisualization_lastBrightness[1],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 0.66f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_BLUE, 1, timeVisualization_lastBrightness[1],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 0.33f);
	}else
	{
		LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_RED, 1, timeVisualization_lastBrightness[1], timeVisualization_lastBrightness[1], 0.f, 0.66f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Hours % 10], &LEDModel, COLOR_BLUE, 1, timeVisualization_lastBrightness[1], timeVisualization_lastBrightness[1], 0.f, 0.33f);
	}

	if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 4) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 5))
	{
		timeVisualization_lastBrightness[2] = LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_RED, 2, timeVisualization_lastBrightness[2],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 0.33f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_BLUE, 2, timeVisualization_lastBrightness[2],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 0.66f);
	}else if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 5) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 6))
	{
		timeVisualization_lastBrightness[2] = LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_RED, 2, timeVisualization_lastBrightness[2],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 0.33f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_BLUE, 2, timeVisualization_lastBrightness[2],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 0.66f);
	}else
	{
		LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_RED, 2, timeVisualization_lastBrightness[2], timeVisualization_lastBrightness[2], 0.f, 0.33f);
		LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes / 10], &LEDModel, COLOR_BLUE, 2, timeVisualization_lastBrightness[2], timeVisualization_lastBrightness[2], 0.f, 0.66f);
	}

	if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 6) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 7))
	{
		timeVisualization_lastBrightness[3] = LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes % 10], &LEDModel, COLOR_BLUE, 3, timeVisualization_lastBrightness[3],
				timeVisualization_brightnessTable[timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP + TIMEVISUALIZATION_TICKS_DIFFERENCE], 0.f, 1.f);
	}else if ((timeVisualization_step >= TIMEVISUALIZATION_TICKS_FOR_STEP * 7) && (timeVisualization_step < TIMEVISUALIZATION_TICKS_FOR_STEP * 8))
	{
		timeVisualization_lastBrightness[3] = LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes % 10], &LEDModel, COLOR_BLUE, 3, timeVisualization_lastBrightness[3],
				timeVisualization_brightnessTable[(TIMEVISUALIZATION_TICKS - 1) - timeVisualization_step % TIMEVISUALIZATION_TICKS_FOR_STEP], 0.f, 1.f);
	}else
	{
		LED_GFX_DrawSymbol(&LEDSymbols[time.Minutes % 10], &LEDModel, COLOR_BLUE, 3, timeVisualization_lastBrightness[3], timeVisualization_lastBrightness[3], 0.f, 1.f);
	}

	timeVisualization_step = (timeVisualization_step > TIMEVISUALIZATION_TICKS_FOR_STEP * 8) ? 0 : timeVisualization_step + 1;
}

void timeVisualizationDeinit()
{
	for (uint32_t i = 0; i < 4; i++) timeVisualization_lastBrightness[i] = 0.f;
	timeVisualization_step = 0;
}

/**
 * Обновление экрана температуры
 */
void temperatureVisualizationUpdate()
{
	float temp = 0;
	float hum = 0;
	HDC1080_ReadData_FromLastDMATransfer(&temp, &hum);

	int32_t tempInt = temp * 10.f;
	uint32_t row = 0;
	LED_GFX_ClearModel(&LEDModel);
	if (tempInt < 0) LED_GFX_DrawSymbol(&LEDSymbols[12], &LEDModel, COLOR_RED, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	tempInt = abs(tempInt);
	if (abs(tempInt) >= 10)
	{
		LED_GFX_DrawSymbol(&LEDSymbols[tempInt / 100], &LEDModel, COLOR_RED, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	}
	LED_GFX_DrawSymbol(&LEDSymbols[(tempInt / 10) % 10], &LEDModel, COLOR_BLUE, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);

	LED_GFX_DrawSymbol(&LEDSymbols[10], &LEDModel, COLOR_BLUE, row, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 0.5f);
	LED_GFX_DrawSymbol(&LEDSymbols[10], &LEDModel, COLOR_RED, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 0.5f);

	if (row <= 3)
	{
		LED_GFX_DrawSymbol(&LEDSymbols[tempInt % 10], &LEDModel, COLOR_RED, row, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	}
}

/**
 * Обновление экрана влажности
 */
void humidityVisualizationUpdate()
{
	float temp = 0;
	float hum = 0;
	HDC1080_ReadData_FromLastDMATransfer(&temp, &hum);

	uint32_t humInt = hum * 1000.f;
	uint32_t row = 0;
	LED_GFX_ClearModel(&LEDModel);
	LED_GFX_DrawSymbol(&LEDSymbols[humInt / 100], &LEDModel, COLOR_RED, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);

	LED_GFX_DrawSymbol(&LEDSymbols[(humInt / 10) % 10], &LEDModel, COLOR_BLUE, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);

	LED_GFX_DrawSymbol(&LEDSymbols[11], &LEDModel, COLOR_BLUE, row, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 0.5f);
	LED_GFX_DrawSymbol(&LEDSymbols[11], &LEDModel, COLOR_RED, row++, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 0.5f);

	if (row <= 3)
	{
		LED_GFX_DrawSymbol(&LEDSymbols[humInt % 10], &LEDModel, COLOR_RED, row, LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0.f, 1.f);
	}
}






/*
 * Ограничение частот для подсчета высоты колонки
 */
const float audioVisualization_stepLimit[16] = {
		100.f, 200.f, 300.f, 400.f,
		550.f, 700.f, 850.f, 1000.f,
		1150.f, 1300.f, 1500.f, 1800.f,
		2500.f, 3500.f, 5500.f, 12000.f,
};
/*
 * Множитель для подсчета высоты колонки
 */
const float audioVisualization_stepMul[16] = {
		0.1f, 0.15f, 0.2f, 0.3f,
		0.3f, 0.35f, 0.45f, 0.55f,
		0.65f, 0.75f, 0.85f, 0.95f,
		1.f, 1.f, 1.f, 1.5f,
};

//Коэффициенты для определения громкости
const float audioVisualization_loudnessMul[16] = {
		0.08f, 0.28f, 0.5f, 0.67f,
		0.84f, 1.f, 1.f, 1.f,
		0.95f, 0.89f, 0.79f, 0.9f,
		1.41f, 1.75f, 1.9f, 1.f,
};

LEDRowsVisualizationFrame lastRVFrame = {0}; //Предыдущие значения высоты колонок
float audioVisualization_fftAvg[16] = {0}; //Среднее значение колонок для вычисления текущего фрейма
float audioVisualization_fftMax[16] = {0}; //Максимальное значение колонок для вычисления текущего фрейма
float audioVisualization_maxPow = 0.f; //Максимальная громкость для текущего фрейма
float audioVisualization_pow = 0.f; //Максимальная громкость для текущего фрейма

/**
 * Обновляет данные для вычисления значений анимации спектрограммы
 * *fft_res - результат fft
 * fft_freq_step - частота для одного отрезка fft
 */
void audioVisualizationDataUpdate(float *fft_res, float fft_freq_step)
{
	uint32_t pos = 0;
	float resM[16] = {0.f};
	float sum = 0.f;
	float vol = 0.f;
	float *data = fft_res;
	uint32_t aId = 0;
	uint32_t num = 0;
	//Подсчет суммы значений fft для каждого периода
	do
	{
		if (++aId >= AUDIO_PROCESSING_FFT_SIZE / 2)
		{
			if (num != 0)
			{
				resM[pos] = sum / (float)num * audioVisualization_stepMul[pos];
				vol += resM[pos] * audioVisualization_loudnessMul[pos];
			}
			break;
		}
		if ((fft_freq_step * aId) < audioVisualization_stepLimit[pos])
		{
			num++;
			sum += data[aId];
		}else
		{
			if (num != 0)
			{
				resM[pos] = sum / (float)num * audioVisualization_stepMul[pos];
				vol += resM[pos] * audioVisualization_loudnessMul[pos];
			}
			pos++;
			num = 1;
			sum = data[aId];
		}
	} while (pos < 16);

	float max = 0;
	arm_max_f32(resM, 16, &max, NULL);
	//Проверка на NAN
	if (!(max > 0.f))
	{
		for (uint32_t i = 0; i < 16; i++)
		{
			audioVisualization_fftAvg[i] = 0.f;
			audioVisualization_fftMax[i] = 0.f;
		}
		return;
	}


	float mul;
	if (audioVisualization_pow != 0) mul = (1.f / max) * (vol / audioVisualization_pow);
	else mul = (1.f / max);
	//Вычисление среднего и максимального значения для полосы с нормализаацией
	for (uint32_t i = 0; i < 16; i++)
	{
		float res = (resM[i] * mul);
		audioVisualization_fftAvg[i] = res;
		audioVisualization_fftMax[i] = fmaxf(audioVisualization_fftMax[i], res);
	}

	//Нахождение максимума громкости от предыдущего и текущего значения
	audioVisualization_pow = fmaxf(audioVisualization_pow, vol);
}

/**
 * Обновляет анимацию спектрограммы
 */
void audioVisualizationUpdate()
{
	LEDRowsVisualizationFrame visFrame = {0};

	//Подстройка максимума громкости для визуализации
	static uint32_t max_detection_tick = 0;
	static float last_max_pow = 0.f;
	static float last_min_pow = INFINITY;
	//Если AUDIO_VISUALIZATION_MAX_VOLUME_RESET_PERIOD тиков максимальное значение не менялось, то начинается подстройка.
	//Если значение громкости не отличается больше 40%, то подстройка не производится
	if ((get_device_counter() - max_detection_tick) > AUDIO_VISUALIZATION_MAX_VOLUME_RESET_PERIOD)
	{
		if (audioVisualization_pow > last_max_pow) last_max_pow = audioVisualization_pow;
		if (audioVisualization_pow < last_min_pow) last_min_pow = audioVisualization_pow;
		if (last_max_pow != last_min_pow && last_min_pow != 0.f)
		{
			if (((last_max_pow - last_min_pow) / last_min_pow) > 0.4f)
			{
				last_max_pow = 0.f;
				last_min_pow = INFINITY;
				if ((audioVisualization_maxPow > 0.f))
				{
					audioVisualization_maxPow *= 0.99f; //Уменьшение макс громкости
					if (audioVisualization_maxPow <= 0.f) audioVisualization_maxPow = 0.f;
				}

			}
		}
	}
	//Вычсление максимума громкости
	if (audioVisualization_pow > audioVisualization_maxPow)
	{
		audioVisualization_maxPow = audioVisualization_pow;
		last_max_pow = 0.f;
		last_min_pow = INFINITY;
		max_detection_tick = get_device_counter();
	}
	//Вычисление такущего фрейма анимации
	float powMul = (audioVisualization_maxPow > 0.f) ? audioVisualization_pow / audioVisualization_maxPow : 0.f;
	audioVisualization_pow = 0.f;
	if (!(powMul > 0.f)) powMul = 0.f;
	uint32_t aId = 0;
	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			//Смешивание макс и средней громкости
			float val = (audioVisualization_fftMax[aId] - (audioVisualization_fftMax[aId] - audioVisualization_fftAvg[aId]) * 0.3f) * powMul;
			if (val < 0.f) val = 0.f;
			//Высота столбца
			uint16_t hval = (uint32_t)((float)LED_ROWS_VISUALIZATION_MAX_VALUE * powf(val, 0.7f));
			if (hval > LED_ROWS_VISUALIZATION_MAX_VALUE) hval = LED_ROWS_VISUALIZATION_MAX_VALUE;
			visFrame.rowHeight[j * 4 + i] = hval;
			audioVisualization_fftMax[aId] = audioVisualization_fftAvg[aId];
			aId++;
			visFrame.rowGrowthSpeed[j * 4 + i] = (abs(visFrame.rowHeight[j * 4 + i] - lastRVFrame.rowHeight[j * 4 + i])) *
					(1.f / (float)LED_ROWS_VISUALIZATION_MAX_VALUE) *
					((float)LED_ROWS_VISUALIZATION_MAX_VALUE / (float)LED_DMA_BUFFER_HALFSIZE_STEPS * 0.1f);
		}
	}
	//Обновление модели
	LED_GFX_ClearModel(&LEDModel);
	LED_GFX_DrawRowVisualization(&lastRVFrame, &visFrame, &LEDModel, 3.0f);
}

/**
 * Сброс значений для анимации спектрограммы в дефолтные значения
 */
void audioVisualizationDeinit()
{
	audioVisualization_pow = 0.f;
	audioVisualization_maxPow = 0.f;
	for (uint32_t i = 0; i < 16; i++)
	{
		lastRVFrame.rowHeight[i] = 0;
		audioVisualization_fftAvg[i] = 0.f;
		audioVisualization_fftMax[i] = 0.f;
	}
}






