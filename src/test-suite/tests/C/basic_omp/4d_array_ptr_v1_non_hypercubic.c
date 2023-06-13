// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 3 %t_modified.x) %s.reference_output
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int**** M = (int****) malloc(sizeof(int***)*2);
    M[0] = (int***)malloc(sizeof(int**)*3);
    M[1] = (int***)malloc(sizeof(int**)*3);
    M[0][0] = (int**)malloc(sizeof(int*)*2);
    M[0][1] = (int**)malloc(sizeof(int*)*2);
    M[0][2] = (int**)malloc(sizeof(int*)*2);
    M[1][0] = (int**)malloc(sizeof(int*)*2);
    M[1][1] = (int**)malloc(sizeof(int*)*2);
    M[1][2] = (int**)malloc(sizeof(int*)*2);
    M[0][0][0] = (int*)malloc(sizeof(int)*3);
    M[0][0][1] = (int*)malloc(sizeof(int)*3);
    M[0][1][0] = (int*)malloc(sizeof(int)*3);
    M[0][1][1] = (int*)malloc(sizeof(int)*3);
    M[0][2][0] = (int*)malloc(sizeof(int)*3);
    M[0][2][1] = (int*)malloc(sizeof(int)*3);
    M[1][0][0] = (int*)malloc(sizeof(int)*3);
    M[1][0][1] = (int*)malloc(sizeof(int)*3);
    M[1][1][0] = (int*)malloc(sizeof(int)*3);
    M[1][1][1] = (int*)malloc(sizeof(int)*3);
    M[1][2][0] = (int*)malloc(sizeof(int)*3);
    M[1][2][1] = (int*)malloc(sizeof(int)*3);

    #pragma omp parallel
    {
        M[0][0][0][0] = 4;
        M[0][0][0][1] = 4;
        M[0][0][0][2] = 4;
        M[0][0][1][0] = 4;
        M[0][0][1][1] = 4;
        M[0][0][1][2] = 4;
        M[0][1][0][0] = 5;
        M[0][1][0][1] = 5;
        M[0][1][0][2] = 5;
        M[0][1][1][0] = 5;
        M[0][1][1][1] = 5;
        M[0][1][1][2] = 5;
        M[0][2][0][0] = 6;
        M[0][2][0][1] = 6;
        M[0][2][0][2] = 6;
        M[0][2][1][0] = 6;
        M[0][2][1][1] = 6;
        M[0][2][1][2] = 6;

        M[1][0][0][0] = 7;
        M[1][0][0][1] = 7;
        M[1][0][0][2] = 7;
        M[1][0][1][0] = 7;
        M[1][0][1][1] = 7;
        M[1][0][1][2] = 7;
        M[1][1][0][0] = 8;
        M[1][1][0][1] = 8;
        M[1][1][0][2] = 8;
        M[1][1][1][0] = 8;
        M[1][1][1][1] = 8;
        M[1][1][1][2] = 8;
        M[1][2][0][0] = 9;
        M[1][2][0][1] = 9;
        M[1][2][0][2] = 9;
        M[1][2][1][0] = 9;
        M[1][2][1][1] = 9;
        M[1][2][1][2] = 9;

        #pragma omp barrier

        printf("[\n [\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n ]\n"
        " [\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n  [[%d,%d,%d]\n   [%d,%d,%d]]\n ]\n]\n",
        M[0][0][0][0],M[0][0][0][1],M[0][0][0][2],M[0][0][1][0],M[0][0][1][1],M[0][0][1][2],
        M[0][1][0][0],M[0][1][0][1],M[0][1][0][2],M[0][1][1][0],M[0][1][1][1],M[0][1][1][2],
        M[0][2][0][0],M[0][2][0][1],M[0][2][0][2],M[0][2][1][0],M[0][2][1][1],M[0][2][1][2],
        M[1][0][0][0],M[1][0][0][1],M[1][0][0][2],M[1][0][1][0],M[1][0][1][1],M[1][0][1][2],
        M[1][1][0][0],M[1][1][0][1],M[1][1][0][2],M[1][1][1][0],M[1][1][1][1],M[1][1][1][2],
        M[1][2][0][0],M[1][2][0][1],M[1][2][0][2],M[1][2][1][0],M[1][2][1][1],M[1][2][1][2]);
    }


    free(M[0][0][0]);
    free(M[0][0][1]);
    free(M[0][1][0]);
    free(M[0][1][1]);
    free(M[0][2][0]);
    free(M[0][2][1]);
    free(M[1][0][0]);
    free(M[1][0][1]);
    free(M[1][1][0]);
    free(M[1][1][1]);
    free(M[1][2][0]);
    free(M[1][2][1]);
    free(M[0][0]);
    free(M[0][1]);
    free(M[0][2]);
    free(M[1][0]);
    free(M[1][1]);
    free(M[1][2]);
    free(M[0]);
    free(M[1]);
    free(M);
}
