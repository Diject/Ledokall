
#include "LEDVisualization_GFX.h"

uint8_t brightnessCode[LED_MAX_BRIGHTNESS + 2] = {0}; //Коды яркостей

/**
 * Значение яркости для кода (val)
 */
static float LED_GFX_BitSumm(int32_t val, float *weights, uint32_t bitCount)
{
	float res = 0;
	for (uint32_t i = 0; i < bitCount; i++)
	{
		if (val & (1 << i)) res += weights[i];
	}
	return res;
}
/**
 * Иницавлизация массива значений яркости светодиода
 */
void LED_GFX_Init()
{
//Действительно только при LED_MODEL_STEP_SIZE == 8
#if LED_MODEL_STEP_SIZE != 8
#error
#endif
	float stepV[LED_MODEL_STEP_SIZE] = {0}; //Размер шага
	float parts = 0; //Сумма шагов в фрейме
	for (uint32_t i = 0; i < LED_MODEL_STEP_SIZE; i++)
	{
		stepV[i] = (i + 1);
		parts += stepV[i];
	}
	//Вычисление таблицы яркостей перебором
	//Сначала вычисляется требуемое значение, потом перебором ищется код для него
	uint32_t startLabel = 0;
	for (uint32_t i = 0; i <= LED_MAX_BRIGHTNESS; i++)
	{
		float cval = (float)i * 1.f / (float)LED_MAX_BRIGHTNESS * parts + 0.5f; //Целевое значение яркости
		for (uint32_t j = startLabel; j < (1 << LED_MODEL_STEP_SIZE); j++)
		{
			float bval = LED_GFX_BitSumm(j, stepV, LED_MODEL_STEP_SIZE); //Значение якрости для (j) кода
			if (((uint32_t)cval) == ((uint32_t)(bval + 0.5f)))
			{
				brightnessCode[i] = j;
				startLabel = j;
				break;
			}
		}
	}
	brightnessCode[LED_MAX_BRIGHTNESS + 1] = brightnessCode[LED_MAX_BRIGHTNESS]; //Костыль для исключения ошибок
}

inline uint8_t LED_GFX_GetCode(uint8_t brightnessValue)
{
	return brightnessCode[brightnessValue];
}

/**
 * Инициализация модели
 * *model - указатель на модель
 * Массивы размером (stepNumber * LED_MODEL_STEP_SIZE * LED_MODEL_FRAME_SIZE):
 * *rowRegPointer_red - указатель на массив с кодами красных светодиодов
 * *rowRegPointer_blue - указатель на массив с кодами синих светодиодов
 * *floorRegPoiter_red - указатель на массив с данными номера строки для красных светодиодов
 * *floorRegPoiter_blue - указатель на массив с данными номера строки для синих светодиодов
 * Массивы размером (LED_MODEL_STEP_SIZE * LED_MODEL_FRAME_SIZE):
 * *counterRegPounter - указатель на массив со значениями счетчика для переключения красный/синий
 *
 * stepNumber - количество шагов в модели
 */
void LED_GFX_InitModel_2Color(LEDDataModel *model, uint16_t *rowRegPointer_red, uint16_t *rowRegPointer_blue,
		uint8_t *floorRegPoiter_red, uint8_t *floorRegPoiter_blue, uint16_t *counterRegPounter, uint16_t stepNumber)
{
	uint16_t numberOfFrames = stepNumber * LED_MODEL_STEP_SIZE;
	model->counterRegIndex = 0;
	model->floorRegIndex_first = 0;
	model->floorRegIndex_second = 0;
	model->rowRegIndex_first = 0;
	model->rowRegIndex_second = 0;
	model->counterRegArray = counterRegPounter;
	model->floorRegArray_first = floorRegPoiter_blue;
	model->floorRegArray_second = floorRegPoiter_red;
	model->rowRegArray_first = rowRegPointer_blue;
	model->rowRegArray_second = rowRegPointer_red;
	model->regFrameQuantity = numberOfFrames;
	model->stepQuantity = stepNumber;
	model->counterRegQuantity = LED_MODEL_STEP_SIZE * LED_MODEL_FRAME_SIZE;
	model->regQuantity = numberOfFrames * LED_MODEL_FRAME_SIZE;
	*(uint32_t *)&model->loaded = MODEL_FULLYLOADED;
	model->busy = 0;
	//Дефолтные значения
	for (uint32_t i = 0; i < model->regQuantity; i++)
	{
		model->rowRegArray_first[i] = 0xffff;
		model->rowRegArray_second[i] = 0;
	}
	//Значения уровней для мультиплексора =
	uint8_t *reg1 = model->floorRegArray_first;
	uint8_t *reg2 = model->floorRegArray_second;
	for (uint32_t i = 0; i < numberOfFrames; i++)
	{
		for (uint32_t j = 0; j < LED_MODEL_FRAME_SIZE; j++)
		{
			*reg1++ = (1 << 7) | (1 << 3) | j;
			*reg2++ = j;
		}
	}

	//Значения счетчика PWM
	uint16_t *counterReg = model->counterRegArray;
	float brightnessStep = (float)(LED_COUNTER_REG_MAX_VALUE) / (float)(LED_MODEL_STEP_SIZE + 1);
	float stepV = 0.f;
	for (uint32_t i = 1; i <= LED_MODEL_STEP_SIZE; i++)
	{
		stepV = i * (brightnessStep + (float)LED_COUNTER_CALIBRATION_SHIFT *
				((float)(LED_MODEL_STEP_SIZE - i + 1) / (float)LED_MODEL_STEP_SIZE));

		for (uint32_t j = 0; j < LED_MODEL_FRAME_SIZE; j++)
		{
			*counterReg++ = (uint16_t)(stepV);
		}
	}
	//Сдвиг для синхронизации
	for (uint32_t i = 0; i < model->counterRegQuantity - 2; i++)
	{
		model->counterRegArray[i] = model->counterRegArray[i + 2];
	}
	model->counterRegArray[model->counterRegQuantity - 2] = model->counterRegArray[0];
	model->counterRegArray[model->counterRegQuantity - 1] = model->counterRegArray[0];
}

/**
 * Очистка данных визуализации
 */
void LED_GFX_ClearModel(LEDDataModel *model)
{
	model->busy = 1;
	for (uint32_t i = 0; i < model->regQuantity; i++)
	{
		model->rowRegArray_first[i] = 0xffff;
		model->rowRegArray_second[i] = 0;
		model->floorRegArray_second[i] &= ~(1 << 7);
		model->floorRegArray_first[i] |= (1 << 7);
	}
	model->busy = 0;
}

/**
 * Визуализация аудио
 * Если значение скорости роста в фрейме равно 0, то оно будет вычислено автоматически
 * *lastFrame - значения прошлого фрейма. В конце будет заполнено текущими значениями
 * *currentFrame - новые значения фрейма
 * *targetModel - модель для заполнения
 * growthRate - множитель для роста столбца в положительную сторону
 */
void LED_GFX_DrawRowVisualization(LEDRowsVisualizationFrame *lastFrame, LEDRowsVisualizationFrame *currentFrame,
		LEDDataModel *targetModel, float growthRate)
{
	float growthSpeed[16];

	//Prepare column growth rate data
	for (uint32_t i = 0; i < 16; i++)
	{
		if (currentFrame->rowGrowthSpeed[i] != 0)
		{
			if (lastFrame->rowHeight[i] > currentFrame->rowHeight[i])
			{
				growthSpeed[i] = -1 * currentFrame->rowGrowthSpeed[i];
			}else growthSpeed[i] = currentFrame->rowGrowthSpeed[i] * growthRate;
		}else
		{
			growthSpeed[i] = (float)(currentFrame->rowHeight[i] - lastFrame->rowHeight[i]) / (float)targetModel->stepQuantity;
		}
	}

	targetModel->busy = 1;
	for (uint32_t ledId = 0; ledId < 16; ledId++) //для каждого столбца
	{
		uint8_t *regB; //led data register
		uint8_t *regR; //led data register
		uint32_t regSize; //led data register size
		uint32_t shift; //led data bit shift

		//Unite led registers
		if (ledId != 11)
		{
			regB = (uint8_t *)targetModel->rowRegArray_first + (ledId / 8);
			regR = (uint8_t *)targetModel->rowRegArray_second + (ledId / 8);
			regSize = sizeof(uint16_t);
			shift = ledId % 8;
		}else
		{
			regB = (uint8_t *)targetModel->floorRegArray_first;
			regR = (uint8_t *)targetModel->floorRegArray_second;
			regSize = sizeof(uint8_t);
			shift = 7;
		}

		//Model filling
		int32_t height = 0;
		for (uint32_t mstep = 0; mstep < targetModel->stepQuantity; mstep++)
		{
			//Высота столбца
			height = (int32_t)((float)lastFrame->rowHeight[ledId] + growthSpeed[ledId] * (mstep + 1));

			//Overflow check
			if ((height < currentFrame->rowHeight[ledId]) && (growthSpeed[ledId] <= 0.f)) height = currentFrame->rowHeight[ledId];
			if ((height > currentFrame->rowHeight[ledId]) && (growthSpeed[ledId] >= 0.f)) height = currentFrame->rowHeight[ledId];


			uint32_t regId = regSize * mstep * LED_MODEL_FRAME_SIZE * LED_MODEL_STEP_SIZE;
			for (uint32_t stepFrame = 0; stepFrame < LED_MODEL_STEP_SIZE; stepFrame++)
			{
				uint32_t regIdBk = regId;
				uint32_t h = 0;
				uint32_t heightFull = (height / (LED_MAX_BRIGHTNESS + 1));
				uint32_t hh = (heightFull > 3) ? (heightFull - 3) / 2 : 0;
				if (hh <= 1) hh = 0;
				//Заливка синим
				for (; h < hh; h++)
				{
					regB[regId] &= ~(1 << shift);
					regId += regSize;
				}
				uint32_t colChV = (LED_MAX_BRIGHTNESS + 1) / (heightFull - hh + 1);
				//Заливка основной части
				for (; h < heightFull; h++)
				{
					uint32_t stepDescrBlue = LED_GFX_GetCode(colChV * (heightFull - h));
					uint32_t stepDescrRed = LED_GFX_GetCode(colChV * (h - hh + 1));
					if (stepDescrRed & (1 << stepFrame)) regR[regId] |= (1 << shift); // COLOR_RED
					if (stepDescrBlue & ((1 << (LED_MODEL_STEP_SIZE - 1)) >> stepFrame)) regB[regId] &= ~(1 << shift); // COLOR_BLUE
					regId += regSize;
				}
				//Заливка последнего горящего диода
				uint32_t heightPart = height % (LED_MAX_BRIGHTNESS + 1);
				if (heightPart)
				{
					if (LED_GFX_GetCode(heightPart) & (1 << stepFrame)) regR[regId] |= (1 << shift); // COLOR_RED
					regId += regSize;
					h++;
				}
				regId = regIdBk + regSize * LED_MODEL_FRAME_SIZE;
			}
		}
		//
		lastFrame->rowHeight[ledId] = (uint16_t)height;
	}
	targetModel->busy = 0;
}

/**
 * *symbol - drawing symbol data pointer
 * *targetModel - visualization model for drawing
 * color - LED color
 * line - position for symbol. (0,1,2,3) LED id for line 0, (4,5,6,7) LED id for line 1, etc.
 * startBrightness - начальная яркость. Макс = LED_MAX_BRIGHTNESS
 * endBrightness - конечная яркость. Макс = LED_MAX_BRIGHTNESS
 * growthSpeed - скорость изменения якрости за один шаг модели. Если == 0, то вычисляется автоматически
 * brightnessMul = множитель яркости. (<= 1.f)
 * return - конечное значение яркости
 */
float LED_GFX_DrawSymbol(const LEDFlatSymbol *symbol, LEDDataModel *targetModel, LEDColor color, uint32_t line, float startBrightness, float endBrightness,
		float growthSpeed, float brightnessMul)
{
	//Fill register buffer
	uint16_t reg1[LED_MODEL_FRAME_SIZE];
	uint8_t reg2[LED_MODEL_FRAME_SIZE];
	for (uint32_t i = 0; i < LED_MODEL_FRAME_SIZE; i++)
	{
		reg1[i] = symbol->dataArray[i] << (line * 4);
		reg2[i] = (reg1[i] >> 4) & 0x80;
	}

	//Overflow check
	if (endBrightness > LED_MAX_BRIGHTNESS) endBrightness = LED_MAX_BRIGHTNESS;
	if (startBrightness > LED_MAX_BRIGHTNESS) startBrightness = LED_MAX_BRIGHTNESS;

	if (growthSpeed == 0.f) growthSpeed = (endBrightness - startBrightness) / (float)targetModel->stepQuantity;

	//Fill the model
	float brightness = startBrightness;
	float brightnessDescr = 0;
	targetModel->busy = 1;
	if (color == COLOR_RED)
	{
		uint32_t regId = 0;
		for (uint32_t step = 0; step < targetModel->stepQuantity; step++)
		{
			brightness += growthSpeed;
			brightnessDescr = brightness;

			//Overflow check
			if (growthSpeed >= 0.f)
			{
				if (brightnessDescr > endBrightness) brightnessDescr = endBrightness;
			}else
			{
				if (brightnessDescr < endBrightness) brightnessDescr = endBrightness;
			}

			uint32_t cdescr = LED_GFX_GetCode((uint8_t)(brightnessDescr * brightnessMul));

			for (uint32_t frame = 0; frame < LED_MODEL_STEP_SIZE; frame++)
			{
				for (uint32_t floor = 0; floor < LED_MODEL_FRAME_SIZE; floor++)
				{
					if (cdescr & (1 << frame))
					{
						targetModel->rowRegArray_second[regId] |= reg1[floor];
						targetModel->floorRegArray_second[regId] |= reg2[floor];
					}
					regId++;
				}
			}
		}
	}else if (color == COLOR_BLUE)
	{
		uint32_t regId = 0;
		for (uint32_t step = 0; step < targetModel->stepQuantity; step++)
		{
			brightness += growthSpeed;
			brightnessDescr = brightness;

			//Overflow check
			if (growthSpeed >= 0.f)
			{
				if (brightnessDescr > endBrightness) brightnessDescr = endBrightness;
			}else
			{
				if (brightnessDescr < endBrightness) brightnessDescr = endBrightness;
			}

			uint32_t cdescr = LED_GFX_GetCode((uint8_t)(brightnessDescr * brightnessMul));

			for (uint32_t frame = 0; frame < LED_MODEL_STEP_SIZE; frame++)
			{
				for (uint32_t floor = 0; floor < LED_MODEL_FRAME_SIZE; floor++)
				{
					if (cdescr & ((1 << (LED_MODEL_STEP_SIZE - 1)) >> frame))
					{
						targetModel->rowRegArray_first[regId] &= ~reg1[floor];
						targetModel->floorRegArray_first[regId] &= ~reg2[floor];
					}
					regId++;
				}
			}
		}
	}
	targetModel->busy = 0;
	return brightness;
}

uint32_t LED_GFX_DrawPixel(LEDDataModel *targetModel, uint8_t x, uint8_t y, LEDColor color, uint32_t startBrightness, uint32_t endBrightness,
		float growthSpeed, float brightnessMul)
{

}
