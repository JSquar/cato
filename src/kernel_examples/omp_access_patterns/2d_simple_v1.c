#include <stdlib.h>
#include <omp.h>

int main()
{
    int** arr = malloc(sizeof(int*));
    arr[0] = malloc(sizeof(int));

    #pragma omp parallel
    {
        arr[0][0] = 42;
        printf("%d\n", arr[0][0]);
    }

    free(arr[0]);
    free(arr);
}
