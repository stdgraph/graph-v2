#include <locale>

void mm_simple1();
void mm_load_example();
void mm_load_file_example();
void bench_co_dijkstra();

int main() {
  std::locale::global(std::locale(""));
  //mm_simple1();
  //mm_load_example();
  //mm_load_file_example();
  bench_co_dijkstra();

  return 0;
}
