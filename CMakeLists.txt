﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

# Set the project name to your project name, my project isn't very descriptive
set(GRAPH_VERSION 0.2.0)
project("graph" VERSION ${GRAPH_VERSION} LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_options INTERFACE $<$<CXX_COMPILER_ID:Clang>:>)
target_compile_options(project_options INTERFACE $<$<CXX_COMPILER_ID:GNU>:--concepts>)
target_compile_options(project_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
target_compile_options(project_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
target_compile_options(project_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_CXX_STANDARD 23) # use std=c++latest in MSVC for concepts
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()
 
# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
# include(cmake/Cache.cmake)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)
option(ENABLE_EXAMPLES "Enable Example Builds" ON)
option(ENABLE_BENCHMARKING "Enable Benchmark Builds" OFF)

# Very basic PCH example
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which is a good idea if any #define's change
  # 
  # consider breaking this out per project as necessary 
  target_precompile_headers(
    project_options
    INTERFACE
    <vector>
    <string>
    <map>
    <utility>)
endif()


# Add thirdparty libraries
include(${PROJECT_SOURCE_DIR}/cmake/FetchCatch.cmake)

add_library(graph INTERFACE)
target_include_directories(graph INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

if(ENABLE_TESTING)
  enable_testing()
  message(STATUS "Building tests.")
  set(TEST_DATA_ROOT_DIR "${CMAKE_SOURCE_DIR}/data/")
  set(TEST_OUTPUT_ROOT_DIR "${CMAKE_SOURCE_DIR}/tests/")
  add_subdirectory(tests)
endif()

if(ENABLE_EXAMPLES)
  set(EXAMPLE_ROOT_DIR "${CMAKE_SOURCE_DIR}/example/")
  add_subdirectory(example)
endif()

if(ENABLE_BENCHMARKING)
  message(STATUS "Building benchmarks.")
  include(${PROJECT_SOURCE_DIR}/cmake/FetchFastMM.cmake)
  set(BENCHMARK_ROOT_DIR "${CMAKE_SOURCE_DIR}/benchmark/")
  set(BENCHMARK_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/benchmark/results/")

  # This is for Phil's home machine. It needs to be made more general
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(BENCHMARK_DATA_DIR "${CMAKE_SOURCE_DIR}/../data/")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BENCHMARK_DATA_DIR "/mnt/c/dev_graph/data/")
  endif()

  add_subdirectory(benchmark)
endif()

#if(ENABLE_FUZZING)
#  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
#  add_subdirectory(fuzz_test)
#endif()

#add_subdirectory(src)
