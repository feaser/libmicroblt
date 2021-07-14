/************************************************************************************//**
* \file         can.c
* \brief        Controller area network driver source file.
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
#include <microtbx.h>                       /* MicroTBX                                */
#include "can.h"                            /* CAN driver                              */
#include "stm32f4xx_hal.h"                  /* HAL drivers                             */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static uint32_t CanConvertBaudrate(tCanBaudrate baudrate);


/************************************************************************************//**
** \brief     Initializes the CAN controller for the specified baudrate and sets the
**            callback function to call, each time a CAN message was received.
** \param     baudrate CAN communication speed.
** \param     callbackFnc Callback function pointer.
**
****************************************************************************************/
void CanInit(tCanBaudrate baudrate, tCanReceivedCallback callbackFcn)
{
  TBX_ASSERT(callbackFcn != NULL);

  /* Only continue with valid parameter. */
  if (callbackFcn != NULL)
  {
    /* TODO ##Vg Implement CanInit(), It should support 11-bit and 29-bit CAN identifier
     * reception. Note that CanConvertBaudrate() can be used to get the raw baudrate.
     */
  }
} /*** end of CanInit ***/


/************************************************************************************//**
** \brief     Terminated the CAN driver.
**
****************************************************************************************/
void CanTerminate(void)
{
  /* TODO ##Vg Implement CanTerminate(). */
} /*** end of CanTerminate ***/


/************************************************************************************//**
** \brief     Submits a CAN message for transmission.
** \param     msg Pointer to the CAN message to transmit.
** \return    TBX_OK if the message could be submitted for transmission, TBX_ERROR
**            otherwise.
**
****************************************************************************************/
uint8_t CanTransmit(tCanMsg const * msg)
{
  uint8_t result = TBX_ERROR;

  /* Verify parameter. */
  TBX_ASSERT(msg != NULL);

  /* Only continue with valid parameter. */
  if (msg != NULL)
  {
    /* TODO ##Vg Implement CanTransmit(). Maybe add a transmit FIFO using MicroTBX. */
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of CanTransmit ***/


/************************************************************************************//**
** \brief     Converts the baudrate enum value to a baudrate in bits per second.
** \param     baudrate CAN communication speed.
**
****************************************************************************************/
static uint32_t CanConvertBaudrate(tCanBaudrate baudrate)
{
  uint32_t result;

  /* Filter based on the specified baudrate enum value. */
  switch (baudrate)
  {
    case CAN_BAUDRATE_1M:
      result = 1000000U;
      break;
    case CAN_BAUDRATE_800K:
      result = 800000U;
      break;
    case CAN_BAUDRATE_500K:
    default:
      result = 500000U;
      break;
    case CAN_BAUDRATE_250K:
      result = 250000U;
      break;
    case CAN_BAUDRATE_125K:
      result = 125000U;
      break;
    case CAN_BAUDRATE_100K:
      result = 100000U;
      break;
    case CAN_BAUDRATE_50K:
      result = 50000U;
      break;
    case CAN_BAUDRATE_20K:
      result = 20000U;
      break;
    case CAN_BAUDRATE_10K:
      result = 10000U;
      break;
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of CanConvertBaudrate ***/


/*********************************** end of can.c **************************************/
