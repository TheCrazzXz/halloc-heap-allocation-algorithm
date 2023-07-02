/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "memory.h"
#include <stdio.h> 
void memcopy(void* dst, void* src, ulong size)
{
	while(size--) /* loop until size reaches 0 */
	{
		*((char*)dst++) = *((char*)src++);
	}
}
void memfill(void* dst, char c, ulong size)
{
	char* mem = (char*)dst;

    while(size--) /* loop until size reaches 0 */
    {
        *mem++ = c;
    }
}

void print_buffer(char* buffer, ulong size)
{
	printf("\n<start %p (size=%lu)>\n", buffer, size);
	ulong i = 0;
	while(i < size)
	{
		printf("%x ", (unsigned char)buffer[i]);
		i++;
	}
	printf("\n<end %p>\n\n", buffer+size);
}

ulong stringlength(char *str)
{
	ulong c = 0;

	while(*str++) /* loop until pointer reaches terminating NULL-BYTE */
	{
		c++;
	}

	return c;
}