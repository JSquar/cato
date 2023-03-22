#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>

#define HANDLENETCDF(x)\
                do { int retval = (x);\
                if (retval) {\
                    fprintf(stderr, "Error: %s\n", nc_strerror(retval));exit(1);}\
                }while(0)\


int main()
{
	int filehandle;
	char* name = "ints1D.nc";
	HANDLENETCDF(nc_open(name, NC_NOWRITE, &filehandle));

	int varId;
	HANDLENETCDF(nc_inq_varid(filehandle, "data", &varId));

	nc_type type;
	HANDLENETCDF(nc_inq_vartype(filehandle, varId, &type));

	if(type != NC_INT)
	{
		fprintf(stderr, "Vartype doesn't match\n");
		return 1;
	}

	int numDims;
	HANDLENETCDF(nc_inq_varndims(filehandle, varId, &numDims));

	int dimIds[numDims];
	HANDLENETCDF(nc_inq_vardimid(filehandle, varId, dimIds));

	size_t dims[numDims];
	size_t bufSize = 1;
	for(int i = 0; i < numDims; i++)
	{
		HANDLENETCDF(nc_inq_dimlen(filehandle, dimIds[i], &dims[i]));
		bufSize *= dims[i];
	}

	int* buf = malloc(bufSize * sizeof(int));
	HANDLENETCDF(nc_get_var_int(filehandle, varId, buf));

	for(size_t i = 0; i < bufSize; i++)
	printf("%d ", buf[i]);
	printf("\n");

	HANDLENETCDF(nc_close(filehandle));
	free(buf);
	return 0;
}
