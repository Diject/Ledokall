#ifndef LEDVISUALIZATION_GFX_H_
#define LEDVISUALIZATION_GFX_H_

#include "main.h"
#include "LEDVisualization.h"
#include "math.h"
#include "string.h"
#include "stdbool.h"

#define LED_MAX_BRIGHTNESS 127U
#define LED_MAX_BRIGHTNESS_BITSIZE 7U
#define LED_ROWS_VISUALIZATION_MAX_VALUE ((LED_MAX_BRIGHTNESS + 1) * LED_MODEL_FRAME_SIZE - 1)

typedef struct
{
	uint16_t *reg;
	uint8_t shift;
} LEDCell;

typedef enum
{
	COLOR_RED = 0b0,
	COLOR_BLUE = 0b1,
	COLOR_NONE = 0xff,
} LEDColor;

typedef enum
{
	LED_SYSTEM_ERROR_NO = 0,
	LED_SYSTEM_ERROR_OVERFLOW = 1 << 0,
	LED_SYSTEM_ERROR_UNDERFLOW = 1 << 1,
} LEDSystemErrors;

typedef struct
{
	uint16_t brightness;
	uint8_t color;
} LEDColorCell;

typedef struct
{
	uint16_t rowStateMatrix;
	uint16_t brightness;
	uint8_t color;
} LEDStepMatrix;

typedef struct
{
	float rowGrowthSpeed[16];
	uint16_t rowHeight[16]; //LED_MODEL_STEP_VALUE_SIZE * LED_MODEL_FRAME_SIZE max
} LEDRowsVisualizationFrame;

typedef struct
{
	uint8_t dataArray[7];
} LEDFlatSymbol;

void LED_GFX_Init();

void LED_GFX_InitModel_2Color(LEDDataModel *model, uint16_t *rowRegPointer_red, uint16_t *rowRegPointer_blue,
		uint8_t *floorRegPoiter_red, uint8_t *floorRegPoiter_blue, uint16_t *counterRegPounter, uint16_t stepNumber);
void LED_GFX_ClearModel(LEDDataModel *model);
void LED_GFX_DrawRowVisualization(LEDRowsVisualizationFrame *lastFrame, LEDRowsVisualizationFrame *currentFrame, LEDDataModel *targetModel, float growthRate);
float LED_GFX_DrawSymbol(const LEDFlatSymbol *symbol, LEDDataModel *targetModel, LEDColor color, uint32_t line, float startBrightness, float endBrightness,
		float growthSpeed, float brightnessMul);

#endif
