#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/neighbors.hpp"
//#include "graph/view/edgelist_view.hpp"
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

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;


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


TEST_CASE("Germany routes examples", "[csv][vofl][germany][example]") {
  init_console();
  using G  = routes_volf_graph_type;
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

#if 0
  SECTION("Incidence iteration") {
    for (auto&& [uid, u] : std::graph::views::vertexlist(g)) {
      for (auto&& [vid, uv] : std::graph::views::edges_view(g, u)) {
      }
    }
  }

  SECTION("Adjacency iteration") {
    for (auto&& [uid, u] : std::graph::views::vertexlist(g)) {
      for (auto&& [vid, v] : std::graph::views::adjacency_view(g, u)) {
      }
    }
  }
#endif

#if 0
  SECTION("Edgelist iteration") {
    for (auto&& [uid, vid, uv] : std::graph::edges_view(g)) {
    }
  }
#endif
}
