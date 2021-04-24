/************************************************************************************//**
* \file         xcploader.h
* \brief        XCP Loader module header file.
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
/************************************************************************************//**
* \defgroup   XcpLoader XCP version 1.0 protocol
* \brief      This module implements the XCP communication protocol that can be linked 
*             to the Session module. 
* \ingroup    Session
* \details
* This XCP Loader module contains functionality according to the standardized XCP 
* protocol version 1.0. XCP is a universal measurement and calibration communication 
* protocol. Note that only those parts of the XCP master functionality are implemented 
* that are applicable to performing a firmware update on the slave. This means 
* functionality for reading, programming, and erasing (non-volatile) memory.
****************************************************************************************/
#ifndef XCPLOADER_H
#define XCPLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* TODO ##Vg figure out how to add seed/key functionality. In LibOpenBLT it is linked
 * as an entry like this:
 *     char const * seedKeyFile;
 * Perhaps simple app/port specific hook functions, just like those for packet rx/tx?
 */

/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief XCP protocol specific settings. */
typedef struct
{
  /** \brief Command response timeout in milliseconds. */
  uint16_t timeoutT1;            
  /** \brief Start programming timeout in milliseconds. */
  uint16_t timeoutT3;            
  /** \brief Erase memory timeout in milliseconds. */
  uint16_t timeoutT4;            
  /** \brief Program memory and reset timeout in milliseconds. */
  uint16_t timeoutT5;            
  /** \brief Connect response timeout in milliseconds. */
  uint16_t timeoutT6;
  /** \brief Busy wait timer timeout in milliseconds. */
  uint16_t timeoutT7;     
  /** \brief Connection mode used in the XCP connect command. */
  uint8_t  connectMode;
} tXcpLoaderSettings;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
tSessionProtocol const * XcpLoaderGetProtocol(void);  

#ifdef __cplusplus
}
#endif

#endif /* XCPLOADER_H */
/*********************************** end of xcploader.h ********************************/
