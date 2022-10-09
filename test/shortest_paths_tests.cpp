#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/shortest_paths.hpp"
#include "graph/container/dynamic_graph.hpp"
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

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;
using std::graph::edge_value_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;

using std::graph::dijkstra_shortest_paths;
using std::graph::dijkstra_shortest_distances;


using routes_volf_graph_traits = std::graph::container::vofl_graph_traits<double, std::string>;
using routes_volf_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_volf_graph_traits>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}


TEST_CASE("Dijkstra's Shortest Paths", "[csv][vofl][shortest_paths][dijkstra]") {
  init_console();
  using G             = routes_volf_graph_type;
  auto&& g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto   frankfurt_id = find_frankfurt_id(g);

  vector<double>         distance(size(vertices(g)));
  vector<vertex_id_t<G>> predecessor(size(vertices(g)));
  dijkstra_shortest_paths(g, frankfurt_id, distance, predecessor);
}

TEST_CASE("Dijkstra's Shortest Distances", "[csv][vofl][shortest_distances][dijkstra]") {
  init_console();
  using G             = routes_volf_graph_type;
  auto&& g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto   frankfurt_id = find_frankfurt_id(g);

  vector<double>         distance(size(vertices(g)));
  vector<vertex_id_t<G>> predecessor(size(vertices(g)));
  dijkstra_shortest_distances(g, frankfurt_id, distance);
}
