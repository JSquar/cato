// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: mpirun -np 3 %t_modified.x
// RUN: diff <(tail -n +2 <(ncdump int64s1D.nc)) <(tail -n +2 <(ncdump %s.reference_output.nc))

#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HANDLENETCDF(x)\
                do { int retval = (x);\
                if (retval) {\
                    fprintf(stderr, "Error: %s\n", nc_strerror(retval));exit(1);}\
                }while(0)\


int main()
{
        int filehandle;
        char* name = "int64s1D.nc";
        HANDLENETCDF(nc_create(name, NC_CLOBBER | NC_NETCDF4, &filehandle));

	int dimId;
	size_t len = 8;
	HANDLENETCDF(nc_def_dim(filehandle, "x", len, &dimId));

        int varId;
        HANDLENETCDF(nc_def_var(filehandle, "data", NC_INT64, 1, &dimId, &varId));
	HANDLENETCDF(nc_enddef(filehandle));

	long long* buf = malloc(sizeof(long long) * len);
	for (size_t i = 0; i < len; i++)
	{
		buf[i] = (long long) i;
	}

	HANDLENETCDF(nc_put_var_longlong(filehandle, varId, buf));

        HANDLENETCDF(nc_close(filehandle));
	free(buf);
        return 0;
}
