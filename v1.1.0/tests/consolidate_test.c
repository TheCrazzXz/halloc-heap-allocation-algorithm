#include "../heap/heap.h"

int main(void)
{
    void* ptr1 = halloc(50);
    void* ptr2 = halloc(100);
    void* ptr3 = halloc(120);
    void* ptr4 = halloc(200);

    print_heap(0);
    print_bin_list();

    hfree(ptr1);
    hfree(ptr3);
    hfree(ptr2);

    print_heap(0);
    print_bin_list();

    hcleanup();
}