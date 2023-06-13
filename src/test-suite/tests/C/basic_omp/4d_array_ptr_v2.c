// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t_modified.x) %s.reference_output
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int* matrix = malloc(sizeof(int)*2*2*2*2);
    int**** M = (int****) malloc(sizeof(int***)*2);

    M[0] = (int***)malloc(sizeof(int**)*2);
    M[1] = (int***)malloc(sizeof(int**)*2);

    M[0][0] = (int**)malloc(sizeof(int*)*2);
    M[0][1] = (int**)malloc(sizeof(int*)*2);
    M[1][0] = (int**)malloc(sizeof(int*)*2);
    M[1][1] = (int**)malloc(sizeof(int*)*2);

    M[0][0][0] = matrix;
    M[0][0][1] = matrix + 2;

    M[0][1][0] = matrix + 2*2;
    M[0][1][1] = matrix + 2*2 + 2;

    M[1][0][0] = matrix + 2*2*2;
    M[1][0][1] = matrix + 2*2*2 + 2;

    M[1][1][0] = matrix + 2*2*2 + 2*2;
    M[1][1][1] = matrix + 2*2*2 + 2*2 + 2;

    #pragma omp parallel
    {
        M[0][0][0][0] = 5;
        M[0][0][0][1] = 5;
        M[0][0][1][0] = 5;
        M[0][0][1][1] = 5;
        M[0][1][0][0] = 6;
        M[0][1][0][1] = 6;
        M[0][1][1][0] = 6;
        M[0][1][1][1] = 6;

        M[1][0][0][0] = 7;
        M[1][0][0][1] = 7;
        M[1][0][1][0] = 7;
        M[1][0][1][1] = 7;
        M[1][1][0][0] = 8;
        M[1][1][0][1] = 8;
        M[1][1][1][0] = 8;
        M[1][1][1][1] = 8;

        #pragma omp barrier

        printf("[\n [\n  [[%d,%d]\n   [%d,%d]]\n  [[%d,%d]\n   [%d,%d]]\n ]\n [\n  [[%d,%d]\n   [%d,%d]]\n  [[%d,%d]\n   [%d,%d]]\n ]\n]\n",
        M[0][0][0][0],M[0][0][0][1],M[0][0][1][0],M[0][0][1][1],M[0][1][0][0],M[0][1][0][1],M[0][1][1][0],M[0][1][1][1],
        M[1][0][0][0],M[1][0][0][1],M[1][0][1][0],M[1][0][1][1],M[1][1][0][0],M[1][1][0][1],M[1][1][1][0],M[1][1][1][1]);
    }


    free(M[0][0]);
    free(M[0][1]);
    free(M[1][0]);
    free(M[1][1]);
    free(M[0]);
    free(M[1]);
    free(M);
}
