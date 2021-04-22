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


/************************************************************************************//**
** \brief     Opens the firmware file and browses through its contents to collect
**            information about the firmware data segment it contains.
** \param     firmwareFile Firmware filename including its full path.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t FirmwareFileOpen(char const * firmwareFile)
{
  uint8_t result = TBX_OK;

  /* Verify parameter. */
  TBX_ASSERT(firmwareFile != NULL);

  /* Only continue with valid parameter. */
  if (firmwareFile != NULL)
  {
    /* Verify the firmware reader. */
    TBX_ASSERT(readerPtr != NULL);

    /* Only continue with a valid firmware reader. */
    if (readerPtr != NULL)
    {
      /* Verify the reader's function pointer. */
      TBX_ASSERT(readerPtr->FileOpen != NULL);
      /* Only continue with a valid function pointer. */
      if (readerPtr->FileOpen != NULL)
      {
        /* Attempt to open the file. */
        result = readerPtr->FileOpen(firmwareFile);
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of FirmwareFileOpen ***/


/************************************************************************************//**
** \brief     Closes the previously opened firmware file.
**
****************************************************************************************/
void FirmwareFileClose(void)
{
  /* Verify the firmware reader. */
  TBX_ASSERT(readerPtr != NULL);

  /* Only continue with a valid firmware reader. */
  if (readerPtr != NULL)
  {
    /* Verify the reader's function pointer. */
    TBX_ASSERT(readerPtr->FileClose != NULL);
    /* Only continue with a valid function pointer. */
    if (readerPtr->FileClose != NULL)
    {
      /* Close the file. */
      readerPtr->FileClose();
    }
  }
} /*** end of FirmwareFileClose ***/


/************************************************************************************//**
** \brief     Obtains the total number of firmware data segments encountered in the
**            firmware file. A firmware data segment consists of a consecutive block
**            of firmware data. A firmware file always has at least one segment. However,
**            it can have more as well. For example if there is a gap between the vector
**            table and the other program data.
** \return    Total number of firmware data segments present in the firmware file.
**
****************************************************************************************/
uint8_t FirmwareSegmentGetCount(void)
{
  uint8_t result = 0U;

  /* Verify the firmware reader. */
  TBX_ASSERT(readerPtr != NULL);

  /* Only continue with a valid firmware reader. */
  if (readerPtr != NULL)
  {
    /* Verify the reader's function pointer. */
    TBX_ASSERT(readerPtr->SegmentGetCount != NULL);
    /* Only continue with a valid function pointer. */
    if (readerPtr->SegmentGetCount != NULL)
    {
      /* Obtains the segment count. */
      result = readerPtr->SegmentGetCount();
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of FirmwareSegmentGetCount ***/


/************************************************************************************//**
** \brief     Obtains information about the specified segment, such as the base memory
**            address that its data belongs to and the total number of data bytes in the
**            segment.
** \param     idx Zero-based segment index. Valid values are between 0 and
**            (SegmentGetCount() - 1).
** \param     address The base memory address of the segment's data is written to this
**            pointer.
** \param     len The total number of data bytes inside this segment is written to this
**            pointer.
**
****************************************************************************************/
void FirmwareSegmentGetInfo(uint8_t idx, uint32_t * address, uint32_t * len)
{
  /* Verify parameters. */
  TBX_ASSERT((idx < FirmwareSegmentGetCount()) && (address != NULL) && (len != NULL));

  /* Only continue with valid parameters. */
  if ((idx < FirmwareSegmentGetCount()) && (address != NULL) && (len != NULL))
  {
    /* Verify the firmware reader. */
    TBX_ASSERT(readerPtr != NULL);

    /* Only continue with a valid firmware reader. */
    if (readerPtr != NULL)
    {
      /* Verify the reader's function pointer. */
      TBX_ASSERT(readerPtr->SegmentGetInfo != NULL);
      /* Only continue with a valid function pointer. */
      if (readerPtr->SegmentGetInfo != NULL)
      {
        /* Obtains the segment info. */
        readerPtr->SegmentGetInfo(idx, address, len);
      }
    }
  }
} /*** end of FirmwareSegmentGetInfo ***/


/************************************************************************************//**
** \brief     Opens the firmware data segment for reading. This should always be called
**            before calling the SegmentGetNextData() function.
** \param     idx Zero-based segment index. Valid values are between 0 and
**            (SegmentGetCount() - 1).
**
****************************************************************************************/
void FirmwareSegmentOpen(uint8_t idx)
{
  /* Verify parameter. */
  TBX_ASSERT(idx < FirmwareSegmentGetCount());

  /* Only continue with valid parameter. */
  if (idx < FirmwareSegmentGetCount())
  {
    /* Verify the firmware reader. */
    TBX_ASSERT(readerPtr != NULL);

    /* Only continue with a valid firmware reader. */
    if (readerPtr != NULL)
    {
      /* Verify the reader's function pointer. */
      TBX_ASSERT(readerPtr->SegmentOpen != NULL);
      /* Only continue with a valid function pointer. */
      if (readerPtr->SegmentOpen != NULL)
      {
        /* Open the segment. */
        readerPtr->SegmentOpen(idx);
      }
    }
  }
} /*** end of FirmwareSegmentOpen ***/


/************************************************************************************//**
** \brief     Reads and stores the next chunk of firmware data in the segment that was
**            opened with function SegmentOpen(). The idea is that you first
**            open the segment and afterwards you can keep calling this function to
**            read out the segment's firmware data. When all data is read, len will be
**            set to zero. The firmware data is stored in the provided buffer. The
**            bufferSize parameter informs this function of how many bytes can be stored
**            in the buffer.
** \param     address The starting memory address of this chunk of firmware data is
**            written to this pointer.
** \param     len The length of the firmware data chunk is written to this pointer.
** \param     buffer Byte array where this function will store the read data bytes.
** \param     bufferSize Maximum number of bytes that can be stored in the buffer.
**
****************************************************************************************/
void FirmwareSegmentGetNextData(uint32_t * address, uint16_t * len,
                                uint8_t * buffer, uint16_t bufferSize)
{
  /* Verify parameters. */
  TBX_ASSERT((address != NULL) && (len != NULL) && (buffer != NULL) && (bufferSize >0U));

  /* Only continue with valid parameters. */
  if ((address != NULL) && (len != NULL) && (buffer != NULL) && (bufferSize > 0U))
  {
    /* Verify the firmware reader. */
    TBX_ASSERT(readerPtr != NULL);

    /* Only continue with a valid firmware reader. */
    if (readerPtr != NULL)
    {
      /* Verify the reader's function pointer. */
      TBX_ASSERT(readerPtr->SegmentGetNextData != NULL);
      /* Only continue with a valid function pointer. */
      if (readerPtr->SegmentGetNextData != NULL)
      {
        /* Read the next chunk of firmware data from the opened segment. */
        readerPtr->SegmentGetNextData(address, len, buffer, bufferSize);
      }
    }
  }
} /*** end of FirmwareSegmentGetNextData ***/


/*********************************** end of firmware.c *********************************/
