// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t_modified.x) %s.reference_output
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define DIM 10
#define ITER 10

int main()
{
    float** Matrix = malloc(sizeof(float*) * DIM);
    float** Matrix2 = malloc(sizeof(float*) * DIM);

    for(int i = 0; i < DIM; i++)
    {
        Matrix[i] = malloc(sizeof(float) * DIM);
        Matrix2[i] = malloc(sizeof(float) * DIM);
    }

    for(int i = 0; i < DIM; i++)
    {
        for (int j = 0; j < DIM; j++)
        {
            Matrix[i][j] = 0.0;
            Matrix2[i][j] = 0.0;
        }
    }

    for (int i = 0; i < DIM; i++)
    {
        Matrix[0][i] = 1.0;
        Matrix[DIM-1][i] = 1.0;
        Matrix[i][0] = 1.0;
        Matrix[i][DIM-1] = 1.0;
        Matrix2[0][i] = 1.0;
        Matrix2[DIM-1][i] = 1.0;
        Matrix2[i][0] = 1.0;
        Matrix2[i][DIM-1] = 1.0;
    }


    for(int iter = 0; iter < ITER; iter++)
    {
        #pragma omp parallel for
        for(int i = 1; i < DIM-1; i++)
        {
            for(int j = 1; j < DIM-1; j++)
            {
                float star = 0.25*(Matrix[i-1][j] + Matrix[i+1][j] + Matrix[i][j-1] + Matrix[i][j+1]);
                Matrix2[i][j] = Matrix[i][j]/2.0 + star/2.0;
            }
        }

        float** tmp = Matrix;
        Matrix = Matrix2;
        Matrix2 = tmp;
    }


    #pragma omp parallel
    {
        if(omp_get_thread_num() == 0)
        {
            for(int i = 0; i < DIM; i++)
            {
                for(int j = 0; j < DIM; j++)
                {
                    printf("%.4f ", Matrix[i][j]);
                }
                printf("\n");
            }
        }
    }

    for(int i = 0; i < DIM; i++)
    {
        free(Matrix[i]);
        free(Matrix2[i]);
    }
    free(Matrix);
    free(Matrix2);

    return 0;
}
