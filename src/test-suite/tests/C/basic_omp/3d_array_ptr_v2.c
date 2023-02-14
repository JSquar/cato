// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t) %s.reference_output
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main()
{
    int* M = (int*)malloc(2*2*2*sizeof(int));
    int*** Matrix = (int***)malloc(2*sizeof(int**));

    Matrix[0] = (int**) malloc(2*sizeof(int*));
    Matrix[0][0] = M;
    Matrix[0][1] = M+2;

    Matrix[1] = (int**) malloc(2*sizeof(int*));
    Matrix[1][0] = M + 2*2;
    Matrix[1][1] = M + 2*2 + 2;

    #pragma omp parallel
    {
        Matrix[0][0][0] = 42;
        Matrix[0][0][1] = 42;
        Matrix[0][1][0] = 42;
        Matrix[0][1][1] = 42;

        Matrix[1][0][0] = 46;
        Matrix[1][0][1] = 46;
        Matrix[1][1][0] = 46;
        Matrix[1][1][1] = 46;

        #pragma omp barrier

        printf("[\n[[%d,%d]\n[%d,%d]]\n[[%d,%d]\n[%d,%d]\n]\n", Matrix[0][0][0],Matrix[0][0][1],Matrix[0][1][0],Matrix[0][1][1],Matrix[1][0][0],Matrix[1][0][1],Matrix[1][1][0],Matrix[1][1][1]);
    }

    free(Matrix[0]);
    free(Matrix[1]);
    free(Matrix);
    free(M);
}
