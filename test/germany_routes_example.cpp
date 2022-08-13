#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views//depth_first_search.hpp"
#include "graph/container/csr_graph.hpp"
#include "graph/algorithm/dijkstra_clrs.hpp"
#include <cassert>

using namespace std::literals;
using namespace std::graph;
using namespace std::graph::views;

using std::cout;
using std::endl;

using routes_csr_graph_type = std::graph::container::csr_graph<double, std::string, std::string>;

template <typename G>
struct out_city {
  const G&                    g;
  vertex_id_t<G>              city_id;
  vertex_reference_t<const G> city;

  out_city(const G& graph, vertex_id_t<G> uid, vertex_reference_t<G> u) : g(graph), city(u), city_id(uid) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const out_city& rhs) {
    auto&& [g, city_id, city] = rhs;
    os << vertex_value(g, city) << " [" << city_id << "]";
    return os;
  }
};

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

  //std::string_view munchen_name = "M\xC3\xBCnchen";
  //auto&& u   = **find_city(g, munchen_name);
  //auto   uid = find_city_id(g, munchen_name);

  cout << "Path Segments:" << endl;
  for (auto&& [uid, u] : vertexlist(g)) {
    cout << "From " << out_city(g, uid, u) << "\n";
    auto dfs = vertices_depth_first_search(g, uid);
    for (auto&& [vid, v] : dfs) {
      cout << "   --> " << out_city(g, vid, v)<< " - " << dfs.depth() << " segments" << endl;
    }
  }

#endif

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  // (uncomment to generate a graphviz file)
}
