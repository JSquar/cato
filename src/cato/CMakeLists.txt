#add_subdirectory(pragmaHandler)
add_subdirectory(rtlib)

add_library(CatoPass MODULE
	#List your source files here.
    cato.cpp
    debug.cpp    
    debug.h    
    helper.cpp
    helper.h
    NetCDFRegion.cpp
    NetCDFRegion.h
    MemoryAllocation.cpp
    MemoryAllocation.h
    Microtask.cpp
    Microtask.h
    RuntimeHandler.cpp
    RuntimeHandler.h
    SharedPointer.cpp
    SharedPointer.h
    SharedValue.cpp
    SharedValue.h
    SharedVariable.cpp
    SharedVariable.h
    UserTree.cpp
    UserTree.h
)

target_include_directories(CatoPass PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_compile_options(CatoPass PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(CatoPass PRIVATE ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})

#target_compile_features(omp2mpiPass PRIVATE cxx_range_for cxx_auto_type)
#need c++ 17 now
target_compile_features(CatoPass PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
#IF(CMAKE_BUILD_TYPE MATCHES Debug)
#    message("debug mode")
#ELSE
#ENDIF()

set_target_properties(CatoPass PROPERTIES
	COMPILE_FLAGS "-O3 -fno-rtti -Wall -Wextra -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable"
)
#TODO: -Wno-deprecated-declarations wieder entfernen

# COMPILE_FLAGS "-O3 -fno-rtti "
