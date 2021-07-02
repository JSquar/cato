#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

void use_pointer(int * p, int x)
{
    *p = x;
}

int main()
{
    int* ptr = (int*)malloc(sizeof(int));
    *ptr = 0;

    #pragma omp parallel
    {
        use_pointer(ptr, 42);
    }

    printf("p: %d\n", *ptr);
    free(ptr);
}
