// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 1 %t_modified.x) %s.reference_output
#include <stdlib.h>
#include <stdio.h>

void use_pointer(int x, int * p)
{
    *p = x;
}

int main()
{
    int* ptr = (int*)malloc(sizeof(int));
    *ptr = 0;
    use_pointer(42, ptr);
    printf("p: %d\n", *ptr);
    free(ptr);
}
