macro(run_conan)
  # Download automatically, you can also just copy the conan.cmake file
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/f8fd649b3058846188d8b12040c354fd50e30780/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
    # The following should be used, but doesn't work': 'DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.17.0/conan.cmake"
  endif()

  include(${CMAKE_BINARY_DIR}/conan.cmake)

# The following is set in ~/.conan/profiles/default
#    compiler=gcc
#    compiler.version=11
#

# See https://github.com/conan-io/cmake-conan for example

conan_cmake_configure(REQUIRES   catch2/2.13.7
                                 docopt.cpp/0.6.3
                                 fmt/8.0.1
                                 spdlog/1.9.2
                                 range-v3/0.11.0
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD             missing
                    REMOTE            conancenter
                    SETTINGS          ${settings} compiler.cppstd=20) 

endmacro()
