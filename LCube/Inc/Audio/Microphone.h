#ifndef MICROPHONE_H_
#define MICROPHONE_H_

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"

#include "usbd_audio.h"
#include "AudioProcessing.h"
#include "I2C_Devices.h"
#include "IIR_Filter.h"

#define MICROPHONE_CAPTURE_SAMPLING_FREQUENCY 32000U
#define MICROPHONE_CAPTURE_DATA_SIZE (512U)
#define MICROPHONE_CAPTURE_DATA_HALFSIZE (MICROPHONE_CAPTURE_DATA_SIZE / 2)
#define MICROPHONE_OUT_DATA_SIZE (MICROPHONE_CAPTURE_DATA_SIZE / 4)
#define MICROPHONE_CAPTURE_ADC_CENTER ((0xFFFF / 2) - 704)

#define MICROPHONE_AMPLIFIER_DEFAULT_CODE 0x20

#define MICROPHONE_CLIPPED_FRAMES_LIMIT 2U
#define MICROPHONE_UNCLIPPED_TIME_LIMIT 30000U //msec

void Microphone_Init();
void Microphone_Start_Capture();
void Microphone_Stop_Capture();

bool Microphone_Data_Ready(bool reset_ready);
int16_t* Microphone_Data();

#endif
