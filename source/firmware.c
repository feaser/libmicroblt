/************************************************************************************//**
* \file         firmware.c
* \brief        Firmware file reader source file.
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

/****************************************************************************************
* Include files
****************************************************************************************/
#include <microtbx.h>                       /* MicroTBX toolbox                        */
#include "firmware.h"                       /* Firmware reader module                  */


/* TODO ##Vg Implement missing functions:
 * -  FirmwareFileOpen()
 * -  FirmwareFileClose()
 * -  FirmwareSegmentGetCount()
 * -  FirmwareSegmentOpen()
 * -  FirmwareSegmentGetNextData()
 */

/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Pointer to the firmware reader that is linked. */
static tFirmwareReader const * readerPtr = NULL;


/************************************************************************************//**
** \brief     Initializes the module.
** \param     reader The firmware file reader to link.
**
****************************************************************************************/
void FirmwareInit(tFirmwareReader const * reader)
{
  /* Verify parameter. */
  TBX_ASSERT(reader != NULL);

  /* Only continue with valid parameter. */
  if (reader != NULL)
  {
    /* Link the firmware reader. */
    readerPtr = reader;
    /* Verify the reader's function pointer. */
    TBX_ASSERT(reader->Init != NULL);
    /* Only continue with a valid function pointer. */
    if (reader->Init != NULL)
    {
      /* Initialize the reader. */
      reader->Init();
    }
  }
} /*** end of FirmwareInit **/


/************************************************************************************//**
** \brief     Terminates the module.
**
****************************************************************************************/
void FirmwareTerminate(void)
{
  /* Verify the firmware reader. */
  TBX_ASSERT(readerPtr != NULL);

  /* Only continue with a valid firmware reader. */
  if (readerPtr != NULL)
  {
    /* Verify the reader's function pointer. */
    TBX_ASSERT(readerPtr->Terminate != NULL);
    /* Only continue with a valid function pointer. */
    if (readerPtr->Terminate != NULL)
    {
      /* Terminate the reader. */
      readerPtr->Terminate();
    }
    /* Unlink the firmware reader. */
    readerPtr = NULL;
  }
} /*** end of FirmwareTerminate ***/


/*********************************** end of firmware.c *********************************/
