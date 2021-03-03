
#include "ringBuffer.h"

ringBufferStatus RingBuffer_Int16_Write(ringBuffer_Int16 *buffer, int16_t *data, int32_t size)
{
	if (size > buffer->size) return RINGBUFFER_ERROR;
	ringBufferStatus ret = RINGBUFFER_OK;
	int32_t end = buffer->endPos;

	for (uint32_t i = size; i != 0; i--)
	{
		buffer->buffer[buffer->endPos++] = *data++;
		if (buffer->endPos == buffer->size) buffer->endPos = 0;
	}

	if (((buffer->startPos + buffer->size) <= (end + size)) || ((end < buffer->startPos) && (buffer->endPos >= buffer->startPos)))
	{
		buffer->startPos = buffer->endPos + 1;
		if (buffer->startPos >= buffer->size) buffer->startPos = buffer->size - buffer->startPos;
		ret = RINGBUFFER_OVERFLOW;
	}

	buffer->dataSize += size;
	if (buffer->dataSize > buffer->size) buffer->dataSize = buffer->size;
	return ret;
}

ringBufferStatus RingBuffer_Int16_Read(ringBuffer_Int16 *buffer, int16_t *data, int32_t size)
{
	if (size > buffer->size) return RINGBUFFER_ERROR;
	if (((buffer->endPos >= buffer->startPos) && (buffer->endPos - buffer->startPos < size)) ||
			((buffer->endPos < buffer->startPos) && ((buffer->size - buffer->startPos + buffer->endPos) < size)))
	{
		return RINGBUFFER_ERROR;
	}

	for (uint32_t i = size; i != 0; i--)
	{
		*data++ = buffer->buffer[buffer->startPos++];
		if (buffer->startPos == buffer->size) buffer->startPos = 0;
	}

	buffer->dataSize -= size;

	return RINGBUFFER_OK;
}

/**
 * Читает (readSize) данных и сдвигает конец буфера на (shift) элементов
 */
ringBufferStatus RingBuffer_Int16_SeekRead(ringBuffer_Int16 *buffer, int16_t *data, int32_t readSize, int32_t shift)
{
	if (readSize > buffer->size) return RINGBUFFER_ERROR;
	if (((buffer->endPos >= buffer->startPos) && (buffer->endPos - buffer->startPos < readSize)) ||
			((buffer->endPos < buffer->startPos) && ((buffer->size - buffer->startPos + buffer->endPos) < readSize)))
	{
		return RINGBUFFER_ERROR;
	}

	uint32_t pos = buffer->startPos;

	for (uint32_t i = readSize; i != 0; i--)
	{
		*data++ = buffer->buffer[pos++];
		if (pos == buffer->size) pos = 0;
	}

	buffer->startPos += shift;
	if (buffer->startPos >= buffer->size) buffer->startPos = buffer->startPos - buffer->size;

	buffer->dataSize -= shift;

	return RINGBUFFER_OK;
}

ringBufferStatus RingBuffer_Delete(void *buffer, uint32_t count)
{
	ringBuffer_Int16 *buf = (ringBuffer_Int16 *)buffer;
	buf->startPos += count;
	if (buf->startPos >= buf->size) buf->startPos = buf->startPos - buf->size;
	if (buf->startPos > buf->endPos)
	{
		buf->startPos = buf->endPos;
		buf->dataSize = 0;
		return RINGBUFFER_OVERFLOW;
	}
	buf->dataSize -= count;
	return RINGBUFFER_OK;
}

void RingBuffer_Int16_Init(ringBuffer_Int16 *buffer, int16_t *data, int32_t size)
{
	buffer->buffer = data;
	buffer->size = size;
	buffer->startPos = 0;
	buffer->endPos = 0;
	buffer->dataSize = 0;
}

uint32_t RingBuffer_DataSize(void *buffer)
{
	ringBuffer_Int16 *buf = (ringBuffer_Int16 *)buffer;
	uint32_t ret = 0;
	if (buf->endPos > buf->startPos) ret = buf->endPos - buf->startPos;
	else if (buf->endPos < buf->startPos) ret = buf->size - buf->startPos + buf->endPos;

	return ret;
}

void RingBuffer_Clear(void *buffer)
{
	ringBuffer_Int16 *buf = (ringBuffer_Int16 *)buffer;
	buf->startPos = 0;
	buf->endPos = 0;
}

ringBufferStatus RingBuffer_UInt8_Write(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t size)
{
	if (size > buffer->size) return RINGBUFFER_ERROR;
	ringBufferStatus ret = RINGBUFFER_OK;
	int32_t end = buffer->endPos;

	for (uint32_t i = size; i != 0; i--)
	{
		buffer->buffer[buffer->endPos++] = *data++;
		if (buffer->endPos == buffer->size) buffer->endPos = 0;
	}

	if (((buffer->startPos + buffer->size) <= (end + size)) || ((end < buffer->startPos) && (buffer->endPos >= buffer->startPos)))
	{
		buffer->startPos = buffer->endPos + 1;
		if (buffer->startPos >= buffer->size) buffer->startPos = buffer->size - buffer->startPos;
		ret = RINGBUFFER_OVERFLOW;
	}

	buffer->dataSize += size;
	if (buffer->dataSize > buffer->size) buffer->dataSize = buffer->size;
	return ret;
}

/**
 * Читает (readSize) данных и сдвигает конец буфера на (shift) элементов
 */
ringBufferStatus RingBuffer_UInt8_SeekRead(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t readSize, int32_t shift)
{
	if (readSize > buffer->size) return RINGBUFFER_ERROR;
	if (((buffer->endPos >= buffer->startPos) && (buffer->endPos - buffer->startPos < readSize)) ||
			((buffer->endPos < buffer->startPos) && ((buffer->size - buffer->startPos + buffer->endPos) < readSize)))
	{
		return RINGBUFFER_ERROR;
	}

	uint32_t pos = buffer->startPos;

	for (uint32_t i = readSize; i != 0; i--)
	{
		*data++ = buffer->buffer[pos++];
		if (pos == buffer->size) pos = 0;
	}

	buffer->startPos += shift;
	if (buffer->startPos >= buffer->size) buffer->startPos = buffer->startPos - buffer->size;

	buffer->dataSize -= shift;

	return RINGBUFFER_OK;
}

ringBufferStatus RingBuffer_UInt8_Delete(ringBuffer_UInt8 *buffer, uint32_t count)
{
	buffer->startPos += count;
	if (buffer->startPos >= buffer->size) buffer->startPos = buffer->startPos - buffer->size;
	if (buffer->startPos > buffer->endPos)
	{
		buffer->startPos = buffer->endPos;
		buffer->dataSize = 0;
		return RINGBUFFER_OVERFLOW;
	}
	buffer->dataSize -= count;
	return RINGBUFFER_OK;
}

void RingBuffer_UInt8_Init(ringBuffer_UInt8 *buffer, uint8_t *data, int32_t size)
{
	buffer->buffer = data;
	buffer->size = size;
	buffer->startPos = 0;
	buffer->endPos = 0;
	buffer->dataSize = 0;
}


