#include <stdlib.h>
#include <omp.h>

int main()
{
    int* data = malloc(sizeof(int));
    int** arr = malloc(sizeof(int*));

    arr[0] = data;

    #pragma omp parallel
    {
        arr[0][0] = 42;
        printf("%d\n", arr[0][0]);
    }

    free(data);
    free(arr);
}
