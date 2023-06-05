// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: mpirun -np 4 %t_modified.x
// RUN: diff shorts2D.nc %s.reference_output.nc

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
        char* name = "shorts2D.nc";
        HANDLENETCDF(nc_create(name, NC_CLOBBER | NC_NETCDF4, &filehandle));

	int dimIds[2];
	size_t xLen = 8;
	size_t yLen = 10;
	size_t numElements = xLen * yLen;
	HANDLENETCDF(nc_def_dim(filehandle, "x", xLen, &dimIds[0]));
	HANDLENETCDF(nc_def_dim(filehandle, "y", yLen, &dimIds[1]));

        int varId;
        HANDLENETCDF(nc_def_var(filehandle, "data", NC_SHORT, 2, dimIds, &varId));
	HANDLENETCDF(nc_enddef(filehandle));

	short* buf = malloc(sizeof(short) * numElements);
	for (size_t i = 0; i < numElements; i++)
	{
		buf[i] = (short) i;
	}

	HANDLENETCDF(nc_put_var_short(filehandle, varId, buf));

        HANDLENETCDF(nc_close(filehandle));
	free(buf);
        return 0;
}
