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

typedef enum MoreMemoryState
{
    MORE_MEMORY_STATE_NONE,
    MORE_MEMORY_STATE_MMAPED,
    MORE_MEMORY_STATE_FAILED
} mmemState;

typedef struct MoreMemoryData
{
    mmemState state;
    void* ptr;
    ulong memSize;
    ulong usedSize;
} mmemData;

/* maps a memory space which has MINIMUM_MMAP_SIZE size*/
mmemData mmem_createMemorySpace();

/* extends a memory space to new minimum size */
mmemState mmem_extend(mmemData *mmdata, ulong added_used_size);

typedef enum UnCreateMemoryState
{
    UNCREATE_MEMORY_STATE_FAILED,
    UNCREATE_MEMORY_STATE_NONE,
    UNCREATE_MEMORY_STATE_UNMAPPED
} unmemState;

/* destroys mapped memory*/
unmemState mmem_uncreate(mmemData* mmdata);

#define extendMemorySpace mmem_extend
#define createMemorySpace mmem_createMemorySpace
#define uncreateMemoryState mmem_uncreate

#define MINIMUM_MMAP_SIZE 0x1000 /* 4kb of memory */
#define MORE_MEMORY_MAP_SIZE_STEP 0x1000
#endif