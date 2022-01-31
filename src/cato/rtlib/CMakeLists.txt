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
)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

target_link_libraries(CatoRuntime ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(CatoRuntime PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

target_compile_features(CatoRuntime PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

set_target_properties(CatoRuntime PROPERTIES
	COMPILE_FLAGS "-O3 -fno-rtti -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable"
)

