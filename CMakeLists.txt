cmake_minimum_required(VERSION 3.16)
project(ncbi-vdb3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
set(GTEST_LIBRARIES gtest )

find_package(Doxygen)
# set DOXYGEN_* variables here
doxygen_add_docs(docs interfaces)

include_directories(external/googletest/googletest/include)
include_directories(interfaces)

add_subdirectory(external)
add_subdirectory(platform)
