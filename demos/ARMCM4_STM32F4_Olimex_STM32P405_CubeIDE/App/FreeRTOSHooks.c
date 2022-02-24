/************************************************************************************//**
* \file         FreeRTOSHooks.c
* \brief        Source file that implements the FreeRTOS hook functions.
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
#include <FreeRTOS.h>                       /* FreeRTOS                                */
#include <task.h>                           /* FreeRTOS tasks                          */


/************************************************************************************//**
** \brief     FreeRTOS hook function that gets called when memory allocation failed.
**
****************************************************************************************/
void vApplicationMallocFailedHook(void)
{
  /* Trigger an assertion for debugging purposes. */
  TBX_ASSERT(TBX_ERROR);
} /*** end of vApplicationMallocFailedHook ***/


/************************************************************************************//**
** \brief     FreeRTOS hook function that gets called when a stack overflow was detected.
** \param     xTask Handle of the task that has a stack overflow.
** \param     pcTaskName Name of the task that has a stack overflow.
**
****************************************************************************************/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
  TBX_UNUSED_ARG(xTask);
  TBX_UNUSED_ARG(pcTaskName);

  /* Trigger an assertion for debugging purposes. */
  TBX_ASSERT(TBX_ERROR);
} /*** end of vApplicationStackOverflowHook ***/


/*********************************** end of FreeRTOSHooks.c ****************************/
