#include "../heap/heap.h"

int main(void)
{
    halloc(256);
    halloc(512);
    void* ptr = halloc(8192);

    printf("pointer : %p\n", ptr);

    print_heap(0);
}