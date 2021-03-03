#ifndef MICROPHONE_H_
#define MICROPHONE_H_

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"

#include "usbd_audio.h"
#include "AudioProcessing.h"
#include "I2C_Devices.h"

#define MICROPHONE_CAPTURE_SAMPLING_FREQUENCY 16000U
#define MICROPHONE_CAPTURE_DATA_SIZE (256U)
#define MICROPHONE_CAPTURE_DATA_HALFSIZE (MICROPHONE_CAPTURE_DATA_SIZE / 2)
#define MICROPHONE_CAPTURE_ADC_CENTER ((0xFFFF / 2) - 704)

#define MICROPHONE_AMPLIFIER_DEFAULT_CODE 0x20

#define MICROPHONE_CLIPPED_FRAMES_LIMIT 6U
#define MICROPHONE_UNCLIPPED_TIME_LIMIT 120000U //msec

void Microphone_Init();
void Microphone_Start_Capture();
void Microphone_Stop_Capture();

bool Microphone_Data_Ready(bool reset_ready);
int16_t* Microphone_Data();

#endif
