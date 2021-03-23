#ifndef AUDIOVISUALIZATION_H_
#define AUDIOVISUALIZATION_H_

#include "main.h"
#include "tim.h"
#include "arm_math.h"
#include "ringBuffer.h"
#include "Device.h"

#include "feature_extraction.h"
#include "NN.h"
#include "NN_Detection.h"

#define NN_AUDIO_BUFFER_SIZE 3072U
#define AUDIO_PROCESSING_BUFFER_SIZE 2048U
#define AUDIO_PROCESSING_FFT_SIZE 1024U

#define NN_MFCC_COLUMNS 15U
#define NN_MFCC_COLUMNS_ALT 29U
#define NN_MFCC_COUNT 21U

#define AUDIO_PROCESSING_MFCC_COUNT 30U
#define AUDIO_PROCESSING_MFCC_SAMPLING_FREQUENCY 16000U
#define AUDIO_PROCESSING_MFCC_NYQUIST_FREQUENCY (AUDIO_PROCESSING_MFCC_SAMPLING_FREQUENCY / 2)

#define AUDIO_VISUALIZATION_MAX_VOLUME_RESET_PERIOD 30000U

void audioProcessing_Init();

ringBufferStatus audioProcessing_AddData(int16_t *data, uint32_t size, audioInputState source);
void audioProcessing_ClearBuffer();
void NN_ClearAudioBuffer();
uint8_t audioProcessing_Run();

float log10_approx(float x);

#endif
