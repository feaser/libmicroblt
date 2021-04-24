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
#include "session.h"                        /* Communication session module            */
#include "xcploader.h"                      /* XCP loader module                       */
#include "firmware.h"                       /* Firmware reader module                  */
#include "srecreader.h"                     /* S-record firmware file reader           */


/****************************************************************************************
*             S E S S I O N   L A Y E R S
****************************************************************************************/
/************************************************************************************//**
** \brief     Initializes the firmware update session for a specific communication
**            protocol. This function is typically called once at the start of the
**            firmware update.
** \param     type The communication protocol to use for this session. It should
**            be a BLT_SESSION_xxx value.
** \param     settings Pointer to a structure with communication protocol specific
**            settings.
**
****************************************************************************************/
void BltSessionInit(uint32_t type, void const * settings)
{
  /* Check parameters. Note that the settings-pointers are allowed to be NULL in case
   * no additional settings are needed for the specified session or transport type.
   */
  TBX_ASSERT(type == BLT_SESSION_XCP_V10);

  /* Initialize the correct session. */
  if (type == BLT_SESSION_XCP_V10)
  {
    /* Verify settings parameter because the XCP loader requires them. */
    TBX_ASSERT(settings != NULL);
    /* Only continue if the settings parameter is valid. */
    if (settings != NULL)
    {
      /* Cast session settings to the correct type. */
      tBltSessionSettingsXcpV10 const * bltSessionSettingsXcpV10Ptr = settings;
      /* Convert session settings to the format supported by the XCP loader module. */
      tXcpLoaderSettings xcpLoaderSettings;
      xcpLoaderSettings.timeoutT1   = bltSessionSettingsXcpV10Ptr->timeoutT1;
      xcpLoaderSettings.timeoutT3   = bltSessionSettingsXcpV10Ptr->timeoutT3;
      xcpLoaderSettings.timeoutT4   = bltSessionSettingsXcpV10Ptr->timeoutT4;
      xcpLoaderSettings.timeoutT5   = bltSessionSettingsXcpV10Ptr->timeoutT5;
      xcpLoaderSettings.timeoutT6   = bltSessionSettingsXcpV10Ptr->timeoutT6;
      xcpLoaderSettings.timeoutT7   = bltSessionSettingsXcpV10Ptr->timeoutT7;
      xcpLoaderSettings.connectMode = bltSessionSettingsXcpV10Ptr->connectMode;
      /* Perform actual session initialization. */
      SessionInit(XcpLoaderGetProtocol(), &xcpLoaderSettings);
    }
  }
} /*** end of BltSessionInit ***/


/************************************************************************************//**
** \brief     Terminates the firmware update session. This function is typically called
**            once at the end of the firmware update.
**
****************************************************************************************/
void BltSessionTerminate(void)
{
  /* Terminate the session. */
  SessionTerminate();
} /*** end of BltSessionTerminate ***/


/************************************************************************************//**
** \brief     Starts the firmware update session. This is were the library attempts to
**            activate and connect with the bootloader running on the target, through
**            the transport layer that was specified during the session's initialization.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t BltSessionStart(void)
{
  uint8_t result;

  /* Attempt to start the session. */
  result = SessionStart();

  /* Give the result back to the caller. */
  return result;
} /*** end of BltSessionStart ***/


/************************************************************************************//**
** \brief     Stops the firmware update session. This is there the library disconnects
**            the transport layer as well.
**
****************************************************************************************/
void BltSessionStop(void)
{
  /* Stop the session. */
  SessionStop();
} /*** end of BltSessionStop ***/


/************************************************************************************//**
** \brief     Requests the target to erase the specified range of memory on the target.
**            Note that the target automatically aligns this to the erasable memory
**            block sizes. This typically results in more memory being erased than the
**            range that was specified here. Refer to the target implementation for
**            details.
** \param     address The starting memory address for the erase operation.
** \param     len The total number of bytes to erase from memory.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t BltSessionClearMemory(uint32_t address, uint32_t len)
{
  uint8_t result = TBX_OK;

  /* Check parameters. */
  TBX_ASSERT(len > 0U);

  /* Only continue if the parameters are valid. */
  if (len > 0U)
  {
    /* Pass the request on to the session module. */
    result = SessionClearMemory(address, len);
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of BltSessionClearMemory ***/


/************************************************************************************//**
** \brief     Requests the target to program the specified data to memory. Note that it
**            is the responsibility of the application to make sure the memory range was
**            erased beforehand.
** \param     address The starting memory address for the write operation.
** \param     len The number of bytes in the data buffer that should be written.
** \param     data Pointer to the byte array with data to write.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t BltSessionWriteData(uint32_t address, uint32_t len, uint8_t const * data)
{
  uint8_t result = TBX_ERROR;

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && (len > 0U));

  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U))
  {
    /* Pass the request on to the session module. */
    result = SessionWriteData(address, len, data);
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of BltSessionWriteData ***/


/************************************************************************************//**
** \brief     Requests the target to upload the specified range from memory and store its
**            contents in the specified data buffer.
** \param     address The starting memory address for the read operation.
** \param     len The number of bytes to upload from the target and store in the data
**            buffer.
** \param     data Pointer to the byte array where the uploaded data should be stored.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t BltSessionReadData(uint32_t address, uint32_t len, uint8_t * data)
{
  uint8_t result = TBX_ERROR;

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && (len > 0U));

  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U))
  {
    /* Pass the request on to the session module. */
    result = SessionReadData(address, len, data);
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of BltSessionReadData ***/


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
** \brief     Obtains the total number of data bytes present in all segments of the
**            firmware file.
** \return    Total number of data bytes.
**
****************************************************************************************/
uint32_t BltFirmwareGetTotalSize(void)
{
  uint32_t result = 0U;
  uint8_t  segmentIdx;
  uint32_t segmentAddress = 0U;

  /* Loop through all segments. */
  for (segmentIdx = 0; segmentIdx < BltFirmwareSegmentGetCount(); segmentIdx++)
  {
    /* Obtain info about the next segment and add the segment's length to the result. */
    result += BltFirmwareSegmentGetInfo(segmentIdx, &segmentAddress);
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of BltFirmwareGetTotalSize ***/


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
uint32_t BltFirmwareSegmentGetInfo(uint8_t idx, uint32_t * address)
{
  /* Pass the request on to the firmware reader module. */
  return FirmwareSegmentGetInfo(idx, address);
} /*** end of BltFirmwareSegmentGetInfo ***/


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
