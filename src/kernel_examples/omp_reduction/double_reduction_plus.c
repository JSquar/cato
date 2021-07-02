
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    double result = 0;

    #pragma omp parallel reduction(+:result)
    {
        int rank = omp_get_thread_num();
        result += rank;
    }

    printf("Result: %f\n", result);
}
