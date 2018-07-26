cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)
message(STATUS ${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/include)
add_library(opencl SHARED IMPORTED)
set_target_properties(opencl PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/libs/libopencl/test/libOpenCL.so")
add_library(openclfilter SHARED
            src/main/cpp/opencl_utils.cpp
            src/main/cpp/vector_addition.cpp
            src/main/cpp/filter.cpp)

target_link_libraries(openclfilter opencl)