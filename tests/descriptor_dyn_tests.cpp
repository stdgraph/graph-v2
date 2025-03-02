#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "graph/graph.hpp"
#include "graph/container/dynamic_graph.hpp"
#include <cassert>
#include <iostream>

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;

using std::ranges::range;
using std::ranges::forward_range;
using std::ranges::random_access_range;
using std::remove_reference_t;
using std::is_const_v;

using graph::vertex_range_t;
using graph::vertex_t;
using graph::vertex_id_t;
using graph::vertex_value_t;
using graph::vertex_edge_range_t;
using graph::edge_t;
using graph::edge_reference_t;
using graph::edge_value_t;

using graph::graph_value;
using graph::vertices;
using graph::edges;
using graph::vertex_id;
using graph::vertex_value;
using graph::target_id;
using graph::target;
using graph::edge_value;
using graph::degree;
using graph::find_vertex;
using graph::find_vertex_edge;
using graph::num_vertices;
using graph::num_edges;
using graph::has_edge;


using volf_graph_traits = graph::container::vofl_graph_traits<>;
using volf_graph_type   = graph::container::dynamic_adjacency_graph<volf_graph_traits>;


TEST_CASE("Descriptor static tests with VoL", "[vofl][descriptor]") {
  //init_console(); //
  using G  = volf_graph_type;
  using VR = graph::vertex_range_t<G>;
  static_assert(range<VR>);
  //static_assert(random_access_range<VR>);
}
