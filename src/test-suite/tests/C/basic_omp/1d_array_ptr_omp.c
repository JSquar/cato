// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t_modified.x) %s.reference_output
#include <omp.h>
#include <stdlib.h>

int main()
{
    int *arr = malloc(sizeof(int) * 4);

#pragma omp parallel
    {
        arr[0] = 0;
        arr[1] = 1;
        arr[2] = 2;
        arr[3] = 3;
        printf("[%d, %d, %d, %d]\n", arr[0], arr[1], arr[2], arr[3]);
    }

    free(arr);
}
