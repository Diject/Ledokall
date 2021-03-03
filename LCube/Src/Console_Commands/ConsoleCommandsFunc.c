
#include "ConsoleCommandsFunc.h"

const uint8_t helpCommandMessage[] = "Commands:\r\n-set_time h m s\r\n-set_date d wday m yy\r\n-get_rtc_calibration\r\n-set_rtc_calibration (float)\r\n"
		"-set_audio_messages_volume (float)\r\n"
		"OK\r\n";
const uint8_t OKMessage[] = "OK\r\n";
const uint8_t TimeChangeMessage[] = "Hours %u, Minutes %u, Seconds %u\r\nOK\r\n";
const uint8_t RTCCalibrationMessage[] = "One day calibration: %.2f sec\r\nOK\r\n";
const uint8_t AudioMessageVolumeMessage[] = "Voice message volume: %.2f. Previous value: %.2f\r\n";

void ConsoleCommandsFunc_Help(uint8_t * data)
{
	CDC_Transmit_FS(helpCommandMessage, sizeof(helpCommandMessage) - 1);
}

void ConsoleCommandsFunc_SetTime(uint8_t * data)
{
	uint32_t sec = 0;
	uint32_t min = 0;
	uint32_t h = 0;
	sscanf((char *)data, "%s %lu %lu %lu", NULL, &h, &min, &sec);
	RTC_TimeTypeDef time = {0};
	time.Seconds = sec;
	time.Minutes = min;
	time.Hours = h;
	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
	uint8_t str[64] = {0};
	sprintf((char *)str, TimeChangeMessage, h, min, sec);
	CDC_Transmit_FS(str, strlen(str));
}

void ConsoleCommandsFunc_SetDate(uint8_t * data)
{
	uint32_t day = 0;
	uint32_t wday = 0;
	uint32_t month = 0;
	uint32_t year = 0;
//	uint8_t str[32] = {0};
	sscanf((char *)data, "%s %lu %lu %lu %lu", NULL, &day, &wday, &month, &year);
	RTC_DateTypeDef date = {0};
	date.Date = day;
	date.WeekDay = wday;
	date.Month = month;
	date.Year = year;
	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
	CDC_Transmit_FS(OKMessage, sizeof(OKMessage) - 1);
}

void ConsoleCommandsFunc_GetRTCCalibration(uint8_t * data)
{
	char str[42] = {0};
	sprintf(str, RTCCalibrationMessage, get1dCalibSec());
	CDC_Transmit_FS(str, strlen(str));
}

void ConsoleCommandsFunc_SetRTCCalibration(uint8_t * data)
{
	float calVal = 0.f;
	sscanf((char *)data, "%s %f", NULL, &calVal);
	set1dCalibSec(calVal);
	char str[42] = {0};
	sprintf(str, RTCCalibrationMessage, get1dCalibSec());
	CDC_Transmit_FS(str, strlen(str));
}

void ConsoleCommandsFunc_SetMessageVolume(uint8_t * data)
{
	float volVal = 0.f;
	sscanf((char *)data, "%s %f", NULL, &volVal);
	if (volVal > 1.f) volVal = 1.f;
	char str[64] = {0};
	sprintf(str, AudioMessageVolumeMessage, volVal, device.audio.messageVolume);
	device.audio.messageVolume = volVal;
	Device_Save_Message_Volume();
	CDC_Transmit_FS(str, strlen(str));
}
