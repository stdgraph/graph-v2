#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/depth_first_search.hpp"
#include "graph/container/dynamic_graph.hpp"

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_value_t;
using std::graph::vertex_edge_range_t;
using std::graph::vertex_reference_t;
using std::graph::edge_t;
using std::graph::edge_value_t;
using std::graph::edge_reference_t;

using std::graph::graph_value;
using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_id;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::degree;
using std::graph::find_vertex;
using std::graph::find_vertex_edge;

using std::graph::views::dfs_vertex_range;

using routes_vol_graph_traits = std::graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

TEST_CASE("dfs vertex test", "[dynamic][dfs][vertex]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("bfs_range") { 
	  //dfs_vertex_range dfs(g, frankfurt_id); 
  }
}
