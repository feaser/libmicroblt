/************************************************************************************//**
* \file         session.h
* \brief        Communication session module header file.
* \ingroup      Session
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
/************************************************************************************//**
* \defgroup   Session Communication Session Module
* \brief      Module with functionality to communicate with the bootloader on the target
*             system.
* \ingroup    Library
* \details
* The Communication Session module handles the communication with the bootloader during
* firmware updates on the target system. It contains an interface to link the desired
* communication protocol that should be used for the communication. For example the XCP
* protocol.
****************************************************************************************/
#ifndef SESSION_H
#define SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Session communication protocol interface. */
typedef struct
{
  /** \brief Initializes the protocol module. */
  void    (* Init) (void const * settings);

  /** \brief Terminates the protocol module. */
  void    (* Terminate) (void);

  /** \brief Starts the firmware update session. This is where the connection with the
   *         target is made and the bootloader on the target is activated.
   */
  uint8_t (* Start) (void);

  /** \brief Stops the firmware update. This is where the bootloader starts the user
   *         program on the target if a valid one is present. After this the connection
   *         with the target is severed.
   */
  void    (* Stop) (void);

  /** \brief Requests the bootloader to erase the specified range of memory on the
   *         target. The bootloader aligns this range to hardware specified erase blocks.
   */
  uint8_t (* ClearMemory) (uint32_t address, uint32_t len);

  /** \brief Requests the bootloader to program the specified data to memory. In case of
   *         non-volatile memory, the application needs to make sure the memory range
   *         was erased beforehand.
   */
  uint8_t (* WriteData) (uint32_t address, uint32_t len, uint8_t const * data);

  /** \brief Request the bootloader to upload the specified range of memory. The data is
   *         stored in the data byte array to which the pointer was specified.
   */
  uint8_t (* ReadData) (uint32_t address, uint32_t len, uint8_t * data);
} tSessionProtocol;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void    SessionInit(tSessionProtocol const * protocol, void const * protocolSettings);
void    SessionTerminate(void);
uint8_t SessionStart(void);
void    SessionStop(void);
uint8_t SessionClearMemory(uint32_t address, uint32_t len);
uint8_t SessionWriteData(uint32_t address, uint32_t len, uint8_t const * data);
uint8_t SessionReadData(uint32_t address, uint32_t len, uint8_t * data);


#ifdef __cplusplus
}
#endif

#endif /* SESSION_H */
/********************************* end of session.h ************************************/

 
 
