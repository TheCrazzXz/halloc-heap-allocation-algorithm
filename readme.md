# HALLOC (Heap-allocation algoritm)
This is an alternative to GLIBC'S MALLOC algorithm  
It includes 3 main functions :
- halloc : allocate memory
- hfree : free memory
- hralloc : re-allocate to bigger memory

Versions : 
- (V 1.0.0 : Released 2nd of July, 2023.) <---- OBSOLETE AND WRONG, USE NEXT ONE
- V 1.1.0 : Released 20th of July, 2023.
  Changelog :
  18/07/2023 - 20/07/2023 : TCHALLOC v1.1.0


Changelog :
	- Fixed issue where prev_used of a chunk is not updated when the previous chunk is allocated before it, so that it's used, and now prev_used of that chunk is set to 1
	- Fixed issue where prev_size of a chunk after a splitted chunk is not updated, it's prev_size is now the size of the second chunk of the split
	- Redisigned realloc :
		- The classical algorithm is the following :
			1) Allocate memory for new size
			2) Copy all previous data to this new memory
			3) Free old memory
		- Particular optimisations :
			If the chunk we're reallocating is placed at the very end of the heap (last chunk), then it's litterally extended
			If the chunk we're allocating (A) is placed before a chunk B such as B is freed and (memory size / user space of A + size of B >= new size), then the chunk we're allocating is extended, B is split and first part of B is used for user space of new A
	- Replaced used byte and prev_used byte of a chunk by a char variable that contains these flags
	- Completely changed more mem algorithm to use brk() and sbrk() instead of mmap()
			
TheCrazz@NO_COPYRIGHT
