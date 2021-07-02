#include<omp.h>
#include<stdio.h>

int main()
{
    int* a = new int;
    *a = 42;

#pragma omp parallel shared(a)
    {
        *a += 1;
    }

    printf("a : %d\n", *a);
}
