#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int MPI_RANK, MPI_SIZE;

void modifiy_bounds(int *lower_bound, int *upper_bound, int increment)
{
    if(MPI_RANK == 0)
    {
        printf("Global lb: %d, ub: %d, inc: %d\n", *lower_bound, *upper_bound, increment);
    }

    if (increment != 1)
    {
        printf("Warning: Currently only loop increments of value 1 are supported\n");
    }

    int local_lbound, local_ubound;

    int total_iterations = *upper_bound - *lower_bound + 1;
    int div = total_iterations / MPI_SIZE;
    int rest = total_iterations % MPI_SIZE;

    if (MPI_RANK < rest)
    {
        local_lbound = *lower_bound + MPI_RANK * (div +1);
        local_ubound = local_lbound + div;
    }
    else
    {
        local_lbound = *lower_bound + MPI_RANK * div + rest;
        local_ubound = local_lbound + div-1;
    }
    printf("Local lower bound: %d\nLocal upper bound: %d\nIncrement: %d\n", local_lbound,
                 local_ubound, increment);

    *lower_bound = local_lbound;
    *upper_bound = local_ubound;

}

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_RANK);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_SIZE);

    int lb, ub, inc;
    sscanf(argv[1], "%d", &lb);
    sscanf(argv[2], "%d", &ub);
    sscanf(argv[3], "%d", &inc);

    modifiy_bounds(&lb, &ub, inc);

    MPI_Finalize();
    
}
