/************************************************************************************//**
* \file         can.h
* \brief        Controller area network driver header file.
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2022 by Feaser     www.feaser.com     All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* \endinternal
****************************************************************************************/
#ifndef CAN_H
#define CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Maximum number of bytes in a CAN message */
#define CAN_DATA_LEN_MAX     (8U)


/****************************************************************************************
* Type definitions
****************************************************************************************/
typedef struct
{
  /** \brief CAN message identifier. */
  uint32_t id;
  /** \brief CAN message data length [0..(CAN_DATA_LEN_MAX-1)]. */
  uint8_t  len;
  /** \brief TBX_TRUE for a 29-bit CAN identifier, TBX_FALSE for 11-bit. */
  uint8_t  ext;
  /** \brief Array with the data bytes of the CAN message. */
  uint8_t  data[CAN_DATA_LEN_MAX];
} tCanMsg;

/** \brief Enumerated type with all supported CAN baudrates. */
typedef enum
{
  CAN_BAUDRATE_1M,
  CAN_BAUDRATE_800K,
  CAN_BAUDRATE_500K,
  CAN_BAUDRATE_250K,
  CAN_BAUDRATE_125K,
  CAN_BAUDRATE_100K,
  CAN_BAUDRATE_50K,
  CAN_BAUDRATE_20K,
  CAN_BAUDRATE_10K
} tCanBaudrate;

/** \brief Function type for the message received callback handler. */
typedef void (* tCanReceivedCallback)(tCanMsg const * msg);


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void    CanInit(tCanBaudrate baudrate, tCanReceivedCallback callbackFcn);
void    CanTerminate(void);
uint8_t CanTransmit(tCanMsg const * msg);


#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
/*********************************** end of can.h **************************************/
