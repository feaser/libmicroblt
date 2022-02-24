/************************************************************************************//**
* \file         session.c
* \brief        Communication session module source file.
* \ingroup      Session
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
#include <microtbx.h>                       /* MicroTBX toolbox                        */
#include "session.h"                        /* Communication session module            */


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Pointer to the communication protocol that is linked. */
static tSessionProtocol const * protocolPtr = NULL;


/************************************************************************************//**
** \brief     Initializes the communication session module for the specified protocol.
** \param     protocol The session protocol module to link.
** \param     protocolSettings Pointer to structure with protocol specific settings.
**
****************************************************************************************/
void SessionInit(tSessionProtocol const * protocol, void const * protocolSettings)
{
  /* Check parameters. Note that the protocolSettings parameter is allowed to be NULL,
   * because not every protocol might need additional settings. 
   */
  TBX_ASSERT(protocol != NULL);
  
  /* Link the protocol module. */
  protocolPtr = protocol;
  
  /* Initialize the protocol and pass on the settings pointer. */
  if (protocolPtr != NULL)
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->Init != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->Init != NULL)
    {
      protocolPtr->Init(protocolSettings);
    }
  }
} /*** end of SessionInit ***/


/************************************************************************************//**
** \brief     Terminates the communication session module.
**
****************************************************************************************/
void SessionTerminate(void)
{
  /* Terminate the linked protocol. */
  if (protocolPtr != NULL) 
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->Terminate != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->Terminate != NULL)
    {
      protocolPtr->Terminate();
    }
  }
  /* Unlink the protocol module. */
  protocolPtr = NULL;
} /*** end of SessionTerminate ***/


/************************************************************************************//**
** \brief     Starts the firmware update session. This is where the connection with the
**            target is made and the bootloader on the target is activated.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t SessionStart(void)
{
  uint8_t result = TBX_ERROR;
  
  /* Pass the request on to the linked protocol module. */
  if (protocolPtr != NULL)
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->Start != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->Start != NULL)
    {
      result = protocolPtr->Start();
    }
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of SessionStart ***/


/************************************************************************************//**
** \brief    Stops the firmware update. This is where the bootloader starts the user
**           program on the target if a valid one is present. After this the connection
**           with the target is severed.
**
****************************************************************************************/
void SessionStop(void)
{
  /* Pass the request on to the linked protocol module. */
  if (protocolPtr != NULL)
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->Stop != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->Stop != NULL)
    {
      protocolPtr->Stop();
    }
  }
} /*** end of SessionStop ***/


/************************************************************************************//**
** \brief     Requests the bootloader to erase the specified range of memory on the
**            target. The bootloader aligns this range to hardware specified erase 
**            blocks.
** \param     address The starting memory address for the erase operation.
** \param     len The total number of bytes to erase from memory.
** \return    TBX_OK if successful, TBX_ERROR otherwise.
**
****************************************************************************************/
uint8_t SessionClearMemory(uint32_t address, uint32_t len)
{
  uint8_t result = TBX_ERROR;
  
  /* Check parameters. */
  TBX_ASSERT(len > 0U);
  
  /* Only continue if the parameters are vald. */
  if (len > 0U)
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->ClearMemory != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->ClearMemory != NULL)
    {
      /* Pass the request on to the linked protocol module. */
      result = protocolPtr->ClearMemory(address, len);
    }
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of SessionClearMemory ***/


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
uint8_t SessionWriteData(uint32_t address, uint32_t len, uint8_t const * data)
{
  uint8_t result = TBX_ERROR;

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && ((len > 0U)));
  
  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U)) /*lint !e774 */
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->WriteData != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->WriteData != NULL)
    {
      /* Pass the request on to the linked protocol module. */
      result = protocolPtr->WriteData(address, len, data);
    }
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of SessionWriteData ***/


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
uint8_t SessionReadData(uint32_t address, uint32_t len, uint8_t * data)
{
  uint8_t result = TBX_ERROR;

  /* Check parameters. */
  TBX_ASSERT((data != NULL) && (len > 0U));
  
  /* Only continue if the parameters are valid. */
  if ((data != NULL) && (len > 0U)) /*lint !e774 */
  {
    /* Verify the protocol's function pointer. */
    TBX_ASSERT(protocolPtr->ReadData != NULL);
    /* Only continue with a valid function pointer. */
    if (protocolPtr->ReadData != NULL)
    {
      /* Pass the request on to the linked protocol module. */
      result = protocolPtr->ReadData(address, len, data);
    }
  }
  /* Give the result back to the caller. */
  return result;
} /*** end of SessionReadData ***/


/*********************************** end of session.c **********************************/
