#ifndef CONSOLECOMMANDS_H_
#define CONSOLECOMMANDS_H_

#include "main.h"
#include "string.h"

#include "ringBuffer.h"

#include "ConsoleCommandsFunc.h"

#define CONSOLECOMMANDS_BUFER_SIZE 256U
#define CONSOLECOMMANDS_COMMANDS_COUNT 6U

typedef struct
{
	uint8_t *command;
	void (*func)(uint8_t *);
} console_command;

ringBufferStatus Console_AddToBuffer(uint8_t *data, uint32_t size);
void Console_DataProcessing();


#endif
