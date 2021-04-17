/************************************************************************************//**
* \file         filecopy.c
* \brief        FatFS file copy test source file.
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*      Copyright (c) by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
*   This software has been carefully tested, but is not guaranteed for any particular
* purpose. The author does not offer any warranties and does not guarantee the accuracy,
*   adequacy, or completeness of the software and is not responsible for any errors or
*              omissions or the results obtained from use of the software.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include <stddef.h>
#include "ff.h"


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief File system object. This is the work area for the logical drive. */
static FATFS   fs;

/** \brief File object for the input file. */
static FIL     fsrc;

/** \brief File object for the output file. */
static FIL     fdst;

/** \brief File copy buffer. */
static BYTE buffer[4096];   /* File copy buffer */


/************************************************************************************//**
** \brief     Function to copy a file for testing purposes of the SD-card access.
** \return    none
**
****************************************************************************************/
void TestCopyFile(void)
{
  FRESULT fr;
  UINT br, bw;         /* File read/write count */

  /* mount the file system, using logical disk 0 */
  fr = f_mount(&fs, "0:", 0);

  /* open source file */
  fr = f_open(&fsrc, "test.txt", FA_READ);
  if (fr) return;

  /* create destination file on the drive 0 */
  fr = f_open(&fdst, "copy.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (fr) return;

  /* copy source to destination */
  for (;;)
  {
    fr = f_read(&fsrc, buffer, sizeof buffer, &br);  /* read a chunk of source file */
    if (fr || br == 0) break;  /* error or eof */
    fr = f_write(&fdst, buffer, br, &bw);  /* write it to the destination file */
    if (fr || bw < br) break;  /* error or disk full */
  }

  /* close open files */
  f_close(&fsrc);
  f_close(&fdst);

  /* unregister work area prior to discard it */
  f_mount(NULL, "0:", 0);
} /*** end of TestCopyFile ***/


/*********************************** end of filecopy.c *********************************/
