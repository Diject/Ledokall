/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_audio.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_AUDIO_H
#define __USB_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include "usbd_cdc.h"
#include "ringBuffer.h"
#include "Device.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_AUDIO
  * @brief This file is the Header file for usbd_audio.c
  * @{
  */


/** @defgroup USBD_AUDIO_Exported_Defines
  * @{
  */
#ifndef USBD_AUDIO_FREQ
/* AUDIO Class Config */
#define USBD_AUDIO_FREQ                               48000U
#endif /* USBD_AUDIO_FREQ */

#ifndef USBD_MAX_NUM_INTERFACES
#define USBD_MAX_NUM_INTERFACES                       1U
#endif /* USBD_AUDIO_FREQ */

#ifndef AUDIO_HS_BINTERVAL
#define AUDIO_HS_BINTERVAL                            0x01U
#endif /* AUDIO_HS_BINTERVAL */

#ifndef AUDIO_FS_BINTERVAL
#define AUDIO_FS_BINTERVAL                            0x01U
#endif /* AUDIO_FS_BINTERVAL */

#define AUDIO_OUT_EP                                  0x01U
#define AUDIO_IN_EP                                  0x81U
//#define USB_AUDIO_CONFIG_DESC_SIZ                     0xB7U
#define USB_AUDIO_CONFIG_DESC_SIZ 					  (222U + 155U)
//#define USB_AUDIO_CONFIG_DESC_SIZ 					  296U
#define AUDIO_INTERFACE_DESC_SIZE                     0x09U
#define USB_AUDIO_DESC_SIZ                            0x09U
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09U
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07U

#define AUDIO_DESCRIPTOR_TYPE                         0x21U
#define USB_DEVICE_CLASS_AUDIO                        0x01U
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01U
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02U
#define AUDIO_PROTOCOL_UNDEFINED                      0x00U
#define AUDIO_STREAMING_GENERAL                       0x01U
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02U

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24U
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25U

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01U
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02U
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03U
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06U

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0CU
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09U
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07U

#define AUDIO_CONTROL_MUTE                            0x0001U

#define AUDIO_FORMAT_TYPE_I                           0x01U
#define AUDIO_FORMAT_TYPE_III                         0x03U

#define AUDIO_ENDPOINT_GENERAL                        0x01U

#define AUDIO_REQ_GET_CUR                             0x81U
#define AUDIO_REQ_SET_CUR                             0x01U

#define AUDIO_OUT_STREAMING_CTRL                      0x02U

#define AUDIO_OUT_TC                                  0x01U
#define AUDIO_IN_TC                                   0x02U


#define AUDIO_OUT_PACKET                              (uint16_t)(((USBD_AUDIO_FREQ * 2U * 2U) / 1000U))
#define AUDIO_DEFAULT_VOLUME                          70U

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define AUDIO_OUT_PACKET_NUM                          80U
/* Total size of the audio transfer buffer */
#define AUDIO_TOTAL_BUF_SIZE                          ((uint16_t)(AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM))





#define USB_AUDIO_CONFIG_RECORD_DEF_FREQ                16000
#define USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE        ((USB_AUDIO_CONFIG_RECORD_DEF_FREQ * 2U * 1U) / 1000U)

#define USB_ATTRIBUTE_DEFAULT                       0x80  // Reserved (set to one)
#define USB_ATTRIBUTE_SELF_POWERED                  0x40  // Self powered

#define USB_DESCRIPTOR_DEVICE           0x01    // bDescriptorType for a Device Descriptor.
#define USB_DESCRIPTOR_CONFIGURATION    0x02    // bDescriptorType for a Configuration Descriptor.
#define USB_DESCRIPTOR_STRING           0x03    // bDescriptorType for a String Descriptor.
#define USB_DESCRIPTOR_INTERFACE        0x04    // bDescriptorType for an Interface Descriptor.
#define USB_DESCRIPTOR_ENDPOINT         0x05    // bDescriptorType for an Endpoint Descriptor.
#define USB_DESCRIPTOR_DEVICE_QUALIFIER 0x06    // bDescriptorType for a Device Qualifier.
#define USB_DESCRIPTOR_OTHER_SPEED      0x07    // bDescriptorType for a Other Speed Configuration.
#define USB_DESCRIPTOR_INTERFACE_POWER  0x08    // bDescriptorType for Interface Power.
#define USB_DESCRIPTOR_OTG              0x09    // bDescriptorType for an OTG Descriptor.
#define USB_DESCRIPTOR_INTERFACE_ASSOCIATION  0x0b
#define USB_DESCRIPTOR_BOS              0x0F    // bDescriptorType for a BOS Descriptor.
#define USB_DESCRIPTOR_DEVICE_CAPABILITY 0x10   // bDescriptorType for a Device Capability Descriptor.

#define  USB_AUDIO_CLASS_CODE 0x01
#define USB_AUDIO_AUDIOCONTROL        0x01
#define USB_AUDIO_PR_PROTOCOL_UNDEFINED    0x0

typedef enum
{
    USB_AUDIO_CS_UNDEFINED       = 0x20,
    USB_AUDIO_CS_DEVICE          = 0x21,
    USB_AUDIO_CS_CONFIGURATION   = 0x22,
    USB_AUDIO_CS_STRING          = 0x23,
    USB_AUDIO_CS_INTERFACE       = 0x24,
    USB_AUDIO_CS_ENDPOINT        = 0x25

} USB_AUDIO_CS_DESCRIPTOR_TYPE;

typedef enum
{
    USB_AUDIO_AC_DESCRIPTOR_UNDEFINED    = 0x00,
    USB_AUDIO_HEADER                     = 0x01,
    USB_AUDIO_INPUT_TERMINAL             = 0x02,
    USB_AUDIO_OUTPUT_TERMINAL            = 0x03,
    USB_AUDIO_MIXER_UNIT                 = 0x04,
    USB_AUDIO_SELECTOR_UNIT              = 0x05,
    USB_AUDIO_FEATURE_UNIT               = 0x06,
    USB_AUDIO_PROCESSING_UNIT            = 0x07,
    USB_AUDIO_EXTENSION_UNIT             = 0x08,

} USB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR_SUBTYPE;

#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_CONTROL_INTERFACE_ID           0x00
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_ID              0x01
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_ID                0x02
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_OUTPUT_TERMINAL_ID                0x03
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_MICROPHONE_ID      0x04
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_MICROPHONE_ID        0x05
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_OUTPUT_TERMINAL_MICROPHONE_ID     0x06
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_SIDE_TONING_ID       0x07
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_MIXER_UNIT_ID                     0x08
#define USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_STREAMING_INTERFACE_ID_1       0x01

#define USB_AUDIO_AUDIOSTREAMING        0x02

typedef enum
{
    USB_AUDIO_AS_DESCRIPTOR_UNDEFINED    = 0x00,
    USB_AUDIO_AS_GENERAL                 = 0x01,
    USB_AUDIO_FORMAT_TYPE                = 0x02,
    USB_AUDIO_FORMAT_SPECIFIC            = 0x03

} USB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR_SUBTYPE;

#define USB_AUDIO_EP_GENERAL              0x01
/* Audio Commands enumeration */
typedef enum
{
  AUDIO_CMD_START = 1,
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
} AUDIO_CMD_TypeDef;


typedef enum
{
  AUDIO_OFFSET_NONE = 0,
  AUDIO_OFFSET_HALF,
  AUDIO_OFFSET_FULL,
  AUDIO_OFFSET_UNKNOWN,
} AUDIO_OffsetTypeDef;
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef struct
{
  uint8_t cmd;
  uint8_t data[USB_MAX_EP0_SIZE];
  uint8_t len;
  uint8_t unit;
} USBD_AUDIO_ControlTypeDef;


typedef struct
{
  uint32_t alt_setting;
  uint8_t buffer[AUDIO_TOTAL_BUF_SIZE];
  AUDIO_OffsetTypeDef offset;
  uint8_t rd_enable;
  int16_t rd_ptr;
  int16_t wr_ptr;
  USBD_AUDIO_ControlTypeDef control;
  uint8_t micTransmittionFlag;

  uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;

  __IO uint32_t TxState;
  __IO uint32_t RxState;
} USBD_AUDIO_HandleTypeDef;


typedef struct
{
  int8_t (*Init)(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
  int8_t (*DeInit)(uint32_t options);
  int8_t (*AudioCmd)(uint8_t *pbuf, uint32_t size, uint8_t cmd);
  int8_t (*VolumeCtl)(uint8_t vol);
  int8_t (*MuteCtl)(uint8_t cmd);
  int8_t (*PeriodicTC)(uint8_t *pbuf, uint32_t size, uint8_t cmd);
  int8_t (*GetState)(void);

  int8_t (* Control)(uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (* Receive)(uint8_t *Buf, uint32_t *Len);
  int8_t (* TransmitCplt)(uint8_t *Buf, uint32_t *Len, uint8_t epnum);
} USBD_AUDIO_ItfTypeDef;
/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_AUDIO;
#define USBD_AUDIO_CLASS &USBD_AUDIO
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops);

void USBD_AUDIO_Sync(USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset, bool pause);

uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef *pdev,
                                   USBD_CDC_ItfTypeDef *fops);

uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff,
                             uint32_t length);

uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev);
uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev);

void USBD_AUDIO_Restart(USBD_HandleTypeDef *pdev);
void USBD_MICROPHONE_AddData(int16_t *data, uint32_t size);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_AUDIO_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
