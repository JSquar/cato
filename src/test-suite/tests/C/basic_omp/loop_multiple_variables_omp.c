// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int* a = (int*)malloc(sizeof(int)*4);
    int* b = (int*)malloc(sizeof(int)*4);

    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;

    #pragma omp parallel for
    {
        for(int i = 0; i < 4; i++)
        {
            b[i] = a[i];
        }
    }

    printf("[%d,%d,%d,%d]\n", b[0], b[1], b[2], b[3]);

    free(a);
    free(b);
}
