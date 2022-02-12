# Graph Library Proposal for the C++ Standard

This library is in the alpha stage that may include significant changes to the interface. It is not recommended for general use.

## Build & Run Requirements
This is being actively developed with the latest releases of MSVC (VS2022) on Windows and gcc (11) on Linux. Any releases
or compilers may or may not work. (At the time of this writing Clang doesn't have a \<concepts\> header and so it hasn't
been used.)

### Prerequesites
C++20 compliant compiler that fully supports concepts and ranges.

CMake 20 or later

Python3 with the conan package manager installed

### Cloning & Building



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
