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

/* get the bit of a number */
#define GET_BIT(n, k) ((n >> (k)) & 1)

/* set the bit k (from 0) of a number */
#define SET_BIT(n, k) (n | (1 << k))

/* clear the bit k (from 0) of a number */
#define CLEAR_BIT(n, k) (n & (~(1 << k)))

/* modify the bit k (from 0) of a number */
#define MODIFY_BIT(n, k, v) ((v == 1) ? SET_BIT(n, k) : CLEAR_BIT(n, k))

/* adds offset to a pointer*/
#define offsetptrfd(ptr, off, T) (T*)((size_t)ptr + (size_t)off)
/* subs offset to a pointer*/
#define offsetptrbk(ptr, off, T) (T*)((size_t)ptr - (size_t)off)

typedef char byte;
typedef unsigned long size_t;

/* will copy `size` bytes from src to dst */
void memcopy(void* dst, void* src, size_t size);

/* will fill `size` bytes from dst with the byte c */
void memfill(void* dst, char c, size_t size);

/* prints a buffer of size `size` */
void print_buffer(char* buffer, size_t size);

/* get the length of a string terminated by a NULL byte (0x00) */
size_t stringlength(char* str);

/* copies the terminated by null-byte buffer src to dst */
#define stringcopy(dst, src) memcopy(dst, src, stringlength(src))

#endif