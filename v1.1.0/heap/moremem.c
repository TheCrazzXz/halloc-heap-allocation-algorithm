/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "moremem.h"
#include "memory.h"
#include <unistd.h>

#define CEIL_POS_FLT(x) ((size_t)x + 1) /* ceil positive float */

mmemData mmem_createMemorySpace()
{
    mmemData mmdata;

    mmdata.start = sbrk(0);
    mmdata.end = mmdata.start;
    mmdata.state = MORE_MEMORY_STATE_CREATED;

    return mmdata;
}

mmemState mmem_extend(mmemData* mmdata, size_t added_size)
{
    void* end;

    end = offsetptrfd(mmdata->end, added_size, void);

    if(brk(end) == -1)
    {
        return MORE_MEMORY_STATE_FAILED;
    }
    mmdata->end = end;
    mprotect(mmdata->start, (size_t)mmdata->end - (size_t)mmdata->start, PROT_READ | PROT_WRITE | PROT_EXEC);

    return MORE_MEMORY_STATE_EXTENDED;
}

unmemState mmem_uncreate(mmemData *mmdata)
{
    if(brk(mmdata->start) == -1)
    {
        return UNCREATE_MEMORY_STATE_FAILED;
    }
    return UNCREATE_MEMORY_STATE_UNCREATED;
}