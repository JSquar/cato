#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int* a = (int*)malloc(sizeof(int)*4);
    int* b = (int*)malloc(sizeof(int)*4);

    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;

    #pragma omp parallel
    {
        int rank = omp_get_thread_num();
        b[rank] = a[rank];
    }

    printf("[%d,%d,%d,%d]\n", b[0], b[1], b[2], b[3]);

    free(a);
    free(b);
}
