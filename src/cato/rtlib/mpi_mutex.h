// source from https://gist.github.com/aprell/1486197
#ifndef MPI_MUTEX_H_
#define MPI_MUTEX_H_

#include <mpi.h>

#define MPI_MUTEX_MSG_TAG_BASE 1023

struct MPI_Mutex
{
    int numprocs, ID, home, tag;
    MPI_Comm comm;
    MPI_Win win;
    char *waitlist;
};

int MPI_Mutex_init(MPI_Mutex **mutex, int home);
int MPI_Mutex_destroy(MPI_Mutex *mutex);
int MPI_Mutex_lock(MPI_Mutex *mutex);
int MPI_Mutex_trylock(MPI_Mutex *mutex);
int MPI_Mutex_unlock(MPI_Mutex *mutex);

#endif /* MPI_MUTEX_H_ */
