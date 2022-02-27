#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertices_view.hpp"
#include "graph/views/incidence_view.hpp"
#include "graph/views/adjacency_edge_view.hpp"
#include "graph/container/csr_graph.hpp"
#include <cassert>

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
using std::graph::vertex_key_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;
using std::graph::edge_value_t;

using std::graph::graph_value;
using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_key;
using std::graph::vertex_value;
using std::graph::target_key;
using std::graph::target;
using std::graph::edge_value;
using std::graph::degree;
using std::graph::find_vertex;
using std::graph::find_vertex_edge;


using routes_csr_graph_type = std::graph::container::csr_graph<double, std::string, std::string>;

template <typename G>
constexpr auto find_frankfurt_key(const G& g) {
  return find_city_key(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

// Things to test
//  csr_graph with VV=void (does it compile?)
//  push_back and emplace_back work correctly when adding city names (applies to csr_graph & dynamic_graph)


TEST_CASE("CSR graph test", "[csr][capabilities]") {
  using G = routes_csr_graph_type; // use it because it's easy

  // This is the type the initializer_list is expecting:
  //using init_edge_value = std::graph::views::copyable_edge_t<vertex_key_t<G>, edge_value_t<G>>;

  // Define the graph. It's the same as the germany routes using source_order_found
  G g = {{0, 1, 85.0},  {0, 4, 217.0}, {0, 6, 173.0}, {1, 2, 80.0},  {2, 3, 250.0}, {3, 8, 84.0},
         {4, 5, 103.0}, {4, 7, 186.0}, {5, 8, 167.0}, {5, 9, 183.0}, {6, 8, 502.0}};

  using init_vertex_value             = std::graph::views::copyable_vertex_t<vertex_key_t<G>, std::string>;
  std::vector<std::string_view> names = {"Frankfürt", "Mannheim", "Karlsruhe", "Augsburg", "Würzburg",
                                         "Nürnberg",  "Kassel",   "Erfurt",    "München",  "Stuttgart"};
  g.load_vertices(names, [&names](std::string_view& nm) {
    auto ukey = static_cast<vertex_key_t<G>>(&nm - names.data());
    return init_vertex_value{ukey, std::string(nm)};
  });

  graph_value(g) = "Germany Routes";

  SECTION("metadata") {
    // Do a simple check
    REQUIRE(10 == std::ranges::size(vertices(g)));
    size_t edge_cnt   = 0;
    double total_dist = 0;
    for (auto&& u : vertices(g)) {
      for (auto&& uv : edges(g, u)) {
        ++edge_cnt;
        total_dist += edge_value(g, uv);
      }
    }
    REQUIRE(edge_cnt == 11);
    REQUIRE(total_dist == 2030.0);
  }

  SECTION("functions") {
    using key_type = uint32_t;
    auto&& d       = graph_value(g);

    auto uit = std::ranges::begin(vertices(g)) + 2;
    auto key = vertex_key(g, uit);
    REQUIRE(key == 2);
    REQUIRE(std::is_same_v<key_type, decltype(key)>);
    auto& uval = vertex_value(g, *uit);
    REQUIRE(std::is_same_v<std::string&, decltype(uval)>);
    REQUIRE("Karlsruhe" == uval);
    auto deg = degree(g, *uit);
    REQUIRE(1 == deg);

    auto&& uu = edges(g, *uit);
    REQUIRE(1 == std::ranges::size(uu));
    auto& uv = *std::ranges::begin(uu);
    REQUIRE(3 == target_key(g, uv));
    REQUIRE(250.0 == edge_value(g, uv));
    auto& v = target(g, uv);
    REQUIRE(vertex_value(g, v) == "Augsburg");

    auto vit = find_vertex(g, 4);
    REQUIRE(4 == vit - std::ranges::begin(vertices(g)));
    auto uvit = find_vertex_edge(g, *vit, 7);
    REQUIRE(edge_value(g, *uvit) == 186.0);
  }

  SECTION("functions") {
    const G& g2    = g;
    using key_type = uint32_t;
    auto&& d       = graph_value(g2);

    auto uit = std::ranges::begin(vertices(g2)) + 2;
    auto key = vertex_key(g2, uit);
    REQUIRE(key == 2);
    REQUIRE(std::is_same_v<key_type, decltype(key)>);
    auto& uval = vertex_value(g2, *uit);
    REQUIRE(std::is_same_v<const std::string&, decltype(uval)>);
    REQUIRE("Karlsruhe" == uval);
    auto deg = degree(g2, *uit);
    REQUIRE(1 == deg);

    auto&& uu = edges(g2, *uit);
    REQUIRE(1 == std::ranges::size(uu));
    auto& uv = *std::ranges::begin(uu);
    REQUIRE(3 == target_key(g2, uv));
    REQUIRE(250.0 == edge_value(g2, uv));
    auto& v = target(g2, uv);
    REQUIRE(vertex_value(g2, v) == "Augsburg");

    auto vit = find_vertex(g2, 4);
    REQUIRE(4 == vit - std::ranges::begin(vertices(g2)));
    auto uvit = find_vertex_edge(g2, *vit, 7);
    REQUIRE(edge_value(g2, *uvit) == 186.0);
  }
};

TEST_CASE("Germany routes CSV+csr test", "[csv][csr][germany]") {
  init_console();

  using G  = routes_csr_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
  // name_order_policy::source_order_found gives best output with least overlap for germany routes

  const auto frankfurt     = find_frankfurt(g);
  const auto frankfurt_key = find_frankfurt_key(g);

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  // (uncomment to generate a graphviz file)
  //output_routes_graphviz(g, "d:/dev/g/test/germany_routes.gv");

  SECTION("metadata") {
    REQUIRE(10 == std::ranges::size(vertices(g)));
    size_t vertex_cnt = 0;
    size_t edge_cnt   = 0;
    double total_dist = 0;
    for (auto&& u : vertices(g)) {
      ++vertex_cnt;
      for (auto&& uv : edges(g, u)) {
        ++edge_cnt; // forward_list doesn't have size()
        total_dist += edge_value(g, uv);
      }
    }
    REQUIRE(vertex_cnt == 10);
    REQUIRE(edge_cnt == 11);
    REQUIRE(total_dist == 2030.0);
  }

#if 0
  SECTION("const_vertices_view") {
    const G& g2 = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>);

    std::graph::views::const_vertices_view_iterator<G> i0; // default construction
    std::graph::views::const_vertices_view_iterator<G> i1(g);
    {
      auto&& [ukey, u] = *i1;
      static_assert(is_const_v<decltype(ukey)>);
      static_assert(is_const_v<remove_reference_t<decltype(u)>>);
      REQUIRE(ukey == 0);
    }
    {
      auto&& [ukey, u] = *++i1;
      REQUIRE(ukey == 1);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    std::graph::views::const_vertices_view_iterator<G> i2(g2);
    {
      auto&& [ukey, u] = *i2;
      static_assert(is_const_v<decltype(ukey)>);
      static_assert(is_const_v<remove_reference_t<decltype(u)>>);
      REQUIRE(ukey == 0);
    }
    {
      auto&& [ukey, u] = *++i2;
      REQUIRE(ukey == 1);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    using view_t = decltype(std::graph::views::vertices_view(g2));
    static_assert(forward_range<view_t>);
    size_t cnt = 0;
    for (auto&& [ukey, u] : std::graph::views::vertices_view(g2)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }

  SECTION("non_const_vertices_view") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    std::graph::views::vertices_view_iterator<G> i0; // default construction
    std::graph::views::vertices_view_iterator<G> i1(g);
    {
      auto&& [ukey, u] = *i1;
      static_assert(is_const_v<decltype(ukey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(u)>>);
      REQUIRE(ukey == 0);
    }
    {
      auto&& [ukey, u] = *++i1;
      REQUIRE(ukey == 1);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    std::graph::views::vertices_view_iterator<G> i2(g);
    {
      auto&& [ukey, u] = *i2;
      static_assert(is_const_v<decltype(ukey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(u)>>);
      REQUIRE(ukey == 0);
    }
    {
      auto&& [ukey, u] = *++i2;
      REQUIRE(ukey == 1);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    size_t cnt = 0;
    for (auto&& [ukey, u] : std::graph::views::vertices_view(g)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    std::graph::views::const_vertices_view_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const_incidence_edge_view") {
    const G& g2 = g;

    std::graph::views::const_vertex_edge_view_iterator<G> i0; // default construction

    auto& u = g2[frankfurt_key];

    std::graph::views::const_vertex_edge_view_iterator<G> i1(g2, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 5);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::views::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("incidence_edge_view") {
    std::graph::views::vertex_edge_view_iterator<G> i0; // default construction

    auto& u = g[frankfurt_key];

    std::graph::views::vertex_edge_view_iterator<G> i1(g, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 5);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::views::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("const_adjacency_edge_view") {
    const G& g2 = g;

    std::graph::views::const_vertex_vertex_view_iterator<G> i0; // default construction

    auto& u = g2[frankfurt_key];

    std::graph::views::const_vertex_vertex_view_iterator<G> i1(g2, u);
    {
      auto&& [vkey, v] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(is_const_v<remove_reference_t<decltype(v)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, v] = *++i1;
      REQUIRE(vkey == 5);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::views::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("adjacency_edge_view") {
    std::graph::views::vertex_vertex_view_iterator<G> i0; // default construction

    auto& u = g[frankfurt_key];

    std::graph::views::vertex_vertex_view_iterator<G> i1(g, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, v] = *++i1;
      REQUIRE(vkey == 5);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::views::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }
#endif

  SECTION("content") {
    std::string_view test_name = "Germany Routes using csr_graph";
#if TEST_OPTION == TEST_OPTION_OUTPUT
    cout << "\n" << test_name << "\n----------------------------------------" << endl << routes_graph(g) << endl;
    int x = 0; // results are identifcal with csv_routes_dov_tests

    //Germany Routes using csr_graph
    //----------------------------------------
    //[0 Frankfürt]
    //  --> [1 Mannheim] 85km
    //  --> [4 Würzburg] 217km
    //  --> [6 Kassel] 173km
    //[1 Mannheim]
    //  --> [2 Karlsruhe] 80km
    //[2 Karlsruhe]
    //  --> [3 Augsburg] 250km
    //[3 Augsburg]
    //  --> [8 München] 84km
    //[4 Würzburg]
    //  --> [5 Nürnberg] 103km
    //  --> [7 Erfurt] 186km
    //[5 Nürnberg]
    //  --> [8 München] 167km
    //  --> [9 Stuttgart] 183km
    //[6 Kassel]
    //  --> [8 München] 502km
    //[7 Erfurt]
    //[8 München]
    //[9 Stuttgart]
#elif TEST_OPTION == TEST_OPTION_GEN
    generate_routes_tests(g, test_name);
    int x = 0;
#elif TEST_OPTION == TEST_OPTION_TEST
    auto            ui   = begin(vertices(g));
    vertex_key_t<G> ukey = 0;
    if (ui != end(vertices(g))) {
      REQUIRE(0 == ukey);
      REQUIRE("Frankf\xc3\xbcrt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(1 == target_key(g, *uvi));
      REQUIRE("Mannheim" == vertex_value(g, target(g, *uvi)));
      REQUIRE(85 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(4 == target_key(g, *uvi));
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(217 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(6 == target_key(g, *uvi));
      REQUIRE("Kassel" == vertex_value(g, target(g, *uvi)));
      REQUIRE(173 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(3 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(1 == ++ukey);
      REQUIRE("Mannheim" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(2 == target_key(g, *uvi));
      REQUIRE("Karlsruhe" == vertex_value(g, target(g, *uvi)));
      REQUIRE(80 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(2 == ++ukey);
      REQUIRE("Karlsruhe" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(3 == target_key(g, *uvi));
      REQUIRE("Augsburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(250 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(3 == ++ukey);
      REQUIRE("Augsburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(84 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(4 == ++ukey);
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(5 == target_key(g, *uvi));
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(103 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(7 == target_key(g, *uvi));
      REQUIRE("Erfurt" == vertex_value(g, target(g, *uvi)));
      REQUIRE(186 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(5 == ++ukey);
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(167 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(9 == target_key(g, *uvi));
      REQUIRE("Stuttgart" == vertex_value(g, target(g, *uvi)));
      REQUIRE(183 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(6 == ++ukey);
      REQUIRE("Kassel" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(502 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(7 == ++ukey);
      REQUIRE("Erfurt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(8 == ++ukey);
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(9 == ++ukey);
      REQUIRE("Stuttgart" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }

    REQUIRE(10 == size(vertices(g))); // all vertices visited?
#endif
  }
}
