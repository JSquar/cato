#include<stdio.h>
#include<omp.h>

int main()
{
    int a = 0;

#pragma omp parallel for shared(a)
    for(int i = 0; i < 10; i++)
    {
        a += 1;
    }

    printf("%d\n", a);
}
