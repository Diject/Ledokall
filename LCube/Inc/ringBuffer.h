#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include "stdint.h"

typedef struct
{
	int16_t *buffer; //Указатель на массив, хранящий данные буфера
	uint32_t size; //Размер буфера
	uint32_t dataSize;
	uint32_t startPos;
	uint32_t endPos;
} ringBuffer_Int16;

typedef struct
{
	uint8_t *buffer; //Указатель на массив, хранящий данные буфера
	uint32_t size; //Размер буфера
	uint32_t dataSize; //Количество записанных данных в буфере
	uint32_t startPos;
	uint32_t endPos;
} ringBuffer_UInt8;

typedef enum
{
	RINGBUFFER_OK = 0,
	RINGBUFFER_OVERFLOW,
	RINGBUFFER_ERROR,
} ringBufferStatus;

void RingBuffer_Int16_Init(ringBuffer_Int16 *buffer, int16_t *data, int32_t size);
ringBufferStatus RingBuffer_Int16_Write(ringBuffer_Int16 *buffer, int16_t *data, int32_t size);
ringBufferStatus RingBuffer_Int16_Read(ringBuffer_Int16 *buffer, int16_t *data, int32_t size);
ringBufferStatus RingBuffer_Int16_SeekRead(ringBuffer_Int16 *buffer, int16_t *data, int32_t readSize, int32_t shift);

uint32_t RingBuffer_DataSize(void *buffer);
void RingBuffer_Clear(void *buffer);
ringBufferStatus RingBuffer_Delete(void *buffer, uint32_t count);

void RingBuffer_UInt8_Init(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t size);
ringBufferStatus RingBuffer_UInt8_Write(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t size);
ringBufferStatus RingBuffer_UInt8_SeekRead(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t readSize, int32_t shift);


#endif
