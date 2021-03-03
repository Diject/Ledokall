#ifndef NN_H_
#define NN_H_

#include "main.h"

#include "Device.h"

#include <string.h>
#include <stdlib.h>

#include "arm_math.h"
#include "ai_platform.h"
#include "voicecommandsnn.h"
#include "voicecommandsnn_data.h"
#include "voice_nn_alt.h"
#include "voice_nn_alt_data.h"
#include "ai_datatypes_defines.h"

//#define AI_MNETWORK_IN_1_SIZE_BYTES AI_VOICECOMMANDSNN_IN_1_SIZE_BYTES
//#define AI_MNETWORK_IN_NUM 1
//#define DEF_DATA_IN \
//  AI_ALIGNED(4) ai_i8 data_in_1[AI_MNETWORK_IN_1_SIZE_BYTES]; \
//  static ai_i8* data_ins[] = { \
//    data_in_1 \
//  }; \
//
//#define AI_MNETWORK_OUT_1_SIZE_BYTES AI_VOICECOMMANDSNN_OUT_1_SIZE_BYTES
//#define AI_MNETWORK_OUT_NUM 1
//#define DEF_DATA_OUT \
//  AI_ALIGNED(4) ai_i8 data_out_1[AI_MNETWORK_OUT_1_SIZE_BYTES]; \
//  static ai_i8* data_outs[] = { \
//    data_out_1 \
//  }; \
//
//#define AI_VOICECOMMANDSNN_DATA_ACTIVATIONS_START_ADDR 0xFFFFFFFF
//
//#define AI_MNETWORK_DATA_ACTIVATIONS_INT_SIZE AI_VOICECOMMANDSNN_DATA_ACTIVATIONS_SIZE
//
//#define AI_MMETWORK_IN                AI_VOICECOMMANDSNN_IN
//#define AI_MNETWORK_OUT               AI_VOICECOMMANDSNN_OUT
//
//
//#define AI_MIN(x_, y_) \
//  ( ((x_)<(y_)) ? (x_) : (y_) )
//
//#define AI_MAX(x_, y_) \
//  ( ((x_)>(y_)) ? (x_) : (y_) )
//
//#define AI_CLAMP(x_, min_, max_, type_) \
//  (type_) (AI_MIN(AI_MAX(x_, min_), max_))
//
//#define AI_ROUND(v_, type_) \
//  (type_) ( ((v_)<0) ? ((v_)-0.5f) : ((v_)+0.5f) )

void NN_Init();
int32_t NN_Run(const float *in_data, float *out_data);

#endif
