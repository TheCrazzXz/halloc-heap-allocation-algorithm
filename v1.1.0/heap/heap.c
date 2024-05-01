/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC V1.1.0
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "heap.h"
#include "memory.h"
#include "moremem.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/*
	Define HEAP_DEBUG_PRINT to print debug at each heap_allocate, hfree or heap_realloc

	To enable it from GCC, use gcc ... -D HEAP_DEBUG_PRINT
*/
/*#define HEAP_DEBUG_PRINT*/

#define print_heap_dbg_start(func) { \
	printf("---------------------------------------------------------------------------------\nHEAP DEBUG <%s> start : \n", func); \
	print_heap(0); \
	printf("end\n"); \
}
#define print_heap_dbg_end(func) { \
	printf("HEAP DEBUG <%s> end : \n", func); \
	print_heap(0); \
	printf("end\n---------------------------------------------------------------------------------\n"); \
}




/* converts a memory from a chunk address to the start of the chunk address */
#define mem2chunk(mem) (offsetptrbk(mem, sizeof(hchunk), void))

/* converts a start-of-the-chunk address to memory of the chunk address */
#define chunk2mem(chunk) (offsetptrfd(chunk, sizeof(hchunk), void))

#define add_chunk_size(size) (size + sizeof(hchunk))
#define sub_chunk_size(size) (size - sizeof(hchunk))

/* get the offset between a start of a chunk and wilderness */
#define chunk_end_wilderness_offset(chunk) ((size_t)halloc_heap_state.mmdata.end - (size_t)chunk - chunk->size)

/* check if a pointer is before the wilderness */
#define ptr_in_chunks_zone(ptr) ((void*)ptr < halloc_heap_state.mmdata.end)

/* move to the next chunk after a chunk*/
#define next_chunk(ptr) (offsetptrfd(ptr, ptr->size, hchunk))

/* check if a chunk is the last chunk of the heap (stick to the wilderness) */
#define is_chunk_last_chunk(chunk) (next_chunk(chunk) == halloc_heap_state.mmdata.end)

/* move the the previous chunk before a chunk */
#define prev_chunk(ptr) (offsetptrbk(ptr, ptr->prev_size, hchunk))
#define follow_next_chunk(ptr) ptr = (offsetptrfd(ptr, ptr->size, hchunk))
#define follow_fd(ptr) ptr = (ptr->fd)

/* Remove a chunk from the binlist :

	- if there's a next chunk :
		set the previous chunk of the next chunk to the previous chunk
	- if there's a previous chunk :
		set the next chunk of the previous chunk to the next chunk

 */

#define binlist_unlink(ptr) { \
	if(ptr->fd != NULL) \
	{ \
		ptr->fd->bk = ptr->bk; \
	} \
	if(ptr->bk != NULL) \
	{ \
		ptr->bk->fd = ptr->fd; \
	} \
}

/* sets fd, bk to 0, used to 1, prev_size to 0 and prev_used to 1 */
#define set_new_chunk_data(chunk) {\
	chunk.fd = NULL; \
	chunk.bk = NULL; \
	modify_chunk_used_flag(chunk, 1);\
	chunk.prev_size = 0; \
	modify_chunk_prev_used_flag(chunk, 1);\
}

/* merges the two adjacent chunks chunk1 and chunk2 (where chunk1 is before chunk2) by extending the size of chunk1 with the size of chunk2 */

#define heap_consolidate(chunk1, chunk2) chunk1->size += chunk2->size;

/* heap state for the heap */
heapstate halloc_heap_state = {{MORE_MEMORY_STATE_NONE, NULL, NULL}, {}};

/* abort the process with an error */
void abortError(const char* msg, const char* function)
{
	fprintf(stderr, "%s", msg);
	perror(function);
	printf("\n");
	abort();
}

/* extends the size of the heap with addsize */
size_t extendHeap(size_t addsize)
{
	mmemState s;
	
	s = extendMemorySpace(&halloc_heap_state.mmdata, addsize);
	if(s == MORE_MEMORY_STATE_FAILED)
	{
		heap_set_last_error("couldn't not extend the heap");
	}

	return s;
}

/* initialize the heap by mapping the whole region */
void initHeap()
{
	halloc_heap_state.mmdata = createMemorySpace();
}

/* printing the heap chunks */
void print_heap(short show_mem)
{
	hchunk* ptr;
	size_t heap_size;

	ptr = halloc_heap_state.mmdata.start;
	heap_size = 0;
	printf("The heap starting at <%p>\n", halloc_heap_state.mmdata.start);
	

	while(ptr_in_chunks_zone(ptr))
	{
		hchunk* current_heap_chunk = (hchunk*)ptr;
		if(is_chunk_in_use(*current_heap_chunk))
			printf("\t\033[32;1;4mchunk <%p-%p> (size=%lu, prev_size=%lu, used=%u, prev_used=%u, fd=%p, bk=%p) [%p (%lu)] \033[0m\n", ptr, (void*)((size_t)ptr+(size_t)current_heap_chunk->size), current_heap_chunk->size, current_heap_chunk->prev_size, is_chunk_in_use(*current_heap_chunk), is_prev_chunk_in_use(*current_heap_chunk), current_heap_chunk->fd, current_heap_chunk->bk, offsetptrfd(current_heap_chunk, sizeof(hchunk), void), sub_chunk_size(current_heap_chunk->size));
		else
			printf("\t\033[31;1;4mchunk <%p-%p> (size=%lu, prev_size=%lu, used=%u, prev_used=%u, fd=%p, bk=%p) [%p (%lu)] \033[0m\n", ptr, (void*)((size_t)ptr+(size_t)current_heap_chunk->size), current_heap_chunk->size, current_heap_chunk->prev_size, is_chunk_in_use(*current_heap_chunk), is_prev_chunk_in_use(*current_heap_chunk), current_heap_chunk->fd, current_heap_chunk->bk, offsetptrfd(current_heap_chunk, sizeof(hchunk), void), sub_chunk_size(current_heap_chunk->size));
		if(is_chunk_in_use(*current_heap_chunk) == 1)
		{
			heap_size += current_heap_chunk->size;
		}

		if(show_mem)
		{
			print_buffer((char*)chunk2mem(ptr), current_heap_chunk->size);
		}
		
		follow_next_chunk(ptr);
	}
	printf("\tWilderness pointer : <%p>\n", halloc_heap_state.mmdata.end);
	printf("End of the heap (size=%lu)\n", heap_size);
}

/* returns whether the bin list doesn't contain any element or does */
bool empty_bin_list()
{
	hchunk* ptr = halloc_heap_state.mmdata.start;

	while(ptr_in_chunks_zone(ptr))
	{
		if(is_chunk_in_use(*ptr) == 0)
		{
			return false;
		}
		follow_next_chunk(ptr);
	}
	return true;
}

/* get the first chunk of the bin list */
hchunk* first_bin_list()
{
	if(empty_bin_list())
	{
		return NULL;
	}
	hchunk* ptr = halloc_heap_state.mmdata.start;

	while(ptr_in_chunks_zone(ptr))
	{
		if(!is_chunk_in_use(*ptr) && ptr->bk == NULL)
		{
			return ptr;
		}
		follow_next_chunk(ptr);
	}
	return NULL;

}

void print_bin_list()
{
	hchunk* ptr = first_bin_list();

	if(ptr == NULL)
	{
		printf("Bin list is empty\n");
		return;
	}

	while(ptr_in_chunks_zone(ptr))
	{
		if(!is_chunk_in_use(*ptr))
		{
			if(ptr->fd == NULL)
			{
				printf("binlist end = %p\n", ptr);
				return;
			}
			else
			{
				printf("binlist : %p\n", ptr);
			}
			follow_fd(ptr);

			
		}
		else
		{
			follow_next_chunk(ptr);
		}
		
		
	}
}

/* get last chunk of the bin list */
hchunk* bin_list_last()
{
	hchunk* ptr = first_bin_list();


	if(empty_bin_list())
	{
		return NULL;
	}

	while(true)
	{
		if(!is_chunk_in_use(*ptr))
		{
			if(ptr->fd == NULL)
			{
				return ptr;
			}
			follow_fd(ptr);
		}
		else
		{
			follow_next_chunk(ptr);
		}
	}
} 

/*
	splits a chunk in 2 parts
	Before :
	Chunk : {[metadata][data]}
	After :
	Chunk : {[metadata 1][data 1]}where{[metadata 2][data 2]}

	returns the adress of the second chunk after the split
	the first one stays the same but has it size reduced

	if firstused is 0 then the first chunk is not considered a chunk anymore, just some free memory

	returns NULL if there's not enough space
*/
hchunk* split_chunk(hchunk* chunk, size_t where, char firstused)
{
	size_t second_size; /* size of the second chunk */
	size_t first_size;
	hchunk new_chunk;
	hchunk* new_chunk_in_heap;
	hchunk* next;
	char lastChunk;

	next = next_chunk(chunk);
	lastChunk = (next == halloc_heap_state.mmdata.end);

	second_size = chunk->size - where;
	
	/* 
		(W : where, S : chunk->size, Sc : sizeof(hchunk))
		W      > S - Sc
		W - S  > - Sc | - S
		-W + S < Sc | multiply by -1
		S - W  < Sc | Rearrange
		second_size < Sc | chunk->size - where = second_size
	
	*/

	/* check if there's enough space */
	if(second_size < sizeof(hchunk))
	{
		return NULL;
	}

	first_size = where;

	if(first_size < sizeof(hchunk) && firstused)
	{
		return NULL;
	}

	/* the second chunk will be located first_size bytes after the first chunk (original chunk with reduced size) */
	new_chunk_in_heap = offsetptrfd(chunk, first_size, hchunk);
	/* copying chunk metadata to new chunk */
	memcopy(&new_chunk, chunk, sizeof(hchunk));

	/* setting it's size */
	new_chunk.size = second_size;

	/* the prev data of this new chunk is the data of the first chunk in the split */
	modify_chunk_prev_used_flag(new_chunk, is_chunk_in_use(*chunk));

	new_chunk.prev_size = first_size;

	/* the chunk before it in the binlist is the first chunk*/
	if(firstused)
	{
		new_chunk.bk = chunk;
	}
	else
	{
		if(chunk->bk != NULL)
		{
			chunk->bk->fd = new_chunk_in_heap;
		}
		new_chunk.bk = chunk->bk;
	}
	
	
	/* the chunk after it is now the chunk that was originally the forward of of the first chunk */
	new_chunk.fd = chunk->fd;

	/* the original first-chunk's next chunk backwards pointer is now this chunk if there's a next chunk */
	if(chunk->fd != NULL)
	{
		chunk->fd->bk = new_chunk_in_heap;
	}

	if(firstused)
	{
		/* the forward of the first chunk is this chunk */
		chunk->fd = new_chunk_in_heap;
			
		/* updating first chunk size */
		chunk->size = first_size;
	}
	

	/* copy chunk data to the heap */
	memcopy(new_chunk_in_heap, &new_chunk, sizeof(hchunk));

	/* update next chunk prev size to the second chunk size */
	if(!lastChunk)
	{
		next->prev_size = second_size;
	}


	return new_chunk_in_heap;

}

/* find and reserve enough space in memory for a chunk with memory / user data of size `size`
   returns NULL if wasn't found
*/
hchunk* find_and_reserve_space_in_bins(size_t size)
{
	hchunk* ptr;
	hchunk* new_ptr;
	size_t csize; /* chunk size */

	csize = add_chunk_size(size);

	/* if there's no freed chunk then return NULL */
	if(empty_bin_list())
	{
		return NULL;
	}
	
	/* start by the first element of the binlist */
	ptr = first_bin_list();

	/* loop through all the freed chunks in the binlist */
	while(ptr_in_chunks_zone(ptr))
	{
		if(!is_chunk_in_use(*ptr)) /* if chunk is free */
		{
			/* if there's enough space for the memory in this chunk, return it */
			if(sub_chunk_size(ptr->size) >= size)
			{
				/* if there's more memory in this chunk than needed */
				if(csize < ptr->size)
				{
					/* then split the chunk in two parts, the first part only used */
					new_ptr = split_chunk(ptr, csize, 1);
				}
				return ptr;
			}
		
			if(ptr->fd == NULL)
			{
				return NULL;
			}

			follow_fd(ptr); /* move to next freed chunk on the binlist */
		}
		
		else
		{
			follow_next_chunk(ptr); /* move to next chunk */
		}
	}
	return NULL;
}

/* find space on the heap */
hchunk* find_and_reserve_space(size_t size, hchunk** prev)
{
	size_t csize; /* chunk size */
	
	void* space_search_in_bins;
	hchunk* created_ptr;

	csize = add_chunk_size(size); /* calculating sizeof the whole chunk */
	space_search_in_bins = find_and_reserve_space_in_bins(size); /* searching for space were chunks have been freed */
	*prev = NULL;
	
	/* if there's no satisfying free chunk were it could be allocated, then extend the heap */
	if(space_search_in_bins == NULL)
	{
		created_ptr = halloc_heap_state.mmdata.end;

		if(extendHeap(csize) == MORE_MEMORY_STATE_FAILED)
		{
			return NULL;
		}
		
	}
	/* else, use free space */
	else
	{
		created_ptr = space_search_in_bins;
		*prev = space_search_in_bins;
	}
	return created_ptr;
}


void* heap_allocate(size_t size)
{
	hchunk* previous;
	hchunk* created_ptr;
	hchunk heap_chunk;
	hchunk* next;

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_start("heap_allocate / halloc");
	#endif

	/* if the heap is not initialized yet, initialize it */
	if(halloc_heap_state.mmdata.state != MORE_MEMORY_STATE_CREATED)
	{
		initHeap();
	}
	
	/* find space on the heap for the new chunk */
	created_ptr = find_and_reserve_space(size, &previous);
	if(created_ptr == NULL)
	{
		return NULL;
	}

	next = next_chunk(created_ptr);

	heap_chunk.size = add_chunk_size(size);

	/* if there was a previous chunk were we're allocating this chunk we unlink this previous chunk and update next chunk prev_used*/
	if(previous)
	{
		heap_chunk.size = previous->size;
		binlist_unlink(previous);
		modify_chunk_prev_used_flag(*next, 1);
	}

	set_new_chunk_data(heap_chunk);


	memcopy(created_ptr, (void*)&heap_chunk, sizeof(hchunk));

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_end("heap_allocate / halloc");
	#endif

	return chunk2mem(created_ptr);
}

void heap_free(void* mem)
{
	hchunk* chunk;
	hchunk* currentChunk;
	hchunk* next;
	hchunk* prev;
	hchunk* binListLastBeforeFree;
	int binListIsEmptyBefore;
	int lastChunk;

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_start("heap_free / hfree");
	#endif

	chunk  = ((hchunk*)mem2chunk(mem));
	next = next_chunk(chunk);
	currentChunk = chunk;
	
	binListLastBeforeFree = NULL;
	binListIsEmptyBefore = empty_bin_list();
	lastChunk = is_chunk_last_chunk(chunk);


	/* If the chunk is already freed, just ignore */
	if(!is_chunk_in_use(*chunk))
	{
		return;
	}

	if(!is_prev_chunk_in_use(*chunk))
	{
		/* merge this chunk with the previous chunk if it's freed */
		prev = prev_chunk(chunk);

		heap_consolidate(prev, chunk);
		
		if(!lastChunk)
		{
			next->prev_size = prev->size;
			modify_chunk_prev_used_flag(*next, is_prev_chunk_in_use(*prev));
		}
		
		currentChunk = prev;
	}
	else
	{
		if(!binListIsEmptyBefore)
		{
			binListLastBeforeFree = bin_list_last();
			/* adding this chunk to the linked list */
			binListLastBeforeFree->fd = chunk;
		}

		modify_chunk_used_flag(*chunk, 0);

		chunk->fd = NULL;
		chunk->bk = binListLastBeforeFree;

		next->prev_size = chunk->size;

		modify_chunk_prev_used_flag(*next, is_chunk_in_use(*chunk));

	}
	/* checking if this is not the last chunk */
	if(!lastChunk)
	{
		if(is_chunk_in_use(*next) == 0)
		{
			/* merge this chunk with the next chunk if it's freed */
			heap_consolidate(currentChunk, next);
			binlist_unlink(next);
			/*if(next->fd != NULL) 
			{ 
				next->fd->bk = next->bk; 
			} 
			if(next->bk != NULL) \
			{ 
				next->bk->fd = next->fd; 
			}*/

			follow_next_chunk(next);

			next->prev_size = currentChunk->size;
			modify_chunk_prev_used_flag(*next, is_chunk_in_use(*currentChunk));
		}
	}

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_end("heap_free / hfree");
	#endif
}



void* heap_realloc(void* mem, size_t size)
{
	hchunk* current_chunk;
	hchunk* new_chunk_mem;
	hchunk* next;
	hchunk* next_next;
	char lastChunk;
	size_t off;
	size_t current_chunk_mem_size;
	void* heap_mem_before;
	
	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_start("heap_realloc / hralloc");
	#endif

	current_chunk = ((hchunk*)mem2chunk(mem));
	next = next_chunk(current_chunk);
	lastChunk = is_chunk_last_chunk(current_chunk);
	current_chunk_mem_size = sub_chunk_size(current_chunk->size);

	/* if we're reallocating the same size we can just use the original chunk */
	if(size <= current_chunk_mem_size)
	{
		return mem;
	}

	/* if the chunk we're reallocating is the last chunk of the heap then directly extend it */
	if(lastChunk)
	{
		/* get difference between user sizes between new chunk and old chunk */
		off = size - current_chunk_mem_size;

		/* extend heap with this difference */
		if(extendHeap(off) != MORE_MEMORY_STATE_FAILED)
		{

			/* extend heap chunk size and wilderness pointer with this difference */
			current_chunk->size += off;
			
			#if defined(HEAP_DEBUG_PRINT)
				print_heap_dbg_end("heap_realloc / hralloc");
			#endif

			return mem;
		}
	}
	/* if next chunk is free and memory size / user space of the chunk we're reallocating added to size of next chunk is enough for allocation, we would extend that chunk with next chunk data */
	else
	{
		if((current_chunk_mem_size + next->size >= size) && (!is_chunk_in_use(*next)))
		{
			/* additionnal size we'll need for reallocation */
			off = size - current_chunk_mem_size;

			next_next = split_chunk(next, off, 0);

			/* split chunk in two parts */
			if(next_next != NULL)
			{
				next_next = offsetptrfd(next, off, hchunk);

				/* extend current chunk */
				current_chunk->size += off;

				/* update second chunk of the split prev size and prev used */
				next_next->prev_size = current_chunk->size;
				modify_chunk_prev_used_flag(*next_next, is_chunk_in_use(*current_chunk));

				#if defined(HEAP_DEBUG_PRINT)
					print_heap_dbg_end("heap_realloc / hralloc");
				#endif
				
				return mem;
			}
		}
	}

	/* allocate enough memory for new size */
	new_chunk_mem = heap_allocate(size);
	if(new_chunk_mem == NULL)
	{
		return NULL;
	}

	/* copy all memory / user data from current chunk to new chunk */
	memcopy(new_chunk_mem, mem, sub_chunk_size(current_chunk->size));

	/* now free the old chunk */
	heap_free(mem);
	
	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_end("heap_realloc / hralloc");
	#endif

	return new_chunk_mem;	
}