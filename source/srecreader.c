/************************************************************************************//**
* \file         srecreader.c
* \brief        S-record firmware file reader source file.
* \ingroup      SRecReader
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
* Configuration check
****************************************************************************************/
/* The current implementation assumes that char's are 1 byte in size. Verify that FatFS
 * is configured accordingly.
 */
#if (_LFN_UNICODE > 0)     /* Unicode (UTF-16) string */
#error "Unicode (UTF-16) mode currently not supported (_LFN_UNICODE must be 0)"
#endif


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Structure that groups segment related info. */
typedef struct
{
  /** \brief Base memory address of the segment's data. */
  uint32_t addr;
  /** \brief Total length of the segment in bytes. */
  uint32_t len;
  /** \brief File pointer inside the firmware file where this segment starts. */
  FSIZE_t  fptr;
} tSRecSegment;

/** \brief Structure that represents the handle to the S-record file, which groups all
 *         its relevent data.
 */
typedef struct
{
  /** \brief Boolean flag to keep track if a file is opened or not. */
  uint8_t              fileOpened;
  /** \brief FatFS file object handle. */
  FIL                  file;
  /** \brief Byte buffer for storing a line from the S-record file. */
  char                 lineBuf[SREC_LINE_BUFFER_SIZE];
  /** \brief Byte buffer for storing the data from an S-record with the help of function
   *         SRecReaderParseLine().
   */
  uint8_t              lineDataBuf[SREC_LINE_BUFFER_SIZE/2];
  /** \brief Byte buffer for storing data extracted from an S-record with the help of
   *         function SRecReaderSegmentGetNextData().
   */
  uint8_t              dataBuf[SREC_DATA_BUFFER_SIZE];
  /** \brief Handle to the linked list with segments. */
  tTbxList           * segmentList;
  /** \brief Pointer to the currently opened segment. */
  tSRecSegment const * openedSegment;
} tSRecHandle;

/** \brief Enumeration for the different S-record line types. */
typedef enum
{
  LINE_TYPE_S1,                                  /**< 16-bit address line              */
  LINE_TYPE_S2,                                  /**< 24-bit address line              */
  LINE_TYPE_S3,                                  /**< 32-bit address line              */
  LINE_TYPE_UNSUPPORTED                          /**< unsupported line                 */
} tSRecLineType;


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
static uint8_t         SRecReaderCompareSegments(void const * item1, void const * item2);
static uint8_t         SRecReaderParseLine(char const * line, uint32_t * address,
                                           uint8_t * len, uint8_t * data);
static tSRecLineType   SRecReaderGetLineType(char const * line);
static uint8_t         SRecReaderVerifyChecksum(char const * line);
static uint8_t         SRecReaderHexStringToByte(char const * hexstring);


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
  /* Initialize the s-record handle members. */
  srecHandle.fileOpened = TBX_FALSE;
  srecHandle.segmentList = NULL;
  srecHandle.openedSegment = NULL;
} /*** end of SRecReaderInit ***/


/************************************************************************************//**
** \brief     Terminated the S-record reader.
**
****************************************************************************************/
static void SRecReaderTerminate(void)
{
  /* Make sure a possibly previously opened file is closed. */
  SRecReaderFileClose();
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
  uint8_t        result = TBX_OK;
  uint32_t       lineAddress = 0U;
  uint8_t        lineDataLen = 0U;
  uint8_t        parseResult;
  uint8_t        stopLineLoop = TBX_FALSE;
  FSIZE_t        lineFPtr;
  tSRecSegment * segment = NULL;

  /* Verify parameter. */
  TBX_ASSERT(firmwareFile != NULL);

  /* Only continue with valid parameter. */
  if (firmwareFile != NULL)
  {
    /* Make sure a possibly previously opened file is first closed. */
    SRecReaderFileClose();
    /* Open the file for reading. */
    if (f_open(&srecHandle.file, firmwareFile, FA_READ) != FR_OK)
    {
      /* Could not open the file. Update the result to flag this problem. */
      result = TBX_ERROR;
    }
    /* File successfully opened. */
    else
    {
      /* Update the flag that tracks the file opened state. */
      srecHandle.fileOpened = TBX_TRUE;
    }

    /* Only continue if the file was successfully opened. */
    if (result == TBX_OK)
    {
      /* Create the linked list with segment information. */
      srecHandle.segmentList = TbxListCreate();
      /* Verify that the linked list could be created. */
      if (srecHandle.segmentList == NULL)
      {
        /* Update the flag that tracks the file opened state. */
        srecHandle.fileOpened = TBX_FALSE;
        /* Close the file and update the result to flag this problem. */
        (void)f_close(&srecHandle.file);
        result = TBX_ERROR;
      }
    }

    /* Only continue if the linked list was successfully created. */
    if (result == TBX_OK)
    {
      /* Loop to read all the lines in the file one at a time. */
      while (stopLineLoop != TBX_TRUE)
      {
        /* Store the file pointer of the current line. Needed later on in case this
         * is a new segment.
         */
        lineFPtr = f_tell(&srecHandle.file);
        /* Attempt to read the next line from the file */
        if (f_gets(srecHandle.lineBuf, SREC_LINE_BUFFER_SIZE, &srecHandle.file) == NULL)
        {
          /* An error occured or we reached the end of the file. Was it an error? */
          if (f_error(&srecHandle.file) > 0U)
          {
            result = TBX_ERROR;
          }
          /* Stop looping when an error occurred or we reached the end of the file. */
          stopLineLoop = TBX_TRUE;
          continue;
        }
        /* Attempt to extract data from the s-record line. */
        parseResult = SRecReaderParseLine(srecHandle.lineBuf, &lineAddress,
                                          &lineDataLen, NULL);
        /* Did an error occur during line parsing? */
        if (parseResult != TBX_OK)
        {
          result = TBX_ERROR;
          stopLineLoop = TBX_TRUE;
          continue;
        }
        /* Still here so parsing was okay, but only continue if data was actually
         * extracted. In the case of a non S1, S2 or S3 line the parsing can still be
         * successful, but did not yield any extracted data bytes.
         */
        if (lineDataLen > 0U)
        {
          /* Does the parsed data fit at the end of the current segment? */
          if (segment != NULL)
          {
            /* Does it fit at the end of the segment? */
            if (lineAddress == (segment->addr + segment->len))
            {
              segment->len += lineDataLen;
            }
            /* Data did not fit in the current segment. */
            else
            {
              /* A new segment should be created. Invalidate the current segment to
               * indicate this situation.
               */
              segment = NULL;
            }
          }

          /* Did the parsed data not fit in the current segment? */
          if (segment == NULL)
          {
            /* Iterate over the linked list to try and find an existing segment that the
             * data fits in.
             */
            segment = TbxListGetFirstItem(srecHandle.segmentList);
            while (segment != NULL)
            {
              /* Does it fit at the end of this segment? */
              if (lineAddress == (segment->addr + segment->len))
              {
                /* Update the length of the segment. */
                segment->len += lineDataLen;
                /* Segment found and updated so break the loop. */
                break;
              }
              /* Continue with the next segment. */
              segment = TbxListGetNextItem(srecHandle.segmentList, segment);
            }

            /* Could a fitting segment not be found? */
            if (segment == NULL)
            {
              /* Attempt to allocate memory to store the new segment. */
              segment = TbxMemPoolAllocate(sizeof(tSRecSegment));
              /* Automatically create or increase the memory pool if it was too small. */
              if (segment == NULL)
              {
                /* No need to check the return value, because we'll attempt to allocate
                 * from the memory pool right way. That will tell us if the memory pool
                 * increase was successful.
                 */
                (void)TbxMemPoolCreate(1, sizeof(tSRecSegment));
                /* Allocation should now work. */
                segment = TbxMemPoolAllocate(sizeof(tSRecSegment));
                /* Verify segment allocation. */
                if (segment == NULL)
                {
                  /* Could not allocate memory. Heap is probably configured too small.
                   * Increase TBX_CONF_HEAP_SIZE to resolve the problem. All we can
                   * do now is flag the error.
                   */
                  result = TBX_ERROR;
                  stopLineLoop = TBX_TRUE;
                  continue;
                }
              }
              /* Initialize the newly created segment. */
              segment->addr = lineAddress;
              segment->len = lineDataLen;
              segment->fptr = lineFPtr;
              /* Add the segment to the linked list. */
              if (TbxListInsertItemBack(srecHandle.segmentList, segment) == TBX_ERROR)
              {
                /* Could not insert the segment into the linked list. Heap is probably
                 * configured too small. Increase TBX_CONF_HEAP_SIZE to resolve the
                 * problem. All we can do now is flag the error.
                 */
                result = TBX_ERROR;
                stopLineLoop = TBX_TRUE;
                continue;
              }
            }
          }
        }
      }
    }

    /* Sort the segments inside the linked list if all went okay so far. */
    if (result == TBX_OK)
    {
      TbxListSortItems(srecHandle.segmentList, SRecReaderCompareSegments);
    }
    /* Perform cleanup in case the file could not be properly opened. */
    else
    {
      /* Make sure the file is closed. This includes the release of the segments linked
       * list.
       */
      SRecReaderFileClose();
    }
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
  tSRecSegment * segment;

  /* Only close the file if one is actually opened. */
  if (srecHandle.fileOpened == TBX_TRUE)
  {
    /* Reset the flag. */
    srecHandle.fileOpened = TBX_FALSE;
    /* Close the file. */
    (void)f_close(&srecHandle.file);
    /* Iterate over the linked list contents. */
    segment = TbxListGetFirstItem(srecHandle.segmentList);
    while (segment != NULL)
    {
      /* Give the allocated memory for the segment back to the memory pool. */
      TbxMemPoolRelease(segment);
      /* Continue with the next segment. */
      segment = TbxListGetNextItem(srecHandle.segmentList, segment);
    }
    /* Delete the linked list with segments. */
    TbxListDelete(srecHandle.segmentList);
    srecHandle.segmentList = NULL;
    /* Reset the opened segment. */
    srecHandle.openedSegment = NULL;
  }
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
  size_t  listSize;

  /* Only continue if a file is actually opened and segments were extracted. */
  if ( (srecHandle.fileOpened == TBX_TRUE) && (srecHandle.segmentList != NULL) )
  {
    /* The number of segments equals the size of the linked list. */
    listSize = TbxListGetSize(srecHandle.segmentList);
    /* Only update the result if the list size fits in it. */
    if ( (listSize > 0U) && (listSize <= (uint8_t)UINT8_MAX) )
    {
      result = (uint8_t)listSize;
    }
  }

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
  tSRecSegment const * segment;

  /* Verify parameters. */
  TBX_ASSERT((idx < SRecReaderSegmentGetCount()) && (address != NULL));

  /* Only continue with valid parameters. */
  if ((idx < SRecReaderSegmentGetCount()) && (address != NULL))
  {
    /* Only continue if a file is actually opened and segments were extracted. */
    if ( (srecHandle.fileOpened == TBX_TRUE) && (srecHandle.segmentList != NULL) )
    {
      /* Only continue if the segment index is valid. */
      if (idx < TbxListGetSize(srecHandle.segmentList))
      {
        /* Iterate over the linked list until the segment specified by the index. */
        segment = TbxListGetFirstItem(srecHandle.segmentList);
        while (idx > 0U)
        {
          /* Move to the next segment. */
          segment = TbxListGetNextItem(srecHandle.segmentList, segment);
          /* Decrement the indexer. */
          idx--;
        }
        /* Make sure a valid segment was found. */
        if (segment != NULL)
        {
          /* Store the base memory address of the data in this segment. */
          *address = segment->addr;
          /* Update the result to hold the total number of bytse inside this segment. */
          result = segment->len;
        }
      }
    }
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
  tSRecSegment const * segment;

  /* Verify parameter. */
  TBX_ASSERT(idx < SRecReaderSegmentGetCount());

  /* Only continue with valid parameter. */
  if (idx < SRecReaderSegmentGetCount())
  {
    /* Only continue if a file is actually opened and segments were extracted. */
    if ( (srecHandle.fileOpened == TBX_TRUE) && (srecHandle.segmentList != NULL) )
    {
      /* Iterate over the linked list until the segment specified by the index. */
      segment = TbxListGetFirstItem(srecHandle.segmentList);
      while (idx > 0U)
      {
        /* Move to the next segment. */
        segment = TbxListGetNextItem(srecHandle.segmentList, segment);
        /* Decrement the indexer. */
        idx--;
      }
      /* Make sure a valid segment was found. */
      if (segment != NULL)
      {
        /* Keep track of the currently openeded segment. */
        srecHandle.openedSegment = segment;
        /* Set the file pointer to the S-record line where this segment starts. */
        (void)f_lseek(&srecHandle.file, segment->fptr);
      }
    }
  }
} /*** end of SRecReaderSegmentOpen ***/


/************************************************************************************//**
** \brief     Obtains a data pointer to the next chunk of firmware data in the segment
**            that was opened with function SegmentOpen(). The idea is that you first
**            open the segment and afterwards you can keep calling this function to
**            read out the segment's firmware data. When all data is read, len will be
**            set to zero and a non-NULL pointer is returned.
** \param     address The starting memory address of this chunk of firmware data is
**            written to this pointer.
** \param     len  The length of the firmware data chunk is written to this pointer.
** \return    Data pointer to the read firmware if successul, NULL otherwise.
** \attention There are three possible outsomes when calling this function:
**            1) len > 0 and a non-NULL pointer is returned. This means valid data was
**               read.
**            2) len = 0 and a non-NULL pointer is returned. This means the end of the
**               segment is reached and therefore no new data was actually read.
**            3) A NULL pointer is returned. This happens only when an error occurred.
**
****************************************************************************************/
static uint8_t const * SRecReaderSegmentGetNextData(uint32_t * address, uint16_t * len)
{
  uint8_t  const * result = NULL;
  uint8_t          dataReadDone = TBX_FALSE;
  uint8_t          parseResult;
  uint32_t         lineAddress;
  uint8_t          lineDataLen;
  FSIZE_t          lineFPtr;
  uint8_t          byteIdx;

  /* Verify parameters. */
  TBX_ASSERT((address != NULL) && (len != NULL));

  /* Only continue with valid parameters. */
  if ((address != NULL) && (len != NULL))
  {
    /* Only continue if a file is actually opened, segments were extracted and a segment
     * was actually opened.
     */
    if ( (srecHandle.fileOpened == TBX_TRUE) && (srecHandle.segmentList != NULL) &&
         (srecHandle.openedSegment != NULL) )
    {
      /* Set the result to the valid databuffer, which indicates success. From now on
       * only set it to NULL, in case an error was detected.
       */
      result = srecHandle.dataBuf;

      /* Initialize the lenght output parameter, since we plan on using it as a data
       * buffer indexer as well.
       */
      *len = 0U;

      /* Loop to read as much data from this segment that will with in the internal
       * data buffer.
       */
      while (dataReadDone != TBX_TRUE)
      {
        /* Store the file pointer of the current line. Might need it later to rewind. */
        lineFPtr = f_tell(&srecHandle.file);
        /* Attempt to read the next line from the file. */
        if (f_gets(srecHandle.lineBuf, SREC_LINE_BUFFER_SIZE, &srecHandle.file) == NULL)
        {
          /* An error occured or we reached the end of the file. Was it an error? */
          if (f_error(&srecHandle.file) > 0U)
          {
            /* Flag the error by updating the result and resetting the length. */
            *len = 0;
            result = NULL;
            /* Rewind the file pointer as well. */
            (void)f_lseek(&srecHandle.file, lineFPtr);
          }
          /* Stop looping when an error occurred or we reached the end of the file. */
          dataReadDone = TBX_TRUE;
          continue;
        }

        /* Still here, so a line was read fron the file. Attempt to extract data from
         * the S-record line.
         */
        parseResult = SRecReaderParseLine(srecHandle.lineBuf, &lineAddress,
                                          &lineDataLen, srecHandle.lineDataBuf);
        /* Did an error occur during line parsing? */
        if (parseResult != TBX_OK)
        {
          /* Flag the error by updating the result and resetting the length. */
          *len = 0;
          result = NULL;
          /* Rewind the file pointer as well. */
          (void)f_lseek(&srecHandle.file, lineFPtr);
          dataReadDone = TBX_TRUE;
          continue;
        }

        /* Still here so parsing was okay, but only continue if data was actually
         * extracted. In the case of a non S1, S2 or S3 line the parsing can still be
         * successful, but did not yield any extracted data bytes.
         */
        if (lineDataLen > 0U)
        {
          /* Was this the first chunk of data? */
          if (*len == 0U)
          {
            /* Set the base address of the data. */
            *address = lineAddress;
          }
          /* Does this newly read data still belong to the same segment? */
          if ( (lineAddress < srecHandle.openedSegment->addr) ||
               ((lineAddress + lineDataLen) >
               (srecHandle.openedSegment->addr + srecHandle.openedSegment->len)) )
          {
            /* The data read from this line belongs to the a different segment. This
             * means we are done and should not copy the data. We do rewind the file
             * pointer, because the data hasn't actually been processed.
             */
            (void)f_lseek(&srecHandle.file, lineFPtr);
            dataReadDone = TBX_TRUE;
            continue;
          }
          /* Data does belong to this segment. This means that it should fit right after
           * the previously read data. Do a quick sanity check to make sure this is the
           * case.
           */
          if (lineAddress != (*address + *len))
          {
            /* Flag the error by updating the result and resetting the length. */
            *len = 0;
            result = NULL;
            /* Rewind the file pointer as well. */
            (void)f_lseek(&srecHandle.file, lineFPtr);
            dataReadDone = TBX_TRUE;
            continue;

          }
          /* Still here so the newly read data does belong to the same segment, but we
           * can only copy it, if there is still space in the data buffer.
           */
          if ((*len + lineDataLen) > (uint16_t)SREC_DATA_BUFFER_SIZE)
          {
            /* Data won't fit in the data buffer. This means we are done, but need to
             * make sure to rewind the file pointer for the next time this function is
             * called.
             */
            (void)f_lseek(&srecHandle.file, lineFPtr);
            dataReadDone = TBX_TRUE;
            continue;
          }
          /* Still here so we know that the data belongs to the same segment and that
           * is will also still fit in the data buffer. Time to copy the data to the
           * data buffer.
           */
          for (byteIdx = 0U; byteIdx < lineDataLen; byteIdx++)
          {
            srecHandle.dataBuf[*len + byteIdx] = srecHandle.lineDataBuf[byteIdx];
          }
          /* Update the data length. */
          *len += lineDataLen;
        }
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderSegmentGetNextData ***/


/************************************************************************************//**
** \brief     Compares the two segments based on their base address. Can be used for
**            sorting the segments inside the linked list.
** \param     item1 Pointer to the first item to compare.
** \param     item2 Pointer to the seconds item to compare.
** \return    TBX_TRUE if item 1 is greater than item 2. TBX_FALSE otherwise.
**
****************************************************************************************/
static uint8_t SRecReaderCompareSegments(void const * item1, void const * item2)
{
  uint8_t              result = TBX_FALSE;
  tSRecSegment const * segment1 = item1;
  tSRecSegment const * segment2 = item2;

  /* Verify parameters. */
  TBX_ASSERT((item1 != NULL) && (item2 != NULL));

  /* Only continue with valid parameters. */
  if ((item1 != NULL) && (item2 != NULL))
  {
    /* Compare the segments based on its base address. */
    if (segment1->addr > segment2->addr)
    {
      result = TBX_TRUE;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderCompareSegments ***/


/************************************************************************************//**
** \brief     Looks for S1, S2 or S3 S-record lines and parses them by extracting the
**            address, length and data.
** \param     line    An S-record line.
** \param     address Base memory address extracted from the S-record data line.
** \param     len     Number of data bytes extracted from the S-record data line.
** \param     data    Byte array where the data bytes from the S-Record data line
**                    are stored.
** \return    TBX_OK if the S-record contained an S1, S2 or S3 line and the data was
**            successfully extracted. TBX_ERROR if an error was detected during the
**            line parsing. For example when the line contains in invalid checksum.
**            Note that if the line was not and S1, S2 or S3 line with data, then
**            TBX_OK is still returned, but len will be set to 0 because no data was
**            present and consequently extracted.
** \attention If a NULL pointer is passed for the data parameter, the actual data
**            extraction and storage in the data byte array is skipped.
**
****************************************************************************************/
static uint8_t SRecReaderParseLine(char const * line, uint32_t * address,
                                   uint8_t * len, uint8_t * data)
{
  uint8_t       result = TBX_ERROR;
  tSRecLineType lineType;
  uint8_t       charIdx = 2U; /* Point to the byte count value. */
  uint8_t       dataByteIdx;
  uint8_t       bytesOnLine;

  /* Verify parameters. Note that a NULL pointer for data is allowed. */
  TBX_ASSERT((line != NULL) && (address != NULL) && (len != NULL));

  /* Only continue with valid parameters. */
  if ((line != NULL) && (address != NULL) && (len != NULL))
  {
    /* All okay so far. Update the result accordingly and from now on only set an error
     * value upon detection of a problem.
     */
    result = TBX_OK;
    /* Determine the s-record line type. */
    lineType = SRecReaderGetLineType(line);
    /* Verify the checksum on the line. Note that this is only needed for S1, S2 and S3
     * line types, because those are the only ones this function will extract data from.
     */
    if (lineType != LINE_TYPE_UNSUPPORTED)
    {
      if (SRecReaderVerifyChecksum(line) != TBX_OK)
      {
        /* Flag error due to incorrect checksum on the s-record line. */
        result = TBX_ERROR;
      }
    }

    /* Only continue with a valid checksum on the S1, S2 or S3 line. */
    if (result == TBX_OK)
    {
      /* The S1, S2 and S3 lines differ in the amount of bytes for the memory address.
       * Therefore, filter on the line type to correctly extract the memory address.
       */
      switch (lineType)
      {
        case LINE_TYPE_S1:
          /* Read out the number of bytes that follow on the line. */
          bytesOnLine = SRecReaderHexStringToByte(&line[charIdx]);
          /* Set the character index to point to the start of the memory address. */
          charIdx += 2U;
          /* Extract the 16-bit memory address. */
          *address = (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 8U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += SRecReaderHexStringToByte(&line[charIdx]);
          /* The number of bytes on the line must be > 3 (2 for address + 1 for cs. */
          if (bytesOnLine > 3U)
          {
            /* Set the data byte length, so the number of data bytes to extract. */
            *len = bytesOnLine - 3U;
          }
          else
          {
            /* Invalid byte count detected on the line. Flag error. */
            result = TBX_ERROR;
          }
          break;
        case LINE_TYPE_S2:
          /* Read out the number of bytes that follow on the line. */
          bytesOnLine = SRecReaderHexStringToByte(&line[charIdx]);
          /* Set the character index to point to the start of the memory address. */
          charIdx += 2U;
          /* Extract the 24-bit memory address. */
          *address = (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 16U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 8U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += SRecReaderHexStringToByte(&line[charIdx]);
          /* The number of bytes on the line must be > 4 (3 for address + 1 for cs. */
          if (bytesOnLine > 4U)
          {
            /* Set the data byte length, so the number of data bytes to extract. */
            *len = bytesOnLine - 4U;
          }
          else
          {
            /* Invalid byte count detected on the line. Flag error. */
            result = TBX_ERROR;
          }
          break;
        case LINE_TYPE_S3:
          /* Read out the number of bytes that follow on the line. */
          bytesOnLine = SRecReaderHexStringToByte(&line[charIdx]);
          /* Set the character index to point to the start of the memory address. */
          charIdx += 2U;
          /* Extract the 32-bit memory address. */
          *address = (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 24U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 16U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += (uint32_t)SRecReaderHexStringToByte(&line[charIdx]) << 8U;
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          *address += SRecReaderHexStringToByte(&line[charIdx]);
          /* The number of bytes on the line must be > 5 (4 for address + 1 for cs. */
          if (bytesOnLine > 5U)
          {
            /* Set the data byte length, so the number of data bytes to extract. */
            *len = bytesOnLine - 5U;
          }
          else
          {
            /* Invalid byte count detected on the line. Flag error. */
            result = TBX_ERROR;
          }
          break;
        case LINE_TYPE_UNSUPPORTED:
        default:
          /* Not a line type with data to extract. This is not an error, but there is
           * just no data to extract. Therefore set the data length to 0 so that the
           * caller knows there was no data to extract.
           */
          *len = 0U;
          break;
      }
    }

    /* Only continue if all is okay so far. This means a properly extracted memory
     * address in the case of an S1, S2 or S3 line, or a valid unsupported line type in
     * which case the length is set to 0.
     */
    if (result == TBX_OK)
    {
      /* Skip the data extraction and copying if a NULL pointer was passed for data. */
      if (data != NULL)
      {
        /* Extract and copy the data bytes. Note that in the case of
         * LINE_TYPE_UNSUPPORTED, len is set to 0, so nothing is actually copied.
         */
        for (dataByteIdx = 0U; dataByteIdx < *len; dataByteIdx++)
        {
          /* Move character index two characters forward to the next byte. */
          charIdx += 2U;
          /* Extract the byte value and store it in the data buffer. */
          data[dataByteIdx] = SRecReaderHexStringToByte(&line[charIdx]);
        }
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderParseLine ***/


/************************************************************************************//**
** \brief     Inspects an S-record line to determine its type. Only S1, S2, and S3
**            lines are interesting, so those are the only ones we look for.
** \param     line A line from the S-Record.
** \return    S-record line type.
**
****************************************************************************************/
static tSRecLineType SRecReaderGetLineType(char const * line)
{
  tSRecLineType result = LINE_TYPE_UNSUPPORTED;

  /* Verify parameter. */
  TBX_ASSERT(line != NULL);

  /* Only continue with valid parameter. */
  if (line != NULL)
  {
    /* Only continue if the line starts with an 's' or and 'S' character. */
    if ( (line[0] == 's') || (line[0] == 'S') )
    {
      /* Filter out the supported line types, so S1, S2 or S3. */
      switch (line[1])
      {
        case '1':
          result = LINE_TYPE_S1;
          break;
        case '2':
          result = LINE_TYPE_S2;
          break;
        case '3':
          result = LINE_TYPE_S3;
          break;
        default:
          result = LINE_TYPE_UNSUPPORTED;
          break;
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderGetLineType ***/


/************************************************************************************//**
** \brief     Inspects an S1, S2 or S3 line from a Motorola S-Record file to
**            determine if the checksum at the end is corrrect.
** \param     line An S1, S2 or S3 line from the S-Record.
** \return    TBX_OK if the checksum is correct, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t SRecReaderVerifyChecksum(char const * line)
{
  uint8_t result = TBX_ERROR;
  uint8_t bytesOnLine;
  uint8_t checksum;
  uint8_t charIdx;

  /* Verify parameter. */
  TBX_ASSERT(line != NULL);

  /* Only continue with valid parameter. */
  if (line != NULL)
  {
    /* Adjust index to point to byte count value. */
    charIdx = 2U;
    /* Read out the number of byte values that follow on the line. */
    bytesOnLine = SRecReaderHexStringToByte(&line[charIdx]);
    /* Checksum starts with the byte count. */
    checksum = bytesOnLine;
    /* Adjust index to the first byte of the address. */
    charIdx += 2U;
    /* Add byte values of address and data, but not the final checksum. */
    do
    {
      /* Add the next byte value to the checksum. */
      checksum += SRecReaderHexStringToByte(&line[charIdx]);
      /* Update counter. */
      bytesOnLine--;
      /* Point to next hex string in the line. */
      charIdx += 2U;
    }
    while (bytesOnLine > 1U);
    /* The checksum is calculated by summing up the values of the byte count, address and
     * databytes and then taking the 1-complement of the sum's least significant byte.
     */
    checksum = ~checksum;

    /* Finally verify the calculated checksum with the one at the end of the line. */
    if (checksum == SRecReaderHexStringToByte(&line[charIdx]))
    {
      /* Checksum correct so update the result. */
      result = TBX_OK;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of SRecReaderVerifyChecksum ***/


/************************************************************************************//**
** \brief     Helper function to convert a sequence of 2 characters that represent
**            a hexadecimal value to the actual byte value.
**              Example: SRecReaderHexStringToByte("2f")  --> returns 47.
** \param     hexstring String beginning with 2 characters that represent a hexa-
**            decimal value.
** \return    The resulting byte value.
**
****************************************************************************************/
static uint8_t SRecReaderHexStringToByte(char const * hexstring)
{
  uint8_t result = 0U;
  /* Conversion table to convert a hexadecimal ASCII character to its 4-bit value. Note
   * that there are more efficient ways of doing this conversion, for example by
   * deducting '0' from the character value. However, MISRA does not allow typecasting
   * a character into an integer type. Note that this constant table was made static
   * to lower the stack load.
   */
  static const struct
  {
    char    c;
    uint8_t b;
  } hexConvTbl[] =
  {
    { '0', 0  }, { '1', 1  },  { '2', 2  },  { '3', 3  },
    { '4', 4  }, { '5', 5  },  { '6', 6  },  { '7', 7  },
    { '8', 8  }, { '9', 9  },  { 'a', 10 },  { 'A', 10 },
    { 'b', 11 }, { 'B', 11 },  { 'c', 12 },  { 'C', 12 },
    { 'd', 13 }, { 'D', 13 },  { 'e', 14 },  { 'E', 14 },
    { 'f', 15 }, { 'F', 15 }
  };
  uint8_t loopCnt;
  uint8_t tblIdx;
  uint8_t nibbles[2];

  /* Verify parameter. */
  TBX_ASSERT(hexstring != NULL);

  /* Only continue with valid parameter. */
  if (hexstring != NULL)
  {
    /* Loop through both hexadecimal characters to convert them to their respective
     * 4-bit value.
     */
    for (loopCnt = 0U; loopCnt < (sizeof(nibbles)/sizeof(nibbles[0])); loopCnt++)
    {
      /* Convert the hexadecimal ASCII character to its 4-bit value. */
      nibbles[loopCnt] = 0;
      for (tblIdx = 0U; tblIdx < (sizeof(hexConvTbl)/sizeof(hexConvTbl[0])); tblIdx++)
      {
        /* Is this the hexadecimal ASCII character to convert. */
        if (hexConvTbl[tblIdx].c == hexstring[loopCnt])
        {
          /* Store its 4-bit value in the nibbles array. */
          nibbles[loopCnt] = hexConvTbl[tblIdx].b;
          /* Done converting this hexadecimal ASCII character. */
          break;
        }
      }
    }
    /* Construct the actual resulting byte value from the nibbles. */
    result = (uint8_t)(nibbles[0] << 4) + nibbles[1];
  }

  /* Give the resuts back to the caller. */
  return result;
} /*** end of SRecReaderHexStringToByte ***/


/*********************************** end of srecreader.c *******************************/
