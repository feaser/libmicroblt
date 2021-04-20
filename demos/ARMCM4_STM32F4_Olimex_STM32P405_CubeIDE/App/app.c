/************************************************************************************//**
* \file         app.c
* \brief        Application source file.
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
#include <ff.h>                                     /* FatFS                           */
#include <microtbx.h>                               /* MicroTBX                        */
#include <FreeRTOS.h>                               /* FreeRTOS                        */
#include <task.h>                                   /* FreeRTOS tasks                  */
#include <microblt.h>                               /* LibMicroBLT                     */
#include "app.h"                                    /* application header              */
#include "stm32f4xx_hal.h"                          /* HAL drivers                     */


/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Priority of the application task. */
#define APP_TASK_PRIO        (( UBaseType_t ) 8U)

/** \brief Priority of the LED blink task. */
#define APP_LED_BLINK_PRIO   (( UBaseType_t ) 6U)


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void AppTask(void * pvParameters);
static void AppLedBlinkTask(void * pvParameters);
static void AppAssertionHandler(const char * const file, uint32_t line);


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Handle of the application task. */
static TaskHandle_t appTaskHandle = NULL;

/** \brief Handle of the LED blink task. */
static TaskHandle_t appLedBlinkTaskHandle = NULL;


/************************************************************************************//**
** \brief     Initializes the application. Should be called once during software program
**            initialization.
**
****************************************************************************************/
void AppInit(void)
{
  /* Register the application specific assertion handler. */
  TbxAssertSetHandler(AppAssertionHandler);

  /* Create the application task. */
  xTaskCreate(AppTask,
              "AppTask",
              configMINIMAL_STACK_SIZE + 128,
              NULL,
              APP_TASK_PRIO,
              &appTaskHandle);
  /* Create the LED blink task. */
  xTaskCreate(AppLedBlinkTask,
              "AppLedBlinkTask",
              configMINIMAL_STACK_SIZE,
              NULL,
              APP_LED_BLINK_PRIO,
              &appLedBlinkTaskHandle);
  /* Start the RTOS scheduler. */
  vTaskStartScheduler();
} /*** end of AppInit ***/


/************************************************************************************//**
** \brief     Task function of the application.
** \param     pvParameters Pointer to optional task parameters
**
****************************************************************************************/
static void AppTask(void * pvParameters)
{
  /* File system object. This is the work area for the logical drive. */
  FATFS fileSystem = { 0 };

  TBX_UNUSED_ARG(pvParameters);

  /* Mount the file system, using logical disk 0 */
  f_mount(&fileSystem, "0:", 0);

  /* Enter infinite task loop. */
  for (;;)
  {
    vTaskDelay(10);
  }

  /* Unregister work area prior to discarding it, if the code were to ever get here. */
  f_mount(NULL, "0:", 0);
} /*** end of AppTask ***/


/************************************************************************************//**
** \brief     LED blink task function.
** \param     pvParameters Pointer to optional task parameters
**
****************************************************************************************/
static void AppLedBlinkTask(void * pvParameters)
{
  const TickType_t ledToggleTicks = 500U / portTICK_PERIOD_MS;

  TBX_UNUSED_ARG(pvParameters);

  /* Enter infinite task loop. */
  for (;;)
  {
    /* Toggle the LED at a fixed interval. */
    vTaskDelay(ledToggleTicks);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_12);
  }
} /*** end of AppLedBlinkTask ***/


/************************************************************************************//**
** \brief     Triggers the run-time assertion. The default implementation is to enter an
**            infinite loop, which halts the program and can be used for debugging
**            purposes. Inspecting the values of the file and line parameters gives a
**            clear indication where the run-time assertion occurred. Note that an
**            alternative application specific assertion handler can be configured with
**            function TbxAssertSetHandler().
** \param     file The filename of the source file where the assertion occurred in.
** \param     line The line number inside the file where the assertion occurred.
**
****************************************************************************************/
static void AppAssertionHandler(const char * const file, uint32_t line)
{
  TBX_UNUSED_ARG(file);
  TBX_UNUSED_ARG(line);

  /* Disable interrupts to prevent task switching. */
  taskDISABLE_INTERRUPTS();

  /* Hang the program by entering an infinite loop. The values for file and line can
   * then be inspected with the debugger to locate the source of the run-time assertion.
   */
  for (;;)
  {
    ;
  }
} /*** end of AppAssertionHandler ***/


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


/*********************************** end of app.c **************************************/
