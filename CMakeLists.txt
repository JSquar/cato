project(Cato CXX)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# target_include_directories(Cato PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/cato)

# Add tests
enable_testing()

# Add test subdirectory if GoogleTest is available
find_package(GTest)
if(GTest_FOUND)
    message("GoogleTest found, building tests.")
    add_subdirectory(regression_tests)
else()
    message("GoogleTest not found, skipping tests.")
endif()


# Add CATO sources
add_subdirectory(src)
