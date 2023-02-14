// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t) %s.reference_output
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>

int main()
{
    int* data = malloc( sizeof(int) * 4);
    int** arr = malloc(sizeof(int*)*2);

    arr[0] = data;
    arr[1] = data + 2;

    #pragma omp parallel
    {
        arr[0][0] = 0;
        arr[0][1] = 1;
        arr[1][0] = 2;
        arr[1][1] = 3;

        printf("[%d, %d],\n[%d, %d]\n",arr[0][0], arr[0][1], arr[1][0], arr[1][1]);
    }

    free(data);
    free(arr);
}
