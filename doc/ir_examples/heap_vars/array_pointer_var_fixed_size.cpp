#include<omp.h>
#include<stdio.h>

int main()
{
    int* a = new int[8];

#pragma omp parallel shared(a)
    {
        int rank = omp_get_thread_num();

        a[rank] = rank;
        a[rank+4] = 42;
    }

    printf("[%d, %d, %d, %d, %d, %d, %d, %d]\n", a[0], a[1], a[2], a[3],a[4], a[5], a[6], a[7]);

}
