#include <stdlib.h>
#include <omp.h>

int main()
{
    int* ptr = malloc(sizeof(int));

    #pragma omp parallel
    {
        *ptr = 42;
        printf("Value: %d\n", *ptr);
    }

    free(ptr);
}
