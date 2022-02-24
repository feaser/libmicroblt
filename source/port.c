/************************************************************************************//**
* \file         port.c
* \brief        Port module source file.
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

/****************************************************************************************
* Include files
****************************************************************************************/
#include <microtbx.h>                       /* MicroTBX toolbox                        */
#include "port.h"                           /* Port module                             */


/****************************************************************************************
* Plausibility check
****************************************************************************************/
#if (PORT_XCP_PACKET_SIZE_MAX > 255U)
#error "PORT_XCP_PACKET_SIZE_MAX must be <= 255"
#endif


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Holds the port that is linked. */
static tPort portInterface;


/************************************************************************************//**
** \brief     Initializes the port module by linking the port specified by the parameter.
** \param     port The port to link.
**
****************************************************************************************/
void PortInit(tPort const * port)
{
  /* Verify parameter. */
  TBX_ASSERT(port != NULL);

  /* Only continue with valid parameter. */
  if (port != NULL)
  {
    /* Shallow copy to link the port module. */
    portInterface = *port;
  }
} /*** end of PortInit ***/


/************************************************************************************//**
** \brief     Terminates the port module.
**
****************************************************************************************/
void PortTerminate(void)
{
  /* Nothing to do currently, so just leave it empty for now. */
} /*** end of PortTerminate ***/


/************************************************************************************//**
** \brief     Obtains a pointer to the linked port module. This function should always
**            be used when hardware specifics need to be accessed in the library.
** \return    The pointer to the currently linked port module, e.g.
**            PortGet()->SystemGetTime();
**
****************************************************************************************/
tPort const * PortGet(void)
{
  tPort const * result = &portInterface;

  /* Give the result back to the caller. */
  return result;
} /*** end of PortGet ***/


/*********************************** end of port.c *************************************/
