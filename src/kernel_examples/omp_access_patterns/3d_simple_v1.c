#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int*** M = (int***) malloc(sizeof(int**)*2);
    M[0] = (int**)malloc(sizeof(int**)*2);
    M[1] = (int**)malloc(sizeof(int**)*2);
    M[0][0] = (int*)malloc(sizeof(int*)*2);
    M[0][1] = (int*)malloc(sizeof(int*)*2);
    M[1][0] = (int*)malloc(sizeof(int*)*2);
    M[1][1] = (int*)malloc(sizeof(int*)*2);

    #pragma omp parallel
    {
        M[0][0][0] = 42;
        M[0][0][1] = 42;
        M[0][1][0] = 42;
        M[0][1][1] = 42;

        M[1][0][0] = 46;
        M[1][0][1] = 46;
        M[1][1][0] = 46;
        M[1][1][1] = 46;

        #pragma omp barrier

        printf("[\n[[%d,%d]\n[%d,%d]]\n[[%d,%d]\n[%d,%d]\n]\n", M[0][0][0],M[0][0][1],M[0][1][0],M[0][1][1],M[1][0][0],M[1][0][1],M[1][1][0],M[1][1][1]);
    }


    free(M[0][0]);
    free(M[0][1]);
    free(M[1][0]);
    free(M[1][1]);
    free(M[0]);
    free(M[1]);
    free(M);
}
