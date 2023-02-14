// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main()
{

#pragma omp parallel
    {
        int thread = omp_get_thread_num();

        if (thread == 0)
        {
            sleep(3);
        }
        if (thread == 1)
        {
            sleep(2);
        }
        if (thread == 2)
        {
            sleep(1);
        }
        if (thread == 3)
        {
            sleep(0);
        }
        // ordering: 3,2,1,0
        printf("Thread %i: before Barrier\n", thread);
        #pragma omp barrier
        // ordering 0,1,2,3
        for(int i = 0; i < omp_get_num_threads(); i++)
        {
            if(i == thread)
            {
                printf("Thread %i: after Barrier\n", thread);
            }
            #pragma omp barrier
        }
    }
}
