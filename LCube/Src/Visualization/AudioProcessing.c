
#include <AudioProcessing.h>
#include "stdlib.h"
#include "Device.h"

float nn_mfcc_buffer[NN_MFCC_COUNT][NN_MFCC_COLUMNS_ALT] = {{0.f}};
uint32_t nn_mfcc_pos = 0;

arm_rfft_fast_instance_f32 fftS1024 = {0};
arm_rfft_fast_instance_f32 fftS512 = {0};
SpectrogramTypeDef spectrS = {0};
MelFilterTypeDef melfS = {0};
MelSpectrogramTypeDef melS = {0};
LogMelSpectrogramTypeDef lmelS = {0};
//float  aProcessingBuffer[AUDIO_PROCESSING_FFT_SIZE];


int16_t audioProcessingDataBuffer[AUDIO_PROCESSING_BUFFER_SIZE] = {0};
//Буфер аудио данных для анимации
ringBuffer_Int16 buffer = {
	.buffer = audioProcessingDataBuffer,
	.size = AUDIO_PROCESSING_BUFFER_SIZE,
	.startPos = 0,
	.endPos = 0,
	.dataSize = 0,
};

//Буфер аудио данных для нейросети
int16_t NNAudioDataBuffer[NN_AUDIO_BUFFER_SIZE] = {0};
ringBuffer_Int16 NNBuffer = {
	.buffer = NNAudioDataBuffer,
	.size = NN_AUDIO_BUFFER_SIZE,
	.startPos = 0,
	.endPos = 0,
	.dataSize = 0,
};

/*
 *  log10 approximation
 *
 *  reference: https://community.arm.com/tools/f/discussions/4292/cmsis-dsp-new-functionality-proposal
 */
const float C[4] = { 1.23149591368684f, -4.11852516267426f,
    6.02197014179219f, -3.13396450166353f };
const float LOG10_2 = log10f(2.0f);
float log10_approx(float x) {
  float f, l;
  int e;
  f = frexpf(fabsf(x), &e);
  l = LOG10_2 * (C[0] * f * f * f + C[1] * f * f + C[2] * f + C[3] + e);
  return l;
  //return (l >= 0.0) ? l : 0.0;  // regard a negative value as featureless
}

/**
 * size - input samples / 2
 * return max value
 */
int16_t performFFT_Int16(int16_t *input, int16_t *output, uint32_t size)
{
	int16_t max;
	uint32_t maxIndex;
	arm_cfft_radix4_instance_q15 fftS;
	arm_cfft_radix4_init_q15(&fftS, size, 0, 1);
	arm_cfft_radix4_q15(&fftS, input);
	arm_cmplx_mag_q15(input, output, size);
	arm_max_q15(output, size, &max, &maxIndex);
	return max;
}

/**
 * out size = 2 * input size
 */
void prepareFFTArray_Int16(int16_t *input, int16_t *output, uint32_t inputSize)
{
	uint32_t cnt = 0;
	for (uint32_t i = 0; i < inputSize; i++)
	{
		output[cnt++] = input[i];
		output[cnt++] = 0;
	}
}

/**
 * size - input complex samples
 * return max value
 */
void performFFT_F32(float *input, float *output, uint32_t size)
{
	arm_rfft_fast_instance_f32 *handle = (device.audio.audioInput == AUDIO_INPUT_MICROPHONE) ?
			&fftS512 : &fftS1024;
	arm_rfft_fast_f32(handle, input, output, 0);
	arm_cmplx_mag_f32(output, output, handle->fftLenRFFT / 4);
}

/**
 * out size = 2 * input size
 */
void prepareFFTArray_F32(int16_t *input, float *output, uint32_t inputSize)
{
	uint32_t cnt = 0;
	for (uint32_t i = 0; i < inputSize; i++)
	{
		output[cnt++] = (float)input[i];
		output[cnt++] = 0.f;
	}
}

/**
 * Добавление аудио в буферы
 */
ringBufferStatus audioProcessing_AddData(int16_t *data, uint32_t size, audioInputState source)
{
	if ((device.audio.audioInput == source))
	{
		RingBuffer_Int16_Write(&buffer, data, size);
		uint32_t bsize = (device.audio.audioInput == AUDIO_INPUT_MICROPHONE) ? AUDIO_PROCESSING_FFT_SIZE / 2 : AUDIO_PROCESSING_FFT_SIZE;
		if (RingBuffer_DataSize(&buffer) >= bsize) HAL_TIM_Base_Start_IT(&htim11); //trigger audioProcessing_Run();
	}
	if ((source == AUDIO_INPUT_MICROPHONE) && (device.neuralNetworkStatus == NEURAL_NETWORK_ENABLE))
	{
		RingBuffer_Int16_Write(&NNBuffer, data, size);
		if (RingBuffer_DataSize(&NNBuffer) >= AUDIO_PROCESSING_FFT_SIZE) HAL_TIM_Base_Start_IT(&htim12); //trigger NN_Processing();
	}
	return RINGBUFFER_OK;
}

/**
 * Очистка аудио буфера с данными для анимации
 */
inline void audioProcessing_ClearBuffer()
{
	RingBuffer_Clear(&buffer);
}

/**
 * Очистка аудио буфера с данными для нейросети
 */
inline void NN_ClearAudioBuffer()
{
	RingBuffer_Clear(&NNBuffer);
	nn_mfcc_pos = 0;
}

/**
 * Генерация mfcc
 */
int32_t NN_MelGeneration(int16_t *pInSignal, float *melCoeffs, uint32_t useHann)
{
	uint32_t n_fft = spectrS.FFTLen;
	uint32_t detect = 0; //количество аудио семплов со значением >4000
//from (SpectrogramColumn)
	float *fftIn = malloc(n_fft * 2 * sizeof(float));
	if (fftIn == NULL){ free(fftIn); return -1; }
	float *dp = fftIn;
	int16_t *di = pInSignal;
	if (useHann)
	{
		float *hannw = spectrS.pWindow;
		for (uint32_t i = 0; i < n_fft; i++)
		{
			*dp = (float)*di++;
			*dp *= *hannw++;
			if (fabsf(*dp) > 4000) detect++;
			dp++;
			*dp++ = 0.f;
		}
	}else
	{
		for (uint32_t i = 0; i < n_fft; i++)
		{
			*dp = (float)*di++;
			if (fabsf(*dp) > 4000) detect++;
			dp++;
			*dp++ = 0.f;
		}
	}

	float *fftOut = malloc(n_fft * 2 * sizeof(float));
	if ((fftOut == NULL)){ free(fftIn); return -1;}
	arm_rfft_fast_f32(&fftS1024, fftIn, fftOut, 0);
	free(fftIn);

	float *pOutCol = malloc((n_fft / 2 + 1) * sizeof(float));
	if (pOutCol == NULL){ free(fftOut); return -1; }

	float first_energy = fftOut[0] * fftOut[0];
	float last_energy = fftOut[1] * fftOut[1];
	pOutCol[0] = first_energy;
	arm_cmplx_mag_squared_f32(&fftOut[2], &pOutCol[1], (n_fft / 2) - 1);
	free(fftOut);
	pOutCol[n_fft / 2] = last_energy;

	if (spectrS.Type == SPECTRUM_TYPE_MAGNITUDE)
	{
		for (uint32_t i = 0; i <= (n_fft / 2); i++)
		{
			pOutCol[i] = sqrtf(pOutCol[i]);
		}
	}

	MelFilterbank(&melfS, pOutCol, melCoeffs);
	free(pOutCol);
	return detect;
}

/**
 * Анимация аудио
 */
uint8_t audioProcessing_Run()
{
	if ((device.visualization == VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS) || (device.visualization == VISUALIZATION_STATE_USB_AUDIO_VIS))
	{
		uint32_t bsize = (device.audio.audioInput == AUDIO_INPUT_MICROPHONE) ? AUDIO_PROCESSING_FFT_SIZE / 2 : AUDIO_PROCESSING_FFT_SIZE;
		if (RingBuffer_DataSize(&buffer) >= bsize)
		{
			int16_t *data_in = malloc(bsize * sizeof(int16_t));
			float *data_out = malloc(bsize * 2 * sizeof(float));
			if (data_in == NULL || data_out == NULL){ free(data_in); free(data_out); return 1;}
			while (!RingBuffer_Int16_SeekRead(&buffer, data_in, bsize, bsize))
			{
				float *dtp = data_out;
				int16_t *di = data_in;
				for (uint32_t i = 0; i < bsize; i++)
				{
					*dtp = (float)*di++;
					dtp++;
					*dtp++ = 0.f;
				}
				performFFT_F32(data_out, data_out, bsize * 2);

				audioVisualizationDataUpdate(data_out, (float)device.audio.frequency / bsize);
			}
			free(data_in);
			free(data_out);
			return 0;
		}
	}
	return 1;
}

uint32_t NN_Processing()
{
	if ((RingBuffer_DataSize(&NNBuffer) < AUDIO_PROCESSING_FFT_SIZE) || ((device.neuralNetworkStatus != NEURAL_NETWORK_ENABLE))) return 0;
	if ((RingBuffer_DataSize(&NNBuffer) > 3000)) HAL_NVIC_SystemReset();
	int16_t *data_in = malloc(AUDIO_PROCESSING_FFT_SIZE * sizeof(int16_t));
	float *mfccs = malloc(AUDIO_PROCESSING_MFCC_COUNT * sizeof(float));
	if ((data_in == NULL) || (mfccs == NULL)){ free(data_in); free(mfccs); return 1;}
	while (!RingBuffer_Int16_SeekRead(&NNBuffer, data_in, AUDIO_PROCESSING_FFT_SIZE,
			!device.neuralNetworkId ? AUDIO_PROCESSING_FFT_SIZE : AUDIO_PROCESSING_FFT_SIZE / 2))
	{
		int32_t detect = 0;
		detect = NN_MelGeneration(data_in, mfccs, device.neuralNetworkId);
		int32_t nn_res = -1;
		if (detect == -1) break;
		uint32_t colums = !device.neuralNetworkId ? NN_MFCC_COLUMNS : NN_MFCC_COLUMNS_ALT;
		if (nn_mfcc_pos >= colums)
		{
			for (uint32_t i = 0; i < NN_MFCC_COUNT; i++)
			{
				for (uint32_t j = 1; j < colums; j++) nn_mfcc_buffer[i][j - 1] = nn_mfcc_buffer[i][j];
				nn_mfcc_buffer[i][colums - 1] = mfccs[i + 1];
			}
			float max = 0.f;
			arm_max_f32((float32_t *)nn_mfcc_buffer, NN_MFCC_COUNT * colums, &max, NULL);
			float *buf = malloc(NN_MFCC_COUNT * colums * sizeof(float));
			if (buf == NULL) { break; };
			float *bufp = buf;
			for (uint32_t i = 0; i < NN_MFCC_COUNT; i++)
			{
				for (uint32_t j = 0; j < colums; j++)
				{
					*bufp++ = nn_mfcc_buffer[i][j];
				}
			}
//			memcpy(buf, nn_mfcc_buffer, NN_MFCC_COUNT * NN_MFCC_COLUMNS * sizeof(float));
			float mul = 1.f / max;
			for (uint32_t i = 0; i < NN_MFCC_COUNT * colums; i++)
			{
				*(buf + i) *= mul;
				*(buf + i) = 10.0f * log10_approx(*(buf + i));
				*(buf + i) = (*(buf + i) < -80.f) ? (-80.f) : (*(buf + i));
			}

			float nn_out[AI_VOICECOMMANDSNN_OUT_1_SIZE] = {0.f};
			static uint32_t wtim = 0;
			//Если есть звук, то выполнять нейросеть 15 циклов
			if (detect >= 5)
			{
				wtim = 15;
			}

			if (wtim)
			{
				nn_res = NN_Run(buf, nn_out);
			}

			wtim = (wtim > 0) ? wtim - 1 : wtim;
			free(buf);
		}else
		{
			for (uint32_t i = 0; i < NN_MFCC_COUNT; i++)
			{
				nn_mfcc_buffer[i][nn_mfcc_pos] = mfccs[i];
			}
			nn_mfcc_pos++;
		}
		if (nn_res == 0)
		{
			static uint32_t  rz = 0;
			rz++;
		}
		NN_Detect_Process(nn_res);
	}
	free(data_in);
	free(mfccs);
	return 0;
}

/**
 * Инициализация аудио обработки
 */
void audioProcessing_Init()
{
	arm_rfft_fast_init_f32(&fftS1024, AUDIO_PROCESSING_FFT_SIZE * 2);
	arm_rfft_fast_init_f32(&fftS512, AUDIO_PROCESSING_FFT_SIZE);

	spectrS.pRfft    = &fftS1024;
	spectrS.Type     = SPECTRUM_TYPE_POWER;
	spectrS.pWindow  = (float32_t *) hannWin_1024;
	spectrS.SampRate = 16000;
	spectrS.FrameLen = AUDIO_PROCESSING_FFT_SIZE;
	spectrS.FFTLen   = AUDIO_PROCESSING_FFT_SIZE;
	spectrS.pScratch = NULL;
//	spectrS.pScratch = aProcessingBuffer; //unused

	melfS.pStartIndices = (uint32_t *) melFiltersStartIndices_1024_30;
	melfS.pStopIndices  = (uint32_t *) melFiltersStopIndices_1024_30;
	melfS.pCoefficients = (float32_t *) melFilterLut_1024_30;
//	melfS.pStartIndices = (uint32_t *) melFiltersStart;
//	melfS.pStopIndices  = (uint32_t *) melFiltersStop;
//	melfS.pCoefficients = (float32_t *) melFiltersLut;
	melfS.NumMels       = AUDIO_PROCESSING_MFCC_COUNT;
//	melfS.FFTLen		= AUDIO_PROCESSING_FFT_SIZE;
//	melfS.SampRate		= 16000;
//	melfS.FMin			= 0.f;
//	melfS.FMax			= 8000.f;
//	melfS.Formula		= MEL_SLANEY;
//	melfS.Normalize		= 0;
//	melfS.Mel2F			= 1;
//	MelFilterbank_Init(&melfS);

	melS.SpectrogramConf = &spectrS;
	melS.MelFilter       = &melfS;

	device.audioFFTProcessing = AUDIOFFTPROCESSING_ENABLE;
//	arm_rfft_fast_init_f32(&fftS, AUDIO_PROCESSING_FFT_SIZE * 2);
//	dct2_init_f32(&S_DCT, AUDIO_PROCESSING_MFCC_NUM_FILTERS);
//	hann(AUDIO_PROCESSING_FFT_SIZE);
//	generate_filters();
}



