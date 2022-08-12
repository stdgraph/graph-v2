#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views//depth_first_search.hpp"
#include "graph/container/csr_graph.hpp"
#include <cassert>

using namespace std::graph;
using namespace std::graph::views;

using routes_csr_graph_type = std::graph::container::csr_graph<double, std::string, std::string>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

TEST_CASE("Germany Routes Example", "[example][germany][routes]") {
  init_console();

  using G                   = routes_csr_graph_type;
  csv::string_view csv_file = TEST_DATA_ROOT_DIR "germany_routes.csv";
  csv::string_view out_file = TEST_OUTPUT_ROOT_DIR "example_routes.gv";

  // Load for graphviz
  {
    auto&& g = load_ordered_graph<G>(csv_file, name_order_policy::order_found, false);
    output_routes_graphviz(g, out_file, directedness::undirected);
    // name_order_policy::source_order_found gives best output with least overlap for germany routes
  }

#if 1
  auto&&     g            = load_ordered_graph<G>(csv_file, name_order_policy::source_order_found, true);
  const auto frankfurt    = find_frankfurt(g);
  const auto frankfurt_id = find_frankfurt_id(g);

  for (auto&& [uid, u] : vertexlist(g)) {
  }

  for (auto&& [vid, uv] : vertices_depth_first_search(g, frankfurt_id)) {
  }
#endif

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  // (uncomment to generate a graphviz file)
}
