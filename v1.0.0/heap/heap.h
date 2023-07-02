/*
	I don't care about any license

	HEAP ALLOCATION ALGORITHM TCHALLOC
							  (ps : TC is The Crazz)
	Author : TheCrazzXz 
	See github : https://github.com/TheCrazzXz/
*/

#include "memory.h"
#include "moremem.h"

/* state of the heap */
typedef struct Heap
{
	void* top_heap_ptr;
	void* wilderness_ptr;
	
	mmemData mmdata;

	char errorBuffer[500];

} heapstate;

/* state of the heap */
extern heapstate halloc_heap_state;

/* halloc chunk */
typedef struct HeapChunk
{
	ulong size; /* size of the whole chunk */
	ulong prev_size; /* whole size of the previous chunk */
	byte prev_used; /* is the previous chunk in use */
	byte used; /* was this chunk free */
	struct HeapChunk* alloc_base; /* first allocation before any realloc */
	struct HeapChunk* fd; // forward pointer in fastbin linked list (free)
	struct HeapChunk* bk; // backward pointer in fastbin linked list (free)
} hchunk;

/* print heap : utility function to print all heap chunks and information about them, if `print_mem` is enabled, it will print the memory (with metadata) of the chunks as well */
void print_heap(short print_mem);

/* print_bin_list : utilitiy function to print all addresses in the binlist */
void print_bin_list();

/* heap_allocate : allocate a heap chunk that has enough space for `size` bytes and the metadata of the chunk */
void* heap_allocate(ulong size);

/* heap_free : when a memory region returned by heap_allocate isn't used by the user anymore, it has to be freed so that the region can be reused for other chunks to be allocated there */
void heap_free(void* mem);

/* heap_realloc : extends the size of a chunk */
void* heap_realloc(void* mem, ulong size);

#define halloc heap_allocate
#define hfree heap_free
#define hralloc heap_realloc 

#define heap_set_last_error(msg) stringcopy(halloc_heap_state.errorBuffer, msg)

/* heap_cleanup : when the heap isn't used anymore, it should be deleted */
#define heap_cleanup() mmem_uncreate(&halloc_heap_state.mmdata)
#define hcleanup heap_cleanup

/* get string about last error that happened in the heap */
#define heap_last_error halloc_heap_state.errorBuffer

#define hlasterr heap_last_error