#include "../heap/heap.h"
#include <unistd.h>

int main(void)
{
    void* currentReturn;
    void* address;
    size_t size;
    int command;
    while(1)
    {
        printf("0) exit\n1) halloc\n2) hfree\n3) hralloc\n");

        printf("Enter command : \n> ");
        scanf("%d", &command);
        fgetc(stdin);

        if(command == 1)
        {
            printf("Enter size for halloc : \n\t> ");
            scanf("%lu", &size);
            fgetc(stdin);

            currentReturn = halloc(size);
            if(currentReturn == NULL)
            {
                printf("Error : Failed at halloc : %s\n", hlasterr);
                continue;
            }
            printf("--> %p\n", currentReturn);
        }
        else if(command == 2)
        {
            printf("Enter address to free : \n\t> ");
            scanf("%p", &address);
            fgetc(stdin);

            hfree(address);
        }
        else if(command == 3)
        {
            printf("Enter address to re-allocate : \n\t>");
            scanf("%p", &address);
            fgetc(stdin);

            printf("Enter size for hralloc : \n\t> ");
            scanf("%lu", &size);
            fgetc(stdin);

            hralloc(address, size);
        }
        else if(command == 0)
        {
            break;
        }
        else
        {
            printf("Error : Commands in range [1 ; 3]\n");
        }
        print_heap(0);
    }
    hcleanup();
}