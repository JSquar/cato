#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int res1 = 0;
    double res2 = 0.0;

    #pragma omp parallel
    {
        #pragma omp for reduction(+:res1)
        for(int i = 0; i < omp_get_num_threads(); i++)
        {
            int rank = omp_get_thread_num();
            res1 = rank;
        }
        #pragma omp for reduction(+:res2)
        for(int i = 0; i < omp_get_num_threads(); i++)
        {
            double rank = omp_get_thread_num() + 0.5;
            res2 = rank;
        }
    }

    printf("%d, %f\n", res1, res2);
}
