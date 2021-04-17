/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*------------------------------------------------------------------------*/


#include "ff.h"
#include "microtbx.h"


#if _FS_REENTRANT
/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object, such as semaphore and mutex. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
	int ret;

	*sobj = xSemaphoreCreateMutex();	/* FreeRTOS */
	ret = (int)(*sobj != NULL);

	return ret;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to any error */
	_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	int ret;

  vSemaphoreDelete(sobj);		/* FreeRTOS */
	ret = 1;

	return ret;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	_SYNC_t sobj	/* Sync object to wait */
)
{
	int ret;

	ret = (int)(xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE);	/* FreeRTOS */

	return ret;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t sobj	/* Sync object to be signaled */
)
{
	xSemaphoreGive(sobj);	/* FreeRTOS */
}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT msize		/* Number of bytes to allocate */
)
{
  void * result;

  /* Attempt to allocate a block from the best fitting memory pool. */
  result = TbxMemPoolAllocate(msize);
  /* Was the allocation not successful? */
  if (result == NULL)
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
    (void)TbxMemPoolCreate(1U, msize);

    /* Assuming sufficient heap was available, the memory pool was extended.
     * Attempt to allocate the block again. Note that if it fails, the result
     * is already NULL to flag this error.
     */
    result = TbxMemPoolAllocate(msize);

    /* Assert the allocation result for debugging purposes. */
    TBX_ASSERT(result != NULL);
  }

  /* Give the result back to the caller. */
  return result;
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free */
)
{
  /* Give the block back to the memory pool. */
  TbxMemPoolRelease(mblock);
}

#endif
