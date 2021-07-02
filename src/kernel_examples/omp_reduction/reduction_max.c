#include <stdio.h>
#include <omp.h>

int main()
{
    int result = 0;

    #pragma omp parallel reduction(max:result)
    {
        result = omp_get_thread_num();
    }

    printf("Result: %d\n", result);
}
