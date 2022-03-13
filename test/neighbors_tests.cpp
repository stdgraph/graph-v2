#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/container/csr_graph.hpp"

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using std::graph::vertex_t;
using std::graph::vertex_key_t;
using std::graph::vertex_value_t;
using std::graph::vertex_edge_range_t;
using std::graph::vertex_reference_t;
using std::graph::edge_t;
using std::graph::edge_value_t;
using std::graph::edge_reference_t;

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

TEST_CASE("neighbors test", "[csr][neighbors]") {

  init_console();

  using G  = routes_csr_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
  // name_order_policy::source_order_found gives best output with least overlap for germany routes

  auto frankfurt     = find_frankfurt(g);
  auto frankfurt_key = find_frankfurt_key(g);

  SECTION("non-const neighbor_iterator") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    REQUIRE(frankfurt);
    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;

    std::graph::views::neighbor_iterator<G> i0; // default construction
    std::graph::views::neighbor_iterator<G> i1(g, ukey);
    static_assert(std::is_move_assignable_v<decltype(i0)>, "neighbor_iterator must be move_assignable");
    static_assert(std::is_copy_assignable_v<decltype(i0)>, "neighbor_iterator must be copy_assignable");
    static_assert(std::input_or_output_iterator<decltype(i1)>, "neighbor_iterator must be an input_output_iterator");
    static_assert(std::input_iterator<decltype(i1)>, "neighbor_iterator must be an input_iterator");
    static_assert(std::forward_iterator<decltype(i1)>, "neighbor_iterator must be a forward_iterator");
    {
      auto& [vkey, v] = *i1;
      auto&& val      = *i1;
      static_assert(std::is_same_v<decltype(val), std::graph::views::neighbor_iterator<G>::reference>);
      static_assert(is_const_v<decltype(vkey)>, "vertex key must be const");
      static_assert(!is_const_v<remove_reference_t<decltype(v)>>, "neighbor must be non-const");
      REQUIRE(vkey == 1);
      REQUIRE(vertex_value(g, v) == "Mannheim");
    }
    {
      auto&& [vkey, v] = *++i1;
      REQUIRE(vkey == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
      REQUIRE(vertex_value(g, v) == "W\xc3\xbcrzburg");
    }

    std::graph::views::neighbor_iterator<G> i2(g, ukey);
    {
      auto&& [vkey, v] = *i2;
      static_assert(is_const_v<decltype(vkey)>, "vertex key must be const");
      static_assert(is_lvalue_reference_v<decltype(v)>, "neighbor must be lvalue reference");
      static_assert(!is_const_v<remove_reference_t<decltype(v)>>, "neighbore must be non-const");
      REQUIRE(vkey == 1);
      REQUIRE(vertex_value(g, v) == "Mannheim");
    }
    {
      auto&& [vkey, v] = *++i2;
      REQUIRE(vkey == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
      REQUIRE(vertex_value(g, v) == "W\xc3\xbcrzburg");
    }

    static_assert(std::input_or_output_iterator<decltype(i1)>);
    using _It = std::graph::views::neighbor_iterator<G>;
    using _Se = std::graph::vertex_iterator_t<G>;
    bool yy   = std::sentinel_for<_Se, _It>;
    bool xx   = std::sized_sentinel_for<_Se, _It>;
    static_assert(std::sized_sentinel_for<_Se, _It> == false);
    auto _Ki =
          std::sized_sentinel_for<_Se, _It> ? std::ranges::subrange_kind::sized : std::ranges::subrange_kind::unsized;

    auto vvf = [&g](vertex_reference_t<G> uu) -> std::string& { //
      return vertex_value(g, uu);
    };
    using VVF = decltype(vvf);

    std::graph::views::neighbor_iterator<G, false, VVF> i3(g, ukey, vvf);
    {
      // The following asserts are used to isolate problem with failing input_or_output_iterator concept for neighbor_iterator
      static_assert(std::movable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT movable");
      static_assert(std::default_initializable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT default_initializable");
      //static_assert(std::__detail::__is_signed_integer_like<std::iter_difference_t<decltype(i3)>>, "neighbor_iterator<G,VVF> is NOT __is_signed_integer_like");
      static_assert(std::weakly_incrementable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT weakly_incrementable");
      static_assert(std::input_or_output_iterator<decltype(i3)>,
                    "neighbor_iterator<G,VVF> is NOT an input_or_output_iterator");

      auto&& [vkey, v, name] = *i3;
      auto&& x               = *i3;
      REQUIRE(vkey == 1);
      REQUIRE(name == "Mannheim");
    }
    {
      auto&& [vkey, v, name] = *++i3;
      REQUIRE(vkey == 4);
      REQUIRE(name == "W\xc3\xbcrzburg");
    }

    //std::graph::views::neighbor_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const neighbor_iterator") {
    using G2 = const G;
    G2& g2   = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>, "graph must be const");

    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;

    //std::graph::views::neighbor_iterator<G2> i0; // default construction
    std::graph::views::neighbor_iterator<G2, false> i1(g2, ukey);
    static_assert(std::forward_iterator<decltype(i1)>, "neighbor_iterator must be a forward_iterator");
    {
      auto&& [vkey, v] = *i1;

      vertex_reference_t<G2> v2 = v;
      static_assert(is_const_v<remove_reference_t<decltype(v2)>>, "neighbor must be const");

      static_assert(is_const_v<decltype(vkey)>, "key must be const");
      static_assert(is_lvalue_reference_v<decltype(v)>, "neighbor must be lvalue reference");
      static_assert(is_const_v<remove_reference_t<decltype(v)>>, "neighbor must be const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    std::graph::views::neighbor_iterator<G2, false> i2(g2, ukey);
    {
      auto&& [vkey, uv] = *i2;
      static_assert(is_const_v<decltype(vkey)>, "key must be const");
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>, "edge must be const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i2;
      REQUIRE(vkey == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    auto vvf  = [&g2](vertex_reference_t<G2> v) -> const std::string& { return vertex_value(g2, v); };
    using VVF = decltype(vvf);
    std::graph::views::neighbor_iterator<G2, false, VVF> i3(g2, ukey, vvf);
    {
      auto&& [vkey, v, name] = *i3;
      REQUIRE(vkey == 1);
      REQUIRE(name == "Mannheim");
    }
    {
      auto&& [vkey, v, name] = *++i3;
      REQUIRE(vkey == 4);
      REQUIRE(name == "W\xc3\xbcrzburg");
    }
  }

  SECTION("non-const neighbors") {
    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;
    using view_t               = decltype(std::graph::views::neighbors(g, ukey));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::views::neighbors(g, ukey)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }

  SECTION("const neighbors") {
    using G2                   = const G;
    G2&                   g2   = g;
    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;

    using view_t = decltype(std::graph::views::neighbors(g2, ukey));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vkey, v] : std::graph::views::neighbors(g2, ukey)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g2, u)));
  }

  SECTION("non-const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;
    size_t                cnt  = 0;
    auto                  vvf  = [&g](vertex_reference_t<G> v) -> std::string& { return vertex_value(g, v); };
    for (auto&& [vkey, v, val] : std::graph::views::neighbors(g, ukey, vvf)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }
  SECTION("const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    using G2                   = const G;
    G2&                   g2   = g;
    vertex_reference_t<G> u    = **frankfurt;
    vertex_key_t<G>       ukey = frankfurt_key;
    auto   edge_fn             = [&g2](vertex_reference_t<G2> v) -> const std::string& { return vertex_value(g2, v); };
    size_t cnt                 = 0;
    for (auto&& [vkey, uv, val] : std::graph::views::neighbors(g2, ukey, edge_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g2, u)));
  }
}
