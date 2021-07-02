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
#include <ctype.h>                          /* for toupper() etc.                      */
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
/** \brief Structure that represents the handle to the S-record file, which groups all
 *         its relevent data.
 */
typedef struct
{
  /** \brief FatFS file object handle. */
  FIL      file;
  /** \brief Byte buffer for storing a line from the S-record file. */
  char     lineBuf[SREC_LINE_BUFFER_SIZE];
  /** \brief Byte buffer for storing the data from an S-record with the help of function
   *         SRecReaderParseLine().
   */
  uint8_t  lineDataBuf[SREC_LINE_BUFFER_SIZE/2];
  /** \brief Byte buffer for storing data extracted from an S-record with the help of
   *         function SRecReaderSegmentGetNextData().
   */
  uint8_t  dataBuf[SREC_DATA_BUFFER_SIZE];
  /** \brief Maximum number of firmware data bytes on the longest S-record line. */
  uint16_t maxLineData;
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
static uint8_t         SRecReaderParseLine(char const * line, uint32_t * address,
                                           uint8_t * len, uint32_t * data);
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
         * Note that SRecReaderParseLine() still needs to be implemented.
         * ============ CONTINUE HERE =============
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
**
****************************************************************************************/
static uint8_t SRecReaderParseLine(char const * line, uint32_t * address,
                                   uint8_t * len, uint32_t * data)
{
  uint8_t result = TBX_ERROR;

  /* Verify parameters. */
  TBX_ASSERT((line != NULL) && (address != NULL) && (len != NULL) && (data != NULL));

  /* Only continue with valid parameters. */
  if ((line != NULL) && (address != NULL) && (len != NULL) && (data != NULL))
  {
    /* TODO ##Vg Implement SRecReaderParseLine(). */
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
