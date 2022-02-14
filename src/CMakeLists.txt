# IF(CMAKE_BUILD_TYPE MATCHES Debug)
#     message("debug mode")
# ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


cmake_minimum_required(VERSION 3.10)

project(Cato)

# add LLVM dependencies
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# add MPI dependencies
find_package(MPI REQUIRED)

add_subdirectory(cato)
