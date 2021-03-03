#ifndef NN_DETECTION_H_
#define NN_DETECTION_H_

#include "main.h"
#include "NN.h"
#include "tim.h"
#include "Device.h"
#include "AudioProcessing.h"

#define NN_DETECTION_COMMANDS_COUNT 16
#define NN_DETECTION_TIME_LIMIT 5000U
#define NN_DETECTION_DELAY_TIME 200U
#define NN_DETECTION_LOCK_TIME 600U

#define NN_DETECTION_RESET_STATE 0xffffffff

typedef struct
{
	uint32_t label;
	void (*action)(uint32_t);
} detection_command;

void NN_Detect_Init();
void NN_Detect_Process(int32_t nn_result);

#endif
