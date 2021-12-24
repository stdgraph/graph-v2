macro(run_conan)
  # Download automatically, you can also just copy the conan.cmake file
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.17.0/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
  endif()

  set (ENV{CONAN_REVISIONS_ENABLED} 1)
  include(${CMAKE_BINARY_DIR}/conan.cmake)

# The following is set in ~/.conan/profiles/default
#    compiler=gcc
#    compiler.version=11
#

# note: conan_cmake_run is deprecated and should be updated with new code shown
#       at https://github.com/conan-io/cmake-conan

conan_cmake_run(
  REQUIRES
  ${CONAN_EXTRA_REQUIRES}
  catch2/2.13.7
  docopt.cpp/0.6.2
  fmt/8.0.1
  spdlog/1.9.2
  range-v3/0.11.0
  OPTIONS
  ${CONAN_EXTRA_OPTIONS}
  SETTINGS compiler.cppstd=20 
  BASIC_SETUP
  CMAKE_TARGETS # individual targets to link to
  BUILD
  missing)
endmacro()
