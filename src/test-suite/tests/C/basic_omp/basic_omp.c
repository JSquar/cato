// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdio.h>
#include <omp.h>

int main()
{
    int* arr = (int*) malloc(sizeof(int) * 4);

    arr[0] = 42;
    arr[1] = 42;
    arr[2] = 42;
    arr[3] = 42;

#pragma omp parallel shared(arr)
    {
        int rank = omp_get_thread_num();

        arr[rank] = rank;
    }
    printf("AFTER: [%d, %d, %d, %d]\n", arr[0], arr[1], arr[2], arr[3]);
}
