#add_subdirectory(pragmaHandler)
add_subdirectory(rtlib)

add_library(CatoPass MODULE
	#List your source files here.
    cato.cpp
    debug.h
    helper.h
    helper.cpp
    RuntimeHandler.h
    RuntimeHandler.cpp
    Microtask.h
    Microtask.cpp
    SharedVariable.h
    SharedVariable.cpp
    SharedValue.h
    SharedValue.cpp
    SharedPointer.h
    SharedPointer.cpp
    MemoryAllocation.h
    MemoryAllocation.cpp
    UserTree.h
    UserTree.cpp
)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

target_link_libraries(CatoPass ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(CatoPass PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(CatoPass PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()


#target_compile_features(omp2mpiPass PRIVATE cxx_range_for cxx_auto_type)
#need c++ 17 now
target_compile_features(CatoPass PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(CatoPass PROPERTIES
	COMPILE_FLAGS "-O3 -fno-rtti -Wall -Wextra -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable"
)
#TODO: -Wno-deprecated-declarations wieder entfernen

# COMPILE_FLAGS "-O3 -fno-rtti "
