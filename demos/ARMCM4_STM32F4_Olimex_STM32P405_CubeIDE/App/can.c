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
* Type definitions
****************************************************************************************/
/** \brief Structure type for grouping CAN bus timing related information. */
typedef struct
{
  uint8_t tseg1;                                      /**< CAN time segment 1          */
  uint8_t tseg2;                                      /**< CAN time segment 2          */
} tCanBusTiming;


/****************************************************************************************
* Local constant declarations
****************************************************************************************/
/** \brief CAN bittiming table for dynamically calculating the bittiming settings.
 *  \details According to the CAN protocol 1 bit-time can be made up of between 8..25
 *           time quanta (TQ). The total TQ in a bit is SYNC + TSEG1 + TSEG2 with SYNC
 *           always being 1. The sample point is (SYNC + TSEG1) / (SYNC + TSEG1 + SEG2) *
 *           100%. This array contains possible and valid time quanta configurations with
 *           a sample point between 68..78%.
 */
static const tCanBusTiming canTiming[] =
{
  /*  TQ | TSEG1 | TSEG2 | SP  */
  /* ------------------------- */
  {  5, 2 },          /*   8 |   5   |   2   | 75% */
  {  6, 2 },          /*   9 |   6   |   2   | 78% */
  {  6, 3 },          /*  10 |   6   |   3   | 70% */
  {  7, 3 },          /*  11 |   7   |   3   | 73% */
  {  8, 3 },          /*  12 |   8   |   3   | 75% */
  {  9, 3 },          /*  13 |   9   |   3   | 77% */
  {  9, 4 },          /*  14 |   9   |   4   | 71% */
  { 10, 4 },          /*  15 |  10   |   4   | 73% */
  { 11, 4 },          /*  16 |  11   |   4   | 75% */
  { 12, 4 },          /*  17 |  12   |   4   | 76% */
  { 12, 5 },          /*  18 |  12   |   5   | 72% */
  { 13, 5 },          /*  19 |  13   |   5   | 74% */
  { 14, 5 },          /*  20 |  14   |   5   | 75% */
  { 15, 5 },          /*  21 |  15   |   5   | 76% */
  { 15, 6 },          /*  22 |  15   |   6   | 73% */
  { 16, 6 },          /*  23 |  16   |   6   | 74% */
  { 16, 7 },          /*  24 |  16   |   7   | 71% */
  { 16, 8 }           /*  25 |  16   |   8   | 68% */
};


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static uint32_t CanConvertBaudrate(tCanBaudrate baudrate);
static uint8_t  CanGetSpeedConfig(uint16_t baud, uint16_t * prescaler, uint8_t * tseg1,
                                  uint8_t * tseg2);


/************************************************************************************//**
** \brief     Initializes the CAN controller for the specified baudrate and sets the
**            callback function to call, each time a CAN message was received.
** \param     baudrate CAN communication speed.
** \param     callbackFnc Callback function pointer.
**
****************************************************************************************/
void CanInit(tCanBaudrate baudrate, tCanReceivedCallback callbackFcn)
{
  uint16_t prescaler = 0;
  uint8_t  tseg1 = 0;
  uint8_t  tseg2 = 0;
  uint32_t baudrateRaw;
  uint8_t  speedConfigFound;

  /* Verify parameter. */
  TBX_ASSERT(callbackFcn != NULL);

  /* Only continue with valid parameter. */
  if (callbackFcn != NULL)
  {
    /* Compute raw baudrate in bits/sec. */
    baudrateRaw = CanConvertBaudrate(baudrate);
    /* Obtain bittiming configuration information. */
    speedConfigFound = CanGetSpeedConfig(baudrateRaw/1000U, &prescaler, &tseg1, &tseg2);
    /* Verify that a valid bittiming configuration could be found. */
    TBX_ASSERT(speedConfigFound == TBX_OK);
    /* Only continue with a valid bittiming configuration. */
    if (speedConfigFound == TBX_OK)
    {
      /* TODO ##Vg Implement CanInit(), It should support 11-bit and 29-bit CAN identifier
       * reception. Note that CanConvertBaudrate() can be used to get the raw baudrate.
       */
    }
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


/************************************************************************************//**
** \brief     Search algorithm to match the desired baudrate to a possible bus
**            timing configuration.
** \param     baud The desired baudrate in kbps. Valid values are 10..1000.
** \param     prescaler Pointer to where the value for the prescaler will be stored.
** \param     tseg1 Pointer to where the value for TSEG2 will be stored.
** \param     tseg2 Pointer to where the value for TSEG2 will be stored.
** \return    TBX_OK if the CAN bustiming register values were found, TBX_ERROR
**            otherwise.
**
****************************************************************************************/
static uint8_t CanGetSpeedConfig(uint16_t baud, uint16_t * prescaler, uint8_t * tseg1,
                                 uint8_t * tseg2)
{
  uint8_t  result = TBX_ERROR;
  uint8_t  cnt;
  uint32_t canClockFreqkHz;

  /* Verify parameters. */
  TBX_ASSERT((baud >= 10U) && (baud <= 1000U) && (prescaler != NULL) &&
             (tseg1 != NULL) && (tseg2 != NULL));

  /* Only continue with valid parameters. */
  if ((baud >= 10U) && (baud <= 1000U) && (prescaler != NULL) &&
      (tseg1 != NULL) && (tseg2 != NULL))
  {
    /* Determine and store CAN peripheral clock speed in kHz. */
    canClockFreqkHz = HAL_RCC_GetPCLK1Freq() / 1000U;


    /* Loop through all possible time quanta configurations to find a match. */
    for (cnt = 0U; cnt < sizeof(canTiming)/sizeof(canTiming[0]); cnt++)
    {
      /* Would this time quanta configuration work? */
      if ((canClockFreqkHz % (baud*(canTiming[cnt].tseg1+canTiming[cnt].tseg2+1))) == 0U)
      {
        /* Compute the prescaler that goes with this TQ configuration. */
        *prescaler = canClockFreqkHz/(baud*(canTiming[cnt].tseg1+canTiming[cnt].tseg2+1U));

        /* Make sure the prescaler is valid. */
        if ((*prescaler > 0U) && (*prescaler <= 1024U))
        {
          /* Store the bustiming configuration. */
          *tseg1 = canTiming[cnt].tseg1;
          *tseg2 = canTiming[cnt].tseg2;
          /* Found a good bus timing configuration. Update the result and stop looing. */
          result = TBX_OK;
          break;
        }
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of CanGetSpeedConfig ***/


/*********************************** end of can.c **************************************/
