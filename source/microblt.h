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
#define BLT_VERSION_MINOR                   ((uint8_t)1U)

/** \brief Patch number of LibMicroBLT. */
#define BLT_VERSION_PATCH                   ((uint8_t)0U)


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
void     BltFirmwareInit(uint8_t readerType);
void     BltFirmwareTerminate(void);
uint8_t  BltFirmwareFileOpen(char const * firmwareFile);
void     BltFirmwareFileClose(void);
uint32_t BltFirmwareGetTotalSize(void);
uint8_t  BltFirmwareSegmentGetCount(void);
void     BltFirmwareSegmentGetInfo(uint8_t idx, uint32_t * address, uint32_t * len);
void     BltFirmwareSegmentOpen(uint8_t idx);
void     BltFirmwareSegmentGetNextData(uint32_t * address, uint16_t * len,
                                       uint8_t * buffer, uint16_t bufferSize);


#ifdef __cplusplus
}
#endif

#endif /* MICROBLT_H */
/*********************************** end of microblt.h *********************************/
