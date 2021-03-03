
#include "I2C_Devices.h"

uint8_t DMA_Tx_Buf[2] = {0};
uint8_t DMA_Rx_Buf[4] = {0};

float sensorTemperature = 0.f;
float sensorHumidity = 0.f;

void HDC1080_Process(float *temperature, float *humidity);

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C3)
	{
		HAL_DMA_Abort_IT(hi2c->hdmatx);
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C3)
	{
		HAL_DMA_Abort_IT(hi2c->hdmatx);
		HDC1080_Process(&sensorTemperature, &sensorHumidity);
	}
}

HAL_StatusTypeDef MCP40D17_SetResistanceCode(uint8_t code)
{
	uint8_t data[2] = {0, code};
	return HAL_I2C_Master_Transmit(&hi2c3, MCP40D17_I2C_ADDRESS << 1, data, 2, 2);
}

inline HAL_I2C_StateTypeDef I2C_Ready_State()
{
	return HAL_I2C_GetState(&hi2c3);
}

HAL_StatusTypeDef MCP40D17_SetResistanceCode_DMA(uint8_t code)
{
	if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) return HAL_ERROR;
	DMA_Tx_Buf[0] = 0;
	DMA_Tx_Buf[1] = code;
	return HAL_I2C_Master_Transmit_IT(&hi2c3, MCP40D17_I2C_ADDRESS << 1, DMA_Tx_Buf, 2);
}

HAL_StatusTypeDef HDC1080_ReadData(float *temperature, float *humidity)
{
	uint8_t data[4] = {0};
	uint16_t temp = 0;
	uint16_t hum = 0;
	HAL_StatusTypeDef ret;
	ret = HAL_I2C_Master_Receive(&hi2c3, HDC1080_I2C_ADDRESS << 1, data, 4, 8);
	temp = (data[0] << 8) | data[1];
	hum = (data[2] << 8) | data[3];
	*temperature = ((float)temp / 0x10000) * 165.f - 40.f;
	*humidity = (float)hum / 0x10000;
	return ret;
}

HAL_StatusTypeDef HDC1080_ReadData_TriggerDMAReceive()
{
	if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) return HAL_ERROR;
	return HAL_I2C_Master_Receive_IT(&hi2c3, HDC1080_I2C_ADDRESS << 1, DMA_Rx_Buf, 4);
}

void HDC1080_Process(float *temperature, float *humidity)
{
	uint16_t temp = (DMA_Rx_Buf[0] << 8) | DMA_Rx_Buf[1];
	uint16_t hum = (DMA_Rx_Buf[2] << 8) | DMA_Rx_Buf[3];
	*temperature = ((float)temp / 0x10000) * 165.f - 40.f;
	*humidity = (float)hum / 0x10000;
}

HAL_StatusTypeDef HDC1080_TriggerMeasurement()
{
	if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) return HAL_ERROR;
	DMA_Tx_Buf[0] = 0;
	return HAL_I2C_Master_Transmit_IT(&hi2c3, HDC1080_I2C_ADDRESS << 1, DMA_Tx_Buf, 1);
}

void HDC1080_ReadData_FromLastDMATransfer(float *temperature, float *humidity)
{
	*temperature = sensorTemperature;
	*humidity = sensorHumidity;
}
