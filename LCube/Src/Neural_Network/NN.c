
#include "NN.h"

static ai_handle voicecommandsnn = AI_HANDLE_NULL;
static ai_buffer ai_input[1] = AI_VOICECOMMANDSNN_IN ;
static ai_buffer ai_output[1] = AI_VOICECOMMANDSNN_OUT ;

//static ai_handle voice_nn_alt = AI_HANDLE_NULL;
//static ai_buffer ai_input_alt[1] = AI_VOICE_NN_ALT_IN ;
//static ai_buffer ai_output_alt[1] = AI_VOICE_NN_ALT_OUT ;

ai_u8 activations[AI_VOICECOMMANDSNN_DATA_ACTIVATIONS_SIZE];
//ai_u8 activations_alt[AI_VOICE_NN_ALT_DATA_ACTIVATIONS_SIZE];

const float detection_coef[20] = {0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.8f,
		0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f, 0.92f};


int aiInit()
{
    ai_error err;

    /* 1 - Specific AI data structure to provide the references of the
     * activation/working memory chunk and the weights/bias parameters */
    const ai_network_params params = {
            AI_VOICECOMMANDSNN_DATA_WEIGHTS(ai_voicecommandsnn_data_weights_get()),
            AI_VOICECOMMANDSNN_DATA_ACTIVATIONS(activations)
    };

    /* 2 - Create an instance of the NN */
    err = ai_voicecommandsnn_create(&voicecommandsnn, AI_VOICECOMMANDSNN_DATA_CONFIG);
    if (err.type != AI_ERROR_NONE) {
	    return -1;
    }

    /* 3 - Initialize the NN - Ready to be used */
    if (!ai_voicecommandsnn_init(voicecommandsnn, &params)) {
        err = ai_voicecommandsnn_get_error(voicecommandsnn);
        ai_voicecommandsnn_destroy(voicecommandsnn);
        voicecommandsnn = AI_HANDLE_NULL;
	    return -2;
    }

//    const ai_network_params params_alt = {
//            AI_VOICE_NN_ALT_DATA_WEIGHTS(ai_voice_nn_alt_data_weights_get()),
//            AI_VOICE_NN_ALT_DATA_ACTIVATIONS(activations_alt)
//    };
//
//    err = ai_voice_nn_alt_create(&voice_nn_alt, AI_VOICE_NN_ALT_DATA_CONFIG);
//    if (err.type != AI_ERROR_NONE) {
//	    return -1;
//    }
//
//    if (!ai_voice_nn_alt_init(voice_nn_alt, &params_alt)) {
//        err = ai_voice_nn_alt_get_error(voice_nn_alt);
//        ai_voice_nn_alt_destroy(voice_nn_alt);
//        voice_nn_alt = AI_HANDLE_NULL;
//	    return -2;
//    }

    return 0;
}

int aiRun(const void *in_data, void *out_data)
{
    ai_i32 nbatch;
    ai_error err;

    /* Parameters checking */
    if (!in_data || !out_data || !voicecommandsnn /*|| !voice_nn_alt*/)
        return -1;

    if (!device.neuralNetworkId)
    {
		/* Initialize input/output buffer handlers */
		ai_input[0].n_batches = 1;
		ai_input[0].data = AI_HANDLE_PTR(in_data);
		ai_output[0].n_batches = 1;
		ai_output[0].data = AI_HANDLE_PTR(out_data);

		/* 2 - Perform the inference */
		nbatch = ai_voicecommandsnn_run(voicecommandsnn, &ai_input, &ai_output);
		if (nbatch != 1) {
			err = ai_voicecommandsnn_get_error(voicecommandsnn);
			// ...
			return err.code;
		}
    }else
    {
//		ai_input_alt[0].n_batches = 1;
//		ai_input_alt[0].data = AI_HANDLE_PTR(in_data);
//		ai_output_alt[0].n_batches = 1;
//		ai_output_alt[0].data = AI_HANDLE_PTR(out_data);
//
//		nbatch = ai_voice_nn_alt_run(voice_nn_alt, &ai_input_alt, &ai_output_alt);
//		if (nbatch != 1) {
//			err = ai_voice_nn_alt_get_error(voice_nn_alt);
//			// ...
//			return err.code;
//		}
    }

    return 0;
}

void NN_Init()
{
    aiInit();
}

float maxr = 0;

inline int32_t NN_Run(const float *in_data, float *out_data)
{
	aiRun(in_data, out_data);

	float max = 0;
	uint32_t maxIndex = 0;
	arm_max_f32(out_data, AI_VOICECOMMANDSNN_OUT_1_SIZE, &max, &maxIndex);
	if (max > maxr) maxr = max;
	if (max > detection_coef[maxIndex])
	{
		return maxIndex;
	}
	return -1;
}





