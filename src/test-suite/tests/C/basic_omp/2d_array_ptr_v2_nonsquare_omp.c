// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t_modified.x) %s.reference_output
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    int *data = malloc(sizeof(int) * 6);
    int **arr = malloc(sizeof(int *) * 3);

    arr[0] = data;
    arr[1] = data + 2;
    arr[2] = data + 4;

#pragma omp parallel
    {
        arr[0][0] = 0;
        arr[0][1] = 1;
        arr[1][0] = 2;
        arr[1][1] = 3;
        arr[2][0] = 4;
        arr[2][1] = 5;

        printf("[%d, %d]\n[%d, %d]\n[%d, %d]\n", arr[0][0], arr[0][1], arr[1][0], arr[1][1], arr[2][0], arr[2][1]);
    }

    free(data);
    free(arr);
}