#ifndef LEDVISUALIZATION_H_
#define LEDVISUALIZATION_H_

#include "main.h"
#include "tim.h"
#include "dma.h"

//#define LED_UPDATE_FREQUENCY 12000U
#define LED_COUNTER_REG_MAX_VALUE 9999U //Максимальное значение счетчика таймера
#define LED_COUNTER_CALIBRATION_SHIFT 225.f //Сдвиг для калибровки яркости. >0 - >красного. <0 - синего
#define LED_MODEL_FRAME_SIZE 7U //Высота куба
#define LED_MODEL_STEP_SIZE 8U //Количество фреймов для шага ярокости
#define LED_DMA_BUFFER_HALFSIZE_STEPS 10U //Количество шагов в модели
#define LED_DMA_BUFFER_HALFSIZE (LED_DMA_BUFFER_HALFSIZE_STEPS * LED_MODEL_STEP_SIZE * LED_MODEL_FRAME_SIZE)
#define LED_DMA_BUFFER_SIZE (LED_DMA_BUFFER_HALFSIZE * 2U)

typedef struct
{
	uint16_t reg1;
	uint16_t reg2;
} Uint32To2Uint16Struct;

typedef struct
{
	uint8_t secondRowReg;
	uint8_t secondFloorReg;
	uint8_t firstRowReg;
	uint8_t firstFloorReg;
} loadedState;

typedef struct
{
	uint16_t *rowRegArray_first; //pointer to register data array, must be aligned
	uint32_t rowRegIndex_first; //Array index for next load
	uint16_t *rowRegArray_second;
	uint32_t rowRegIndex_second;
	uint8_t *floorRegArray_first;
	uint32_t floorRegIndex_first;
	uint8_t *floorRegArray_second;
	uint32_t floorRegIndex_second;
	uint32_t regQuantity; //Array data quantity
	uint32_t regFrameQuantity; //Frames quantity = (Array data quantity) / LED_MODEL_FRAME_SIZE
	uint32_t stepQuantity; //
	uint16_t *counterRegArray;
	uint32_t counterRegIndex;
	uint32_t counterRegQuantity;
	loadedState loaded;
	uint8_t busy;
} LEDDataModel;

enum
{
	MODEL_FULLYLOADED = (uint32_t)0xFFFFFFFFU,
};

/**
 * LEDs states struct
 */
typedef struct
{
	uint16_t rowReg_first[LED_DMA_BUFFER_SIZE]; //GPIOB
	uint16_t rowReg_second[LED_DMA_BUFFER_SIZE]; //GPIOB
	uint8_t floorReg_first[LED_DMA_BUFFER_SIZE]; //GPIOC
	uint8_t floorReg_second[LED_DMA_BUFFER_SIZE]; //GPIOC
	uint32_t cntReg[LED_DMA_BUFFER_SIZE]; //PWM counter value
} LEDDMADataStruct;

void LED_Init();
void LED_Start();
void LED_Stop();
void LED_Pause();
void LED_LoadModel(LEDDataModel *model);

#endif
