macro(run_conan)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

  # Download automatically, you can also just copy the conan.cmake file
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake"
                  TLS_VERIFY ON)
  endif()

  include(${CMAKE_BINARY_DIR}/conan.cmake)

# The following is set in ~/.conan/profiles/default
#    compiler=gcc
#    compiler.version=11
#

# See https://github.com/conan-io/cmake-conan for example


  conan_cmake_configure(REQUIRES boost/1.78.0
                                 catch2/2.13.8
                                 docopt.cpp/0.6.3
                                 fmt/8.1.1
                                 range-v3/0.11.0
                                 spdlog/1.9.2
                        GENERATORS cmake_find_package_multi) #cmake_find_package|cmake_find_package_multi?

  conan_cmake_autodetect(settings)

  conan_cmake_install(PATH_OR_REFERENCE .
                      BUILD missing
                      REMOTE conancenter
                      SETTINGS ${settings})

  find_package(Boost)
  find_package(Catch2)
  find_package(fmt)

endmacro()
