#ifndef CONSOLECOMMANDSFUNC_H_
#define CONSOLECOMMANDSFUNC_H_

#include "main.h"
#include "stdlib.h"
#include "string.h"

#include "usbd_audio.h"
#include "usbd_audio_if.h"
#include "rtc.h"

void ConsoleCommandsFunc_Help(uint8_t * data);
void ConsoleCommandsFunc_SetTime(uint8_t * data);
void ConsoleCommandsFunc_SetDate(uint8_t * data);
void ConsoleCommandsFunc_GetRTCCalibration(uint8_t * data);
void ConsoleCommandsFunc_SetRTCCalibration(uint8_t * data);
void ConsoleCommandsFunc_SetMessageVolume(uint8_t * data);

#endif
