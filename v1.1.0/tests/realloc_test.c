#include "../heap/heap.h"

int main(void)
{
    size_t s;

    void* ptr1;
    void* ptr2;
    void* ptr3;
    
    s = 256;

    ptr1 = halloc(64);

    print_heap(0);

    ptr2 = halloc(s);

    print_heap(0);

    ptr3 = halloc(512);

    print_heap(0);

    hfree(ptr3);

    print_heap(0);

    while(1)
    {
        printf("Enter size for ptr2 (realloc : %lu, enter 0 to stop) : ", s);
        scanf("%lu", &s);
        fgetc(stdin);

        if(s == 0)
        {
            break;
        }

        ptr2 = hralloc(ptr2, s);

        if(ptr2 == NULL)
        {
            fprintf(stderr, "ERROR WITH THE HEAP : %s\n", hlasterr);
        }

        print_heap(0);
    }

    hcleanup();
}