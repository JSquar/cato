#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    double result = 0;

    #pragma omp parallel reduction(-:result)
    {
        result -= omp_get_thread_num();
    }

    printf("Result: %f\n", result);
}
