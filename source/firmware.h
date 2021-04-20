/************************************************************************************//**
* \file         firmware.h
* \brief        Firmware file reader header file.
* \ingroup      Firmware
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
* \defgroup   Firmware Firmware Reader Module
* \brief      Module with functionality to load firmware data from a file.
* \ingroup    Library
* \details
* The Firmwarwe reader module contains functionality to load firmware data from a file.
* It contains an interface for linking firmware file parsers that handle the parsing
* of firmware data from a file in the correct format. For example the Motorola S-record
* format.
****************************************************************************************/
#ifndef FIRMWARE_H
#define FIRMWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Firmware file reader. It provides the API interface for linking specific
 *         firmware file type readers. For example a reader for S-records.
 */
typedef struct
{
  /** \brief Initializes the firmware reader. */
  void            (* Init) (void);

  /** \brief Terminates the firmware reader. */
  void            (* Terminate) (void);

  /** \brief Opens the firmware file for reading. */
  uint8_t         (* FileOpen) (char const * firmwareFile);

  /** \brief Closes an opened firmware file. */
  void            (* FileClose) (void);

  /** \brief Obtain the number of firmware data segments detected in the file. */
  uint8_t         (* SegmentGetCount) (void);

  /** \brief Opens a firmware data segment for reading. */
  void            (* SegmentOpen) (uint8_t idx);

  /** \brief Obtains a data point to the segment's next chunk of firmware data. */
  uint8_t const * (* SegmentGetNextData) (uint32_t * address, uint16_t * len);
} tFirmwareReader;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void            FirmwareInit(tFirmwareReader const * reader);
void            FirmwareTerminate(void);
uint8_t         FirmwareFileOpen(char const * firmwareFile);
void            FirmwareFileClose(void);
uint8_t         FirmwareSegmentGetCount(void);
void            FirmwareSegmentOpen(uint8_t idx);
uint8_t const * FirmwareSegmentGetNextData(uint32_t * address, uint16_t * len);


#ifdef __cplusplus
}
#endif

#endif /* FIRMWARE_H */
/*********************************** end of firmware.h *********************************/
