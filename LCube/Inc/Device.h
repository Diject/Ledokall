#ifndef DEVICE_H_
#define DEVICE_H_

#include "main.h"

#include "LEDVisualization.h"
#include "LEDVisualization_GFX.h"
#include "LEDSymbols.h"

#include "rtc.h"
#include "I2C_Devices.h"

#include "WavePlayer.h"

#define DEVICE_SAVE_AUDIO_MUTE_ID RTC_BKP_DR0
#define DEVICE_SAVE_MESSAGE_MUTE_ID RTC_BKP_DR1
#define DEVICE_SAVE_MESSAGE_VOLUME_ID RTC_BKP_DR2
#define DEVICE_SAVE_VISUALIZATION_ID RTC_BKP_DR3
#define DEVICE_SAVE_MICROPHOME_MUTE_ID RTC_BKP_DR4

typedef enum
{
	AUDIO_INPUT_NONE = 0,
	AUDIO_INPUT_MICROPHONE,
	AUDIO_INPUT_USB,
	AUDIO_INPUT_SDCARD,
} audioInputState;

typedef enum
{
	VOLTAGE_INPUT_UNKNOWN = 0,
	VOLTAGE_INPUT_USBC = 1,
	VOLTAGE_INPUT_MICROUSB = 1 << 1,
} voltageInputState;

typedef enum
{
	SDCARD_UNCONNECTED = 0,
	SDCARD_CONNECTED = 1,
	SDCARD_INIT,
} sdcardConnectionState;

typedef enum
{
	VISUALIZATION_STATE_OFF = 0,
	VISUALIZATION_STATE_MICROPHONE_AUDIO_VIS = 1,
	VISUALIZATION_STATE_USB_AUDIO_VIS = 2,
	VISUALIZATION_STATE_TEMPERATURE_VIS = 3,
	VISUALIZATION_STATE_HUMIDITY_VIS = 4,
	VISUALIZATION_STATE_TIME_VIS = 5,
	VISUALIZATION_STATE_CUSTOM_VIS,
} visualizationType;

typedef enum
{
	USB_INPUT_UNDEFINED = 0,
	USB_INPUT_NONE,
	USB_INPUT_TYPEC,
	USB_INPUT_MICRO,
} USBDataInput;

typedef enum
{
	AUDIO_OUTPUT_HEADPHONES_UNCONNECTED = 0,
	AUDIO_OUTPUT_HEADPHONES_CONNECTED,
} audioOutputConnectionState;

typedef enum
{
	AUDIO_OUTPUT_DISABLE = 0,
	AUDIO_OUTPUT_ENABLE,
} audioOutputState;

typedef enum
{
	NEURAL_NETWORK_DISABLE = 0,
	NEURAL_NETWORK_ENABLE,
	NEURAL_NETWORK_PAUSE,
} neuralNetworkState;

typedef enum
{
	AUDIOTRANSMISSION_STOPPED = 0,
	AUDIOTRANSMISSION_RUNS = 1,
} audioTransmissionState;

typedef enum
{
	AUDIOFFTPROCESSING_DISABLE = 0,
	AUDIOFFTPROCESSING_ENABLE = 1,
} audioFFTProcessingState;

typedef enum
{
	SDCARD_INITIALIZATION_OFF = 0,
	SDCARD_INITIALIZATION_START,
	SDCARD_INITIALIZATION_POWERON,
	SDCARD_INITIALIZATION_MOUNTED,
} sdcardInitializationStep;

typedef struct
{
	audioInputState audioInput;
	audioTransmissionState USBAudioTransmission;
	audioTransmissionState customAudioTransmission;
	FunctionalState muteAudio;
	FunctionalState muteVoice;
	FunctionalState muteMicrophoneOutput;
	uint32_t frequency;
	float messageVolume;
} audioState;

typedef struct
{
	sdcardConnectionState sdcardConnection;
	sdcardInitializationStep sdcardInitialization;
} sdcardState;

typedef struct
{
	audioState audio;
	voltageInputState voltageInput;
	USBDataInput USBInput; //Only for detection process
	sdcardState sdcard;
	FunctionalState visualizationStatus;
	visualizationType visualization;
	neuralNetworkState neuralNetworkStatus;
	uint8_t neuralNetworkId;
	audioFFTProcessingState audioFFTProcessing;
} deviceState;

extern deviceState device;
extern LEDDataModel LEDModel;

void InitDevice();
void InitLEDModel();

void VisualizationPause(uint32_t timeout_ms);
void VisualizationStop(FunctionalState state);

void InitLEDVisualization(visualizationType newVis);
void DeinitLEDVisualization(visualizationType vis);
void VisualizationModelUpdate();

void audioVisualizationDataUpdate(float *fft_res, float fft_freq_step);
void audioVisualizationUpdate();
void audioVisualizationDeinit();

void timeVisualizationUpdate();
void timeVisualizationDeinit();

void temperatureVisualizationUpdate();

void humidityVisualizationUpdate();


void Device_Save_Mute_State();
void Device_Save_Message_Volume();
void Device_Save_Visualization();

//NN
void NN_Action_ShowTimeScreen(uint32_t nn_command);
void NN_Action_ShowMicSpectrogramScreen(uint32_t nn_command);
void NN_Action_ShowTemperatureScreen(uint32_t nn_command);
void NN_Action_ShowHumidityScreen(uint32_t nn_command);
void NN_Action_SayTime(uint32_t nn_command);
void NN_Action_SayTemperature(uint32_t nn_command);
void NN_Action_SayHumidity(uint32_t nn_command);
void NN_Action_MuteSound(uint32_t nn_command);
void NN_Action_UnMuteSound(uint32_t nn_command);
void NN_Action_MuteMicrophone(uint32_t nn_command);
void NN_Action_UnMuteMicrophone(uint32_t nn_command);
void NN_Action_MuteCommands(uint32_t nn_command);
void NN_Action_UnMuteCommands(uint32_t nn_command);
void NN_Action_Joke(uint32_t nn_command);

#endif
