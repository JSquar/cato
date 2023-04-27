project(Cato CXX)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# target_include_directories(Cato PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/cato)

# Add tests
enable_testing()

# Add Google Test as a subdirectory
add_subdirectory(regression_tests)

# Add CATO sources
add_subdirectory(src)