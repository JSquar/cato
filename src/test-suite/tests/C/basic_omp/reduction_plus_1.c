// PASS: *
// RUN: ${DAS_TOOL_ROOT}/src/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main()
{
    int result = 0;

    #pragma omp parallel reduction(+:result)
    {
        int rank = omp_get_thread_num();
        result += rank;
    }

    printf("Result: %d\n", result);
}
