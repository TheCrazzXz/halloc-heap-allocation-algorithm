/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

/*
    Utility memory functions
*/

#pragma once

#ifndef __MEMORY_H__
#define __MEMORY_H__

typedef char byte;
typedef unsigned long ulong;

/* will copy `size` bytes from src to dst */
void memcopy(void* dst, void* src, ulong size);

/* will fill `size` bytes from dst with the byte c */
void memfill(void* dst, char c, ulong size);

/* prints a buffer of size `size` */
void print_buffer(char* buffer, ulong size);

/* get the length of a string terminated by a NULL byte (0x00) */
ulong stringlength(char* str);

/* copies the terminated by null-byte buffer src to dst */
#define stringcopy(dst, src) memcopy(dst, src, stringlength(src))

#endif