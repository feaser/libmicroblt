/************************************************************************************//**
* \file         xcploader.c
* \brief        XCP Loader module source file.
* \ingroup      XcpLoader
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
#include "session.h"                        /* Communication session module            */
#include "xcploader.h"                      /* XCP communication protocol module       */
#include "port.h"                           /* Port module                             */


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief The settings that should be used by the XCP loader. */
static tXcpLoaderSettings xcpSettings;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void    XcpLoaderInit(void const * settings);
static void    XcpLoaderTerminate(void);
static uint8_t XcpLoaderStart(void);
static void    XcpLoaderStop(void);
static uint8_t XcpLoaderClearMemory(uint32_t address, uint32_t len);
static uint8_t XcpLoaderWriteData(uint32_t address, uint32_t len, uint8_t const * data);
static uint8_t XcpLoaderReadData(uint32_t address, uint32_t len, uint8_t * data);
static uint8_t XcpExchangePacket(tPortXcpPacket const * txPacket,
                                 tPortXcpPacket * rxPacket, uint16_t timeout);


/***********************************************************************************//**
** \brief     Obtains a pointer to the session communication protocol interface, so that
**            it can be linked to the session module.
** \return    Pointer to session communication protocol interface structure.
**
****************************************************************************************/
tSessionProtocol const * XcpLoaderGetProtocol(void)
{
  /** \brief Session communication protocol interface structure filled with XCP
   *         communication protocol specifics.
   */
  static const tSessionProtocol xcpLoader =
  {
    .Init = XcpLoaderInit,
    .Terminate = XcpLoaderTerminate,
    .Start = XcpLoaderStart,
    .Stop = XcpLoaderStop,
    .ClearMemory = XcpLoaderClearMemory,
    .WriteData = XcpLoaderWriteData,
    .ReadData = XcpLoaderReadData
  };

  /* Give the pointer to the session communication protocol interface structure back to
   * the caller.
   */
  return &xcpLoader;
} /*** end of XcpLoaderGetProtocol ***/


/************************************************************************************//**
** \brief     Initializes the XCP communication protocol module.
** \param     protocolSettings Pointer to structure with XCP protocol specific settings.
**
****************************************************************************************/
static void XcpLoaderInit(void const * settings)
{
  tXcpLoaderSettings const * xcpSettingsPtr = settings;

  /* Verify parameter. */
  TBX_ASSERT(settings != NULL);

  /* Reset the XCP protocol specific settings. */
  xcpSettings.timeoutT1 = 1000U;
  xcpSettings.timeoutT3 = 2000U;
  xcpSettings.timeoutT4 = 10000U;
  xcpSettings.timeoutT5 = 1000U;
  xcpSettings.timeoutT6 = 50U;
  xcpSettings.timeoutT7 = 2000U;
  xcpSettings.connectMode = 0U;

  /* Only continue with valid parameter. */
  if (settings != NULL)
  {
    /* Shallow copy the XCP settings for later usage. */
    xcpSettings = *xcpSettingsPtr;
  }
} /*** end of XcpLoaderInit ***/


/************************************************************************************//**
** \brief     Terminates the XCP communication protocol module.
**
****************************************************************************************/
static void XcpLoaderTerminate(void)
{
  /* Nothing to do currently, so just leave it empty for now. */
} /*** end of XcpLoaderTerminate ***/


/************************************************************************************//**
** \brief     Starts the firmware update session. This is where the connection with the
**            target is made and the bootloader on the target is activated.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderStart(void)
{
  uint8_t result = TBX_ERROR;

  /* TODO ##Vg Implement XcpLoaderStart(). */

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderStart ***/


/************************************************************************************//**
** \brief    Stops the firmware update. This is where the bootloader starts the user
**           program on the target if a valid one is present. After this the connection
**           with the target is severed.
**
****************************************************************************************/
static void XcpLoaderStop(void)
{
  /* TODO ##Vg Implement XcpLoaderStop(). */
} /*** end of XcpLoaderStop ***/


/************************************************************************************//**
** \brief     Requests the bootloader to erase the specified range of memory on the
**            target. The bootloader aligns this range to hardware specified erase
**            blocks.
** \param     address The starting memory address for the erase operation.
** \param     len The total number of bytes to erase from memory.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderClearMemory(uint32_t address, uint32_t len)
{
  uint8_t result = TBX_ERROR;

  TBX_UNUSED_ARG(address);

  /* Verify parameters. */
  TBX_ASSERT(len > 0U);

  /* Only continue with valid parameter. */
  if (len > 0U)
  {
    /* TODO ##Vg Implement XcpLoaderClearMemory(). */
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderClearMemory ***/


/************************************************************************************//**
** \brief     Requests the bootloader to program the specified data to memory. In case of
**            non-volatile memory, the application needs to make sure the memory range
**            was erased beforehand.
** \param     address The starting memory address for the write operation.
** \param     len The number of bytes in the data buffer that should be written.
** \param     data Pointer to the byte array with data to write.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderWriteData(uint32_t address, uint32_t len, uint8_t const * data)
{
  uint8_t result = TBX_ERROR;

  TBX_UNUSED_ARG(address);

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && ((len > 0U)));

  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U)) /*lint !e774 */
  {
    /* TODO ##Vg Implement XcpLoaderWriteData(). */
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderWriteData ***/


/************************************************************************************//**
** \brief     Request the bootloader to upload the specified range of memory. The data is
**            stored in the data byte array to which the pointer was specified.
** \param     address The starting memory address for the read operation.
** \param     len The number of bytes to upload from the target and store in the data
**            buffer.
** \param     data Pointer to the byte array where the uploaded data should be stored.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderReadData(uint32_t address, uint32_t len, uint8_t * data)
{
  uint8_t result = TBX_ERROR;

  TBX_UNUSED_ARG(address);

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && (len > 0U));

  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U)) /*lint !e774 */
  {
    /* TODO ##Vg Implement XcpLoaderWriteData(). */
    data[0] = 0U;
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderReadData ***/


/************************************************************************************//**
** \brief     Transmits an XCP packet on the transport layer and attempts to receive the
**            response packet within the specified timeout.
** \param     txPacket Pointer to the packet to transmit.
** \param     rxPacket Pointer where the received packet info is stored.
** \param     timeout Maximum time in milliseconds to wait for the reception of the
**            response packet.
** \return    TBX_OK if successful and a response packet was received, TBX_ERROR
**            otherwise.
**
****************************************************************************************/
static uint8_t XcpExchangePacket(tPortXcpPacket const * txPacket,
                                 tPortXcpPacket * rxPacket, uint16_t timeout)
{
  uint8_t result = TBX_ERROR;

  TBX_UNUSED_ARG(timeout);

  /* Check parameters. */
  TBX_ASSERT((txPacket != NULL) && (rxPacket != NULL) && (timeout > 0U));

  /* Only continue if the parameters are valid. */
  if ((txPacket != NULL) && (rxPacket != NULL) && (timeout > 0U))
  {
    /* TODO ##Vg Implement XcpExchangePacket(). Note that for accessing the port
     * specifics, you can use something like: PortGet()->SystemGetTime();
     */
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpExchangePacket ***/


/*********************************** end of xcploader.c ********************************/
