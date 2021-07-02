#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int* array = (int*)malloc(sizeof(int)*10);

    #pragma omp parallel for
    {
        for(int i = 0; i < 10; i++)
        {
            array[i] = i;
        }
    }

    printf("[%d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n", array[0], array[1],array[2], array[3],array[4], array[5],array[6], array[7],array[8], array[9]);
}
