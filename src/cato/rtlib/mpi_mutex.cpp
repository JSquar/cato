// source from https://gist.github.com/aprell/1486197

#include <assert.h>
#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_mutex.h"

// do not suggest to inline any mutex functions

int MPI_Mutex_init(MPI_Mutex **mutex, int home)
{
    static int tag = MPI_MUTEX_MSG_TAG_BASE;
    int numprocs, ID;
    MPI_Mutex *mtx;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &ID);

    mtx = (MPI_Mutex *)malloc(sizeof(MPI_Mutex));
    if (!mtx)
    {
        std::cerr << "Warning: malloc failed on worker " << ID << "\n";
        return 1;
    }

    mtx->numprocs = numprocs;
    mtx->ID = ID;
    mtx->home = home;
    mtx->tag = tag++;
    mtx->comm = MPI_COMM_WORLD;

    if (ID == mtx->home)
    {
        // Allocate and expose waitlist
        MPI_Alloc_mem(mtx->numprocs, MPI_INFO_NULL, &mtx->waitlist);
        if (!mtx->waitlist)
        {
            std::cerr << "Warning: MPI_Alloc_mem failed on worker " << ID << "\n";
            return 1;
        }
        memset(mtx->waitlist, 0, mtx->numprocs);
        MPI_Win_create(mtx->waitlist, mtx->numprocs, 1, MPI_INFO_NULL, mtx->comm, &mtx->win);
    }
    else
    {
        // Don't expose anything
        mtx->waitlist = NULL;
        MPI_Win_create(mtx->waitlist, 0, 1, MPI_INFO_NULL, mtx->comm, &mtx->win);
    }

    *mutex = mtx;

    MPI_Barrier(MPI_COMM_WORLD);

    return 0;
}

int MPI_Mutex_destroy(MPI_Mutex *mutex)
{
    assert(mutex != NULL);

    int ID;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Comm_rank(MPI_COMM_WORLD, &ID);

    if (ID == mutex->home)
    {
        // Free waitlist
        MPI_Win_free(&mutex->win);
        MPI_Free_mem(mutex->waitlist);
    }
    else
    {
        MPI_Win_free(&mutex->win);
        assert(mutex->waitlist == NULL);
    }

    return 0;
}

int MPI_Mutex_lock(MPI_Mutex *mutex)
{
    assert(mutex != NULL);

    char waitlist[mutex->numprocs];
    char lock = 1;
    int i;

    // Try to acquire lock in one access epoch
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, mutex->home, 0, mutex->win);
    MPI_Put(&lock, 1, MPI_CHAR, mutex->home, mutex->ID /* &win[mutex->ID] */, 1, MPI_CHAR,
            mutex->win);
    MPI_Get(waitlist, mutex->numprocs, MPI_CHAR, mutex->home, 0, mutex->numprocs, MPI_CHAR,
            mutex->win);
    MPI_Win_unlock(mutex->home, mutex->win);

    assert(waitlist[mutex->ID] == 1);

    // Count the 1's
    for (i = 0; i < mutex->numprocs; i++)
    {
        if (waitlist[i] == 1 && i != mutex->ID)
        {
            // We have to wait for the lock
            // Dummy receive, no payload
            // std::cout << "Worker " << mutex->ID << " waits for lock\n";
            MPI_Recv(&lock, 0, MPI_CHAR, MPI_ANY_SOURCE, mutex->tag, mutex->comm,
                     MPI_STATUS_IGNORE);
            break;
        }
    }

    // std::cout << "Worker " << mutex->ID << " has the lock\n";

    return 0;
}

int MPI_Mutex_trylock(MPI_Mutex *mutex)
{
    assert(mutex != NULL);

    char waitlist[mutex->numprocs];
    char lock = 1;
    int i;

    // Try to acquire lock in one access epoch
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, mutex->home, 0, mutex->win);
    MPI_Put(&lock, 1, MPI_CHAR, mutex->home, mutex->ID /* &win[mutex->ID] */, 1, MPI_CHAR,
            mutex->win);
    MPI_Get(waitlist, mutex->numprocs, MPI_CHAR, mutex->home, 0, mutex->numprocs, MPI_CHAR,
            mutex->win);
    MPI_Win_unlock(mutex->home, mutex->win);

    assert(waitlist[mutex->ID] == 1);

    // Count the 1's
    for (i = 0; i < mutex->numprocs; i++)
    {
        if (waitlist[i] == 1 && i != mutex->ID)
        {
            // Lock is already held, return immediately
            return 1;
        }
    }

    // std::cout << "Worker " << mutex->ID << " has the lock\n";

    return 0;
}

int MPI_Mutex_unlock(MPI_Mutex *mutex)
{
    assert(mutex != NULL);

    char waitlist[mutex->numprocs];
    char lock = 0;
    int i, next;

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, mutex->home, 0, mutex->win);
    MPI_Put(&lock, 1, MPI_CHAR, mutex->home, mutex->ID /* &win[mutex->ID] */, 1, MPI_CHAR,
            mutex->win);
    MPI_Get(waitlist, mutex->numprocs, MPI_CHAR, mutex->home, 0, mutex->numprocs, MPI_CHAR,
            mutex->win);
    MPI_Win_unlock(mutex->home, mutex->win);

    assert(waitlist[mutex->ID] == 0);

    // If there are other processes waiting for the lock, transfer ownership
    next = (mutex->ID + 1 + mutex->numprocs) % mutex->numprocs;
    for (i = 0; i < mutex->numprocs; i++, next = (next + 1) % mutex->numprocs)
    {
        if (waitlist[next] == 1)
        {
            // Dummy send, no payload
            // std::cout << "Worker " << mutex->ID << " transfers lock ownership to worker " <<
            // mutex->ID << "\n";
            MPI_Send(&lock, 0, MPI_CHAR, next, mutex->tag, mutex->comm);
            break;
        }
    }

    return 0;
}
