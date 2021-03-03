
#include <Console_Commands/ConsoleCommands.h>

const uint8_t help_command_descr[] = "-help";
const uint8_t settime_command_descr[] = "-set_time";
const uint8_t setdate_command_descr[] = "-set_date";
const uint8_t setRTCCalibration_command_descr[] = "-set_rtc_calibration";
const uint8_t getRTCCalibration_command_descr[] = "-get_rtc_calibration";
const uint8_t setAudioMessageVolume_command_descr[] = "-set_audio_messages_volume";

console_command comsole_commands[CONSOLECOMMANDS_COMMANDS_COUNT] = {
		{.command = help_command_descr, .func = &ConsoleCommandsFunc_Help},
		{.command = settime_command_descr, .func = &ConsoleCommandsFunc_SetTime},
		{.command = setdate_command_descr, .func = &ConsoleCommandsFunc_SetDate},
		{.command = setRTCCalibration_command_descr, .func = &ConsoleCommandsFunc_SetRTCCalibration},
		{.command = getRTCCalibration_command_descr, .func = &ConsoleCommandsFunc_GetRTCCalibration},
		{.command = setAudioMessageVolume_command_descr, .func = &ConsoleCommandsFunc_SetMessageVolume},
};

uint8_t commandsBuferData[CONSOLECOMMANDS_BUFER_SIZE] = {0};
ringBuffer_UInt8 commandsBuffer = {
		.buffer = commandsBuferData,
		.size = CONSOLECOMMANDS_BUFER_SIZE,
		.dataSize = 0,
		.endPos = 0,
		.startPos = 0,

};


inline ringBufferStatus Console_AddToBuffer(uint8_t *data, uint32_t size)
{
	return RingBuffer_UInt8_Write(&commandsBuffer, data, size);
}

void Console_CommandProcessing(uint8_t *data, uint32_t size)
{
	char str[32] = {0};
	*(data + size - 1) = 0;
	sscanf((char *)data, "%s", str);
	for (uint32_t i = 0; i < CONSOLECOMMANDS_COMMANDS_COUNT; i++)
	{
		if (strstr(str, (char *)comsole_commands[i].command) != NULL)
		{
			comsole_commands[i].func(data);
			break;
		}
	}
}

void Console_DataProcessing()
{
	uint8_t *data = malloc(CONSOLECOMMANDS_BUFER_SIZE);
	uint32_t size = commandsBuffer.dataSize;
	RingBuffer_UInt8_SeekRead(&commandsBuffer, data, size, 0);

	uint32_t processedSize = 0;

	for (uint32_t i = 1; i < size; i++)
	{
		if (data[i - 1] == '\r' && data[i] == '\n')
		{
			Console_CommandProcessing(data + processedSize, i + 1);
			processedSize = i + 1;
		}
	}
	RingBuffer_Delete(&commandsBuffer, processedSize);
	free(data);
}
