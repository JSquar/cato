// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t_modified.x) %s.reference_output
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#define DIM 10
#define DENSITY 0.2

int main()
{
    //No access to realloc, so just allocate some more space than necessary
    size_t buff_size = (size_t)(DIM*DIM*DENSITY*2);

    double* data = malloc(sizeof(double) * buff_size);
    size_t* indices = malloc(sizeof(size_t) * buff_size);
    size_t* ptr = malloc(sizeof(size_t) * (DIM + 1));

    double* result = malloc(sizeof(double) * DIM);
    double* vec = malloc(sizeof(double) * DIM);

    srand48(42);

    size_t current_index = 0;
    for (size_t i = 0; i < DIM; i++)
    {
        vec[i] = drand48();
        ptr[i] = current_index;
        for (size_t j = 0; j < DIM; j++)
        {
            if (drand48() < DENSITY)
            {
                data[current_index] = drand48();
                indices[current_index] = j;
                current_index++;
            }
        }
    }
    ptr[DIM] = current_index;

    #pragma omp parallel for default(none) shared(result, ptr, data, indices, vec)
    for (size_t i = 0; i < DIM; i++)
    {
        result[i] = 0.0;
        for (size_t k = ptr[i]; k < ptr[i+1]; k++)
        {
            result[i] = result[i] + data[k] * vec[indices[k]];
        }
    }

    #pragma omp parallel
    {
        if (omp_get_thread_num() == 0)
        {
            for (size_t i = 0; i < DIM; i++)
            printf("%lf\n", result[i]);
        }
    }

    free(data);
    free(indices);
    free(ptr);
    free(vec);
    free(result);

    return 0;
}
