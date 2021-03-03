
#include "buttons.h"

buttonState button[NUMBER_OF_TOUCH_BUTTONS] = {0};

void button_pressedCallback(uint8_t id)
{
	button[id].state = 1;
	uint32_t secs = (uint8_t)((hrtc.Instance->TR & (RTC_TR_ST | RTC_TR_SU)) >> RTC_TR_SU_Pos);
	uint32_t ssec = (hrtc.Instance->PRER & 0x7f) - hrtc.Instance->SSR;
	hrtc.Instance->DR;
	secs = RTC_Bcd2ToByte(secs);
	button[id].pressedStartLabel = (secs << 8) + ssec;
}

void button_releasedCallback(uint8_t id)
{
	if (button[id].state)
	{
		uint32_t secs = (uint8_t)((hrtc.Instance->TR & (RTC_TR_ST | RTC_TR_SU)) >> RTC_TR_SU_Pos);
		uint32_t ssec = (hrtc.Instance->PRER & 0x7f) - hrtc.Instance->SSR;
		hrtc.Instance->DR;
		secs = RTC_Bcd2ToByte(secs);
		volatile uint32_t tm = (secs << 8) + ssec;
		volatile uint32_t tmdif = 0;
		if (tm > button[id].pressedStartLabel)
		{
			tmdif = tm - button[id].pressedStartLabel;
		}else
		{
			tmdif = tm + ((60U << 8) - 1) - button[id].pressedStartLabel;
		}
		button[id].pressedTime = (tmdif > 0xFFFF) ? 0xFFFF : (uint16_t)tmdif;
		button[id].pressedCount++;
		button[id].state = 0;
	}
}

inline uint16_t button_getPressedTime(uint8_t id)
{
	return button[id].pressedTime;
}

uint16_t button_getPressedTimeEx(uint8_t id)
{
	uint16_t res = button[id].pressedTime;
	button[id].pressedTime = 0;
	button[id].pressedCount = 0;
	return res;
}

void button_stateReset(uint8_t id)
{
	button[id].pressedTime = 0;
	button[id].pressedCount = 0;
}

buttonsReg buttons_state()
{
	uint8_t res = 0;
	if (!(Button0_GPIO_Port->IDR & (Button0_Pin | Button1_Pin | Button2_Pin)))
	{
		for (uint8_t i = 0; i < NUMBER_OF_TOUCH_BUTTONS; i++)
		{
			if (button[i].pressedTime) res |= 1 << i;
		}
	}
	return res;
}

buttonsReg buttons_stateEx(uint8_t *pCount, uint16_t *pTime)
{
	uint8_t res = 0;
	if (!(Button0_GPIO_Port->IDR & (Button0_Pin | Button1_Pin | Button2_Pin)))
	{
		for (uint8_t i = 0; i < NUMBER_OF_TOUCH_BUTTONS; i++)
		{
			if (button[i].pressedTime)
			{
				res |= 1 << i;
				pTime[i] = button[i].pressedTime;
				button[i].pressedTime = 0;
				pCount[i] = button[i].pressedCount;
				button[i].pressedCount = 0;
			}
		}
	}
	return res;
}

inline uint8_t button_getPressedCount(uint8_t id)
{
	return button[id].pressedCount;
}

uint8_t button_getPressedCountEx(uint8_t id)
{
	uint8_t res = button[id].pressedCount;
	button[id].pressedTime = 0;
	button[id].pressedCount = 0;
	return res;
}
