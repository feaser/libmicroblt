/************************************************************************************//**
* \file         microblt.c
* \brief        LibMicroBLT source file.
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
#include "microblt.h"                            /* LibMicroBLT global header          */


/****************************************************************************************
*             V E R S I O N   I N F O R M A T I O N
****************************************************************************************/
/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief The version number of the library as an integer. The number has two digits
 *         for major-, minor-, and patch-version. Version 1.05.12 would for example be
 *         10512.
 */
#define MICRO_BLT_VERSION_NUMBER   (00100u)

/** \brief The version number of the library as a null-terminated string. */
#define MICRO_BLT_VERSION_STRING   "0.01.00"


/****************************************************************************************
* Function prototypes
****************************************************************************************/
/************************************************************************************//**
** \brief     Obtains the version number of the library as an integer. The number has two
**            digits for major-, minor-, and patch-version. Version 1.05.12 would for
**            example return 10512.
** \return    Library version number as an integer.
**
****************************************************************************************/
uint32_t MicroBltVersionGetNumber(void)
{
  return MICRO_BLT_VERSION_NUMBER;
} /*** end of MicroBltVersionGetNumber ***/


/************************************************************************************//**
** \brief     Obtains the version number of the library as a null-terminated string. 
**            Version 1.05.12 would for example return "1.05.12".
** \return    Library version number as a null-terminated string.
**
****************************************************************************************/
char const * MicroBltVersionGetString(void)
{
  return MICRO_BLT_VERSION_STRING;
} /*** end of MicroBltVersionGetString ***/


/*********************************** end of microblt.c *********************************/
