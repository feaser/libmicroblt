/************************************************************************************//**
* \file         FreeRTOSHeap.c
* \brief        Heap management source file for FreeRTOS based on MicroTBX.
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

/*
 * An implementation of pvPortMalloc() and vPortFree() based on the memory pools module
 * of MicroTBX. Note that this implementation allows allocated memory to be freed again.
 *
 * See heap_1.c, heap_2.c, heap_3.c and heap_4.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

#include "microtbx.h"

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
  #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xWantedSize )
{
  void * pvReturn;

  vTaskSuspendAll();
  {
    /* Attempt to allocate a block from the best fitting memory pool. */
    pvReturn = TbxMemPoolAllocate(xWantedSize);
    /* Was the allocation not successful? */
    if (pvReturn == NULL)
    {
      /* The allocation failed. This can have two reasons:
       *   1. A memory pool for the requested size hasn't yet been created.
       *   2. The memory pool for the requested size has no more free blocks.
       * Both situations can be solved by calling TbxMemPoolCreate(), as this
       * function automatically extends a memory pool, if it was created before.
       * Note that ther is not need to check the return value, because we will
       * attempts to allocate again right afterwards. We can catch the error
       * there in case the allocation fails.
       */
      (void)TbxMemPoolCreate(1U, xWantedSize);

      /* Assuming sufficient heap was available, the memory pool was extended.
       * Attempt to allocate the block again.
       */
      pvReturn = TbxMemPoolAllocate(xWantedSize);
    }
    traceMALLOC( pvReturn, xWantedSize );
  }
  ( void ) xTaskResumeAll();

  #if( configUSE_MALLOC_FAILED_HOOK == 1 )
  {
    if( pvReturn == NULL )
    {
      extern void vApplicationMallocFailedHook( void );
      vApplicationMallocFailedHook();
    }
  }
  #endif

  return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
  /* Give the block back to the memory pool. */
  TbxMemPoolRelease(pv);
}


/*********************************** end of FreeRTOSHeap.c *****************************/
