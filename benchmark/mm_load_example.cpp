#include <vector>
#include <tuple>

#include "mm_load.hpp"

using std::tuple;
using std::vector;

using namespace std::graph::container;

// Dataset: gap_twitter, symmetry_type::general, 1,468,364,884 rows
//  Deb/Rel parallel_ok num_threads Read        Rows/Sec     LoadSimple  Edges/Sec    LoadCompressed  Edges/Sec
//  ------- ----------- ----------- ----------- ----------   ---------- -----------   --------------  -----------
//  Debug   false       1           6m0s(360)    4,077,499   5m32s(332)   4,077,499   5m49s(348)      4,213,302
//  Debug   true        2           12m42s(761)  1,927,867   5m13s(313)   4,688,601   5m36s(335)      4,373,910
//  Release false       1           2m18s(138)  10,619,350   1m24s(83)   17,557,093   1m2s(62)        23,531,613
//  Release true        2           1m19s(78)   18,625,828
//  Release true        4           1m20s(79)   18,460,595   1m18s(78)   18,752,507   0m45s(44)       32,708,977

void mm_load_file_example() {

  triplet_matrix<int64_t, int64_t> triplet;
  array_matrix<int64_t>            sources;

  load_matrix_market(gap_road, triplet, sources, true);

  // Load a simple graph: vector<vector<tuple<int64_t, int64_t>>>
  {
    vector<vector<tuple<int64_t, int64_t>>> g;
    graph_stats stats = load_graph(triplet, g);
    fmt::println("Graph stats: {}", stats);
  }

  // Load a compressed_graph
  {
    compressed_graph<int64_t, void, void, int64_t, int64_t> g;
    graph_stats                                             stats = load_graph(triplet, g);
    fmt::println("Graph stats: {}", stats);
  }
}
