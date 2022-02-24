/************************************************************************************//**
* \file         led.c
* \brief        LED driver source file.
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
#include <microtbx.h>                       /* MicroTBX                                */
#include "led.h"                            /* LED driver                              */
#include "stm32f4xx_hal.h"                  /* HAL drivers                             */


/****************************************************************************************
* Local data declarations
****************************************************************************************/
static tLedState ledState = LED_STATE_OFF;


/************************************************************************************//**
** \brief     Initializes the LED driver.
**
****************************************************************************************/
void LedInit(void)
{
  /* Note that the GPIO pin initialization for the LED is already handled by
   * MX_GPIO_Init(). Just make sure the LED is turned off here.
   */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
  ledState = LED_STATE_OFF;
} /*** end of LedInit ***/


/************************************************************************************//**
** \brief     Change the state of the LED.
** \param     state New state for the LED. It can be LED_STATE_OFF or LED_STATE_ON.
**
****************************************************************************************/
void LedSetState(tLedState state)
{
  /* Only change the state if necessary. */
  if (state != ledState)
  {
    TbxCriticalSectionEnter();
    /* Turn off? */
    if (state == LED_STATE_OFF)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
    }
    /* Turn on. */
    else
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
    }
    /* Store the new state. */
    ledState = state;
    TbxCriticalSectionExit();
  }
} /*** end of LedSetState ***/


/************************************************************************************//**
** \brief     Obtain the current state of the LED.
** \return    LED_STATE_ON if the LED is turned on, LED_STATE_OFF otherwise.
**
****************************************************************************************/
tLedState LedGetState(void)
{
  return ledState;
} /*** end of LedGetState ***/


/************************************************************************************//**
** \brief     Toggle the current state of the LED.
**
****************************************************************************************/
void LedToggleState(void)
{
  tLedState newState;

  /* Determine the toggled state. */
  newState = (LedGetState() == LED_STATE_OFF) ? LED_STATE_ON : LED_STATE_OFF;
  /* Set the LED to the toggled state. */
  LedSetState(newState);
} /*** end of LedToggleState ***/


/*********************************** end of led.c **************************************/
