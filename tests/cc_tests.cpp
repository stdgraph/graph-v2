#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/connected_components.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/incidence.hpp"
#ifdef _MSC_VER
#  include "Windows.h"
#endif

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;

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

using routes_vol_graph_traits = std::graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

#if 1
TEST_CASE("strongly connected components test", "[strong cc]") {
  init_console();

  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "cc_directed.csv", name_order_policy::alphabetical);
  G gt;
  
  std::vector<std::tuple<vertex_id_t<G>,vertex_id_t<G>,double>> reverse;
  vertex_id_t<G> vid = 0;
  for ( auto && u : vertices(g) ) {
    for ( auto && v : edges(g, u)) {
      reverse.push_back(std::make_tuple(target_id(g,v), vid, edge_value(g,v)));
    }
    ++vid;
  }
                             

  using value = std::ranges::range_value_t<decltype(reverse)>;

  vertex_id_t<G> N = size(vertices(g));
  using edge_desc = std::graph::edge_descriptor<vertex_id_t<G>, true, void, double>;
  auto edge_proj  = [](const value& val) -> edge_desc {
    return edge_desc{std::get<0>(val), std::get<1>(val), std::get<2>(val)};
  };

  gt.load_edges(reverse, edge_proj, N);

  std::vector<size_t> component(size(vertices(g)));
  std::graph::kosaraju(g, gt, component);
  
  REQUIRE( *std::ranges::max_element( component ) == 2 );
}
#endif

TEST_CASE("connected components test", "[cc]") {
  init_console();
  
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "cc_undirected.csv", name_order_policy::alphabetical);

  std::vector<size_t> component(size(vertices(g)));
  std::graph::connected_components(g, component);
  
  REQUIRE( *std::ranges::max_element( component ) == 2 );
}