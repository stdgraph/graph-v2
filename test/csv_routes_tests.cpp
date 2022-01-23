#include <catch2/catch.hpp>
#include "csv_routes_csr.hpp"
#include "csv_routes_vol.hpp"
#include "graph/graph.hpp"
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

// for djikstra
#include <queue>
#include <tuple>

#define TEST_OPTION_OUTPUT (1)
#define TEST_OPTION_GEN (2)
#define TEST_OPTION_TEST (3)
#define TEST_OPTION TEST_OPTION_OUTPUT

using std::cout;
using std::endl;

using std::graph::vertex_t;
using std::graph::vertex_key_t;
using std::graph::vertex_edge_range_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::target_key;
using std::graph::edge_value;

template <typename OStream>
OStream& utf8_out(OStream& os, const char ch) {
  if (ch < 0x7f)
    os << ch;
  else
    os << "\\0x" << std::setiosflags(std::ios_base::hex) << ch << std::setiosflags(std::ios_base::dec);
  return os;
}

template <typename OStream>
OStream& utf8_out(OStream& os, const std::string& s) {
  for (auto ch : s)
    os << ch;
}

template <typename OStream>
OStream& utf8_out(OStream& os, const std::string_view s) {
  for (auto ch : s)
    os << ch;
}

template <typename OStream>
OStream& utf8_out(OStream& os, const char* s) {
  for (const char* ch = s; ch && *ch; ++ch)
    os << *ch;
}


/* template <typename G, typename F>
concept target_function = // e.g. target_id(uv)
      std::copy_constructible<F> && std::regular_invocable<F&, std::ranges::range_reference_t<inner_range_t<G>>>;
template <typename G, typename F>
concept source_function = // e.g. source_id(uv)
      target_function<G, F>;*/

template <typename G, typename F>
concept edge_weight_function = // e.g. weight(uv)
      std::copy_constructible<F> && std::regular_invocable<F&, std::ranges::range_reference_t<vertex_edge_range_t<G>>>;


#if 1
// The index into weight vector stored as the first property
template <std::graph::incidence_graph G, typename WF>
requires edge_weight_function<G, WF> &&
      std::is_arithmetic_v<std::invoke_result_t<WF, std::ranges::range_reference_t<vertex_edge_range_t<G>>>>
auto dijkstra(
      G&& g, vertex_key_t<G> source, WF weight = [](std::ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
        return 1;
      }) {
  using vertex_key_type = vertex_key_t<G>;
  using weight_type     = decltype(weight(std::declval<std::ranges::range_reference_t<vertex_edge_range_t<G>>>()));

  size_t N(size(g));
  assert(source < N);

  std::vector<weight_type> distance(N, std::numeric_limits<weight_type>::max());
  distance[source] = 0;

  using weighted_vertex = std::tuple<vertex_key_type, weight_type>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>,
                      decltype([](auto&& a, auto&& b) { return (std::get<1>(a) > std::get<1>(b)); })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto u = std::get<0>(Q.top());
    Q.pop();

    // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

    auto transform_incidence_edge = [&g](auto&& uv) { return std::tuple(target(g, uv), uv); };
    auto vw                       = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    //auto vw = incidence_edges_view(g, u, target_key); // (can't put it in for stmt below right now)
    for (auto&& [v, uv] : vw) {
      weight_type w = weight(uv);
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        Q.push({v, distance[v]});
      }
    }
  }

  return distance;
}
#endif // 0/1

TEST_CASE("Germany routes CSV+csr test", "[csv][csr]") {
  init_console();
  routes_csv_csr_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
}

TEST_CASE("Germany routes CSV+vol test", "[csv][vol][germany]") {
  init_console();
  routes_vol_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
  using G = routes_vol_graph::graph_type;
  G& g    = germany_routes.graph();

  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  SECTION("metadata") {
    REQUIRE(std::ranges::size(g) == std::ranges::size(germany_routes.cities()));
    size_t edge_cnt   = 0;
    double total_dist = 0;
    for (auto&& u : vertices(g)) {
      for (auto&& uv : edges(g, u)) {
        ++edge_cnt; // forward_list doesn't have size()
        total_dist += edge_value(g, uv);
      }
    }
    REQUIRE(edge_cnt == 11);
    REQUIRE(total_dist == 2030.0);
  }

  SECTION("content") {
#if TEST_OPTION == TEST_OPTION_OUTPUT
    cout << "\nGermany Routes"
         << "\n-------------------------------" << germany_routes << endl;
#elif TEST_OPTION == TEST_OPTION_GEN
    for (vertex_key_t<G> ukey = 0; auto&& u : vertices(g)) {
      cout << "\n";
      cout << "u = begin(g) + " << ui << ";\n";
      cout << "EXPECT_EQ(\"" << u->name << "\", u->name);\n";
      cout << "EXPECT_EQ(" << size(edges(g, u)) << ", size(edges(g, u)));\n";
      cout << "uv = begin(g, *u);\n";
      size_t uvi = 0;
      for (uv = begin(edges(g, u)); uv != end(edges(g, u)); ++uv, ++uvi) {
        if (uvi > 0) {
          cout << "++uv;\n";
        }
        cout << "EXPECT_EQ(" << target_vertex_key(g, uv) << ", target_vertex_key(g, uv));\n";
        cout << "EXPECT_EQ(\"" << target_vertex(g, uv)->name << "\", target_vertex(g, uv)->name);\n";
        cout << "EXPECT_EQ(" << uv->weight << ", uv->weight);\n";
      }
    }
#elif TEST_OPTION == TEST_OPTION_TEST
#endif
  }
}
