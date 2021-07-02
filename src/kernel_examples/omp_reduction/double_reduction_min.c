#include <stdio.h>
#include <omp.h>

int main()
{
    double result = 0.0;

    #pragma omp parallel reduction(min:result)
    {
        result = omp_get_thread_num();
    }

    printf("Result: %f\n", result);
}
