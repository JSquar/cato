// PASS: *
// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    int i;
    #pragma omp parallel shared(i)
    {
        int thread = omp_get_thread_num();

        if (thread == 0)
        {
            sleep(0);
        }
        if (thread == 1)
        {
            sleep(1);
        }
        if (thread == 2)
        {
            sleep(4);
        }
        if (thread == 3)
        {
            sleep(5);
        }
        // ordering: 0,1,   2,3
        printf("Thread %i before critical\n", thread);
        #pragma omp critical
        {
            if (thread == 0)
            {
                sleep(2);
            }
            if (thread == 1)
            {
                usleep(10);
            }
            if (thread == 2)
            {
                sleep(2);
            }
            if (thread == 3)
            {
                usleep(10);
            }
        }
        // ordering: 0,1, 2,3
        // if critical does not work 1 will "overtake" 0 same for 3 and 2
        printf("Thread %i after critical\n", thread);
    }
}
