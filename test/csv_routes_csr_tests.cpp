#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/view/vertices_view.hpp"
#include "graph/view/incidence_edge_view.hpp"
#include "graph/view/adjacency_edge_view.hpp"
#include "graph/container/csr_graph.hpp"
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_OUTPUT

using std::cout;
using std::endl;

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;

using std::graph::vertex_t;
using std::graph::vertex_key_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_key;
using std::graph::target;
using std::graph::edge_value;


using routes_csr_graph_type   = std::graph::container::csr_graph<double, std::string>;

template <typename G>
constexpr auto find_frankfurt_key(const G& g) {
  return find_city_key(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

TEST_CASE("Germany routes CSV+csr test", "[csv][csr][germany]") {
  init_console();

  using G  = routes_csr_graph_type;
#if 0
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  auto frankfurt     = find_frankfurt(g);
  auto frankfurt_key = find_frankfurt_key(g);

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  SECTION("metadata") {
    REQUIRE(10 == std::ranges::size(vertices(g)));
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

  SECTION("const_vertices_view") {
    const G& g2 = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>);

    std::graph::view::const_vertices_view_iterator<G> i0; // default construction
    std::graph::view::const_vertices_view_iterator<G> i1(g);
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

    std::graph::view::const_vertices_view_iterator<G> i2(g2);
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

    using view_t = decltype(std::graph::view::vertices_view(g2));
    static_assert(forward_range<view_t>);
    size_t cnt = 0;
    for (auto&& [ukey, u] : std::graph::view::vertices_view(g2)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }

  SECTION("non_const_vertices_view") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    std::graph::view::vertices_view_iterator<G> i0; // default construction
    std::graph::view::vertices_view_iterator<G> i1(g);
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

    std::graph::view::vertices_view_iterator<G> i2(g);
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
    for (auto&& [ukey, u] : std::graph::view::vertices_view(g)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    std::graph::view::const_vertices_view_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const_incidence_edge_view") {
    const G& g2 = g;

    std::graph::view::const_vertex_edge_view_iterator<G> i0; // default construction

    auto& u = g2[frankfurt_key];

    std::graph::view::const_vertex_edge_view_iterator<G> i1(g2, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 9);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::view::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("incidence_edge_view") {
    std::graph::view::vertex_edge_view_iterator<G> i0; // default construction

    auto& u = g[frankfurt_key];

    std::graph::view::vertex_edge_view_iterator<G> i1(g, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 9);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::view::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("const_adjacency_edge_view") {
    const G& g2 = g;

    std::graph::view::const_vertex_vertex_view_iterator<G> i0; // default construction

    auto& u = g2[frankfurt_key];

    std::graph::view::const_vertex_vertex_view_iterator<G> i1(g2, u);
    {
      auto&& [vkey, v] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(is_const_v<remove_reference_t<decltype(v)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, v] = *++i1;
      REQUIRE(vkey == 9);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::view::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("adjacency_edge_view") {
    std::graph::view::vertex_vertex_view_iterator<G> i0; // default construction

    auto& u = g[frankfurt_key];

    std::graph::view::vertex_vertex_view_iterator<G> i1(g, u);
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>);
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>);
      REQUIRE(vkey == 4);
    }
    {
      auto&& [vkey, v] = *++i1;
      REQUIRE(vkey == 9);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::view::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("content") {
#if TEST_OPTION == TEST_OPTION_OUTPUT
    cout << "\nGermany Routes using vector+forward_list"
         << "\n----------------------------------------" << endl
         << routes_graph(g) << endl;
#elif TEST_OPTION == TEST_OPTION_GEN
    ostream_indenter indent;
    cout << endl << indent << "auto ui = begin(vertices(g));" << endl;
    cout << indent << "vertex_key_t<G> ukey = 0;" << endl;
    for (vertex_key_t<G> ukey = 0; auto&& u : vertices(g)) {

      if (ukey > 0) {
        cout << indent << "if(++ui != end(vertices(g))) {" << endl;
      } else {
        cout << indent << "if(ui != end(vertices(g))) {" << endl;
      }
      ++indent;
      {
        if (ukey > 0)
          cout << indent << "REQUIRE(" << ukey << " == ++ukey);" << endl;
        else
          cout << indent << "REQUIRE(" << ukey << " == ukey);" << endl;

        size_t uv_cnt = 0;
        cout << indent << "REQUIRE(\"" << quoted_utf8(vertex_value(g, u)) << "\" == vertex_value(g,*ui));" << endl;
        cout << endl << indent << "auto uvi = begin(edges(g, *ui)); size_t uv_cnt = 0;" << endl;
        for (auto&& uv : edges(g, u)) {
          if (uv_cnt > 0) {
            cout << endl << indent << "++uvi;" << endl;
          }
          auto&& v = target(g, uv);
          cout << indent << "REQUIRE(" << target_key(g, uv) << " == target_key(g, *uvi));\n";
          cout << indent << "REQUIRE(\"" << quoted_utf8(vertex_value(g, target(g, uv)))
               << "\" == vertex_value(g, target(g, *uvi)));\n";
          cout << indent << "REQUIRE(" << edge_value(g, uv) << " == edge_value(g,*uvi));\n";
          cout << indent << "++uv_cnt;" << endl;
          ++uv_cnt;
        }
        cout << endl << indent << "REQUIRE(" << uv_cnt << " == uv_cnt);" << endl;
      }
      cout << "}" << endl;
      --indent;
      ++ukey;
    }

    cout << endl
         << indent << "REQUIRE(" << size(vertices(g)) << " == size(vertices(g))); // all vertices visited?" << endl;
#elif TEST_OPTION == TEST_OPTION_TEST

    auto            ui   = begin(vertices(g));
    vertex_key_t<G> ukey = 0;
    if (ui != end(vertices(g))) {
      REQUIRE(0 == ukey);
      REQUIRE("Augsburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(6 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(84 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(1 == ++ukey);
      REQUIRE("Erfurt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(2 == ++ukey);
      REQUIRE("Frankf\xc3\xbcrt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(4 == target_key(g, *uvi));
      REQUIRE("Kassel" == vertex_value(g, target(g, *uvi)));
      REQUIRE(173 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(9 == target_key(g, *uvi));
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(217 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(5 == target_key(g, *uvi));
      REQUIRE("Mannheim" == vertex_value(g, target(g, *uvi)));
      REQUIRE(85 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(3 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(3 == ++ukey);
      REQUIRE("Karlsruhe" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(0 == target_key(g, *uvi));
      REQUIRE("Augsburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(250 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(4 == ++ukey);
      REQUIRE("Kassel" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(6 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(502 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(5 == ++ukey);
      REQUIRE("Mannheim" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(3 == target_key(g, *uvi));
      REQUIRE("Karlsruhe" == vertex_value(g, target(g, *uvi)));
      REQUIRE(80 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(6 == ++ukey);
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(7 == ++ukey);
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(6 == target_key(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(167 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(8 == target_key(g, *uvi));
      REQUIRE("Stuttgart" == vertex_value(g, target(g, *uvi)));
      REQUIRE(183 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(8 == ++ukey);
      REQUIRE("Stuttgart" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(9 == ++ukey);
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(7 == target_key(g, *uvi));
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(103 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(1 == target_key(g, *uvi));
      REQUIRE("Erfurt" == vertex_value(g, target(g, *uvi)));
      REQUIRE(186 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }

    REQUIRE(10 == size(vertices(g))); // all vertices visited?
#endif
  }
#endif //0
}

