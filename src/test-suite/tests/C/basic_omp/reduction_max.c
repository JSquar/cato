// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t_modified.x) %s.reference_output
#include <stdio.h>
#include <omp.h>

int main()
{
    int result = 0;

    #pragma omp parallel reduction(max:result)
    {
        result = omp_get_thread_num();
    }

    printf("Result: %d\n", result);
}
