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
* Macro definitions
****************************************************************************************/
/* XCP command codes as defined by the protocol currently supported by this module. */
#define XCPLOADER_CMD_CONNECT         (0xFFU)    /**< XCP connect command code.        */
#define XCPLOADER_CMD_GET_STATUS      (0xFDU)    /**< XCP get status command code.     */
#define XCPLOADER_CMD_PROGRAM_START   (0xD2U)    /**< XCP program start command code.  */
#define XCPLOADER_CMD_PROGRAM_RESET   (0xCFU)    /**< XCP program reset command code.  */
#define XCPLOADER_CMD_PROGRAM         (0xD0U)    /**< XCP program command code.        */

/* XCP supported resources. */
#define XCPLOADER_RESOURCE_PGM        (0x10U)    /**< ProGraMing resource.             */

/* XCP response packet IDs as defined by the protocol. */
#define XCPLOADER_CMD_PID_RES         (0xFFU)    /**< Positive response.               */

/** \brief Number of retries to connect to the XCP slave. */
#define XCPLOADER_CONNECT_RETRIES     (5U)


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief The settings that should be used by the XCP loader. */
static tXcpLoaderSettings xcpSettings;

/** \brief Flag to keep track of the connection status. */
static uint8_t            xcpConnected;

/** \brief Store the byte ordering of the XCP slave. */
static uint8_t            xcpSlaveIsIntel;

/** \brief The max number of bytes in the command transmit object (master->slave). */
static uint16_t           xcpMaxCto;

/** \brief The max number of bytes in the command transmit object (master->slave) during
 *         a programming session.
 */
static uint16_t           xcpMaxProgCto;

/** \brief The max number of bytes in the data transmit object (slave->master). */
static uint16_t           xcpMaxDto;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
/* Protocol functions for linking to the session module. */
static void    XcpLoaderInit(void const * settings);
static void    XcpLoaderTerminate(void);
static uint8_t XcpLoaderStart(void);
static void    XcpLoaderStop(void);
static uint8_t XcpLoaderClearMemory(uint32_t address, uint32_t len);
static uint8_t XcpLoaderWriteData(uint32_t address, uint32_t len, uint8_t const * data);
static uint8_t XcpLoaderReadData(uint32_t address, uint32_t len, uint8_t * data);
/* Port dependent functions for low level XCP communication packet exchange. */
static uint8_t XcpExchangePacket(tPortXcpPacket const * txPacket,
                                 tPortXcpPacket * rxPacket, uint16_t timeout);
/* XCP Command functions. */
static uint8_t XcpLoaderSendCmdConnect(void);
static uint8_t XcpLoaderSendCmdGetStatus(uint8_t * protectedResources);
static uint8_t XcpLoaderSendCmdProgramStart(void);
static uint8_t XcpLoaderSendCmdProgramReset(void);
static uint8_t XcpLoaderSendCmdProgram(uint8_t len, uint8_t const * data);


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

  /* Initialize locals. */
  xcpConnected = TBX_FALSE;
  xcpSlaveIsIntel = TBX_FALSE;
  xcpMaxCto = 0U;
  xcpMaxProgCto = 0U;
  xcpMaxDto = 0U;

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
  /* Make sure to stop the firmware update, in case was is in progress. */
  XcpLoaderStop();
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
  uint8_t retryCnt;
  uint8_t protectedResources = 0U;

  /* Make sure the session is stopped before starting a new one. */
  XcpLoaderStop();

  /* Attempt to connect to the target with a finite amount of retries. */
  for (retryCnt=0U; retryCnt<XCPLOADER_CONNECT_RETRIES; retryCnt++)
  {
    /* Send the connect command. */
    if (XcpLoaderSendCmdConnect() == TBX_OK)
    {
      /* Update connection state. */
      xcpConnected = TBX_TRUE;
      /* Set a positive result and only negate it upon error detection from here on. */
      result = TBX_OK;
      /* Connected so no need to retry. */
      break;
    }
  }

  /* Only continue when connected to the target. */
  if (result == TBX_OK)
  {
    /* Obtain the current resource protection status. */
    if (XcpLoaderSendCmdGetStatus(&protectedResources) != TBX_OK)
    {
      /* Could not obtain resource protection status. Flag error. */
      result = TBX_ERROR;
    }
  }

  /* Only continue when resource protection status was obtained. */
  if (result == TBX_OK)
  {
    /* Check if the programming resource needs to be unlocked. */
    if ((protectedResources & XCPLOADER_RESOURCE_PGM) != 0U)
    {
      /* TODO Implement support for seed/key mechanism to unlock programming resource. */
      /* Support for seed/key unlocking mechanism not yet implemented. Trigger error
       * in case the programming resource is currently locked.
       */
      result = TBX_ERROR;
    }
  }

  /* Only continue with an unlocked programming resource. */
  if (result == TBX_OK)
  {
    /* Attempt to place the target in programming mode. */
    if (XcpLoaderSendCmdProgramStart() != TBX_OK)
    {
      /* Could not activate programming mode. Flag error. */
      result = TBX_ERROR;
    }
  }

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
  /* Only continue if actually connected. */
  if (xcpConnected == TBX_TRUE)
  {
    /* End the programming session by sending the program command with size 0. */
    if (XcpLoaderSendCmdProgram(0U, NULL) == TBX_OK)
    {
      /* Disconnect the target. Here the reset command is used instead of the disconnect
       * command, because the bootloader should start the user program on the target.
       */
      (void)XcpLoaderSendCmdProgramReset();
    }
    /* Reset connection status. */
    xcpConnected = TBX_FALSE;
  }
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
    /* TODO Implement XcpLoaderClearMemory(). */
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
    /* TODO Implement XcpLoaderWriteData(). */
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
    /* TODO Implement XcpLoaderReadData(). */
    data[0] = 0U;
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderReadData ***/


/************************************************************************************//**
** \brief     Transmits an XCP packet on the transport layer and attempts to receive the
**            response packet within the specified timeout. Note that this function is
**            blocking.
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
  uint8_t  result = TBX_ERROR;
  uint8_t  portFcnsValid = TBX_FALSE;
  uint32_t startTime;
  uint32_t deltaTime;
  uint8_t  stopReception = TBX_FALSE;

  /* Check parameters. */
  TBX_ASSERT((txPacket != NULL) && (rxPacket != NULL) && (timeout > 0U));

  /* A few port specific function will be used. Make sure they are valid before calling
   * them.
   */
  if (PortGet() != NULL)
  {
    /* Assume the port functions are okay and only flag an error when one is not okay.
     * Note that when combining these conditionals, the MISRA check complains about
     * side effects, so do them individually.
     */
    portFcnsValid = TBX_TRUE;
    if (PortGet()->SystemGetTime == NULL)     { portFcnsValid = TBX_FALSE; }
    if (PortGet()->XcpTransmitPacket == NULL) { portFcnsValid = TBX_FALSE; }
    if (PortGet()->XcpReceivePacket == NULL)  { portFcnsValid = TBX_FALSE; }
  }
  TBX_ASSERT(portFcnsValid == TBX_TRUE);

  /* Only continue if the parameters and the port functions are valid. */
  if ( (txPacket != NULL) && (rxPacket != NULL) && (timeout > 0U) &&
       (portFcnsValid == TBX_TRUE) )
  {
    /* Set the result to success at this point and only update it upon error. */
    result = TBX_OK;

    /* Request the port to transmit the XCP packet using the application's implemented
     * transport layer.
     */
    if (PortGet()->XcpTransmitPacket(txPacket) != TBX_OK)
    {
      /* Flag error. */
      result = TBX_ERROR;
    }

    /* Only continue if all is okay so far. */
    if (result == TBX_OK)
    {
      /* Store the start time of the response reception. */
      startTime = PortGet()->SystemGetTime();

      /* Attempt to receive the XCP response packet within the timeout in a blocking
       * manner.
       */
      while (stopReception == TBX_FALSE)
      {
        /* Check if a new XCP reponse package was received. */
        if (PortGet()->XcpReceivePacket(rxPacket) == TBX_TRUE)
        {
          /* Response complete, so stop looping. */
          stopReception = TBX_TRUE;
        }
        /* Check if the timeout time elapsed before continuing with the XCP packet
         * reception.
         */
        else
        {
          /* Calculate elapsed time while waiting for the XCP response packet. Note that
           * this calculation is 32-bit time overflow safe.
           */
          deltaTime = PortGet()->SystemGetTime() - startTime;
          if (deltaTime > timeout)
          {
            /* Reception timeout occurred. Update the result to reflect this and stop
             * looping.
             */
            result = TBX_ERROR;
            stopReception = TBX_TRUE;
          }
        }
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpExchangePacket ***/


/************************************************************************************//**
** \brief     Sends the XCP Connect command.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderSendCmdConnect(void)
{
  uint8_t        result = TBX_OK;
  tPortXcpPacket reqPacket;
  tPortXcpPacket resPacket;

  /* Prepare the command request packet. */
  reqPacket.data[0] = XCPLOADER_CMD_CONNECT;
  reqPacket.data[1] = xcpSettings.connectMode;
  reqPacket.len = 2U;

  /* Send the request packet and attempt to receive the response packet. */
  if (XcpExchangePacket(&reqPacket, &resPacket, xcpSettings.timeoutT6) != TBX_OK)
  {
    /* Did not receive a response packet in time. Flag the error. */
    result = TBX_ERROR;
  }

  /* Only continue if a response packet was received. */
  if (result == TBX_OK)
  {
    /* Check if the response was valid. */
    if ( (resPacket.len != 8U) || (resPacket.data[0U] != XCPLOADER_CMD_PID_RES) )
    {
      /* Not a valid or positive response. Flag the error. */
      result = TBX_ERROR;
    }
  }

  /* Only process the response data in case the response was valid. */
  if (result == TBX_OK)
  {
    /* Store slave's byte ordering information. */
    if ((resPacket.data[2] & 0x01U) == 0U)
    {
      xcpSlaveIsIntel = TBX_TRUE;
    }
    else
    {
      xcpSlaveIsIntel = TBX_FALSE;
    }
    /* Store max number of bytes the slave allows for master->slave packets. */
    xcpMaxCto = resPacket.data[3];
    xcpMaxProgCto = xcpMaxCto;
    /* Store max number of bytes the slave allows for slave->master packets. */
    if (xcpSlaveIsIntel == TBX_TRUE)
    {
      xcpMaxDto = (uint16_t)(resPacket.data[4] + ((uint16_t)resPacket.data[5] << 8U));
    }
    else
    {
      xcpMaxDto = (uint16_t)(resPacket.data[5] + ((uint16_t)resPacket.data[4] << 8U));
    }
    /* Double check size configuration. CTO length can be adjusted. DTO not. */
    if (xcpMaxCto > PORT_XCP_PACKET_SIZE_MAX)
    {
      xcpMaxCto = PORT_XCP_PACKET_SIZE_MAX;
    }
    if (xcpMaxDto > PORT_XCP_PACKET_SIZE_MAX)
    {
      /* Cannot process slave response messages of this length. Flag error. */
      result = TBX_ERROR;
    }
    if ( (xcpMaxCto == 0U) || (xcpMaxDto == 0U) )
    {
      /* Invalid CTO or DTO settings detected. Flag error. */
      result = TBX_ERROR;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderSendCmdConnect ***/


/************************************************************************************//**
** \brief     Sends the XCP Get Status command.
** \param     protectedResources Current resource protection status.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderSendCmdGetStatus(uint8_t * protectedResources)
{
  uint8_t        result = TBX_ERROR;
  tPortXcpPacket reqPacket;
  tPortXcpPacket resPacket;

  /* Verify parameter. */
  TBX_ASSERT(protectedResources != NULL);

  /* Only continue with valid parameter. */
  if (protectedResources != NULL)
  {
    /* Set a positive result and only negate upon error detection from here on. */
    result = TBX_OK;
    /* Prepare the command request packet. */
    reqPacket.data[0] = XCPLOADER_CMD_GET_STATUS;
    reqPacket.len = 1U;

    /* Send the request packet and attempt to receive the response packet. */
    if (XcpExchangePacket(&reqPacket, &resPacket, xcpSettings.timeoutT1) != TBX_OK)
    {
      /* Did not receive a response packet in time. Flag the error. */
      result = TBX_ERROR;
    }

    /* Only continue if a response packet was received. */
    if (result == TBX_OK)
    {
      /* Check if the response was valid. */
      if ( (resPacket.len != 6U) || (resPacket.data[0U] != XCPLOADER_CMD_PID_RES) )
      {
        /* Not a valid or positive response. Flag the error. */
        result = TBX_ERROR;
      }
    }

    /* Only process the response data in case the response was valid. */
    if (result == TBX_OK)
    {
      /* Store the current resource protection status. */
      *protectedResources = resPacket.data[2];
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderSendCmdGetStatus ***/


/************************************************************************************//**
** \brief     Sends the XCP PROGRAM START command.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderSendCmdProgramStart(void)
{
  uint8_t        result = TBX_OK;
  tPortXcpPacket reqPacket;
  tPortXcpPacket resPacket;

  /* Prepare the command request packet. */
  reqPacket.data[0] = XCPLOADER_CMD_PROGRAM_START;
  reqPacket.len = 1U;

  /* Send the request packet and attempt to receive the response packet. */
  if (XcpExchangePacket(&reqPacket, &resPacket, xcpSettings.timeoutT3) != TBX_OK)
  {
    /* Did not receive a response packet in time. Flag the error. */
    result = TBX_ERROR;
  }

  /* Only continue if a response packet was received. */
  if (result == TBX_OK)
  {
    /* Check if the response was valid. */
    if ( (resPacket.len != 7U) || (resPacket.data[0U] != XCPLOADER_CMD_PID_RES) )
    {
      /* Not a valid or positive response. Flag the error. */
      result = TBX_ERROR;
    }
  }

  /* Only process the response data in case the response was valid. */
  if (result == TBX_OK)
  {
    /* Store max number of bytes the slave allows for master->slave packets during the
     * programming session.
     */
    xcpMaxProgCto = resPacket.data[3];
    if (xcpMaxProgCto > PORT_XCP_PACKET_SIZE_MAX)
    {
      xcpMaxProgCto = PORT_XCP_PACKET_SIZE_MAX;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderSendCmdProgramStart ***/


/************************************************************************************//**
** \brief     Sends the XCP PROGRAM RESET command. Note that this command is a bit
**            different as in it does not require a response.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderSendCmdProgramReset(void)
{
  uint8_t        result = TBX_OK;
  tPortXcpPacket reqPacket;
  tPortXcpPacket resPacket;

  /* Prepare the command request packet. */
  reqPacket.data[0] = XCPLOADER_CMD_PROGRAM_RESET;
  reqPacket.len = 1U;

  /* Send the request packet and attempt to receive the response packet. Note that it is
   * okay if no response is received as this is allowed for the program reset command.
   * Just make sure to only process the response if one was actually received.
   */
  if (XcpExchangePacket(&reqPacket, &resPacket, xcpSettings.timeoutT5) == TBX_OK)
  {
    /* Check if the response was valid. */
    if ( (resPacket.len != 1U) || (resPacket.data[0U] != XCPLOADER_CMD_PID_RES) )
    {
      /* Not a valid or positive response. Flag the error. */
      result = TBX_ERROR;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderSendCmdProgramReset ***/


/************************************************************************************//**
** \brief     Sends the XCP PROGRAM command.
** \param     len Number of bytes in the data array to program.
** \param     data Array with data bytes to program.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
static uint8_t XcpLoaderSendCmdProgram(uint8_t len, uint8_t const * data)
{
  uint8_t        result = TBX_ERROR;
  tPortXcpPacket reqPacket;
  tPortXcpPacket resPacket;
  uint8_t        cnt;

  /* Verify parameter. Note that a NULL pointer for parameter data is allowed in case
   * a lenght of 0 was specified.
   */
  if (len > 0U)
  {
    /* Verify parameter. */
    TBX_ASSERT(data != NULL);
  }

  /* Only continue if this number of bytes actually fits in this command. */
  if( (len <= (xcpMaxProgCto-2U)) && (xcpMaxProgCto <= PORT_XCP_PACKET_SIZE_MAX) )
  {
    /* Set a positive result and only negate upon error detection from here on. */
    result = TBX_OK;
    /* Prepare the command request packet. */
    reqPacket.data[0] = XCPLOADER_CMD_PROGRAM;
    reqPacket.data[1] = len;

    /* Only access data if it is not a NULL pointer. */
    if (data != NULL)
    {
      /* Copy the date bytes to program. */
      for (cnt=0U; cnt<len; cnt++)
      {
        reqPacket.data[cnt+2U] = data[cnt];
      }
    }
    reqPacket.len = len + 2U;

    /* Send the request packet and attempt to receive the response packet. */
    if (XcpExchangePacket(&reqPacket, &resPacket, xcpSettings.timeoutT5) != TBX_OK)
    {
      /* Did not receive a response packet in time. Flag the error. */
      result = TBX_ERROR;
    }

    /* Only continue if a response packet was received. */
    if (result == TBX_OK)
    {
      /* Check if the response was valid. */
      if ( (resPacket.len != 1U) || (resPacket.data[0U] != XCPLOADER_CMD_PID_RES) )
      {
        /* Not a valid or positive response. Flag the error. */
        result = TBX_ERROR;
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of XcpLoaderSendCmdProgram ***/


/*********************************** end of xcploader.c ********************************/
