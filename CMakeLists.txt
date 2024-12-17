cmake_minimum_required(VERSION 3.14)

# Set the project name and version
project(HeatEquationSolver VERSION 1.0 LANGUAGES C CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/headers)

# Add library from src files
add_library(HeatSolverLib src/cheat.c src/heat_solver.c src/ncurses_elements.c)
target_include_directories(HeatSolverLib PUBLIC ${PROJECT_SOURCE_DIR}/headers)

# Add executable for samples
add_executable(HeatSolverSample samples/main.c)
target_link_libraries(HeatSolverSample PRIVATE HeatSolverLib m ncursesw)

# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add test executable
file(GLOB TEST_FILES tests/*.cpp)
add_executable(HeatSolverTests ${TEST_FILES})
target_link_libraries(HeatSolverTests PRIVATE HeatSolverLib gtest_main m ncursesw)

include(GoogleTest)
gtest_discover_tests(HeatSolverTests)

# Copy show.py to the build directory
add_custom_command(
    TARGET HeatSolverSample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/show.py ${CMAKE_BINARY_DIR}/show.py
)

# Create Python virtual environment and install required packages
add_custom_target(setup_venv
    COMMAND python3 -m venv ${CMAKE_BINARY_DIR}/venv
    COMMAND ${CMAKE_BINARY_DIR}/venv/bin/pip install --upgrade pip
    COMMAND ${CMAKE_BINARY_DIR}/venv/bin/pip install matplotlib pandas openpyxl numpy
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Setting up Python virtual environment"
)

# Ensure the virtual environment is set up before building the sample
add_dependencies(HeatSolverSample setup_venv)