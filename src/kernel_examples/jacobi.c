#include <stdio.h>
#include <stdlib.h>
#include <omp.h>



int main()
{
    int INTERLINES = 50;
    int TERM_ITERATION = 3;
    /* double ***Matrix = NULL; */
    /* double *M = NULL; */

    int N = (INTERLINES * 8) + 9 - 1;
    double h = 1.0/N;

    int stat_iteration = 0;
    double stat_precision = 0.0;

    double *M = (double*)malloc(2 * (N+1) * (N+1) * sizeof(double));
    double ***Matrix = (double***)malloc(2 * sizeof(double**));

    for(int i = 0; i < 2; i++)
    {
        Matrix[i] = (double**)malloc((N+1) * sizeof(double*));

        for(int j = 0; j <= N; j++)
        {
            Matrix[i][j] = M + (i * (N+1) * (N+1)) + (j * (N+1));
        }
    }

	/* initialize matrix/matrices with zeros */
	for (int g = 0; g < 2; g++)
	{
		for (int i = 0; i <= N; i++)
		{
			for (int j = 0; j <= N; j++)
			{
				Matrix[g][i][j] = 0.0;
			}
		}
	}

	/* initialize borders */
    for (int g = 0; g < 2; g++)
    {
        for (int i = 0; i <= N; i++)
        {
            Matrix[g][i][0] = 1.0 - (h * i);
            Matrix[g][i][N] = h * i;
            Matrix[g][0][i] = 1.0 - (h * i);
            Matrix[g][N][i] = h * i;
        }

        Matrix[g][N][0] = 0.0;
        Matrix[g][0][N] = 0.0;
    }

    //calculate
    
    int m1, m2;
    double star, residuum, maxresiduum, pih, fpisin;

    int term_iteration = TERM_ITERATION;

    m1 = 0;
    m2 = 1;

    while (term_iteration > 0)
    {
        printf("%d\n", term_iteration);
        maxresiduum = 0.0;
#pragma omp parallel for private(residuum, star) reduction(max:maxresiduum)
        {
            for(int j = 1; j < N; j++)
            {
                for(int i = 1; i < N; i++)
                {
                    star = 0.25 * (Matrix[m2][i-1][j] + Matrix[m2][i][j-1] + Matrix[m2][i][j+1] 
                            + Matrix[m2][i+1][j]);

                    if(term_iteration == 1)
                    {
                        residuum = Matrix[m2][i][j] - star;
                        residuum = (residuum < 0) ? -residuum : residuum;
                        maxresiduum = (residuum < maxresiduum) ? maxresiduum : residuum;
                    }

                    Matrix[m1][i][j] = star;
                }
            }
        }
        stat_iteration++;
        stat_precision = maxresiduum;

    
        int a = m1;
        m1 = m2;
        m2 = a;
        
        term_iteration--;
    }


    // Output
	int x, y;


#pragma omp parallel
    {
        if(omp_get_thread_num() == 0)
        {
            printf("Maxresiduum: %7.4f\n", maxresiduum);
            printf("Matrix:\n");
            for (y = 0; y < 9; y++)
            {
                for (x = 0; x < 9; x++)
                {
                    printf ("%7.4f", Matrix[m2][y * (INTERLINES + 1)][x * (INTERLINES + 1)]);
                }

                printf ("\n");
            }
            fflush (stdout);
        }
    }

    for(int i = 0; i < 2; i++)
    {
        free(Matrix[i]);
    }
    free(Matrix);
    free(M);
}
