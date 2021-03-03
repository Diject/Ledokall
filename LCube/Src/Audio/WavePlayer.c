
#include "WavePlayer.h"
#include "stdio.h"
#include "string.h"
#include "sdio.h"

const uint8_t waveHeader_16k[] = {
		0x52, 0x49, 0x46, 0x46, 0x24, 0x78, 0x00, 0x00,
		0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x80, 0x3e, 0x00, 0x00, 0x00, 0x7d, 0x00, 0x00,
		0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
		0x00, 0x78, 0x00, 0x00,
};

extern USBD_HandleTypeDef hUsbDeviceFS;

int16_t audio_buffer[AUDIO_PLAYER_BUFFER_SIZE] = {0};
uint16_t commands_buffer[AUDIO_PLAYER_MAXIMUM_COMMANDS_NUMBER] = {0};
wavePlayer_PlaybackData plData = {
		.buffer = audio_buffer,
		.bufferSize = AUDIO_PLAYER_BUFFER_SIZE,
		.commands = commands_buffer,
		.commandPos = 0,
		.commandsSize = 0,
};

wavePlayer_PlaybackData *playbackDataHandle = &plData;

neuralNetworkState NN_Last_State;
uint32_t fileOpened = 0;
FIL* waveFile = NULL;
uint32_t recordsCount = 0;
int32_t recordLen = 0;

/**
 * Заполнение буфера воспроизведения
 * *playbackData - структура с настройками
 * *array - указатель на буфер воспроизведения
 * size - количество аудио семплов (int16_t) для записи
 */
uint32_t wavePlayer_Playback_Update(wavePlayer_PlaybackData *playbackData, int16_t *array, uint32_t size)
{
	if (playbackData->commandPos == playbackData->commandsSize) return 1;
	uint32_t command = playbackData->commands[playbackData->commandPos];
	if (command & AUDIO_PLAYER_COMMAND_SILENCE_LABEL) //Воспоизведение тишины
	{
		memset(array, 0, size * sizeof(uint16_t));
		command = ((command & 0xff) - 1);
		playbackData->commands[playbackData->commandPos] = command | AUDIO_PLAYER_COMMAND_SILENCE_LABEL;
		if (!command) playbackData->commandPos++;
	}else if (command & (AUDIO_PLAYER_COMMAND_WORD_LABEL | AUDIO_PLAYER_COMMAND_NUMBER_LABEL | AUDIO_PLAYER_COMMAND_DECLIN_NUMBER_LABEL)) //Воспроизведение файлов
	{
		volatile FRESULT res;
		if (!fileOpened)
		{
			char str[32] = {0};
			if (command & AUDIO_PLAYER_COMMAND_WORD_LABEL) sprintf(str, "0:/NN_Words/w%02lu.wav", command & 0xff); //Маска для слов
			else if (command & AUDIO_PLAYER_COMMAND_NUMBER_LABEL) sprintf(str, "0:/NN_Words/n%02lu.wav", command & 0xff); //Маска для чисел
			else sprintf(str, "0:/NN_Words/e%02lu.wav", command & 0xff); //Маска для склоняемых чисел
			res = f_open(&SDFile, str, FA_READ);
			if (res == FR_OK) fileOpened = 1;
			if (fileOpened) res = f_lseek(&SDFile, 44);
			if (res != FR_OK) fileOpened = 0;
		}
		uint32_t readNum = 0;
		uint32_t readSize = size * sizeof(int16_t);
		if (fileOpened) res = f_read(&SDFile, array, readSize, &readNum);
		if (readNum != readSize)
		{
			f_close(&SDFile);
			fileOpened = 0;
			memset(((uint8_t *)array) + readNum, 0, readSize - readNum);
			playbackData->commandPos++;
		}
		if (device.audio.messageVolume < 1.f) //Применение громкости
		{
			for (uint32_t i = 0; i < size; i++)
			{
				array[i] *= device.audio.messageVolume;
			}
		}
	}
	if (playbackData->commandPos == playbackData->commandsSize) return 1;
	return 0;
}

/**
 * Инициализация воспроизведения
 */
void wavePlayer_StartPlayback(wavePlayer_PlaybackData *playbackData)
{
	if (device.audio.muteVoice) return;
	NN_Last_State = device.neuralNetworkStatus;
	device.neuralNetworkStatus = NEURAL_NETWORK_PAUSE;
	device.audio.customAudioTransmission = AUDIOTRANSMISSION_RUNS;
	if (HAL_I2S_GetState(&hi2s1) != HAL_I2S_STATE_READY)
	{
		HAL_I2S_DMAStop(&hi2s1);
	}
	memset(playbackData->buffer, 0, playbackData->bufferSize * sizeof(int16_t));
	HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)playbackData->buffer, playbackData->bufferSize);
}

/**
 * Остановка воспроизведения
 */
void wavePlayer_StopPlayback(wavePlayer_PlaybackData *playbackData)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS)
	{
		HAL_I2S_DMAStop(&hi2s1);
		playbackData->commandsSize = 0;
		playbackData->commandPos = 0;
		if (device.neuralNetworkStatus == NEURAL_NETWORK_PAUSE) device.neuralNetworkStatus = NN_Last_State;
	}
	device.audio.customAudioTransmission = AUDIOTRANSMISSION_STOPPED;
	if (device.audio.USBAudioTransmission == AUDIOTRANSMISSION_RUNS)
	{
		USBD_AUDIO_Restart(&hUsbDeviceFS); //Перезапуск usb-аудио
	}
}

void wavePlayer_HalfTransferCallback()
{
	if (wavePlayer_Playback_Update(playbackDataHandle, playbackDataHandle->buffer, playbackDataHandle->bufferSize / 2) || device.audio.muteVoice)
	{
		wavePlayer_StopPlayback(playbackDataHandle);
	}
}

void wavePlayer_FullTransferCallback()
{
	if (wavePlayer_Playback_Update(playbackDataHandle, playbackDataHandle->buffer + playbackDataHandle->bufferSize / 2, playbackDataHandle->bufferSize / 2) || device.audio.muteVoice)
	{
		wavePlayer_StopPlayback(playbackDataHandle);
	}
}

/**
 * Перевод из числа в команду для его произношения
 * number - число
 * *commands - указатель на начало массива с командами
 * *commandPos - указатель на позицию в массиве команд
 * decl - использовать ли склонение в произношении
 */
void wavePlayer_NumberToCommand(uint8_t number, uint16_t *commands, uint32_t *commandPos, bool decl)
{
	if (number > 20)
	{
		commands[(*commandPos)++] = AUDIO_PLAYER_COMMAND_NUMBER_LABEL | ((number / 10) * 10);
		if ((number % 10) != 0)
		{
			uint16_t label = (decl) ? AUDIO_PLAYER_COMMAND_DECLIN_NUMBER_LABEL : AUDIO_PLAYER_COMMAND_NUMBER_LABEL;
			commands[(*commandPos)++] = label | (number % 10);
		}
	}else
	{
		int16_t label = (decl && (number < 10)) ? AUDIO_PLAYER_COMMAND_DECLIN_NUMBER_LABEL : AUDIO_PLAYER_COMMAND_NUMBER_LABEL;
		commands[(*commandPos)++] = label | number;
	}
}

void wavePlayer_PlayTime(uint8_t hours, uint8_t minutes)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) return;
	uint32_t wordPos = 0;
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);
	wavePlayer_NumberToCommand(hours, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(2);

	if ((hours % 10) == 0 || (hours % 10) >= 5 || ((hours < 20) && (hours > 10))) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(0);
	else if ((hours % 10) == 1) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(1);
	else playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(2);

	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(3);
	wavePlayer_NumberToCommand(minutes, playbackDataHandle->commands, &wordPos, 1);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(2);

	if ((minutes % 10) == 0 || (minutes % 10) >= 5 || ((minutes < 20) && (minutes > 10))) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(3);
	else if ((minutes % 10) == 1) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(4);
	else playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(5);

	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);

	playbackDataHandle->commandsSize = wordPos;
	playbackDataHandle->commandPos = 0;

	wavePlayer_StartPlayback(playbackDataHandle);
}

void wavePlayer_PlayHumidity(float value)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) return;
	uint32_t wordPos = 0;
	uint32_t val = (uint32_t)(value * 100.f + 0.5f);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);
	wavePlayer_NumberToCommand(val, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(2);

	if ((val % 10) == 0 || (val % 10) >= 5 || ((val < 20) && (val > 10))) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(9);
	else if ((val % 10) == 1) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(10);
	else playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(11);

	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);

	playbackDataHandle->commandsSize = wordPos;
	playbackDataHandle->commandPos = 0;

	wavePlayer_StartPlayback(playbackDataHandle);
}

void wavePlayer_PlayTemperature(float value)
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) return;
	uint32_t wordPos = 0;
	uint32_t val = (uint32_t)(value + 0.5f);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);
	wavePlayer_NumberToCommand(val, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(2);

	if ((val % 10) == 0 || (val % 10) >= 5 || ((val < 20) && (val > 10))) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(6);
	else if ((val % 10) == 1) playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(7);
	else playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(8);

	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);

	playbackDataHandle->commandsSize = wordPos;
	playbackDataHandle->commandPos = 0;

	wavePlayer_StartPlayback(playbackDataHandle);
}

void wavePlayer_PlayJoke()
{
	if (device.audio.customAudioTransmission == AUDIOTRANSMISSION_RUNS) return;
	uint32_t wordPos = 0;
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(50);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);
	wavePlayer_NumberToCommand(3, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(25);
	wavePlayer_NumberToCommand(2, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(25);
	wavePlayer_NumberToCommand(1, playbackDataHandle->commands, &wordPos, 0);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(45);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_WORD(51);
	playbackDataHandle->commands[wordPos++] = AUDIO_PLAYER_ADD_SILENCE(10);

	playbackDataHandle->commandsSize = wordPos;
	playbackDataHandle->commandPos = 0;

	wavePlayer_StartPlayback(playbackDataHandle);
}
