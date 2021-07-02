#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int result = 0;

    #pragma omp parallel reduction(-:result)
    {
        result -= omp_get_thread_num();
    }

    printf("Result: %d\n", result);
}
