#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "main.h"
#include "rtc.h"

#define NUMBER_OF_TOUCH_BUTTONS 3U

typedef enum
{
	BUTTON0 = 1,
	BUTTON1 = 1 << 1,
	BUTTON2 = 1 << 2,
} buttonsReg;

typedef struct
{
	uint8_t state;
	uint8_t pressedCount;
	uint16_t pressedTime;
	uint32_t pressedStartLabel;
} buttonState;

void button_pressedCallback(uint8_t id);
void button_releasedCallback(uint8_t id);
uint16_t button_getPressedTime(uint8_t id);
uint16_t button_getPressedTimeEx(uint8_t id);
void button_stateReset(uint8_t id);

buttonsReg buttons_state();
buttonsReg buttons_stateEx(uint8_t *pCount, uint16_t *pTime);

uint8_t button_getPressedCount(uint8_t id);
uint8_t button_getPressedCountEx(uint8_t id);

#endif /* BUTTONS_H_ */
