#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


struct data
{
    double *Matrix;
    int counter;
};

int main()
{
    struct data d;
    d.Matrix = malloc(sizeof(double)*4);

    d.Matrix[0] = 42;
    d.Matrix[1] = 43;
    d.Matrix[2] = 44;
    d.Matrix[3] = 45;
    d.counter = 0;

    #pragma omp parallel
    {
        int rank = omp_get_thread_num();
        d.Matrix[rank] = rank;
        d.counter++;
    }

    printf("%d, %f %f %f %f\n", d.counter, d.Matrix[0],d.Matrix[1],d.Matrix[2],d.Matrix[3]);

    free(d.Matrix);

}
