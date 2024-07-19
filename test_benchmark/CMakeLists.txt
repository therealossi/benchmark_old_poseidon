cmake_minimum_required(VERSION 3.20)

# Set the project name and version
project(BufferManager VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for header files
include_directories(${CMAKE_SOURCE_DIR})

set (FETCHCONTENT_FULLY_DISCONNECTED "OFF")
include(FetchContent REQUIRED)

# Add main library files
add_library(buffermanager
    src/bufferpool.cpp
    src/paged_file.cpp
    src/lru_list.cpp
)

target_include_directories(buffermanager PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Optionally, automatically find all cpp files in the tests directory
file(GLOB TEST_SOURCES "test/*.cpp")

# Enable testing with CTest
enable_testing()

# Catch2 =================================================================== #
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)

FetchContent_MakeAvailable(Catch2)

# Create a test executable for each test source file
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} Catch2::Catch2WithMain buffermanager)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()


# Benchmarks
add_executable(bench bench/time.cpp bench/dbutils.cpp bench/benchmarksCreate.cpp)
target_link_libraries(bench buffermanager)

