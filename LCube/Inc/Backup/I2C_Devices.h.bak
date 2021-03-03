#ifndef MCP40D17_H_
#define MCP40D17_H_

#include "i2c.h"
#include "dma.h"
#include "stdbool.h"

#define MCP40D17_FULLSCALE_RESISTANCE 100000U
#define MCP40D17_STEP_RESISTANCE ((MCP40D17_FULLSCALE_RESISTANCE) / 0x7f)
#define MCP40D17_I2C_ADDRESS 0b0101110

#define HDC1080_I2C_ADDRESS 0b1000000

HAL_I2C_StateTypeDef I2C_Ready_State();

HAL_StatusTypeDef MCP40D17_SetResistanceCode(uint8_t code);
HAL_StatusTypeDef MCP40D17_SetResistanceCode_DMA(uint8_t code);

HAL_StatusTypeDef HDC1080_ReadData(float *temperature, float *humidity);
void HDC1080_ReadData_FromLastDMATransfer(float *temperature, float *humidity);
HAL_StatusTypeDef HDC1080_ReadData_TriggerDMAReceive();
HAL_StatusTypeDef HDC1080_TriggerMeasurement();

#endif
