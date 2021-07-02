#include <stdio.h>
#include <omp.h>

int main()
{
    int x = 0;

    #pragma omp parallel
    {
        int y = x;
        printf("Y: %d\n", y);
        if(omp_get_thread_num() == 0)
        {
            x = 42;
        }
    }

    printf("X: %d\n", x);
}
