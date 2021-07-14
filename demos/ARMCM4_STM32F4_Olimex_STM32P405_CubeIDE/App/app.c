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
#include <microtbx.h>                       /* MicroTBX                                */
#include <ff.h>                             /* FatFS                                   */
#include <FreeRTOS.h>                       /* FreeRTOS                                */
#include <task.h>                           /* FreeRTOS tasks                          */
#include <event_groups.h>                   /* FreeRTOS event groups                   */
#include <microblt.h>                       /* LibMicroBLT                             */
#include "app.h"                            /* Application header                      */
#include "led.h"                            /* LED driver                              */
#include "button.h"                         /* Push button driver                      */
#include "can.h"                            /* CAN driver                              */


/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Priority of the application task. */
#define APP_TASK_PRIO                  ((UBaseType_t) 8U)

/** \brief Priority of the LED blink task. */
#define APP_LED_BLINK_TASK_PRIO        ((UBaseType_t) 6U)

/** \brief Priority of the push button scan task. */
#define APP_BUTTON_SCAN_TASK_PRIO      ((UBaseType_t) 6U)

/** \brief Event flag bit to request the default LED blink rate. */
#define APP_EVENT_LED_NORMAL_BLINKING  ((uint8_t)0x01U)

/** \brief Event flag bit to request a faster LED blink rate. */
#define APP_EVENT_LED_FAST_BLINKING    ((uint8_t)0x02U)

/** \brief Event flag bit for the push button pressed event. */
#define APP_EVENT_BUTTON_PRESSED       ((uint8_t)0x04U)


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void AppTask(void * pvParameters);
static void AppLedBlinkTask(void * pvParameters);
static void AppButtonScanTask(void * pvParameters);
static void AppCanMessageReceived(tCanMsg const * msg);
static void AppAssertionHandler(const char * const file, uint32_t line);


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Handle of the application task. */
static TaskHandle_t appTaskHandle = NULL;

/** \brief Handle of the LED blink task. */
static TaskHandle_t appLedBlinkTaskHandle = NULL;

/** \brief Handle of the push button scan task. */
static TaskHandle_t appButtonScanTaskHandle = NULL;

/** \brief Handle of the application event group. */
static EventGroupHandle_t appEvents;


/************************************************************************************//**
** \brief     Initializes the application. Should be called once during software program
**            initialization.
**
****************************************************************************************/
void AppInit(void)
{
  /* Register the application specific assertion handler. */
  TbxAssertSetHandler(AppAssertionHandler);
  /* Initialize the LED driver. */
  LedInit();
  /* Initialize the push button driver. */
  ButtonInit();
  /* Initialize the CAN driver. */
  CanInit(CAN_BAUDRATE_500K, AppCanMessageReceived);
  /* Create the application events group. */
  appEvents = xEventGroupCreate();
  /* Create the application task. */
  xTaskCreate(AppTask,
              "AppTask",
              configMINIMAL_STACK_SIZE + 256,
              NULL,
              APP_TASK_PRIO,
              &appTaskHandle);
  /* Create the LED blink task. */
  xTaskCreate(AppLedBlinkTask,
              "AppLedBlinkTask",
              configMINIMAL_STACK_SIZE,
              NULL,
              APP_LED_BLINK_TASK_PRIO,
              &appLedBlinkTaskHandle);
  /* Create the push button scan task. */
  xTaskCreate(AppButtonScanTask,
              "AppButtonScanTask",
              configMINIMAL_STACK_SIZE,
              NULL,
              APP_BUTTON_SCAN_TASK_PRIO,
              &appButtonScanTaskHandle);
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
  FATFS   fileSystem = { 0 };
  uint8_t updateResult = TBX_OK;

  TBX_UNUSED_ARG(pvParameters);

  /* Mount the file system, using logical disk 0 */
  f_mount(&fileSystem, "0:", 0);
  /* Initialize the firmware module for reading S-record firmware files. */
  BltFirmwareInit(BLT_FIRMWARE_READER_SRECORD);

  /* Enter infinite task loop. */
  for (;;)
  {
    /* Wait indefinitely for the push button to be pressed, which this application uses
     * as a trigger to start the firmware update.
     */
    (void)xEventGroupWaitBits(appEvents, APP_EVENT_BUTTON_PRESSED, pdFALSE, pdTRUE,
                              portMAX_DELAY);

    /* Trigger event to request a faster LED link rate to indicate that a firmware
     * update is in progress.
     */
    (void)xEventGroupSetBits(appEvents, APP_EVENT_LED_FAST_BLINKING);

    /* Start the firmware update by opening the firmware file. */
    updateResult = BltFirmwareFileOpen("demoprog.srec");
    /* Only continue with the firmware update if the firmware file could be opened. */
    if (updateResult == TBX_OK)
    {
      /* TODO ##Vg Implement firmware update logic here. Perhaps implement the entire
       * firmware update part in a seperate function. It is then easier to reuse
       * in another application.
       */
      vTaskDelay(1000);
      /* Make sure to close the firmware file again. */
      BltFirmwareFileClose();
    }

    /* Trigger event to request the default LED link rate to indicate that the firmware
     * update is no longer active.
     */
    (void)xEventGroupSetBits(appEvents, APP_EVENT_LED_NORMAL_BLINKING);

    /* Clear the push button pressed event, now that the firmwrae update completed. */
    (void)xEventGroupClearBits(appEvents, APP_EVENT_BUTTON_PRESSED);
  }

  /* Terminate the firmware module, if the code were to ever get here. */
  BltFirmwareTerminate();
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
  const TickType_t ledNormalToggleTicks = 500U / portTICK_PERIOD_MS;
  const TickType_t ledFastToggleTicks   = 100U / portTICK_PERIOD_MS;
  TickType_t       ledToggleTicks = ledNormalToggleTicks;
  EventBits_t      eventBits;

  TBX_UNUSED_ARG(pvParameters);

  /* Enter infinite task loop. */
  for (;;)
  {
    /* Obtain the current state of the event bits. */
    eventBits = xEventGroupGetBits(appEvents);
    /* Check if a different blink rate for the LED is requested. */
    if ((eventBits & APP_EVENT_LED_NORMAL_BLINKING) != 0)
    {
      /* Update the LED blink rate. */
      ledToggleTicks = ledNormalToggleTicks;
      /* Clear the event bits after processing the event. */
      (void)xEventGroupClearBits(appEvents, APP_EVENT_LED_NORMAL_BLINKING);
    }
    else if ((eventBits & APP_EVENT_LED_FAST_BLINKING) != 0)
    {
      /* Update the LED blink rate. */
      ledToggleTicks = ledFastToggleTicks;
      /* Clear the event bits after processing the event. */
      (void)xEventGroupClearBits(appEvents, APP_EVENT_LED_FAST_BLINKING);
    }
    /* Toggle the LED at the currently requested interval. */
    vTaskDelay(ledToggleTicks);
    LedToggleState();
  }
} /*** end of AppLedBlinkTask ***/


/************************************************************************************//**
** \brief     LED blink task function.
** \param     pvParameters Pointer to optional task parameters
**
****************************************************************************************/
static void AppButtonScanTask(void * pvParameters)
{
  const TickType_t scanIntervalTicks = 5U / portTICK_PERIOD_MS;
  tButtonState     lastButtonState = BUTTON_STATE_RELEASED;
  tButtonState     currentButtonState;
  const TickType_t debounceTicks = 50U / portTICK_PERIOD_MS;
  uint8_t          debounceCount = 0U;
  uint8_t          debouncing = TBX_FALSE;

  TBX_UNUSED_ARG(pvParameters);

  /* Enter infinite task loop. */
  for (;;)
  {
    /* Are we debouncing the push button's pressed event? */
    if (debouncing == TBX_TRUE)
    {
      /* Did the button go back to the released state? */
      if (ButtonGetState() == BUTTON_STATE_RELEASED)
      {
        /* Button is still bouncing so go back to detecting the initial button pressed
         * event.
         */
        debouncing = TBX_FALSE;
      }
      /* Button is still in the pressed state. */
      else
      {
        /* Decrement the debounce counter. */
        if (debounceCount > 0U)
        {
          debounceCount--;
          /* Are we done debouncing? */
          if (debounceCount == 0U)
          {
            /* The button pressed event is now stable. */
            debouncing = TBX_FALSE;
            /* Set the push button pressed event flag. */
            (void)xEventGroupSetBits(appEvents, APP_EVENT_BUTTON_PRESSED);
          }
        }
      }
    }
    /* Not debouncing so see if the initial button pressed event occurred. */
    else
    {
      /* Read the current state of the push button. */
      currentButtonState = ButtonGetState();
      /* Did the state change to being pressed? */
      if ( (currentButtonState != lastButtonState) &&
           (currentButtonState == BUTTON_STATE_PRESSED) )
      {
        /* Initialize the debounce counter and enable debouncing */
        debounceCount = debounceTicks / scanIntervalTicks;
        debouncing = TBX_TRUE;
      }
      /* Store the button state, as it is needed for change detection. */
      lastButtonState = currentButtonState;
    }

    /* Scan the state of the push button at a fixed interval. */
    vTaskDelay(scanIntervalTicks);
  }
} /*** end of AppButtonScanTask ***/


/************************************************************************************//**
** \brief     Callback function that gets called each time a new CAN message was
**            received.
** \param     msg Pointer to the newly received CAN message.
**
****************************************************************************************/
static void AppCanMessageReceived(tCanMsg const * msg)
{
  /* Verify parameter. */
  TBX_ASSERT(msg != NULL);

  /* Only continue with valid parameter. */
  if (msg != NULL)
  {
    /* TODO ##Vg Implement AppCanMessageReceived(). */
  }
} /*** end of AppCanMessageReceived ***/


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


/*********************************** end of app.c **************************************/
