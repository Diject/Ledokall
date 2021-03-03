#ifndef WAVEPLAYER_H_
#define WAVEPLAYER_H_

#include "device.h"
#include "fatfs.h"
#include "i2s.h"
#include "usb_device.h"
#include "usbd_audio.h"

#define AUDIO_PLAYER_BUFFER_SIZE (2048U * 2U)
#define AUDIO_PLAYER_MAXIMUM_COMMANDS_NUMBER 20

#define AUDIO_PLAYER_COMMAND_SILENCE_LABEL (1 << 8)
#define AUDIO_PLAYER_COMMAND_WORD_LABEL (1 << 9)
#define AUDIO_PLAYER_COMMAND_NUMBER_LABEL (1 << 10)
#define AUDIO_PLAYER_COMMAND_DECLIN_NUMBER_LABEL (1 << 11)

//Команда добавления тишины. s - количество блоков
#define AUDIO_PLAYER_ADD_SILENCE(s) (AUDIO_PLAYER_COMMAND_SILENCE_LABEL | s)
#define AUDIO_PLAYER_ADD_WORD(id) (AUDIO_PLAYER_COMMAND_WORD_LABEL | id)

typedef struct
{
	int16_t *buffer;
	uint32_t bufferSize;
	uint16_t *commands;
	uint32_t commandsSize;
	uint32_t commandPos;
} wavePlayer_PlaybackData;

void wavePlayer_PlayTime(uint8_t hours, uint8_t minutes);
void wavePlayer_PlayHumidity(float value);
void wavePlayer_PlayTemperature(float value);

void wavePlayer_StartPlayback(wavePlayer_PlaybackData *playbackData);
void wavePlayer_StopPlayback(wavePlayer_PlaybackData *playbackData);

void wavePlayer_FullTransferCallback();
void wavePlayer_HalfTransferCallback();

void wavePlayer_PlayJoke();

//FRESULT wavePlayer_InitRecording();
//FRESULT wavePlayer_Recording_Write(const uint8_t *data, uint32_t size, uint32_t detect);
//void wavePlayer_CloseRecord();

#endif /* WAVEPLAYER_H_ */
