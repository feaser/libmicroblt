/************************************************************************************//**
* \file         srecreader.c
* \brief        S-record firmware file reader source file.
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
#include "srecreader.h"                     /* S-record firmware file reader           */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void            SRecReaderInit(void);
static void            SRecReaderTerminate(void);
static uint8_t         SRecReaderFileOpen(char const * firmwareFile);
static void            SRecReaderFileClose(void);
static uint16_t        SRecReaderSegmentGetCount(void);
static uint8_t         SRecReaderSegmentOpen(uint16_t idx);
static uint8_t const * SRecReaderSegmentGetNextData(uint32_t * address, uint32_t * len);


/***********************************************************************************//**
** \brief     Obtains a pointer to the reader structure, so that it can be linked to the
**            firmware reader module.
** \return    Pointer to firmware reader structure.
**
****************************************************************************************/
tFirmwareReader const * SRecReaderGet(void)
{
  /** \brief File reader structure filled with Motorola S-record parsing specifics. */
  static const tFirmwareReader srecReader =
  {
    .Init = SRecReaderInit,
    .Terminate = SRecReaderTerminate,
    .FileOpen = SRecReaderFileOpen,
    .FileClose = SRecReaderFileClose,
    .SegmentGetCount = SRecReaderSegmentGetCount,
    .SegmentOpen = SRecReaderSegmentOpen,
    .SegmentGetNextData = SRecReaderSegmentGetNextData
  };

  /* Give the pointer to the firmware reader back to the caller. */
  return &srecReader;
} /*** end of SRecReaderGet ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static void SRecReaderInit(void)
{
  /* TODO ##Vg Implement SRecReaderInit. */
} /*** end of SRecReaderInit ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static void SRecReaderTerminate(void)
{
  /* TODO ##Vg Implement SRecReaderTerminate. */
} /*** end of SRecReaderTerminate ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static uint8_t SRecReaderFileOpen(char const * firmwareFile)
{
  uint8_t result = TBX_OK;

  /* Verify parameter. */
  TBX_ASSERT(firmwareFile != NULL);

  /* Only continue with valid parameter. */
  if (firmwareFile != NULL)
  {
    /* TODO ##Vg Implement SRecReaderFileOpen. */
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderFileOpen ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static void SRecReaderFileClose(void)
{
  /* TODO ##Vg Implement SRecReaderFileClose. */
} /*** end of SRecReaderFileClose ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static uint16_t SRecReaderSegmentGetCount(void)
{
  uint16_t result = 0;

  /* TODO ##Vg Implement SRecReaderSegmentGetCount. */

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentGetCount ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static uint8_t SRecReaderSegmentOpen(uint16_t idx)
{
  uint8_t result = TBX_OK;

  /* Verify parameter. */
  TBX_ASSERT(idx < SRecReaderSegmentGetCount());

  /* Only continue with valid parameter. */
  if (idx < SRecReaderSegmentGetCount())
  {
    /* TODO ##Vg Implement SRecReaderSegmentOpen. */
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentOpen ***/


/************************************************************************************//**
** \brief     ...
**
****************************************************************************************/
static uint8_t const * SRecReaderSegmentGetNextData(uint32_t * address, uint32_t * len)
{
  uint8_t const * result = NULL;

  /* Verify parameters. */
  TBX_ASSERT((address != NULL) && (len != NULL));

  /* Only continue with valid parameters. */
  if ((address != NULL) && (len != NULL))
  {
    /* TODO ##Vg Implement SRecReaderSegmentGetNextData. */
    *address = 0;
    *len = 0;
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentGetNextData ***/


/*********************************** end of srecreader.c *******************************/
