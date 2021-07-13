/************************************************************************************//**
* \file         button.c
* \brief        Push button driver source file.
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
#include "button.h"                         /* Push button driver                      */
#include "stm32f4xx_hal.h"                  /* HAL drivers                             */


/************************************************************************************//**
** \brief     Initializes the Push button driver.
**
****************************************************************************************/
void ButtonInit(void)
{
  /* Note that the GPIO pin initialization for the push button is already handled by
   * MX_GPIO_Init().
   */
} /*** end of ButtonInit ***/


/************************************************************************************//**
** \brief     Obtain the current state of the push button.
** \return    BUTTON_STATE_PRESSED if the push button is pressed, BUTTON_STATE_RELEASED
**            otherwise.
**
****************************************************************************************/
tButtonState ButtonGetState(void)
{
  tButtonState result = BUTTON_STATE_RELEASED;

  /* When the button is pressed, the GPIO pin wil be logic high on the board. */
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
  {
    /* Button is currently pressed, so update the result accordingly. */
    result = BUTTON_STATE_PRESSED;
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of ButtonGetState ***/


/*********************************** end of button.c ***********************************/
