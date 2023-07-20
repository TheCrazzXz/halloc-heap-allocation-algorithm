/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "memory.h"
#include <stdio.h> 

void memcopy(void* dst, void* src, size_t size)
{
	while(size--) /* loop until size reaches 0 */
	{
		*((char*)dst++) = *((char*)src++);
	}
}
void memfill(void* dst, char c, size_t size)
{
	char* mem = (char*)dst;

    while(size--) /* loop until size reaches 0 */
    {
        *mem++ = c;
    }
}

void print_buffer(char* buffer, size_t size)
{
	printf("\n<start %p (size=%lu)>\n", buffer, size);
	size_t i = 0;
	while(i < size)
	{
		printf("%x ", (unsigned char)buffer[i]);
		i++;
	}
	printf("\n<end %p>\n\n", buffer+size);
}

size_t stringlength(char *str)
{
	size_t c = 0;

	while(*str++) /* loop until pointer reaches terminating NULL-BYTE */
	{
		c++;
	}

	return c;
}