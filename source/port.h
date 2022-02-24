/************************************************************************************//**
* \file         port.h
* \brief        Port module header file.
* \ingroup      Port
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
/************************************************************************************//**
* \defgroup   Port Port Module
* \brief      Module for interfacing with the hardware dependent parts.
* \ingroup    Library
* \details
* The library dependents on some microcontroller specifics. For example when a time
* reference is needed or for accessing communication peripherals. This port module
* offers an interface through which these hardware dependent parts can be accessed.
****************************************************************************************/
#ifndef PORT_H
#define PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Total number of bytes in a XCP data packet. It should be at least equal or
 *         larger than that configured on the microcontroller, which runs the OpenBLT
 *         bootloader. In any case, it cannot be larger than 255.
 */
#define PORT_XCP_PACKET_SIZE_MAX   (255U)


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief XCP packet type. */
typedef struct
{
  uint8_t data[PORT_XCP_PACKET_SIZE_MAX];        /**< Packet data.                     */
  uint8_t len;                                   /**< Packet length.                   */
} tPortXcpPacket;

/** \brief Port interface. */
typedef struct
{
  /** \brief Obtains the current system time in milliseconds. */
  uint32_t (* SystemGetTime) (void);

  /** \brief Transmits an XCP packet using the transport layer implemented by the port.
   *         The transmission itself can be blocking. The function should return TBX_OK
   *         if the packet could be transmitted, TBX_ERROR otherwise.
   */
  uint8_t  (* XcpTransmitPacket) (tPortXcpPacket const * txPacket);

  /** \brief Attempts to receive an XCP packet using the transport layer implemented by
   *         the port. The reception should be non-blocking. The function should return
   *         TBX_TRUE if a packet was received, TBX_FALSE otherwise. A newly received
   *         packet should be stored in the rxPacket parameter.
   */
  uint8_t  (* XcpReceivePacket) (tPortXcpPacket * rxPacket);

  /** \brief Calculates the key to unlock the programming resource, based on the given
   *         seed. This function should return TBX_OK if the key could be calculated,
   *         TBX_ERROR otherwise.
   */
  uint8_t  (* XcpComputeKeyFromSeed) (uint8_t seedLen, uint8_t const * seedPtr,
                                      uint8_t * keyLenPtr, uint8_t * keyPtr);
} tPort;


/****************************************************************************************
* Function prototypes
****************************************************************************************/
void          PortInit(tPort const * port);
void          PortTerminate(void);
tPort const * PortGet(void);


#ifdef __cplusplus
}
#endif

#endif /* PORT_H */
/********************************* end of port.h ***************************************/

 
 
