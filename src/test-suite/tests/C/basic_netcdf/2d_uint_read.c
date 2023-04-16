// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 2 %t_modified.x) %s.reference_output

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
        char* name = "uints2D.nc";
        HANDLENETCDF(nc_open(name, NC_NOWRITE, &filehandle));

        int varId;
        HANDLENETCDF(nc_inq_varid(filehandle, "data", &varId));

        nc_type type;
        HANDLENETCDF(nc_inq_vartype(filehandle, varId, &type));

        if(type != NC_UINT)
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

        unsigned int* buf = malloc(bufSize * sizeof(unsigned int));
        HANDLENETCDF(nc_get_var_uint(filehandle, varId, buf));

        printf("%u %u %u \n%u %u %u \n%u %u %u \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);

        HANDLENETCDF(nc_close(filehandle));
        free(buf);
        return 0;
}
