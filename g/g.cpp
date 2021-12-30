// g.cpp : Defines the entry point for the application.
//

#include "g.h"

using namespace std;

int main() {
  cout << "Hello CMake";
#ifdef WIN32
  cout << " on Windows";
#else
  cout << " on Linux";
#endif
#if defined(_DEBUG) || !defined(NDEBUG)
  cout << " (Debug)";
#else
  cout << " (Release)";
#endif
  cout << "." << endl;

  return 0;
}
