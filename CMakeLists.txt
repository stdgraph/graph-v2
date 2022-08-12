# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

# Set the project name to your project name, my project isn't very descriptive
set(GRAPH_VERSION 0.2.0)
project("graph" VERSION ${GRAPH_VERSION} LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)


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
include(cmake/Cache.cmake)

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

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")
set(CONAN_DISABLE_CHECK_COMPILER ON)

include(cmake/Conan.cmake)
run_conan()

# Add other libraries
add_library(csv_parser INTERFACE)
target_include_directories(csv_parser INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/csv-parser/")

add_library(graph INTERFACE)
target_include_directories(graph INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#target_link_libraries(graph INTERFACE CONAN_PKG::range-v3)

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
  set(TEST_DATA_ROOT_DIR "${CMAKE_SOURCE_DIR}/data/")
  set(TEST_OUTPUT_ROOT_DIR "${CMAKE_SOURCE_DIR}/test/")
  add_subdirectory(test)
endif()

if(ENABLE_EXAMPLES)
  add_subdirectory(example)
endif()

#if(ENABLE_FUZZING)
#  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
#  add_subdirectory(fuzz_test)
#endif()


# Include sub-projects.
#add_subdirectory ("g")

#add_subdirectory(src)
