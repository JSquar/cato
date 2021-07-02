#include<omp.h>
#include<stdio.h>

int main()
{
    int* a = new int;
    *a = 0;

#pragma omp parallel shared(a)
    {
        *a += 1;
        *a += 3;
    }

    printf("a : %d\n", *a);
}
