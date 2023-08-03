add_library(CatoRuntime MODULE
    rtlib.h
    rtlib.cpp
    MemoryAbstractionHandler.h
    MemoryAbstractionHandler.cpp
    MemoryAbstraction.h
    MemoryAbstraction.cpp
    MemoryAbstractionDefault.h
    MemoryAbstractionDefault.cpp
    MemoryAbstractionSingleValue.h
    MemoryAbstractionSingleValue.cpp
    MemoryAbstractionSingleValueDefault.h
    MemoryAbstractionSingleValueDefault.cpp
    mpi_mutex.h
    mpi_mutex.cpp
    CatoRuntimeLogger.h
    CatoRuntimeLogger.cpp
    Cache.h
    Cache.cpp
    Cacheline.h
    Cacheline.cpp
    Indexline.h
)

find_package(LLVM REQUIRED CONFIG)
find_package(MPI REQUIRED)
# find_package(NetCDF REQUIRED)
#find_file(CMAKE_PREFIX_PATH netCDFConfig.make)
pkg_search_module(NETCDF REQUIRED netcdf)
include_directories(${NETCDF_INCLUDE_DIRS})

include_directories(${MPI_INCLUDE_PATH})
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

target_link_libraries(CatoRuntime ${MPI_LIBRARIES})
target_link_libraries(CatoRuntime ${NETCDF_LINK_LIBRARIES})
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(CatoRuntime ${llvm_libs})

if(MPI_COMPILE_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

if(NETCDF_COMPILE_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    COMPILE_FLAGS "${NETCDF_COMPILE_FLAGS}")
endif()

if(NETCDF_LINK_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    LINK_FLAGS "${NETCDF_LINK_FLAGS}")
endif()

target_compile_features(CatoRuntime PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

set_target_properties(CatoRuntime PROPERTIES
	COMPILE_FLAGS "-O0 -fno-rtti -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable"
)
