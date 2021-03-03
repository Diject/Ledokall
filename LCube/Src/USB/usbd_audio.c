/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                AUDIO Class  Description
  *          ===================================================================
  *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz.
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
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

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}_audio.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include <usbd_audio.h>
#include <usbd_cdc.h>
#include "usbd_ctlreq.h"
#include "IIR_Filter.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq)         (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2U * 2U)/1000U) & 0xFFU), \
                                       (uint8_t)((((frq * 2U * 2U)/1000U) >> 8) & 0xFFU)
#define AUDIO_MIC_PACKET_SZE(frq)          (uint8_t)(((frq * 2U * 1U)/1000U) & 0xFFU), \
                                       (uint8_t)((((frq * 2U * 1U)/1000U) >> 8) & 0xFFU)
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);

static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

void USBD_MICROPHONE_SEND(USBD_HandleTypeDef *pdev);
/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};

extern uint8_t UserRxBufferFS[2048];
extern uint8_t UserTxBufferFS[2048];
extern deviceState device;
int16_t micTxBuffer[USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2 + 1] = {0};
int16_t micData[256] = {0};
ringBuffer_Int16 micBuffer = {
		.buffer = micData,
		.size = 256,
		.dataSize = 0,
		.endPos = 0,
		.startPos = 0,
};

USBD_AUDIO_HandleTypeDef USBD_AUDIO_Handle = {0};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
	0x09,                                                   // Size of this descriptor in bytes
	USB_DESCRIPTOR_CONFIGURATION,                           // Descriptor Type
	LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength bytes*/
	HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),                   //Size of the Configuration descriptor
	5,                                                      // Number of interfaces in this configuration
	0x01,                                                   // Index value of this configuration
	0x00,                                                   // Configuration string index
	USB_ATTRIBUTE_DEFAULT | USB_ATTRIBUTE_SELF_POWERED, // Attributes
	250U,

	0x08, /* bLength */
	0x0B, /* bDescriptorType */
	0x00, /* bFirstInterface */
	0x03, /* bInterfaceCount */
	USB_AUDIO_CLASS_CODE, /* bFunctionClass */
	0x00, /* bFunctionSubClass */
	0x00, /* bFunctionProtocol */
	0x00, /* iFunction (Index of string descriptor describing this function) */
	/* 08 bytes */
/* Descriptor for Function 1 - Audio     */

	/* USB Headset Standard Audio Control Interface Descriptor          */
	0x09,                            // Size of this descriptor in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,        // INTERFACE descriptor type (bDescriptorType)
	0,
	0x00,                            // Alternate Setting Number (bAlternateSetting)
	0x00,                            // Number of endpoints in this intf (bNumEndpoints)
	USB_AUDIO_CLASS_CODE,            // Class code  (bInterfaceClass)
	USB_AUDIO_AUDIOCONTROL,          // Subclass code (bInterfaceSubclass)
	USB_AUDIO_PR_PROTOCOL_UNDEFINED, // Protocol code  (bInterfaceProtocol)
	0x00,                            // Interface string index (iInterface)

	/* USB Headset Class-specific AC Interface Descriptor  */
	0x0A,                           // Size of this descriptor in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,         // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_HEADER,               // HEADER descriptor subtype      (bDescriptorSubtype)
	0x00,0x01,                      /* Audio Device compliant to the USB Audio
									 * specification version 1.00 (bcdADC) */

	0x64,0x00,                      /* Total number of bytes returned for the
									 * class-specific AudioControl interface
									 * descriptor. (wTotalLength). Includes the
									 * combined length of this descriptor header
									 * and all Unit and Terminal descriptors. */

	2,                              /* The number of AudioStreaming interfaces
									 * in the Audio Interface Collection to which
									 * this AudioControl interface belongs
									 * (bInCollection) */

	1,                              /* AudioStreaming interface 1 belongs to this
									 * AudioControl interface. (baInterfaceNr(1))*/
	2,                              /* AudioStreaming interface 2 belongs to this
									 * AudioControl interface. (baInterfaceNr(2))*/

	/* USB Headset Input Terminal Descriptor */
	0x0C,                           // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,         // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_INPUT_TERMINAL,                // INPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_ID,          // ID of this Terminal.(bTerminalID)
	0x01,0x01,                      // (wTerminalType)
	0x00,                           // No association (bAssocTerminal)
	0x02,                           // Two Channels (bNrChannels)
	0x03,0x00,                      // (wChannelConfig)
	0x00,                           // Unused.(iChannelNames)
	0x00,                           // Unused. (iTerminal)

	/* USB Headset Microphone Input Terminal Descriptor */
	0x0C,                           // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,         // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_INPUT_TERMINAL,                // INPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_MICROPHONE_ID,          // ID of this Terminal.(bTerminalID)
	0x01,0x02,                      // (wTerminalType)
	0x00,                           // No association (bAssocTerminal)
	0x01,                           // Two Channels (bNrChannels)
	0x04,0x00,                      // (wChannelConfig) 0x0004
	0x00,                           // Unused.(iChannelNames)
	0x00,                           // Unused. (iTerminal)

	/* USB Headset Feature Unit ID8 Descriptor */
	0x0D,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,             // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_FEATURE_UNIT,             // FEATURE_UNIT  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_ID,                // ID of this Unit ( bUnitID  ).
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_MIXER_UNIT_ID,                  // Input terminal is connected to this unit(bSourceID)
	0x02,                               // (bControlSize) //was 0x03
	0x01,0x00,                          // (bmaControls(0)) Controls for Master Channel
	0x00,0x00,                          // (bmaControls(1)) Controls for Channel 1
	0x00,0x00,                          // (bmaControls(2)) Controls for Channel 2
	0x00,                               //  iFeature

	/* USB Headset Feature Unit ID5 Descriptor */
	0x0B,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,             // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_FEATURE_UNIT,             // FEATURE_UNIT  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_MICROPHONE_ID,     // ID of this Unit ( bUnitID  ).
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_MICROPHONE_ID,   // Input terminal is connected to this unit(bSourceID)
	0x02,                               // (bControlSize) //was 0x03
	0x01,0x00,                          // (bmaControls(0)) Controls for Master Channel
	0x00,0x00,                          // (bmaControls(1)) Controls for Channel 1
	0x00,                               //  iFeature

	/* USB Headset Feature Unit ID7 Descriptor */
	0x0B,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,             // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_FEATURE_UNIT,             // FEATURE_UNIT  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_SIDE_TONING_ID,    // ID of this Unit ( bUnitID  ).
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_MICROPHONE_ID,   // Input terminal is connected to this unit(bSourceID)
	0x02,                               // (bControlSize) //was 0x03
	0x01,0x00,                          // (bmaControls(0)) Controls for Master Channel
	0x00,0x00,                          // (bmaControls(1)) Controls for Channel 1
	0x00,                               //  iFeature

	/* USB Headset Mixer Unit ID8 Descriptor */
	0x0D,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,             // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_MIXER_UNIT,               // FEATURE_UNIT  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_MIXER_UNIT_ID,                  // ID of this Unit ( bUnitID  ).
	2,                                  // Number of input pins
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_ID,              // sourceID 1
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_SIDE_TONING_ID,    // sourceID 2
	0x02,                               // number of channels
	0x03,0x00,                          // channel config
	0x00,                               // iChannelNames
	0x00,                               // bmControls
	0x00,                               // iMixer

	/* USB Headset Output Terminal Descriptor */
	0x09,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,             // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_OUTPUT_TERMINAL,          // OUTPUT_TERMINAL  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_OUTPUT_TERMINAL_ID,             // ID of this Terminal.(bTerminalID)
	0x01,0x03,                          // (wTerminalType)See USB Audio Terminal Types.
	0x00,                               // No association (bAssocTerminal)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_ID,                // (bSourceID)
	0x00,                               // Unused. (iTerminal)

	/* USB Headset Output Terminal Microphone Descriptor */
	0x09,                           // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,    // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_OUTPUT_TERMINAL,      // OUTPUT_TERMINAL  descriptor subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_OUTPUT_TERMINAL_MICROPHONE_ID,          // ID of this Terminal.(bTerminalID)
	0x01,0x01,                       // (wTerminalType)See USB Audio Terminal Types.
	0x00,                            // No association (bAssocTerminal)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_FEATURE_UNIT_MICROPHONE_ID,             // (bSourceID)
	0x00,                            // Unused. (iTerminal)

	/* USB Headset Standard AS Interface Descriptor (Alt. Set. 0) */
	0x09,                            // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,        // INTERFACE descriptor type (bDescriptorType)
	1,        // Interface Number  (bInterfaceNumber)
	0x00,                            // Alternate Setting Number (bAlternateSetting)
	0x00,                            // Number of endpoints in this intf (bNumEndpoints)
	USB_AUDIO_CLASS_CODE,            // Class code  (bInterfaceClass)
	USB_AUDIO_AUDIOSTREAMING,        // Subclass code (bInterfaceSubclass)
	0x00,                            // Protocol code  (bInterfaceProtocol)
	0x00,                            // Interface string index (iInterface)

	/* USB Headset Standard AS Interface Descriptor (Alt. Set. 1) */
	0x09,                            // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,        // INTERFACE descriptor type (bDescriptorType)
	1,                              // Interface Number  (bInterfaceNumber)
	0x01,                            // Alternate Setting Number (bAlternateSetting)
	0x01,                            // Number of endpoints in this intf (bNumEndpoints)
	USB_AUDIO_CLASS_CODE,            // Class code  (bInterfaceClass)
	USB_AUDIO_AUDIOSTREAMING,        // Subclass code (bInterfaceSubclass)
	0x00,                            // Protocol code  (bInterfaceProtocol)
	0x00,                            // Interface string index (iInterface)

	/*  USB Headset Class-specific AS General Interface Descriptor */
	0x07,                            // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,     // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_AS_GENERAL,    // GENERAL subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_INPUT_TERMINAL_ID,           // Unit ID of the Output Terminal.(bTerminalLink)
	0x01,                            // Interface delay. (bDelay)
	0x01,0x00,                       // PCM Format (wFormatTag)

	/*  USB Headset Type 1 Format Type Descriptor */
	(0x11 - 6U),                            // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,     // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_FORMAT_TYPE ,          // FORMAT_TYPE subtype. (bDescriptorSubtype)
	0x01,                            // FORMAT_TYPE_1. (bFormatType)
	0x02,                            // two channel.(bNrChannels)
//    0x01,
	0x02,                            // 2 byte per audio subframe.(bSubFrameSize)
	0x10,                            // 16 bits per sample.(bBitResolution)
	0x01,                            // two frequency supported. (bSamFreqType)

	AUDIO_SAMPLE_FREQ(44100),
//	0x80,0x3E,0x00,                  // 16000
//    0xC0,0x5D,0x00,                  // 24000
//	0x00,0x7D,0x00,                  // 32000
//	0x80,0xBB,0x00,                  // Sampling Frequency = 48000 Hz(tSamFreq)

	/*  USB Headset Standard Endpoint Descriptor */
	0x09,                            // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_ENDPOINT,         // ENDPOINT descriptor (bDescriptorType)
	0x01,                            // OUT Endpoint 1. (bEndpointAddress)
	0x09,                            /* ?(bmAttributes) Isochronous,
									  * asynchronous, data endpoint */
	AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),                      // ?(wMaxPacketSize) //48 * 4
//    0x40, 0x00,                     // 16*4
	0x01,                            // One packet per frame.(bInterval)
	0x00,                            // Unused. (bRefresh)
	0x00,                            // Unused. (bSynchAddress)

	/* USB Speaker Class-specific Isoc. Audio Data Endpoint Descriptor*/
	0x07,                            // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_ENDPOINT,           // CS_ENDPOINT Descriptor Type (bDescriptorType)
	USB_AUDIO_EP_GENERAL,            // GENERAL subtype. (bDescriptorSubtype)
	0x01,                            /* Turn on sampling frequency control, no pitch
										control, no packet padding.(bmAttributes)*/
	0x00,                            // Unused. (bLockDelayUnits)
	0x00,0x00,                        // Unused. (wLockDelay)

	/* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) */
	0x09,                            // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,        // INTERFACE descriptor type (bDescriptorType)
	0x02,                              // Interface Number  (bInterfaceNumber)
	0x00,                            // Alternate Setting Number (bAlternateSetting)
	0x00,                            // Number of endpoints in this intf (bNumEndpoints)
	USB_AUDIO_CLASS_CODE,            // Class code  (bInterfaceClass)
	USB_AUDIO_AUDIOSTREAMING,        // Subclass code (bInterfaceSubclass)
	0x00,                            // Protocol code  (bInterfaceProtocol)
	0x00,                            // Interface string index (iInterface)

	/* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) */
	0x09,                           // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,       // INTERFACE descriptor type (bDescriptorType)
	0x02,                           // Interface Number  (bInterfaceNumber)
	0x01,                           // Alternate Setting Number (bAlternateSetting)
	0x01,                           // Number of endpoints in this intf (bNumEndpoints)
	USB_AUDIO_CLASS_CODE,           // Class code  (bInterfaceClass)
	USB_AUDIO_AUDIOSTREAMING,       // Subclass code (bInterfaceSubclass)
	0x00,                           // Protocol code  (bInterfaceProtocol)
	0x00,                               // Interface string index (iInterface)

	/*  USB Microphone Class-specific AS General Interface Descriptor */
	0x07,                               // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,                  // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_AS_GENERAL,                     // GENERAL subtype (bDescriptorSubtype)
	USB_DEVICE_AUDIO_IDX0_DESCRIPTOR_OUTPUT_TERMINAL_MICROPHONE_ID,           // Unit ID of the Output Terminal.(bTerminalLink)
	0x00,                            // Interface delay. (bDelay)
	0x01,0x00,                       // PCM Format (wFormatTag)

	/*  USB Microphone Type 1 Format Type Descriptor */
	(0x11 - 6U),                            // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_INTERFACE,                   // CS_INTERFACE Descriptor Type (bDescriptorType)
	USB_AUDIO_FORMAT_TYPE ,          // FORMAT_TYPE subtype. (bDescriptorSubtype)
	0x01,                            // FORMAT_TYPE_1. (bFormatType)
	0x01,                            // one channel.(bNrChannels)
	0x02,                            // 2 bytes per audio subframe.(bSubFrameSize)
	0x10,                            // 16 bits per sample.(bBitResolution)
	0x01,                            // One frequency supported. (bSamFreqType)
	0x80,0x3E,0x00,                  // Sampling Frequency = 16000 Hz(tSamFreq)
//    0xC0,0x5D,0x00,                  // 24000
//	0x00,0x7D,0x00,                  // 32000
//	0x80,0xBB,0x00,                  // Sampling Frequency = 48000 Hz(tSamFreq)

	/*  USB Microphone Standard Endpoint Descriptor */
	0x09,                            // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_ENDPOINT,         // ENDPOINT descriptor (bDescriptorType)
	AUDIO_IN_EP,                            // OUT Endpoint 1. (bEndpointAddress)
	USBD_EP_TYPE_ISOC,                            /* ?(bmAttributes) Isochronous, // 0x0D!!!!!!!!!!!!!!!
									  * asynchronous, data endpoint */
	AUDIO_MIC_PACKET_SZE(USB_AUDIO_CONFIG_RECORD_DEF_FREQ),                      // ?(wMaxPacketSize) //48 * 2
//    0x40, 0x00,
	0x01,                            // One packet per frame.(bInterval)
	0x00,                            // Unused. (bRefresh)
	0x00,                            // Unused. (bSynchAddress)

	/* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor*/
	0x07,                            // Size of the descriptor, in bytes (bLength)
	USB_AUDIO_CS_ENDPOINT,           // CS_ENDPOINT Descriptor Type (bDescriptorType)
	USB_AUDIO_EP_GENERAL,            // GENERAL subtype. (bDescriptorSubtype)
	0x00,                            /* Turn on sampling frequency control, no pitch
										control, no packet padding.(bmAttributes)*/
	0x00,                            // Unused. (bLockDelayUnits)
	0x00,0x00,                        // Unused. (wLockDelay)








	/******** IAD should be positioned just before the CDC interfaces ******
							IAD to associate the two CDC interfaces */

	0x08, /* bLength */
	0x0B, /* bDescriptorType */
	CDC_INTERFACE_IDX, /* bFirstInterface */
	0x02, /* bInterfaceCount */
	0x02, /* bFunctionClass */
	0x02, /* bFunctionSubClass */
	0x01, /* bFunctionProtocol */
	0x00, /* iFunction (Index of string descriptor describing this function) */
	/* 08 bytes */

	/********************  CDC interfaces ********************/

	/*Interface Descriptor */
	0x09,   /* bLength: Interface Descriptor size */
	USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
	/* Interface descriptor type */
	CDC_INTERFACE_IDX,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x01,   /* bNumEndpoints: One endpoints used */
	0x02,   /* bInterfaceClass: Communication Interface Class */
	0x02,   /* bInterfaceSubClass: Abstract Control Model */
	0x01,   /* bInterfaceProtocol: Common AT commands */
	0x01,   /* iInterface: */
	/* 09 bytes */

	/*Header Functional Descriptor*/
	0x05,   /* bLength: Endpoint Descriptor size */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x00,   /* bDescriptorSubtype: Header Func Desc */
	0x10,   /* bcdCDC: spec release number */
	0x01,
	/* 05 bytes */

	/*Call Management Functional Descriptor*/
	0x05,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x01,   /* bDescriptorSubtype: Call Management Func Desc */
	0x00,   /* bmCapabilities: D0+D1 */
	CDC_INTERFACE_IDX + 1,   /* bDataInterface: 2 */
	/* 05 bytes */

	/*ACM Functional Descriptor*/
	0x04,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
	0x02,   /* bmCapabilities */
	/* 04 bytes */

	/*Union Functional Descriptor*/
	0x05,   /* bFunctionLength */
	0x24,   /* bDescriptorType: CS_INTERFACE */
	0x06,   /* bDescriptorSubtype: Union func desc */
	CDC_INTERFACE_IDX,   /* bMasterInterface: Communication class interface */
	CDC_INTERFACE_IDX + 1,   /* bSlaveInterface0: Data Class Interface */
	/* 05 bytes */

	/*Endpoint 2 Descriptor*/
	0x07,                          /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: Endpoint */
	CDC_CMD_EP,                    /* bEndpointAddress */
	0x03,                          /* bmAttributes: Interrupt */
	LOBYTE(CDC_CMD_PACKET_SIZE),   /* wMaxPacketSize: */
	HIBYTE(CDC_CMD_PACKET_SIZE),
	0x10,                          /* bInterval: */
	/* 07 bytes */

	/*Data class interface descriptor*/
	0x09,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_INTERFACE,       /* bDescriptorType: */
	CDC_INTERFACE_IDX + 1,         /* bInterfaceNumber: Number of Interface */
	0x00,                          /* bAlternateSetting: Alternate setting */
	0x02,                          /* bNumEndpoints: Two endpoints used */
	0x0A,                          /* bInterfaceClass: CDC */
	0x00,                          /* bInterfaceSubClass: */
	0x00,                          /* bInterfaceProtocol: */
	0x00,                          /* iInterface: */
	/* 09 bytes */

	/*Endpoint OUT Descriptor*/
	0x07,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: Endpoint */
	CDC_OUT_EP,                    /* bEndpointAddress */
	0x02,                          /* bmAttributes: Bulk */
	LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
	HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	0x00,                          /* bInterval: ignore for Bulk transfer */
	/* 07 bytes */

	/*Endpoint IN Descriptor*/
	0x07,   /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: Endpoint */
	CDC_IN_EP,                     /* bEndpointAddress */
	0x02,                          /* bmAttributes: Bulk */
	LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
	HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	0x00,                          /* bInterval */
	/* 07 bytes */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */
/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_AUDIO_HandleTypeDef *haudio;

    /* Allocate Audio structure */
  haudio = &USBD_AUDIO_Handle;

  if (haudio == NULL)
  {
    pdev->pClassData = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassData = (void *)haudio;

  haudio->alt_setting = 0U;
  haudio->offset = AUDIO_OFFSET_UNKNOWN;
  haudio->wr_ptr = 0U;
  haudio->rd_ptr = 0U;
  haudio->rd_enable = 0U;
  haudio->RxBuffer = UserRxBufferFS;
  haudio->TxBuffer = UserTxBufferFS;
  haudio->TxState = 0;
  haudio->TxLength = 0;

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_out[AUDIO_OUT_EP & 0xFU].bInterval = AUDIO_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_out[AUDIO_OUT_EP & 0xFU].bInterval = AUDIO_FS_BINTERVAL;
  }

  /* Open EP OUT */
  if (USBD_LL_OpenEP(pdev, AUDIO_OUT_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET) != USBD_OK) return (uint8_t)USBD_FAIL;
  pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 1U;

  /* Open EP IN */
  pdev->ep_in[AUDIO_IN_EP & 0xFU].bInterval = 0x01;
  if (USBD_LL_OpenEP(pdev, AUDIO_IN_EP, USBD_EP_TYPE_ISOC, CDC_DATA_FS_IN_PACKET_SIZE) != USBD_OK) return (uint8_t)USBD_FAIL;

  pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 1U;

  /* Open EP IN */
  if (USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_IN_PACKET_SIZE) != USBD_OK) return (uint8_t)USBD_FAIL;

  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

   /* Open EP OUT */
  if (USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_OUT_PACKET_SIZE) != USBD_OK) return (uint8_t)USBD_FAIL;

  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

    /* Set bInterval for CMD Endpoint */
  pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;

  if (USBD_LL_OpenEP(pdev, CDC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE) != USBD_OK) return (uint8_t)USBD_FAIL;
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 1U;

  /* Initialize the Audio output Hardware layer */
  if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U) != 0U)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, haudio->buffer,
                               AUDIO_OUT_PACKET);

  (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, haudio->RxBuffer,
                               CDC_DATA_FS_OUT_PACKET_SIZE);

  USB_OTG_FS->GINTMSK |= 1 << 3; //Enable SOF Interrupt
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

  /* Open EP OUT */
  (void)USBD_LL_CloseEP(pdev, AUDIO_OUT_EP);
  pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 0U;
  pdev->ep_out[AUDIO_OUT_EP & 0xFU].bInterval = 0U;

  (void)USBD_LL_CloseEP(pdev, CDC_IN_EP);
  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, CDC_OUT_EP);
  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;

  /* Close Command IN EP */
  (void)USBD_LL_CloseEP(pdev, CDC_CMD_EP);
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 0U;
  pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = 0U;

  (void)USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
  pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 0U;
  pdev->ep_in[AUDIO_IN_EP & 0xFU].bInterval = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0U);
//    (void)USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  USBD_AUDIO_HandleTypeDef *hcdc = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;
  uint16_t len;
  uint8_t *pbuf;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS:
    switch (req->bRequest)
    {
    case AUDIO_REQ_GET_CUR:
      AUDIO_REQ_GetCurrent(pdev, req);
      break;

    case AUDIO_REQ_SET_CUR:
      AUDIO_REQ_SetCurrent(pdev, req);
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }

    if (req->wLength != 0U)
    {
      if ((req->bmRequest & 0x80U) != 0U)
      {
        ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)hcdc->data,
                                                          req->wLength);

          (void)USBD_CtlSendData(pdev, (uint8_t *)hcdc->data, req->wLength);
      }
      else
      {
        hcdc->CmdOpCode = req->bRequest;
        hcdc->CmdLength = (uint8_t)req->wLength;

        (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcdc->data, req->wLength);
      }
    }
    else
    {
      ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                        (uint8_t *)req, 0U);
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
      {
        (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
      }
      else
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_GET_DESCRIPTOR:
      if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USB_AUDIO_DESC_SIZ, req->wLength);

        (void)USBD_CtlSendData(pdev, pbuf, len);
      }
      break;

    case USB_REQ_GET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
      {
        (void)USBD_CtlSendData(pdev, (uint8_t *)&haudio->alt_setting, 1U);
      }
      else
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_SET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
      {
        if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
        {
          haudio->alt_setting = (uint8_t)(req->wValue);
          //Microphone
          if (haudio->alt_setting == 1)
          {
        	  haudio->micTransmittionFlag = 1;
        	  USBD_MICROPHONE_SEND(pdev);
          }else
          {
        	  haudio->micTransmittionFlag = 0;
        	  USBD_LL_FlushEP(pdev, AUDIO_IN_EP);
          }
        }
        else
        {
          /* Call the error management function (command will be nacked */
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
        }
      }
      else
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_CLEAR_FEATURE:
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }
    break;
  default:
    USBD_CtlError(pdev, req);
    ret = USBD_FAIL;
    break;
  }

  return (uint8_t)ret;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_CfgDesc);

  return USBD_AUDIO_CfgDesc;
}

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef *hcdc;
  PCD_HandleTypeDef *hpcd = pdev->pData;

  if (pdev->pClassData == NULL)
  {
	return (uint8_t)USBD_FAIL;
  }

  hcdc = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (epnum != (AUDIO_IN_EP & 0xF))
  {
	  if ((pdev->ep_in[epnum].total_length > 0U) &&
		  ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
	  {
		/* Update the packet total length */
		pdev->ep_in[epnum].total_length = 0U;

		/* Send ZLP */
		(void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
	  }
	  else
	  {
		hcdc->TxState = 0U;
		((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->TransmitCplt(hcdc->TxBuffer, &hcdc->TxLength, epnum);
	  }
  }else
  {
	  USBD_MICROPHONE_SEND(pdev);
  }
  /* Only OUT data are processed */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {
    /* In this driver, to simplify code, only SET_CUR request is managed */

    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
      ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);
      haudio->control.cmd = 0U;
      haudio->control.len = 0U;
    }
  }

  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if ((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFFU))
  {
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(hcdc->CmdOpCode,
                                                      (uint8_t *)hcdc->data,
                                                      (uint16_t)hcdc->CmdLength);
    hcdc->CmdOpCode = 0xFFU;

  }

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);

  /* Only OUT control data are processed */
  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev)
{
	USB_OTG_FS->GINTMSK &= ~(1 << 3); //Disable SOF Interrupt
	if (HAL_GPIO_ReadPin(USB_Select_GPIO_Port, USB_Select_Pin))
	{
		device.USBInput = USB_INPUT_MICRO;
	}else
	{
		device.USBInput = USB_INPUT_TYPEC;
	}
//  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

//  if (haudio->micTransmittionFlag == 1)
//  {
//	  USBD_LL_FlushEP(pdev, AUDIO_IN_EP);
//	  USBD_LL_Transmit(pdev, AUDIO_IN_EP, NULL, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE);
//	  haudio->micTransmittionFlag = 2;
//  }

  return (uint8_t)USBD_OK;
}

extern void audio_clock_increase();
extern void audio_clock_decrease();
uint32_t audio_data_update_counter = 1;

void USBD_AUDIO_Sync(USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset, bool pause)
{

  USBD_AUDIO_HandleTypeDef *haudio;
  int32_t BufferSize = AUDIO_TOTAL_BUF_SIZE / 2U;

  if (pdev->pClassData == NULL)
  {
    return;
  }

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (audio_data_update_counter < (AUDIO_OUT_PACKET_NUM / 2 - 1) || pause) //Отключение, если недостаточно данных с usb
  {
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->rd_ptr = 0U;
    haudio->wr_ptr = 0U;
    memset(haudio->buffer, 0, AUDIO_TOTAL_BUF_SIZE);
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0], AUDIO_TOTAL_BUF_SIZE / 2U, AUDIO_CMD_STOP);
    return;
  }
  audio_data_update_counter = 0;

  haudio->offset = offset;
  //Посдстройка частоты
  if (haudio->rd_enable == 1U)
  {
    haudio->rd_ptr += (uint16_t)BufferSize;

    if (haudio->rd_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* roll back */
      haudio->rd_ptr = 0U;
    }
  }

  if (haudio->rd_ptr == 0)
  {
	  if ((BufferSize - haudio->wr_ptr) > AUDIO_OUT_PACKET)
	  {
		  audio_clock_decrease();
	  }else if ((haudio->wr_ptr - BufferSize) > AUDIO_OUT_PACKET)
	  {
		  audio_clock_increase();
	  }
  }else if (haudio->rd_ptr == BufferSize)
  {
	  if ((haudio->wr_ptr < (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET)) && (haudio->wr_ptr > BufferSize))
	  {
		  audio_clock_decrease();
	  }else if ((haudio->wr_ptr > AUDIO_OUT_PACKET) && (haudio->wr_ptr < BufferSize))
	  {
		  audio_clock_increase();
	  }
  }
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(pdev);
  UNUSED(epnum);

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(pdev);
  UNUSED(epnum);

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  uint16_t PacketSize;
  USBD_AUDIO_HandleTypeDef *haudio;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (epnum == AUDIO_OUT_EP)
  {
    /* Get received data packet length */
    PacketSize = (uint16_t)USBD_LL_GetRxDataSize(pdev, epnum);

    /* Packet received Callback */
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->PeriodicTC(&haudio->buffer[haudio->wr_ptr],
                                                           PacketSize, AUDIO_OUT_TC);

    /* Increment the Buffer pointer or roll it back when all buffers are full */
    haudio->wr_ptr += PacketSize;

    if (haudio->offset == AUDIO_OFFSET_UNKNOWN  && haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE / 2)
    {
      ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0],
                                                           AUDIO_TOTAL_BUF_SIZE / 2U,
                                                           AUDIO_CMD_START);
    }

    if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* All buffers are full: roll back */
      haudio->wr_ptr = 0U;
//        haudio->offset = AUDIO_OFFSET_NONE;
    }

    if (haudio->rd_enable == 0U)
    {
      if (haudio->wr_ptr == (AUDIO_TOTAL_BUF_SIZE / 2U))
      {
        haudio->rd_enable = 1U;
      }
    }

    /* Prepare Out endpoint to receive next audio packet */
    (void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP,
                                 &haudio->buffer[haudio->wr_ptr],
                                 AUDIO_OUT_PACKET);
    audio_data_update_counter++;
  }else if (epnum == CDC_OUT_EP)
  {
	  USBD_AUDIO_HandleTypeDef *hcdc = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

	  if (pdev->pClassData == NULL)
	  {
	    return (uint8_t)USBD_FAIL;
	  }

	  /* Get the received data length */
	  hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

	  /* USB data will be immediately processed, this allow next USB traffic being
	  NAKed till the end of the application Xfer */

	  ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Receive(hcdc->RxBuffer, &hcdc->RxLength);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  (void)USBD_memset(haudio->control.data, 0, 64U);

  /* Send the current mute state */
  (void)USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (req->wLength != 0U)
  {
    /* Prepare the reception of the buffer over EP0 */
    (void)USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);

    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}


/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_DeviceQualifierDesc);

  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData = fops;

  return (uint8_t)USBD_OK;
}

uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *hcdc = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}

uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *hcdc = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;
  USBD_StatusTypeDef ret = USBD_BUSY;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hcdc->TxState == 0U)
  {
    /* Tx Transfer in progress */
    hcdc->TxState = 1U;

    /* Update the packet total length */
    pdev->ep_in[CDC_IN_EP & 0xFU].total_length = hcdc->TxLength;

    /* Transmit next packet */
    (void)USBD_LL_Transmit(pdev, CDC_IN_EP, hcdc->TxBuffer, hcdc->TxLength);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_CDC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CDC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  hcdc->RxBuffer = pbuff;

  return (uint8_t)USBD_OK;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

void USBD_AUDIO_Restart(USBD_HandleTypeDef *pdev)
{
	USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;
	haudio->wr_ptr = 0;
	haudio->rd_ptr = 0;
	haudio->offset = AUDIO_OFFSET_UNKNOWN;
}

inline void USBD_MICROPHONE_AddData(int16_t *data, uint32_t size)
{
	RingBuffer_Int16_Write(&micBuffer, data, size);
}

/**
 * Отправка данных по usb
 */
void USBD_MICROPHONE_SEND(USBD_HandleTypeDef *pdev)
{
	uint32_t size = micBuffer.dataSize;
	memset(micTxBuffer, 0, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE);
	//Если mute включен, то удаляем данные из буфера
	//Иначе считываем из буфера
	if (device.audio.muteMicrophoneOutput)
	{
		RingBuffer_Delete(&micBuffer, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2);
	}else
	{
		if (size >= USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2)
		{
			  RingBuffer_Int16_SeekRead(&micBuffer, (int16_t *)micTxBuffer, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2);
		}else
		{
			  RingBuffer_Int16_SeekRead(&micBuffer, micTxBuffer + USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2 - size, size, size);
		}
	}
	//IIR фильтр
	for (uint32_t i = 0; i < USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE / 2; i++)
	{
		micTxBuffer[i] = IIR_Calc(micTxBuffer[i]);
	}
	USBD_LL_FlushEP(pdev, AUDIO_IN_EP);
	pdev->ep_in[AUDIO_IN_EP & 0xFU].total_length = USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE;
	USBD_LL_Transmit(pdev, AUDIO_IN_EP, micTxBuffer, USBD_AUDIO_CONFIG_RECORD_MAX_PACKET_SIZE);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
