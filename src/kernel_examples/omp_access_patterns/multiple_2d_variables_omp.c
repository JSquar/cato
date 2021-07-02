#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    int** array1 = (int**) malloc(sizeof(int*)*4);
    int** array2 = (int**) malloc(sizeof(int*)*4);

    for(int i = 0; i < 4; i++)
    {
        array1[i] = (int*) malloc(sizeof(int)*4);
        array2[i] = (int*) malloc(sizeof(int)*4);

        for(int j = 0; j < 4; j++)
        {
            array1[i][j] = 0;
            array2[i][j] = 0;
        }
    }

    #pragma omp parallel
    {
        int rank = omp_get_thread_num();

        array1[rank][rank] = rank;
        //int tmp = array1[rank][rank];
        array2[rank][rank] = rank;
    }



    printf("%d,%d,%d,%d\n%d,%d,%d,%d\n%d,%d,%d,%d\n%d,%d,%d,%d\n", array2[0][0],array2[0][1],array2[0][2],array2[0][3],
            array2[1][0],array2[1][1],array2[1][2],array2[1][3],
            array2[2][0],array2[2][1],array2[2][2],array2[2][3],
            array2[3][0],array2[3][1],array2[3][2],array2[3][3]);

    for(int i = 0; i < 4; i++)
    {
        free(array1[i]);
        free(array2[i]);
    }
    free(array1);
    free(array2);
}
