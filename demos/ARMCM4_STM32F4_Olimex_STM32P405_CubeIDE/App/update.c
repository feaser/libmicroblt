/************************************************************************************//**
* \file         update.c
* \brief        Firmware update module source file.
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
  uint8_t                            result = TBX_ERROR;
  uint8_t                            segmentIdx;
  uint32_t                           segmentLen;
  uint32_t                           segmentBase;
  uint8_t                   const *  chunkData;
  uint16_t                           chunkLen;
  uint32_t                           chunkBase;
  uint32_t                  const    connectTimeout = 5000U;
  uint32_t                           connectStartTime;
  uint32_t                           connectDeltaTime;
  uint8_t                            continueLoop;
  uint32_t                        (* portSystemGetTimeFcn)(void) = NULL;
  tBltSessionSettingsXcpV10 const    sessionSettings =
  {
    .timeoutT1   = 1000U,
    .timeoutT3   = 2000U,
    .timeoutT4   = 10000U,
    .timeoutT5   = 1000U,
    .timeoutT6   = 50U,
    .timeoutT7   = 2000U,
    .connectMode = nodeId
  };

  /* Attempt to set function pointer to the port's SystemGetTime() function. */
  if (PortGet() != NULL)
  {
    portSystemGetTimeFcn = PortGet()->SystemGetTime;
  }

  /* Verify parameter and port function. */
  TBX_ASSERT((firmwareFile != NULL) && (portSystemGetTimeFcn != NULL));

  /* Initialize the firmware module for reading S-record firmware files. */
  BltFirmwareInit(BLT_FIRMWARE_READER_SRECORD);
  /* Initialize the session module for firmware updates using the XCP protocol. */
  BltSessionInit(BLT_SESSION_XCP_V10, &sessionSettings);

  /* Only continue with valid parameter and port function. */
  if ( (firmwareFile != NULL) && (portSystemGetTimeFcn != NULL) )
  {
    /* Set a positive result and only negate upon error detection from here on. */
    result = TBX_OK;

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Open the firmware file ------------------------------------- */
    /* ------------------------------------------------------------------------------- */
    /* Start the firmware update by opening the firmware file. */
    if (BltFirmwareFileOpen(firmwareFile) != TBX_OK)
    {
      /* Could not open the firmware file. Flag error. */
      result = TBX_ERROR;
    }

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Connect to the target -------------------------------------- */
    /* ------------------------------------------------------------------------------- */
    /* Only continue with an opened firmware file. */
    if (result == TBX_OK)
    {
      /* Store time at which the connection attempt with the target started. */
      connectStartTime = portSystemGetTimeFcn();
      /* Attempt to connect to the target. */
      while (BltSessionStart() != TBX_OK)
      {
        /* Calculate elapsed time while waiting for the connection to establish. Note
         * that this calculation is 32-bit time overflow safe.
         */
        connectDeltaTime = portSystemGetTimeFcn() - connectStartTime;
        /* Did a timeout occur? */
        if (connectDeltaTime > connectTimeout)
        {
          /* Could not connect to the target. Flag error. */
          result = TBX_ERROR;
          /* Stop connection attempts. */
          break;
        }
      }
    }

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Erase memory segments -------------------------------------- */
    /* ------------------------------------------------------------------------------- */
    /* Only continue when connected to the target. */
    if (result == TBX_OK)
    {
      /* Erase the memory segments on the target that the firmware data covers. */
      for (segmentIdx = 0U; segmentIdx < BltFirmwareSegmentGetCount(); segmentIdx++)
      {
        /* Obtain segment information such as its base memory adddress and length. */
        segmentLen = BltFirmwareSegmentGetInfo(segmentIdx, &segmentBase);
        /* Erase the segment. */
        if (BltSessionClearMemory(segmentBase, segmentLen) == TBX_ERROR)
        {
          /* The the erase error and stop the loop. */
          result = TBX_ERROR;
          break;
        }
      }
    }

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Program memory segments ------------------------------------ */
    /* ------------------------------------------------------------------------------- */
    /* Only continue when memory segments were successfully erased. */
    if (result == TBX_OK)
    {
      /* Program the memory segments on the target one segment at a time. */
      for (segmentIdx = 0U; segmentIdx < BltFirmwareSegmentGetCount(); segmentIdx++)
      {
        /* Open the segment for reading. */
        BltFirmwareSegmentOpen(segmentIdx);

        /* Set flag to start the loop. */
        continueLoop = TBX_TRUE;

        /* Program the segment data, one chunk at a time. */
        while (continueLoop == TBX_TRUE)
        {
          /* Attempt to read the next chunk of data in this segment. */
          chunkData = BltFirmwareSegmentGetNextData(&chunkBase, &chunkLen);
          /* Did an error occur? */
          if (chunkData == NULL)
          {
            /* Could not read the data chunk. Flag error and request the loop to stop. */
            result = TBX_ERROR;
            continueLoop = TBX_FALSE;
          }
          /* No error occured. The segment end was reached or new data was read. */
          else
          {
            /* Segment end reached? */
            if (chunkLen == 0U)
            {
              /* All done with this segment so request the loop to stop. */
              continueLoop = TBX_FALSE;
            }
            /* New data chunk was read. */
            else
            {
              /* Program the newly read data chunk. */
              if (BltSessionWriteData(chunkBase, chunkLen, chunkData) != TBX_OK)
              {
                /* Could not program the data. Flag error and request the loop to stop.*/
                result = TBX_ERROR;
                continueLoop = TBX_FALSE;
              }
            }
          }
        }

        /* Check if an error occured while programming the segment's data chunks. */
        if (result != TBX_OK)
        {
          /* Stop segment loop, because an error was detected. */
          break;
        }
      }
    }

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Disconnect from the target --------------------------------- */
    /* ------------------------------------------------------------------------------- */
    /* Make sure to stop the session again. This causes the bootloader on the target to
     * start the firmware, if present.
     */
    BltSessionStop();

    /* ------------------------------------------------------------------------------- */
    /* ------------------ Close the firmware file ------------------------------------ */
    /* ------------------------------------------------------------------------------- */
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
