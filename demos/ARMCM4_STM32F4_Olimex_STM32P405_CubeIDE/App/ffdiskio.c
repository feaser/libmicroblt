/*------------------------------------------------------------------------/
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/


/* Details on creating a FatFS port can be found on the following website:
 *   - http://elm-chan.org/fsw/ff/doc/appnote.html#port
 */
#include "diskio.h"


static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
BYTE DiskInitialized = 0;



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive number (0) */
)
{
  DSTATUS stat = RES_OK;

  if (DiskInitialized == 0)
  {
    DiskInitialized = 1;
    /* TODO ##Port Initialize the drive. */
    stat = STA_NOINIT;
  }
  return stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv  /* Physical drive nmuber (0) */
)
{
  DSTATUS stat;

  /* TODO ##Port Obtain disk status. */
  stat = STA_NOINIT;
  return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,     /* Physical drive nmuber to identify the drive */
	BYTE *buff,    /* Data buffer to store read data */
	DWORD sector,  /* Sector address in LBA */
	UINT count     /* Number of sectors to read */
)
{
  DRESULT res;

  /* TODO ##Port Read sector(s) from the disk. */
  res = RES_ERROR;
  return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,         /* Physical drive nmuber to identify the drive */
	const BYTE *buff,  /* Data to be written */
	DWORD sector,      /* Sector address in LBA */
	UINT count         /* Number of sectors to write */
)
{
  DRESULT res;

  /* TODO ##Port Write sector(s) to the disk. */
  res = RES_ERROR;
  return res;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,    /* Physical drive nmuber (0) */
	BYTE cmd,     /* Control code */
	void *buff    /* Buffer to send/receive data block */
)
{
  DRESULT res;

  /* TODO ##Port Perform requested disk I/O control operation. */
  res = RES_ERROR;
  return res;
}



