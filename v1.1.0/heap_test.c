#include "heap/heap.h"

int main(void)
{
    void* ptr = halloc(16384);

    if(ptr == NULL)
    {
        printf("HALLOC ERROR\n");
    }

    hchunk* chunk = (hchunk*)( (ulong)ptr - sizeof(hchunk) );

    printf("%p\n", chunk);

    hfree(ptr);

    hcleanup();
}