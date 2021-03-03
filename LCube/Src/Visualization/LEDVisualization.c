
#include "LEDVisualization.h"

extern DMA_HandleTypeDef hdma_tim1_ch1;
extern DMA_HandleTypeDef hdma_tim1_ch2;
extern DMA_HandleTypeDef hdma_tim1_ch3;
extern DMA_HandleTypeDef hdma_tim1_ch4_trig_com;
extern DMA_HandleTypeDef hdma_tim1_up;

LEDDMADataStruct LEDData = {0};
uint16_t defaulRowRegArray_f[LED_MODEL_FRAME_SIZE] = {0};
uint16_t defaulRowRegArray_s[LED_MODEL_FRAME_SIZE] = {0};
uint8_t defaulfloorRegArray_f[LED_MODEL_FRAME_SIZE] = {0};
uint8_t defaulfloorRegArray_s[LED_MODEL_FRAME_SIZE] = {0};
uint16_t defaulCounterRegArray[LED_MODEL_FRAME_SIZE] = {0};
LEDDataModel defaultModel =
{
	.rowRegArray_first = defaulRowRegArray_f,
	.rowRegArray_second = defaulRowRegArray_s,
	.rowRegIndex_first = 0,
	.rowRegIndex_second = 0,
	.floorRegArray_first = defaulfloorRegArray_f,
	.floorRegArray_second = defaulfloorRegArray_s,
	.floorRegIndex_first = 0,
	.floorRegIndex_second = 0,
	.regQuantity = LED_MODEL_FRAME_SIZE,
	.regFrameQuantity = 1,
	.counterRegArray = defaulCounterRegArray,
	.counterRegIndex = 0,
	.counterRegQuantity = LED_MODEL_FRAME_SIZE,
	.stepQuantity = 0,
	.loaded = {0, 0, 0, 0},
	.busy = 0,
};

LEDDataModel *currentModel = &defaultModel;

void loadRegData_uint8(uint8_t *inData, uint32_t *inIndex, uint32_t inFullSize, uint8_t *outData, uint32_t outIndex, uint8_t *loadedState)
{
	for (uint32_t i = LED_DMA_BUFFER_HALFSIZE; i != 0; i--)
	{
		outData[outIndex++] = inData[(*inIndex)++];
		if (*inIndex >= inFullSize){ *inIndex = 0; *loadedState = 0xff;}
	}
}

void loadRegData_uint16(uint16_t *inData, uint32_t *inIndex, uint32_t inFullSize, uint16_t *outData, uint32_t outIndex, uint8_t *loadedState)
{
	for (uint32_t i = LED_DMA_BUFFER_HALFSIZE; i != 0; i--)
	{
		outData[outIndex++] = inData[(*inIndex)++];
		if (*inIndex >= inFullSize){ *inIndex = 0; *loadedState = 0xff;}
	}
}

/**
 * Load (LED_DMA_BUFFER_HALFSIZE) sized block into DMA LED data array
 * startStep - start DMA array index
 */
void loadCounterRegData(LEDDataModel *model, uint16_t startStep)
{
	for (uint32_t i = 0; i < LED_DMA_BUFFER_HALFSIZE; i++)
	{
		LEDData.cntReg[startStep] = model->counterRegArray[model->counterRegIndex] << 16 | model->counterRegArray[model->counterRegIndex];
		model->counterRegIndex++;
		startStep++;
		if (model->counterRegIndex >= model->counterRegQuantity)
		{
			model->counterRegIndex = 0;
		}
	}
}

void rowRegFCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint16(currentModel->rowRegArray_first, &currentModel->rowRegIndex_first, currentModel->regQuantity, LEDData.rowReg_first, LED_DMA_BUFFER_HALFSIZE, &currentModel->loaded.firstRowReg);
}

void rowRegFHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint16(currentModel->rowRegArray_first, &currentModel->rowRegIndex_first, currentModel->regQuantity, LEDData.rowReg_first, 0, &currentModel->loaded.firstRowReg);
}

void floorRegFCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint8(currentModel->floorRegArray_first, &currentModel->floorRegIndex_first, currentModel->regQuantity, LEDData.floorReg_first, LED_DMA_BUFFER_HALFSIZE, &currentModel->loaded.firstFloorReg);
}

void floorRegFHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint8(currentModel->floorRegArray_first, &currentModel->floorRegIndex_first, currentModel->regQuantity, LEDData.floorReg_first, 0, &currentModel->loaded.firstFloorReg);
}

void rowRegSCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint16(currentModel->rowRegArray_second, &currentModel->rowRegIndex_second, currentModel->regQuantity, LEDData.rowReg_second, LED_DMA_BUFFER_HALFSIZE, &currentModel->loaded.secondRowReg);
}

void rowRegSHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint16(currentModel->rowRegArray_second, &currentModel->rowRegIndex_second, currentModel->regQuantity, LEDData.rowReg_second, 0, &currentModel->loaded.secondRowReg);
}

void floorRegSCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint8(currentModel->floorRegArray_second, &currentModel->floorRegIndex_second, currentModel->regQuantity, LEDData.floorReg_second, LED_DMA_BUFFER_HALFSIZE, &currentModel->loaded.secondFloorReg);
	HAL_TIM_Base_Start_IT(&htim10);
}

void floorRegSHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadRegData_uint8(currentModel->floorRegArray_second, &currentModel->floorRegIndex_second, currentModel->regQuantity, LEDData.floorReg_second, 0, &currentModel->loaded.secondFloorReg);
	HAL_TIM_Base_Start_IT(&htim10);
}

void cntRegCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadCounterRegData(currentModel, LED_DMA_BUFFER_HALFSIZE);
}

void cntRegHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
	loadCounterRegData(currentModel, 0);
}

/**
 * Init LED illumination sys
 */
void LED_Init()
{
	//DMA callback init
	hdma_tim1_ch1.XferCpltCallback = rowRegFCpltCallback;
	hdma_tim1_ch1.XferHalfCpltCallback = rowRegFHalfCpltCallback;
	hdma_tim1_ch2.XferCpltCallback = floorRegFCpltCallback;
	hdma_tim1_ch2.XferHalfCpltCallback = floorRegFHalfCpltCallback;
	hdma_tim1_ch3.XferCpltCallback = rowRegSCpltCallback;
	hdma_tim1_ch3.XferHalfCpltCallback = rowRegSHalfCpltCallback;
	hdma_tim1_ch4_trig_com.XferCpltCallback = floorRegSCpltCallback;
	hdma_tim1_ch4_trig_com.XferHalfCpltCallback = floorRegSHalfCpltCallback;

	//Data transmission DMA register
	HAL_DMA_Start_IT(&hdma_tim1_ch1, (uint32_t)&LEDData.rowReg_first, (uint32_t)&GPIOB->ODR, LED_DMA_BUFFER_SIZE);
	HAL_DMA_Start_IT(&hdma_tim1_ch2, (uint32_t)&LEDData.floorReg_first, (uint32_t)&GPIOC->ODR, LED_DMA_BUFFER_SIZE);
	HAL_DMA_Start_IT(&hdma_tim1_ch3, (uint32_t)&LEDData.rowReg_second, (uint32_t)&GPIOB->ODR, LED_DMA_BUFFER_SIZE);
	HAL_DMA_Start_IT(&hdma_tim1_ch4_trig_com, (uint32_t)&LEDData.floorReg_second, (uint32_t)&GPIOC->ODR, LED_DMA_BUFFER_SIZE);

	//DMA channel counter data burst transmission
//	HAL_TIM_DMABurst_WriteStart(&htim1, TIM_DMABASE_CCR3, TIM_DMA_UPDATE, (uint32_t *)&LEDData.cntReg, TIM_DMABURSTLENGTH_2TRANSFERS);
	//From HAL function
	//Set HAL callbacks
    htim1.hdma[TIM_DMA_ID_UPDATE]->XferCpltCallback = cntRegCpltCallback;
    htim1.hdma[TIM_DMA_ID_UPDATE]->XferHalfCpltCallback = cntRegHalfCpltCallback;
    htim1.hdma[TIM_DMA_ID_UPDATE]->XferErrorCallback = NULL;
    //Start DMA burst to channel 1,2 counter reg
    HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_UPDATE], (uint32_t)&LEDData.cntReg, (uint32_t)&htim1.Instance->DMAR, LED_DMA_BUFFER_SIZE * 2);
    htim1.Instance->DCR = (TIM_DMABASE_CCR3 | TIM_DMABURSTLENGTH_2TRANSFERS);
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);

	//Tim channels start
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC1);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC2);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC3);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC4);
	TIM_CCxChannelCmd(htim1.Instance, TIM_CHANNEL_ALL, TIM_CCx_ENABLE);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

/**
 * Start LED illumination
 */
void LED_Start()
{
	htim1.Instance->CR1 |= TIM_CR1_CEN;
//	__HAL_TIM_ENABLE(&htim1);
}

/**
 * Stop LED illumination
 * Data remains in the buffer
 */
void LED_Stop()
{
	htim1.Instance->CR1 &= ~TIM_CR1_CEN;
	currentModel = &defaultModel;
}

void LED_Pause()
{
	htim1.Instance->CR1 &= ~TIM_CR1_CEN;
	GPIOC->BSRR = 0xff << 16;
	GPIOB->BSRR = 0xffff << 16;
}

void LED_LoadModel(LEDDataModel *model)
{
	currentModel = model;
}
