/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

/*
    moremem.h : this header is about getting more memory for the process

    mapped data is always aligned with a step of 0x1000

    So size of each mmaped data is always n * 0x1000 (with n an entire positive number)
*/


#ifndef __MOREMEM_H__
#define __MOREMEM_H__

#include "memory.h"
#include "sys/mman.h"
#include <stdio.h>

#define MMEM_DATA_SIZE(mmdata) (((unsigned long)(mmdata).end) - ((unsigned long)(mmdata).start))

typedef enum MoreMemoryState
{
    MORE_MEMORY_STATE_NONE,
    MORE_MEMORY_STATE_EXTENDED,
    MORE_MEMORY_STATE_CREATED,
    MORE_MEMORY_STATE_FAILED
} mmemState;

typedef struct MoreMemoryData
{
    mmemState state;
    void* start;
    void* end;
} mmemData;

/* maps a memory space which has MINIMUM_MMAP_SIZE size*/
mmemData mmem_createMemorySpace();

mmemState mmem_destroyPreviousMap(mmemData* mmdata);

/* extends a memory space to new minimum size */
mmemState mmem_extend(mmemData *mmdata, size_t added_used_size);

typedef enum UnCreateMemoryState
{
    UNCREATE_MEMORY_STATE_FAILED,
    UNCREATE_MEMORY_STATE_NONE,
    UNCREATE_MEMORY_STATE_UNCREATED
} unmemState;

/* destroys mapped memory*/
unmemState mmem_uncreate(mmemData* mmdata);

#define extendMemorySpace mmem_extend
#define createMemorySpace mmem_createMemorySpace
#define uncreateMemoryState mmem_uncreate
#define destroyPreviousMemorySpace mmem_destroyPreviousMap

#endif