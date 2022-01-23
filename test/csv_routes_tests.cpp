#include <catch2/catch.hpp>
#include "csv_routes_csr.hpp"
#include "csv_routes_vol.hpp"
#include "graph/graph.hpp"
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using std::cout;
using std::endl;

using std::graph::vertex_t;
using std::graph::vertex_key_t;

using std::graph::vertices;
using std::graph::edges;

#if 0
/* template <typename G, typename F>
concept target_function = // e.g. target_id(uv)
      std::copy_constructible<F> && std::regular_invocable<F&, std::ranges::range_reference_t<inner_range_t<G>>>;
template <typename G, typename F>
concept source_function = // e.g. source_id(uv)
      target_function<G, F>;*/

// The index into weight vector stored as the first property
template <std::graph::incidence_graph Graph, typename TF, typename WF>
requires target_function<Graph, TF> && edge_weight_function<Graph, WF> &&
      std::is_arithmetic_v<std::invoke_result_t<WF, std::ranges::range_reference_t<inner_range_t<Graph>>>>
auto dijkstra(
      Graph&&             graph,
      vertex_key_t<Graph> source,
      TF                  target_id,
      WF                  weight = [](ranges::range_reference_t<inner_range_t<Graph>> uv) { return 1; }) {
  using vertex_key_type = vertex_key_t<Graph>;
  using weight_type     = decltype(weight(std::declval<std::ranges::range_reference_t<inner_range_t<Graph>>>()));

  size_t N(size(graph));
  assert(source < N);

  std::vector<weight_type> distance(N, std::numeric_limits<weight_type>::max());
  distance[source] = 0;

  using weighted_vertex = std::tuple<vertex_id_type, weight_type>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>,
                      decltype([](auto&& a, auto&& b) { return (std::get<1>(a) > std::get<1>(b)); })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto u = std::get<0>(Q.top());
    Q.pop();

    // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

    auto vw = incidence_edges_view(graph, u, target_id); // (can't put it in for stmt below right now)
    for (auto&& [v, e] : vw) {
      auto w = weight(e);

      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        Q.push({v, distance[v]});
      }
    }
  }

  return distance;
}
#endif

#if 0
TEST_CASE("Dummy CSV test", "[csv]") {
  init_console();
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator
    string_view from = row[0].get<string_view>();
    string_view to   = row[1].get<string_view>();
    auto        dist = row[2].get<double>();
    cout << from << "," << to << "," << dist << endl;
  }
}
#endif

TEST_CASE("Germany routes CSV+csr test", "[csv][csr]") {
  init_console();
  routes_csv_csr_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
}

TEST_CASE("Germany routes CSV+vol test", "[csv][vol]") {
  init_console();
  routes_vol_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
  using G = routes_vol_graph::graph_type;
  //germany_routes.output_routes();

  //cout << "\nUsing CPO functions" << endl;
  //auto&& g = germany_routes.graph();
}
