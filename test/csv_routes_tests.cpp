#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/dijkstra_book.hpp"
#include "graph/view/vertices_view.hpp"
#include "graph/view/incidence_edge_view.hpp"
#include "graph/view/adjacency_edge_view.hpp"
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
using std::graph::vertex_key_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_key;
using std::graph::target;
using std::graph::edge_value;

void utf8_append(std::string& out, const char ch) {
  if ((ch & 0x80) == 0)
    out += ch;
  else {
    const char* digits = "0123456789abcdef";
    out += "\\x";
    int lsb = ch & 0x0f;
    int msb = (ch >> 4) & 0xf;
    out += digits[msb];
    out += digits[lsb];
  }
}

// create a string that can be pasted into the source code
std::string quoted_utf8(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (auto ch : s)
    utf8_append(out, ch);
  return out;
}

std::string quoted_utf8(const std::string_view& s) {
  std::string out;
  out.reserve(s.size());
  for (auto ch : s)
    utf8_append(out, ch);
  return out;
}

std::string quoted_utf8(const char* s) {
  std::string out;
  for (const char* ch = s; ch && *ch; ++ch)
    utf8_append(out, *ch);
  return out;
}

class ostream_indenter {
public:
  ostream_indenter(int level) : level_(level) {}
  ostream_indenter()                        = default;
  ostream_indenter(const ostream_indenter&) = default;
  ~ostream_indenter()                       = default;

  ostream_indenter& operator=(const ostream_indenter&) = default;

  int level() const { return level_; }

  ostream_indenter& operator++() {
    ++level_;
    return *this;
  }
  ostream_indenter operator++(int) {
    ostream_indenter tmp(*this);
    ++*this;
    return tmp;
  }
  ostream_indenter& operator--() {
    --level_;
    return *this;
  }
  ostream_indenter operator--(int) {
    ostream_indenter tmp(*this);
    --*this;
    return tmp;
  }

private:
  int level_ = 0;
};
template <typename OStream>
OStream& operator<<(OStream& os, const ostream_indenter& osi) {
  for (int i = 0; i < osi.level(); ++i)
    os << "  ";
  return os;
}

using routes_volf_graph_traits = std::graph::container::vofl_graph_traits<double, std::string>;
using routes_volf_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_volf_graph_traits>;

//TEST_CASE("Germany routes CSV+csr test", "[csv][csr]") {
//  init_console();
//  routes_csv_csr_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
//}

template<typename G>
constexpr auto find_frankfurt_key(const G& g) {
  return find_city_key(g, "Frankf\xC3\xBCrt");
}

template<typename G>
auto find_frankfurt(G&& g) { return find_city(g, "Frankf\xC3\xBCrt"); }


TEST_CASE("Germany routes CSV+vol dijkstra_book", "[csv][vol][germany][dijkstra][book]") {
  init_console();
  using G = routes_volf_graph_type;
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  auto frankfurt     = find_frankfurt(g);
  auto frankfurt_key = find_frankfurt_key(g);
  auto weight        = [&g](std::ranges::range_reference_t<vertex_edge_range_t<G>> uv) { return edge_value(g, uv); };
  auto result        = std::graph::dijkstra_book(g, frankfurt_key, weight);
}


TEST_CASE("Germany routes CSV+vol test", "[csv][vol][germany]") {
  init_console();
  using G  = routes_volf_graph_type;
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

    std::graph::const_vertices_view_iterator<G> i0; // default construction
    std::graph::const_vertices_view_iterator<G> i1(g);
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

    std::graph::const_vertices_view_iterator<G> i2(g2);
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

    using view_t = decltype(std::graph::vertices_view(g2));
    static_assert(forward_range<view_t>);
    size_t cnt = 0;
    for (auto&& [ukey, u] : std::graph::vertices_view(g2)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }

  SECTION("non_const_vertices_view") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    std::graph::vertices_view_iterator<G> i0; // default construction
    std::graph::vertices_view_iterator<G> i1(g);
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

    std::graph::vertices_view_iterator<G> i2(g);
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
    for (auto&& [ukey, u] : std::graph::vertices_view(g)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    std::graph::const_vertices_view_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const_incidence_edge_view") {
    const G& g2 = g;

    std::graph::const_vertex_edge_view_iterator<G> i0; // default construction

    auto& u             = g2[frankfurt_key];

    std::graph::const_vertex_edge_view_iterator<G> i1(g2, u);
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
    for (auto&& [vkey, uv] : std::graph::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("incidence_edge_view") {
    std::graph::vertex_edge_view_iterator<G> i0; // default construction

    auto& u             = g[frankfurt_key];

    std::graph::vertex_edge_view_iterator<G> i1(g, u);
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
    for (auto&& [vkey, uv] : std::graph::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("const_adjacency_edge_view") {
    const G& g2 = g;

    std::graph::const_vertex_vertex_view_iterator<G> i0; // default construction

    auto& u             = g2[frankfurt_key];

    std::graph::const_vertex_vertex_view_iterator<G> i1(g2, u);
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
    for (auto&& [vkey, v] : std::graph::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("adjacency_edge_view") {
    std::graph::vertex_vertex_view_iterator<G> i0; // default construction

    auto& u             = g[frankfurt_key];

    std::graph::vertex_vertex_view_iterator<G> i1(g, u);
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
    for (auto&& [vkey, v] : std::graph::edges_view(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == 3);
  }

  SECTION("content") {
#if TEST_OPTION == TEST_OPTION_OUTPUT
    cout << "\nGermany Routes"
         << "\n-------------------------------" << routes_graph(g) << endl;
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
}
