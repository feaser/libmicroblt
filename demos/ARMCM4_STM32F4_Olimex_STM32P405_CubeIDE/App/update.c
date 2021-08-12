/************************************************************************************//**
* \file         update.c
* \brief        Firmware update module source file.
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
#include <microtbx.h>                       /* MicroTBX                                */
#include <microblt.h>                       /* LibMicroBLT                             */
#include "update.h"                         /* Firmware update module                  */


/************************************************************************************//**
** \brief     Performs a firmware update on a connected microcontroller that runs the
**            OpenBLT bootloader.
** \param     firmwareFile Full path to the S-record firmware file on the file system.
** \param     nodeId Node identifier of the microcontroller to update. Only applicable
**            on a master-slave type system. Otherwise specify 0.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t UpdateFirmware(char const * firmwareFile, uint8_t nodeId)
{
  uint8_t                         result = TBX_ERROR;
  uint32_t                        segmentIdx;
  tBltSessionSettingsXcpV10 const sessionSettings =
  {
    .timeoutT1   = 1000U,
    .timeoutT3   = 2000U,
    .timeoutT4   = 10000U,
    .timeoutT5   = 1000U,
    .timeoutT6   = 50U,
    .timeoutT7   = 2000U,
    .connectMode = nodeId
  };

  /* Verify parameter. */
  TBX_ASSERT(firmwareFile != NULL);

  /* Initialize the firmware module for reading S-record firmware files. */
  BltFirmwareInit(BLT_FIRMWARE_READER_SRECORD);
  /* Initialize the session module for firmware updates using the XCP protocol. */
  BltSessionInit(BLT_SESSION_XCP_V10, &sessionSettings);

  /* Only continue with valid parameter. */
  if (firmwareFile != NULL)
  {
    /* Set a positive result and only negate upon error detection from here on. */
    result = TBX_OK;

    /* Start the firmware update by opening the firmware file. */
    if (BltFirmwareFileOpen(firmwareFile) != TBX_OK)
    {
      /* Could not open the firmware file. Flag error. */
      result = TBX_ERROR;
    }

    /* Only continue with an opened firmware file. */
    if (result == TBX_OK)
    {
      /* TODO Maybe add a timeout feature here or do it indefinitely. */
      /* Connect to the target. */
      if (BltSessionStart() != TBX_OK)
      {
        /* Could not connect to the target. Flag error. */
        result = TBX_ERROR;
      }
    }

    /* Only continue when connected to the target. */
    if (result == TBX_OK)
    {
      /* Erase the memory segments on the target that the firmwware data covers. */
      for (segmentIdx = 0U; segmentIdx < BltFirmwareSegmentGetCount(); segmentIdx++)
      {
        /* TODO Erase the memory segment. */
      }
    }

    /* Make sure to stop the session again. This causes the bootloader on the target to
     * start the firmware, if present.
     */
    BltSessionStop();
    /* Make sure to close the firmware file again. */
    BltFirmwareFileClose();
  }

  /* Terminate the session module. */
  BltSessionTerminate();
  /* Terminate the firware module. */
  BltFirmwareTerminate();

  /* Give the result back to the caller. */
  return result;
} /*** end of UpdateFirmware ***/


/*********************************** end of update.c ***********************************/
