/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "moremem.h"

#define CEIL_POS_FLT(x) ((ulong)x + 1) /* ceil positive float */

mmemData mmem_createMemorySpace()
{
    mmemData mmdata;
    
    /* maps the space */
    mmdata.ptr = mmap(NULL, MINIMUM_MMAP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    
    mmdata.usedSize = 0;
    mmdata.memSize = MINIMUM_MMAP_SIZE;

    if(mmdata.ptr == MAP_FAILED)
    {
        mmdata.state = MORE_MEMORY_STATE_FAILED;
        
    }
    else
    {
        mmdata.state = MORE_MEMORY_STATE_MMAPED;
    }
    return mmdata;   
}

mmemState mmem_remap(mmemData* mmdata, ulong newsize)
{

    void* memBefore;
	void* tempMap;
    void* memNow;

    /* saving memory address before */
	memBefore = mmdata->ptr;

    /* create a temporary buffer  */
    tempMap = mmap(NULL, mmdata->memSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if(tempMap == MAP_FAILED)
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    /* copying the whole memory to this temporary buffer */
    memcopy(tempMap, mmdata->ptr, mmdata->memSize);

    /* unmap / delete the previous mem data */
    if(munmap(mmdata->ptr, mmdata->memSize) != 0)
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    /* allocate new mem */
    memNow = mmap(NULL, newsize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    
    if(memNow == MAP_FAILED)
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    mmdata->ptr = memNow;

    /* copying the saved mem to new mem */
    memcopy(mmdata->ptr, tempMap, mmdata->memSize);

    /* unmap / delete saved mem */
    if(munmap(tempMap, mmdata->memSize) != 0)
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    mmdata->memSize = newsize;

    return MORE_MEMORY_STATE_MMAPED;
}

mmemState mmem_extend(mmemData* mmdata, ulong added_used_size)
{
    ulong newUsedSize;
    ulong size;
    ulong times;

    newUsedSize = mmdata->usedSize + added_used_size;

    if(mmdata->state == MORE_MEMORY_STATE_MMAPED)
    {
        mmdata->usedSize = newUsedSize;
        /* if it doesn't require more memory */
        if(newUsedSize < mmdata->memSize)
        {
            return MORE_MEMORY_STATE_NONE;
        }

        times = CEIL_POS_FLT((float)added_used_size / (float)MORE_MEMORY_MAP_SIZE_STEP);

        /* we need new space */
        return mmem_remap(mmdata, MORE_MEMORY_MAP_SIZE_STEP * times);
        
    }
    else
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    
}

unmemState mmem_uncreate(mmemData *mmdata)
{
    /* if data is mapped*/
    if(mmdata->state == MORE_MEMORY_STATE_MMAPED)
    {
        if(munmap(mmdata->ptr, mmdata->memSize) != 0)
        {
            /* could not unmap data */
            return UNCREATE_MEMORY_STATE_FAILED;
        }
        
        /* data was unmapped */
        mmdata->state = MORE_MEMORY_STATE_NONE;
        return UNCREATE_MEMORY_STATE_UNMAPPED;
    }
    else
    {
        /* perfrom no operation and return */
        return UNCREATE_MEMORY_STATE_NONE;
    }
    
}