#include <locale>

#ifdef _MSC_VER
#  include <Windows.h>
#endif


void mm_simple1();
void mm_load_example();
void mm_load_file_example();
void bench_dijkstra();

int main() {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);            // Change console from OEM to UTF-8 in Windows
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
  std::locale::global(std::locale(""));
  //mm_simple1();
  //mm_load_example();
  //mm_load_file_example();
  bench_dijkstra();

  return 0;
}
