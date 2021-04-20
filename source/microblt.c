/************************************************************************************//**
* \file         microblt.c
* \brief        LibMicroBLT source file.
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

/****************************************************************************************
* Include files
****************************************************************************************/
#include <microtbx.h>                       /* MicroTBX toolbox                        */
#include "microblt.h"                       /* LibMiroBLT                              */
#include "firmware.h"                       /* Firmware reader module                  */
#include "srecreader.h"                     /* S-record firmware file reader           */


/****************************************************************************************
*             F I R M W A R E   F I L E   R E A D E R
****************************************************************************************/
/************************************************************************************//**
** \brief     Initializes the firmware reader module for a specified firmware file
**            reader. Typically called once upon application initialization.
** \param     readerType The firmware file reader to use in this module. It should be a
**            BLT_FIRMWARE_READER_xxx value.
**
****************************************************************************************/
void BltFirmwareInit(uint8_t readerType)
{
  tFirmwareReader const * firmwareReader;

  /* Process the reader type. */
  switch (readerType)
  {
    /* Reader for S-record firmware files. */
    case BLT_FIRMWARE_READER_SRECORD:
      firmwareReader = SRecReaderGet();
      break;
      /* Unsupported reader type specified. */
    default:
      firmwareReader = NULL;
      break;
  }

  /* Verify that a valid firmware reader was set. */
  TBX_ASSERT(firmwareReader != NULL)

  /* Only continue if a valid firmware reader was set. */
  if (firmwareReader != NULL)
  {
    /* Initialize the firmware reader module by linking the firmware file reader. */
    FirmwareInit(firmwareReader);
  }
} /*** end of BltFirmwareInit ***/


/************************************************************************************//**
** \brief     Terminates the firmware reader module. Typically called at the end of the
**            application when the firmware reader module is no longer needed.
**
****************************************************************************************/
void BltFirmwareTerminate(void)
{
  /* Pass the request on to the firmware reader module. */
  FirmwareTerminate();
} /*** end of BltFirmwareTerminate ***/


/************************************************************************************//**
** \brief     Opens the firmware file and browses through its contents to collect
**            information about the firmware data segment it contains.
** \param     firmwareFile Firmware filename including its full path.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t BltFirmwareFileOpen(char const * firmwareFile)
{
  /* Pass the request on to the firmware reader module. */
  return FirmwareFileOpen(firmwareFile);
} /*** end of BltFirmwareFileOpen ***/


/************************************************************************************//**
** \brief     Closes the previously opened firmware file.
**
****************************************************************************************/
void BltFirmwareFileClose(void)
{
  /* Pass the request on to the firmware reader module. */
  FirmwareFileClose();
} /*** end of BltFirmwareFileClose ***/


/************************************************************************************//**
** \brief     Obtains the total number of firmware data segments encountered in the
**            firmware file. A firmware data segment consists of a consecutive block
**            of firmware data. A firmware file always has at least one segment. However,
**            it can have more as well. For example if there is a gap between the vector
**            table and the other program data.
** \return    Total number of firmware data segments present in the firmware file.
**
****************************************************************************************/
uint8_t BltFirmwareSegmentGetCount(void)
{
  /* Pass the request on to the firmware reader module. */
  return FirmwareSegmentGetCount();
} /*** end of BltFirmwareSegmentGetCount ***/


/************************************************************************************//**
** \brief     Opens the firmware data segment for reading. This should always be called
**            before calling the SegmentGetNextData() function.
** \param     idx Zero-based segment index. Valid values are between 0 and
**            (SegmentGetCount() - 1).
**
****************************************************************************************/
void BltFirmwareSegmentOpen(uint8_t idx)
{
  /* Pass the request on to the firmware reader module. */
  FirmwareSegmentOpen(idx);
} /*** end of BltFirmwareSegmentOpen ***/


/************************************************************************************//**
** \brief     Obtains a data pointer to the next chunk of firmware data in the segment
**            that was opened with function SegmentOpen(). The idea is that you first
**            open the segment and afterwards you can keep calling this function to
**            read out the segment's firmware data. When all data is read, len will be
**            set to zero and a NULL pointer is returned.
** \param     address The starting memory address of this chunk of firmware data is
**            written to this pointer.
** \param     len  The length of the firmware data chunk is written to this pointer.
** \return    Data pointer to the read firmware if successul, NULL otherwise.
**
****************************************************************************************/
uint8_t const * BltFirmwareSegmentGetNextData(uint32_t * address, uint16_t * len)
{
  /* Pass the request on to the firmware reader module. */
  return FirmwareSegmentGetNextData(address, len);
} /*** end of BltFirmwareSegmentGetNextData ***/


/*********************************** end of microblt.c *********************************/
