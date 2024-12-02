#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/mst.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/utility_edgelist.hpp"
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using std::cout;
using std::endl;

using graph::vertex_t;
using graph::vertex_id_t;
using graph::vertex_reference_t;
using graph::vertex_iterator_t;
using graph::vertex_edge_range_t;
using graph::edge_t;
using graph::edge_reference_t;
using graph::edge_value_t;

using graph::vertices;
using graph::edges;
using graph::vertex_value;
using graph::target_id;
using graph::target;
using graph::edge_value;
using graph::find_vertex;
using graph::vertex_id;

using routes_vol_graph_traits = graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

using edgelist = graph::container::
      utility_edgelist<vertex_id_t<routes_vol_graph_type>, vertex_id_t<routes_vol_graph_type>, double>;

TEST_CASE("Kruskal Min ST Algorithm", "[min st]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  size_t N(size(vertices(g)));

  auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> e, t;
  for (auto&& [uid, vid, uv] : graph::views::edgelist(g)) {
    e.push_back(std::ranges::range_value_t<decltype(e)>());
    e.back().source_id = uid;
    e.back().target_id = vid;
    e.back().value     = evf(uv);
  }

  // Kruskal with separate edgelist data structure, don't modify edgelist
  graph::kruskal(e, t);
  double last = -1;
  for (auto&& [u, v, val] : t) {
    REQUIRE(val > last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }
  auto it = e.begin();
  for (auto&& [uid, vid, uv] : graph::views::edgelist(g)) {
    REQUIRE((*it++).value == evf(uv));
  }

  // Kruskal inplace, modifiy input edgelist
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> t2;
  graph::inplace_kruskal(e, t2);
  last = -1;
  for (auto&& [u, v, val] : t2) {
    REQUIRE(val > last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }

  // Kruskal inplace from adjacency list
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> t3;
  graph::kruskal(graph::views::edgelist(g, evf), t3);
  last = -1;
  for (auto&& [u, v, val] : t3) {
    REQUIRE(val > last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }
}

TEST_CASE("Kruskal Max ST Algorithm", "[max st]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  size_t N(size(vertices(g)));

  auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> e, t;
  for (auto&& [uid, vid, uv] : graph::views::edgelist(g)) {
    e.push_back(std::ranges::range_value_t<decltype(e)>());
    e.back().source_id = uid;
    e.back().target_id = vid;
    e.back().value     = evf(uv);
  }

  // Kruskal with separate edgelist data structure, don't modify edgelist
  graph::kruskal(e, t, [](auto&& i, auto&& j) { return i > j; });
  double last = std::numeric_limits<double>::max();
  for (auto&& [u, v, val] : t) {
    REQUIRE(val < last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }
  auto it = e.begin();
  for (auto&& [uid, vid, uv] : graph::views::edgelist(g)) {
    REQUIRE((*it++).value == evf(uv));
  }

  // Kruskal inplace, modifiy input edgelist
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> t2;
  graph::inplace_kruskal(e, t2, [](auto&& i, auto&& j) { return i > j; });
  last = std::numeric_limits<double>::max();
  for (auto&& [u, v, val] : t2) {
    REQUIRE(val < last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }

  // Kruskal inplace from adjacency list
  std::vector<graph::edge_info<vertex_id_t<G>, true, void, double>> t3;
  graph::kruskal(graph::views::edgelist(g, evf), t3, [](auto&& i, auto&& j) { return i > j; });
  last = std::numeric_limits<double>::max();
  for (auto&& [u, v, val] : t3) {
    REQUIRE(val < last);
    last = val;
    //cout << u << " " << v << " " << val << endl;
  }
}

TEST_CASE("Prim Min ST Algorithm", "[prim min st]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  std::vector<vertex_id_t<G>> preds(size(vertices(g)));
  std::vector<double>         weights(size(vertices(g)));
  graph::prim(g, preds, weights);
  double treeweight = 0;
  for (auto&& [uid, u] : graph::views::vertexlist(g)) {
    //cout << "pred of " << uid << " is " << preds[uid] << " with val " << weights[uid] << endl;
    treeweight += weights[uid];
  }
  REQUIRE(treeweight == 1361);
}

TEST_CASE("Prim Max ST Algorithm", "[prim max st]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  std::vector<vertex_id_t<G>> preds(size(vertices(g)));
  std::vector<double>         weights(size(vertices(g)));

  graph::prim(g, preds, weights, [](auto&& i, auto&& j) { return i > j; }, 0);
  double treeweight = 0;
  for (auto&& [uid, u] : graph::views::vertexlist(g)) {
    //cout << "pred of " << uid << " is " << preds[uid] << " with val " << weights[uid] << endl;
    treeweight += weights[uid];
  }
  REQUIRE(treeweight == 1779);
}
