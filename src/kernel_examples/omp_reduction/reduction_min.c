#include <stdio.h>
#include <omp.h>

int main()
{
    int result = 100;

    #pragma omp parallel reduction(min:result)
    {
        result = omp_get_thread_num();
    }

    printf("Result: %d\n", result);
}
