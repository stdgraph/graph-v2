# Graph Library Proposal for the C++ Standard

This library is in the alpha stage that may include significant changes to the interface. It is not recommended for general use.

## Build & Run Requirements
This is being actively developed with the latest releases of MSVC (VS2022) on Windows and gcc (11) on Linux. Any releases
or compilers may or may not work. (At the time of this writing Clang doesn't have a \<concepts\> header and so it hasn't
been used.)

### Prerequesites
1. C++20 compliant compiler that fully supports concepts and ranges. 
   gcc 11 and the latest version of MSVC have been used for development. 
   Others may also work but haven't been tested.
2. CMake 20 or later (needed for CMake Presets)
3. Python3
4. Conan package manager (Python: pip install conan)

### Cloning & Building

```C++
git clone https://github.com/pratzl/graph-v2.git
cd graph-v2
mkdir build
cd build
cmake ../???
make
```

You'll need to assure CMake Presets are enabled in your IDE or other development tool. 
See https://docs.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170 for configuring Microsoft tools.

The following libraries will automatically be installed by Conan

1. Catch2 unit test framework
2. fmt library
3. spdlog

Other Useful Tools

1. clang-format


## Thanks to:
Andrew Lumsdaine and the NWGraph team for numerous insights and collaborations with their C++20 
[NWGraph Library](https://github.com/NWmath/NWgr)

Numerous comments and support from the Machine Learning study group (SG19) in the ISO C++ Standards
Committee ([WG21](https://isocpp.org/std/the-committee)).

Bob Steagal for his [gcc-builder & clang-builder scripts](https://github.com/BobSteagall)

Jason Turner for his [cpp_starter_project](https://github.com/lefticus/cpp_starter_project)

René FerdinandRivera Morell for his [duck_invoke](https://github.com/bfgroup/duck_invoke), an implementation
of tag_invoke ([P1895](https://wg21.link/P1895)) that works for both gcc and msvc. Minor modifications have
been made so it it in the std namespace.

Vincent La for his [cvs-parser](https://github.com/vincentlaucsb/csv-parser)

The ISO C++ Committee for [C++](http://eel.is/c++draft/)
