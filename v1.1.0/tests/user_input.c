#include <stdio.h>
#include <stdlib.h>

#include "../heap/heap.h"

ulong keyboard_input(char** output)
{
	printf("Keyboard input start\n");
	ulong len = 1;

	char* buffer = halloc(1);
	print_heap(0);
	char c;
	while(c != '\n')
	{
		c = getc(stdin);
		if(c == '\n')
		{
			break;  
		}
		if(len > 1)
		{
			buffer = hralloc(buffer, len);
			printf("Buffer at : %p\n", buffer);
			print_heap(0);

		}
		buffer[len-1] = c;

		len++;
		//printf("Buffer size : %lu\n", ((struct HeapChunk*)mem2chunk(buffer))->size);
	}
	buffer = hralloc(buffer, len);
	buffer[len] = '\x00';

	*output = buffer;

	return len;
}


int main(void)
{
	while(1)
	{
		char* buffer;
		ulong size = keyboard_input(&buffer);
		printf("Buffer (%p) : (size=%lu) [%s]\n", buffer, size, buffer);
		print_heap(0);

		hfree(buffer);

		print_heap(0);
		print_bin_list();
	}	
}