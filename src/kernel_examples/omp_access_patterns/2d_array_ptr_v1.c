#include <stdlib.h>
#include <omp.h>

int main()
{
    int** arr = malloc(sizeof(int*) * 2);
    
    arr[0] = malloc(sizeof(int) * 2);
    arr[1] = malloc(sizeof(int) * 2);

    #pragma omp parallel
    {
        arr[0][0] = 0;
        arr[0][1] = 1;
        arr[1][0] = 2;
        arr[1][1] = 3;

        printf("[%d, %d]\n[%d, %d]\n", arr[0][0], arr[0][1], arr[1][0], arr[1][1]);
    }

    free(arr[0]);
    free(arr[1]);
    free(arr);
}
