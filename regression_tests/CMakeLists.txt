# Add your project test files
add_executable(cato_regression_tests.x 
main.cpp 
environment_interaction_test.cpp
../src/cato/environment_interaction.cpp
)

# Add your project include directories
target_include_directories(cato_regression_tests.x PRIVATE ../src/cato)

# target_sources(cato_regression_tests.x PRIVATE ../src/cato)

# Link your project test against Google Test
target_link_libraries(cato_regression_tests.x PRIVATE gtest)

add_test(NAME CatoTest COMMAND cato_regression_tests.x)
