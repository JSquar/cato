#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    int** a = (int**)malloc(sizeof(int*));
    int** b = (int**)malloc(sizeof(int*));

    a[0] = (int*)malloc(sizeof(int)*4);
    b[0] = (int*)malloc(sizeof(int)*4);

    a[0][0] = 0;
    b[0][0] = 0;

    #pragma omp parallel
    {
        a[0][0] = 42;
        b[0][0] = a[0][0];
    }

    printf("%d\n", b[0][0]);

    free(a[0]);
    free(b[0]);
    free(a);
    free(b);
}
