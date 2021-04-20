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


/* TODO ##Vg Implement missing functions:
 * -  MicroBltFirmwareFileOpen()
 * -  MicroBltFirmwareFileClose()
 * -  MicroBltFirmwareSegmentGetCount()
 * -  MicroBltFirmwareSegmentOpen()
 * -  MicroBltFirmwareSegmentGetNextData()
 */

/****************************************************************************************
*             F I R M W A R E   F I L E   R E A D E R
****************************************************************************************/
/************************************************************************************//**
** \brief     Initializes the firmware reader module for a specified firmware file
**            reader. Typically called once upon application initialization.
** \param     readerType The firmware file reader to use in this module. It should be a
**            MICRO_BLT_FIRMWARE_READER_xxx value.
**
****************************************************************************************/
void MicroBltFirmwareInit(uint8_t readerType)
{
  tFirmwareReader const * firmwareReader;

  /* Process the reader type. */
  switch (readerType)
  {
    /* Reader for S-record firmware files. */
    case MICRO_BLT_FIRMWARE_READER_SRECORD:
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
} /*** end of MicroBltFirmwareInit ***/


/************************************************************************************//**
** \brief     Terminates the firmware reader module. Typically called at the end of the
**            application when the firmware reader module is no longer needed.
**
****************************************************************************************/
void MicroBltFirmwareTerminate(void)
{
  /* Terminate the firmware reader module. */
  FirmwareTerminate();
} /*** end of MicroBltFirmwareTerminate ***/


/*********************************** end of microblt.c *********************************/
