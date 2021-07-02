#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    //Fails with 512 * 4081
    long size = 512 * 5000;

    int *data = (int*)malloc(size * sizeof(int));

#pragma omp parallel for 
        for( long i = 0; i < 512 * 5000; i++)
        {
            int rank = omp_get_thread_num();
            /* printf("rank: %d, before %ld\n", rank, i); */
            data[i] = rank;
            /* printf("rank: %d, after %ld\n", rank, i); */
        }
    
    free(data);
    return 0;
}
