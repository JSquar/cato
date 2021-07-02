#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    int* a = (int*)malloc(sizeof(int));
    int* b = (int*)malloc(sizeof(int));

    *a = 0;
    *b = 0;

    #pragma omp parallel
    {
        *a = 42;
        *b = *a;
    }

    printf("%d\n", *b);

    free(a);
    free(b);
}
