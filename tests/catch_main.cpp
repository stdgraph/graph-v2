//#define CATCH_CONFIG_MAIN // This tells the catch header to generate a main

#ifdef _MSC_VER
#  include <Windows.h>
#endif

#include <cstdio>


// Intialize the console for outputting UTF-8 using cout
void init_console() {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8); // Change console from OEM to UTF-8 in Windows
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
}
