/************************************************************************************//**
* \file         tbx_conf.h
* \brief        Project specific MicroTBX configuration header file.
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
#ifndef TBX_CONF_H
#define TBX_CONF_H

/* Note that this is a project specific configuration file for MicroTBX. It is enabled
 * by setting the macro definition PROJ_TBX_CONF_H="MicroTBXConfig.h" in the compiler's
 * preprocessor settings. It overrides the default tbx_conf.h file.
 */

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************************
*   A S S E R T I O N S   M O D U L E   C O N F I G U R A T I O N
****************************************************************************************/
/** \brief Enable/disable run-time assertions. */
#define TBX_CONF_ASSERTIONS_ENABLE               (1U)


/****************************************************************************************
*   H E A P   M O D U L E   C O N F I G U R A T I O N
****************************************************************************************/
/** \brief Configure the size of the heap in bytes. */
#define TBX_CONF_HEAP_SIZE                       (16U * 1024U)


#ifdef __cplusplus
}
#endif

#endif /* TBX_CONF_H */
/*********************************** end of tbx_conf.h *********************************/
