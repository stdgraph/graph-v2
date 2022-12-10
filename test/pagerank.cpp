#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/pagerank.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/incidence.hpp"
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;
using std::vector;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_reference_t;
using std::graph::vertex_iterator_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::find_vertex;
using std::graph::vertex_id;

using routes_volf_graph_traits = std::graph::container::vofl_graph_traits<double, std::string>;
using routes_volf_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_volf_graph_traits>;

TEST_CASE("PageRank", "[pagerank]") {
  init_console();
  using G             = routes_volf_graph_type;
  auto&& g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  vector<double>         page_rank(size(vertices(g)));
  // pagerank(g, page_rank, float(0.85), float(1e-4), 100);
}