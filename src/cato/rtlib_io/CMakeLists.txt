add_library(CatoRuntime_IO MODULE
    rtlib_io.h
    rtlib_io.cpp
    netCDFHandler.h
    netCDFHandler.cpp
    NetCDFRegion.h
    NetCDFRegion.cpp
    ../rtlib/rtlib.h
    ../rtlib/rtlib.cpp
)

# find_package(NetCDF REQUIRED)
find_file(CMAKE_PREFIX_PATH netCDFConfig.make)

# include_directories(${MPI_INCLUDE_PATH})

target_link_libraries(CatoRuntime_IO ${NETCDF_LIBRARIES})

if(NETCDF_COMPILE_FLAGS)
  set_target_properties(CatoRuntime_IO PROPERTIES
    COMPILE_FLAGS "${NETCDF_COMPILE_FLAGS}")
endif()

if(NETCDF_LINK_FLAGS)
  set_target_properties(CatoRuntime_IO PROPERTIES
    LINK_FLAGS "${NETCDF_LINK_FLAGS}")
endif()

target_compile_features(CatoRuntime_IO PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

set_target_properties(CatoRuntime_IO PROPERTIES
	COMPILE_FLAGS "-O3 -fno-rtti -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable"
)

