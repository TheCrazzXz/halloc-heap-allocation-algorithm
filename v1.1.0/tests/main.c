#include <stdio.h>
#include <string.h>
#include "../heap/heap.h"

/*
	[CHUNK 1] : free (fd = C3, bk=C2) (3)
	[CHUNK 2] : free (fd = C1, bk = NULL) (1)
	[CHUNK 3] : free (fd = NULL, bk = C1) (2)

	allocate

	[CHUNK 1] : USED (3)
	[CHUNK 2] : free (fd = C1, bk = NULL) (1)
	[CHUNK 3] : free (fd = NULL, bk = C1) (2)

*/



int main()
{
	void* ptr1;
	void* ptr2;
	void* ptr3;
	void* other_ptr;

	printf("size of struct HeapChunk = %lu\n", sizeof(struct __heap_chunk_t));

	ptr1 = halloc(200);
	printf("ptr : %p\n", ptr1);

	ptr2 = halloc(100);
	printf("ptr : %p\n", ptr2);

	ptr3 = halloc(150);
	printf("ptr : %p\n", ptr3);

	printf("freeing ptr 1\n");
	hfree(ptr1);

	print_heap(0);
	print_bin_list();

	printf("freeing ptr 2\n");
	hfree(ptr2);

	print_heap(0);
	print_bin_list();

	printf("freeing ptr 3\n");
	hfree(ptr3);

	print_heap(0);
	print_bin_list();

	printf("halloc(100)\n");

	other_ptr = halloc(100);

	printf("otherptr : %p\n", other_ptr);

	print_heap(0);
	print_bin_list();

	hfree(other_ptr);

	hcleanup();
	
}