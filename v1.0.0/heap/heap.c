/*
    I don't care about any license

	This file is part of the HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "heap.h"
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


/* adds offset to a pointer*/
#define offsetptrfd(ptr, off, T) (T*)((ulong)ptr + (ulong)off)
/* subs offset to a pointer*/
#define offsetptrbk(ptr, off, T) (T*)((ulong)ptr - (ulong)off)

#define mem2chunk(mem) (offsetptrbk(mem, sizeof(hchunk), void))
#define chunk2mem(chunk) (offsetptrfd(chunk, sizeof(hchunk), void))

#define add_chunk_size(size) (size + sizeof(hchunk))
#define sub_chunk_size(size) (size - sizeof(hchunk))

#define chunk_end_wilderness_offset(chunk) ((ulong)halloc_heap_state.wilderness_ptr - (ulong)chunk - chunk->size)

#define ptr_in_chunks_zone(ptr, heap) ((void*)ptr < (heap).wilderness_ptr)
#define next_chunk(ptr) (offsetptrfd(ptr, ptr->size, hchunk))
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
	chunk.used = (char)1; \
	chunk.prev_size = 0; \
	chunk.prev_used = 1; \
}

/* merges the two adjacent chunks chunk1 and chunk2 (where chunk1 is before chunk2) by extending the size of chunk1 with the size of chunk2 */

#define heap_consolidate(chunk1, chunk2) chunk1->size += chunk2->size;

/* heap state for the heap */
heapstate halloc_heap_state = {NULL, NULL, {MORE_MEMORY_STATE_NONE, NULL, 0, 0}, {}};

/* abort the process with an error */
void abortError(const char* msg, const char* function)
{
	fprintf(stderr, "%s", msg);
	perror(function);
	printf("\n");
	abort();
}

/* extends the size of the heap with addsize */
ulong extendHeap(ulong addsize)
{
	ulong wilderness_off;
	mmemState s;
	
	/* getting wilderness pointer offset from top heap */
	wilderness_off = (ulong)halloc_heap_state.wilderness_ptr - (ulong)halloc_heap_state.top_heap_ptr;
	
	s = extendMemorySpace(&halloc_heap_state.mmdata, addsize);
	if(s == MORE_MEMORY_STATE_FAILED)
	{
		heap_set_last_error("couldn't not extend the heap");
	}

	halloc_heap_state.top_heap_ptr = halloc_heap_state.mmdata.ptr;
	halloc_heap_state.wilderness_ptr = offsetptrfd(halloc_heap_state.mmdata.ptr, wilderness_off, void);

	return s;
}

/* initialize the heap by mapping the whole region */
void initHeap()
{
	halloc_heap_state.mmdata = createMemorySpace();
	halloc_heap_state.top_heap_ptr = halloc_heap_state.mmdata.ptr;
	halloc_heap_state.wilderness_ptr = halloc_heap_state.mmdata.ptr;
}

void print_heap(short show_mem)
{
	printf("The heap starting at <%p>\n", halloc_heap_state.top_heap_ptr);
	hchunk* ptr = halloc_heap_state.top_heap_ptr;

	

	while(ptr_in_chunks_zone(ptr, halloc_heap_state))
	{
		hchunk* current_heap_chunk = (hchunk*)ptr;
		printf("\tchunk <%p-%p> (size=%lu, prev_size=%lu, used=%u, prev_used=%u, alloc_base=%p, fd=%p, bk=%p)\n", ptr, (void*)((ulong)ptr+(ulong)current_heap_chunk->size), current_heap_chunk->size, current_heap_chunk->prev_size, (unsigned char)current_heap_chunk->used, (unsigned char)current_heap_chunk->prev_used, current_heap_chunk->alloc_base, current_heap_chunk->fd, current_heap_chunk->bk);
		
		if(show_mem)
		{
			print_buffer((char*)chunk2mem(ptr), current_heap_chunk->size);
		}
		
		follow_next_chunk(ptr);
	}
	printf("\tWilderness pointer : <%p>\n", halloc_heap_state.wilderness_ptr);
	printf("End of the heap (size=%lu)\n", halloc_heap_state.wilderness_ptr-halloc_heap_state.top_heap_ptr);
}

/* returns whether the bin list doesn't contain any element or does */
bool empty_bin_list()
{
	hchunk* ptr = halloc_heap_state.top_heap_ptr;

	while(ptr_in_chunks_zone(ptr, halloc_heap_state))
	{
		if(ptr->used == 0)
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
	hchunk* ptr = halloc_heap_state.top_heap_ptr;

	while(ptr_in_chunks_zone(ptr, halloc_heap_state))
	{
		if(ptr->used == 0 && ptr->bk == NULL)
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

	while(ptr_in_chunks_zone(ptr, halloc_heap_state))
	{
		if(ptr->used == 0)
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
		if(ptr->used == 0)
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

	returns NULL if there's not enough space
*/
hchunk* split_chunk(hchunk* chunk, ulong where)
{
	ulong second_size; /* size of the second chunk */
	ulong first_size;
	hchunk new_chunk;
	hchunk* new_chunk_in_heap;
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
	/* the second chunk will be located first_size bytes after the first chunk (original chunk with reduced size) */
	new_chunk_in_heap = offsetptrfd(chunk, first_size, hchunk);
	/* copying chunk metadata to new chunk */
	memcopy(&new_chunk, chunk, sizeof(hchunk));

	/* setting it's size */
	new_chunk.size = second_size;

	/* the prev data of this new chunk is the data of the first chunk in the split */
	new_chunk.prev_used = chunk->used;
	new_chunk.prev_size = first_size;

	/* the chunk before it in the binlist is the first chunk*/
	new_chunk.bk = chunk;
	
	/* the chunk after it is now the chunk that was originally the forward of of the first chunk */
	new_chunk.fd = chunk->fd;

	/* the original first-chunk's next chunk backwards pointer is now this chunk if there's a next chunk */
	if(chunk->fd != NULL)
	{
		chunk->fd->bk = new_chunk_in_heap;
	}
	
	/* the forward of the first chunk is this chunk */
	chunk->fd = new_chunk_in_heap;

	/* updating first chunk size */
	chunk->size = first_size;

	/* copy chunk data to the heap */
	memcopy(new_chunk_in_heap, &new_chunk, sizeof(hchunk));
	
	return new_chunk_in_heap;

}

void* find_space_in_bins(ulong size)
{
	hchunk* ptr;
	hchunk* new_ptr;
	ulong csize; /* chunk size */

	csize = add_chunk_size(size);

	/* if there's no freed chunk then return NULL */
	if(empty_bin_list())
	{
		return NULL;
	}
	
	ptr = first_bin_list();

	/* loop through all the freed chunks in the binlist */
	while(ptr_in_chunks_zone(ptr, halloc_heap_state))
	{
		if(ptr->used == 0) /* if chunk is free */
		{
			/* if there's enough space for the memory in this chunk, return it */
			if(sub_chunk_size(ptr->size) >= size)
			{
				/* if there's more memory in this chunk than needed */
				if(csize < ptr->size)
				{
					/* then split the chunk in two parts, the first part only used */
					new_ptr = split_chunk(ptr, csize);
				}
				return (void*)ptr;
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


void* find_space(ulong size, hchunk** prev)
{
	ulong csize; /* chunk size */
	
	void* space_search_in_bins;
	void* created_ptr;

	csize = add_chunk_size(size); /* calculating sizeof the whole chunk */
	space_search_in_bins = find_space_in_bins(size); /* searching for space were chunks have been freed */
	*prev = NULL;
	
	/* if there's no satisfying free chunk were it could be allocated, then extend the heap */
	if(space_search_in_bins == NULL)
	{
		if(extendHeap(csize) == MORE_MEMORY_STATE_FAILED)
		{
			return NULL;
		}
		
		created_ptr = halloc_heap_state.wilderness_ptr;
		halloc_heap_state.wilderness_ptr += csize;
	}
	/* else, use free space */
	else
	{
		created_ptr = space_search_in_bins;
		*prev = space_search_in_bins;
	}
	return created_ptr;
}


void* heap_allocate(ulong size)
{
	hchunk* previous;
	void* created_ptr;
	hchunk heap_chunk;

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_start("heap_allocate / halloc");
	#endif

	/* if the heap is not initialized yet, initialize it */
	if(halloc_heap_state.mmdata.state != MORE_MEMORY_STATE_MMAPED)
	{
		initHeap();
	}
	
	created_ptr = find_space(size, &previous);
	if(created_ptr == NULL)
	{
		return NULL;
	}

	heap_chunk.size = add_chunk_size(size);
	if(previous)
	{
		heap_chunk.size = previous->size;
		binlist_unlink(previous);
	}

	set_new_chunk_data(heap_chunk);
	heap_chunk.alloc_base = NULL;

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
	lastChunk = chunk_end_wilderness_offset(chunk) <= 1;


	if(chunk->prev_used == 0)
	{
		/* merge this chunk with the previous chunk if it's freed */
		prev = prev_chunk(chunk);

		heap_consolidate(prev, chunk);
		
		if(!lastChunk)
		{
			next->prev_size = prev->size;
			next->prev_used = prev->used;
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

		chunk->used = 0;
		chunk->fd = NULL;
		chunk->bk = binListLastBeforeFree;

		next->prev_size = chunk->size;
		next->prev_used = chunk->used;
	}
	/* checking if this is not the last chunk */
	if(!lastChunk)
	{
		if(next->used == 0)
		{
			/* merge this chunk with the next chunk if it's freed */
			heap_consolidate(currentChunk, next);
			binlist_unlink(next);

			follow_next_chunk(next);

			next->prev_size = currentChunk->size;
			next->prev_used = currentChunk->used;
		}
	}

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_end("heap_free / hfree");
	#endif
}



void* heap_realloc(void* mem, ulong size)
{
	hchunk* current_chunk = ((hchunk*)mem2chunk(mem));
	void* created_ptr;
	hchunk* previous;
	ulong offset;
	hchunk heap_chunk;

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_start("heap_realloc / hralloc");
	#endif

	if(add_chunk_size(size) < current_chunk->size)
	{
		heap_set_last_error("Cannot realloc a lower size chunk");
		return NULL;
	}

	
	offset = chunk_end_wilderness_offset(current_chunk);

	/* Checking if current_chunk (to realloc) is the last chunk on the heap */
	if(offset == 0)
	{
		created_ptr = (void*)current_chunk;
		halloc_heap_state.wilderness_ptr += add_chunk_size(size) - current_chunk->size;
	}
	else
	{
		
		created_ptr = find_space(size, &previous);

		if(created_ptr == NULL)
		{
			return NULL;
		}

		heap_free(mem);
	}

	heap_chunk.size = add_chunk_size(size);
	if(previous)
	{
		heap_chunk.size = previous->size;
		binlist_unlink(previous);
	}
	set_new_chunk_data(heap_chunk);

	if(current_chunk->alloc_base == NULL)
	{
		heap_chunk.alloc_base = current_chunk;
	}
	else
	{
		heap_chunk.alloc_base = current_chunk->alloc_base;
	}
	memcopy(created_ptr, (void*)&heap_chunk, sizeof(hchunk));
	memcopy(chunk2mem((void*)created_ptr), (void*)mem, sub_chunk_size(current_chunk->size));

	#if defined(HEAP_DEBUG_PRINT)
		print_heap_dbg_end("heap_realloc / hralloc");
	#endif

	return chunk2mem((void*)created_ptr);
}