/************************************************************************************//**
* \file         microblt.h
* \brief        LibMicroBLT header file.
* \ingroup      Library
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2021 by Feaser     www.feaser.com     All rights reserved
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
/************************************************************************************//**
* \defgroup   Library Library API
* \brief      LibMicroBLT API
* \details
* The Library API contains the application programming interface for the LibMicroBLT
* library. It defines the functions and definitions that a microcontroler application
* uses to access the library's functionality.
* LibMicroBLT contains functionality for communicating with and performing a firmware
* update on another microcontroller, which runs the OpenBLT bootloader.
****************************************************************************************/
#ifndef MICROBLT_H
#define MICROBLT_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
*             V E R S I O N   I N F O R M A T I O N
****************************************************************************************/
/** \brief Main version number of LibMicroBLT. */
#define BLT_VERSION_MAIN                    ((uint8_t)0U)

/** \brief Minor version number of LibMicroBLT. */
#define BLT_VERSION_MINOR                   ((uint8_t)9U)

/** \brief Patch number of LibMicroBLT. */
#define BLT_VERSION_PATCH                   ((uint8_t)0U)


/****************************************************************************************
*             P O R T   I N T E R F A C E
****************************************************************************************/
/****************************************************************************************
* Include files
****************************************************************************************/
#include "port.h"                           /* Port module                             */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void BltPortInit(tPort const * port);
void BltPortTerminate(void);


/****************************************************************************************
*             S E S S I O N   L A Y E R S
****************************************************************************************/
/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief XCP protocol version 1.0. XCP is a universal measurement and calibration
 *         communication protocol. It contains functionality for reading, programming,
 *         and erasing (non-volatile) memory making it a good fit for bootloader
 *         purposes.
 */
#define BLT_SESSION_XCP_V10                 ((uint32_t)0U)


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Structure layout of the XCP version 1.0 session settings. */
typedef struct
{
  uint16_t timeoutT1;            /**< Command response timeout in milliseconds.        */
  uint16_t timeoutT3;            /**< Start programming timeout in milliseconds.       */
  uint16_t timeoutT4;            /**< Erase memory timeout in milliseconds.            */
  uint16_t timeoutT5;            /**< Program memory and reset timeout in milliseconds.*/
  uint16_t timeoutT6;            /**< Connect response timeout in milliseconds.        */
  uint16_t timeoutT7;            /**< Busy wait timer timeout in milliseonds.          */
  uint8_t  connectMode;          /**< Connection mode parameter in XCP connect command.*/
} tBltSessionSettingsXcpV10;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void    BltSessionInit(uint32_t type, void const * settings);
void    BltSessionTerminate(void);
uint8_t BltSessionStart(void);
void    BltSessionStop(void);
uint8_t BltSessionClearMemory(uint32_t address, uint32_t len);
uint8_t BltSessionWriteData(uint32_t address, uint32_t len, uint8_t const * data);
uint8_t BltSessionReadData(uint32_t address, uint32_t len, uint8_t * data);


/****************************************************************************************
*             F I R M W A R E   F I L E   R E A D E R
****************************************************************************************/
/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief The S-record reader enables reading firmware data from a file formatted as
 *         a Motorola S-record. This is a widely known file format and pretty much all
 *         microcontroller compiler toolchains included functionality to output or
 *         convert the firmware's data as an S-record.
 */
#define BLT_FIRMWARE_READER_SRECORD         ((uint8_t)0U)


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void            BltFirmwareInit(uint8_t readerType);
void            BltFirmwareTerminate(void);
uint8_t         BltFirmwareFileOpen(char const * firmwareFile);
void            BltFirmwareFileClose(void);
uint32_t        BltFirmwareGetTotalSize(void);
uint8_t         BltFirmwareSegmentGetCount(void);
uint32_t        BltFirmwareSegmentGetInfo(uint8_t idx, uint32_t * address);
void            BltFirmwareSegmentOpen(uint8_t idx);
uint8_t const * BltFirmwareSegmentGetNextData(uint32_t * address, uint16_t * len);


#ifdef __cplusplus
}
#endif

#endif /* MICROBLT_H */
/*********************************** end of microblt.h *********************************/
