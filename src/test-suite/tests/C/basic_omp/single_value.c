// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t) %s.reference_output
#include <stdio.h>
#include <omp.h>

int main()
{
    int x = 0;

    #pragma omp parallel shared(x)
    {
        x = 42;
    }
    printf("X: %d\n", x);
}
