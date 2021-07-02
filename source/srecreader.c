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
#include <ff.h>                             /* FatFS                                   */
#include <string.h>                         /* C library string handling               */
#include "firmware.h"                       /* Firmware reader module                  */
#include "srecreader.h"                     /* S-record firmware file reader           */


/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Size of the byte buffer for storing a line from the S-record file. */
#define SREC_LINE_BUFFER_SIZE          (256)

/** \brief Size of the byte buffer to store firmware data extracted from an S-record.*/
#define SREC_DATA_BUFFER_SIZE          (512)


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Structure that represents the handle to the S-record file, which groups all
 *         its relevent data.
 */
typedef struct
{
  /** \brief FatFS file object handle. */
  FIL      file;
  /** \brief Byte buffer for storing a line from the S-record file. */
  TCHAR    lineBuf[SREC_LINE_BUFFER_SIZE];
  /** \brief Byte buffer for storing data extracted from an S-record. */
  uint8_t  dataBuf[SREC_DATA_BUFFER_SIZE];
  /** \brief Maximum number of firmware data bytes on the longest S-record line. */
  uint16_t maxLineData;
} tSRecHandle;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void            SRecReaderInit(void);
static void            SRecReaderTerminate(void);
static uint8_t         SRecReaderFileOpen(char const * firmwareFile);
static void            SRecReaderFileClose(void);
static uint8_t         SRecReaderSegmentGetCount(void);
static uint32_t        SRecReaderSegmentGetInfo(uint8_t idx, uint32_t * address);
static void            SRecReaderSegmentOpen(uint8_t idx);
static uint8_t const * SRecReaderSegmentGetNextData(uint32_t * address, uint16_t * len);


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Handle to the S-record file. */
static tSRecHandle srecHandle;


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
    .SegmentGetInfo = SRecReaderSegmentGetInfo,
    .SegmentOpen = SRecReaderSegmentOpen,
    .SegmentGetNextData = SRecReaderSegmentGetNextData
  };

  /* Give the pointer to the firmware reader back to the caller. */
  return &srecReader;
} /*** end of SRecReaderGet ***/


/************************************************************************************//**
** \brief     Initializes the S-record reader.
**
****************************************************************************************/
static void SRecReaderInit(void)
{
  /* Initialize the file handle to its reset value. */
  memset(&srecHandle, 0, sizeof(srecHandle));

  /* TODO ##Vg Implement SRecReaderInit. */
} /*** end of SRecReaderInit ***/


/************************************************************************************//**
** \brief     Terminated the S-record reader.
**
****************************************************************************************/
static void SRecReaderTerminate(void)
{
  /* TODO ##Vg Implement SRecReaderTerminate. */
} /*** end of SRecReaderTerminate ***/


/************************************************************************************//**
** \brief     Opens the firmware file and browses through its contents to collect
**            information about the firmware data segment it contains.
** \param     firmwareFile Firmware filename including its full path.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
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
    /* Open the file for reading. */
    if (f_open(&srecHandle.file, firmwareFile, FA_READ) != FR_OK)
    {
      /* Could not open the file. Update the result to flag this problem. */
      result = TBX_ERROR;
    }

    /* Only continue if the file was successfully opened. */
    if (result == TBX_OK)
    {
      /* Reset max line data counter. */
      srecHandle.maxLineData = 0U;
      /* Loop to read all the lines in the file one at a time. */
      for (;;)
      {
        /* Attempt to read the next line from the file */
        if (f_gets(srecHandle.lineBuf, SREC_LINE_BUFFER_SIZE, &srecHandle.file) == NULL)
        {
          /* An error occured or we reached the end of the file. Was it an error? */
          if (f_error(&srecHandle.file) > 0U)
          {
            /* Close the file and update the result to flag this problem. */
            (void)f_close(&srecHandle.file);
            result = TBX_ERROR;
          }
          /* Stop looping when an error occurred or we reached the end of the file. */
          break;
        }
        /* TODO ##Vg Continue here by processing the line. Need to keep the TCHAR stuff
         * in mind though for unicode stuff. As a next step I need to extract the
         * address and data length from the line. And I need to implement the segment
         * linked list. Probably also need to keep track of the file pointer before
         * reading the line, to be able to track the file pointer at the start of a
         * segment.
         */
      }
    }

    /* TODO ##Vg Implement SRecReaderFileOpen.
     * - Open the file
     * - Read all lines one at a time
     * - Keep track of the size of the longest s-record line in the file
     * - Determine segment info:
     *   - file pointer to the first s-record in the file
     *   - base address
     *   - length
     *
     * There should be a local file info object that stores:
     * - File handle
     * - Linked list with segment info
     * - Size of the largest s-record line
     * - Data buffer for storing an s-record line
     * - Data buffer for storing the segment data
     */
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderFileOpen ***/


/************************************************************************************//**
** \brief     Closes the previously opened firmware file.
**
****************************************************************************************/
static void SRecReaderFileClose(void)
{
  /* TODO ##Vg Implement SRecReaderFileClose. */

  /* Close the file. */
  (void)f_close(&srecHandle.file);
} /*** end of SRecReaderFileClose ***/


/************************************************************************************//**
** \brief     Obtains the total number of firmware data segments encountered in the
**            firmware file. A firmware data segment consists of a consecutive block
**            of firmware data. A firmware file always has at least one segment. However,
**            it can have more as well. For example if there is a gap between the vector
**            table and the other program data.
** \return    Total number of firmware data segments present in the firmware file.
**
****************************************************************************************/
static uint8_t SRecReaderSegmentGetCount(void)
{
  uint8_t result = 0U;

  /* TODO ##Vg Implement SRecReaderSegmentGetCount. */

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentGetCount ***/


/************************************************************************************//**
** \brief     Obtains information about the specified segment, such as the base memory
**            address that its data belongs to and the total number of data bytes in the
**            segment.
** \param     idx Zero-based segment index. Valid values are between 0 and
**            (SegmentGetCount() - 1).
** \param     address The base memory address of the segment's data is written to this
**            pointer.
** \return    The total number of data bytes inside this segment.
**
****************************************************************************************/
static uint32_t SRecReaderSegmentGetInfo(uint8_t idx, uint32_t * address)
{
  uint32_t result = 0U;

  /* Verify parameters. */
  TBX_ASSERT((idx < SRecReaderSegmentGetCount()) && (address != NULL));

  /* Only continue with valid parameters. */
  if ((idx < SRecReaderSegmentGetCount()) && (address != NULL))
  {
    /* TODO ##Vg Implement SRecReaderSegmentGetInfo. */
    *address = 0U;
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentGetInfo ***/


/************************************************************************************//**
** \brief     Opens the firmware data segment for reading. This should always be called
**            before calling the SegmentGetNextData() function.
** \param     idx Zero-based segment index. Valid values are between 0 and
**            (SegmentGetCount() - 1).
**
****************************************************************************************/
static void SRecReaderSegmentOpen(uint8_t idx)
{
  /* Verify parameter. */
  TBX_ASSERT(idx < SRecReaderSegmentGetCount());

  /* Only continue with valid parameter. */
  if (idx < SRecReaderSegmentGetCount())
  {
    /* TODO ##Vg Implement SRecReaderSegmentOpen. */
  }
} /*** end of SRecReaderSegmentOpen ***/


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
static uint8_t const * SRecReaderSegmentGetNextData(uint32_t * address, uint16_t * len)
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
