#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/container/compressed_graph.hpp"

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using graph::vertex_t;
using graph::vertex_id_t;
using graph::vertex_value_t;
using graph::vertex_edge_range_t;
using graph::vertex_reference_t;
using graph::edge_t;
using graph::edge_value_t;
using graph::edge_reference_t;

using graph::graph_value;
using graph::vertices;
using graph::edges;
using graph::vertex_id;
using graph::vertex_value;
using graph::target_id;
using graph::target;
using graph::edge_value;
using graph::degree;
using graph::find_vertex;
using graph::find_vertex_edge;

using graph::views::neighbors;

using routes_compressed_graph_type = graph::container::compressed_graph<double, std::string, std::string>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

// Things to test
//  compressed_graph with VV=void (does it compile?)
//  push_back and emplace_back work correctly when adding city names (applies to compressed_graph & dynamic_graph)

TEST_CASE("neighbors test", "[csr][neighbors]") {

  init_console();

  using G  = routes_compressed_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
  // name_order_policy::source_order_found gives best output with least overlap for germany routes

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("non-const neighbor_iterator") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    REQUIRE(frankfurt);
    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;

    graph::neighbor_iterator<G> i0; // default construction
    graph::neighbor_iterator<G> i1(g, uid);
    static_assert(std::is_move_assignable_v<decltype(i0)>, "neighbor_iterator must be move_assignable");
    static_assert(std::is_copy_assignable_v<decltype(i0)>, "neighbor_iterator must be copy_assignable");
    static_assert(std::input_or_output_iterator<decltype(i1)>, "neighbor_iterator must be an input_output_iterator");
    static_assert(std::input_iterator<decltype(i1)>, "neighbor_iterator must be an input_iterator");
    static_assert(std::forward_iterator<decltype(i1)>, "neighbor_iterator must be a forward_iterator");
    {
      auto& [vid, v] = *i1;
      auto&& val     = *i1;
      static_assert(std::is_same_v<decltype(val), graph::neighbor_iterator<G>::reference>);
      static_assert(is_const_v<decltype(vid)>, "vertex id must be const");
      static_assert(!is_const_v<remove_reference_t<decltype(v)>>, "neighbor must be non-const");
      REQUIRE(vid == 1);
      REQUIRE(vertex_value(g, v) == "Mannheim");
    }
    {
      auto&& [vid, v] = *++i1;
      REQUIRE(vid == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
      REQUIRE(vertex_value(g, v) == "W\xc3\xbcrzburg");
    }

    graph::neighbor_iterator<G> i2(g, uid);
    {
      auto&& [vid, v] = *i2;
      static_assert(is_const_v<decltype(vid)>, "vertex id must be const");
      static_assert(std::is_lvalue_reference_v<decltype(v)>, "neighbor must be lvalue reference");
      static_assert(!is_const_v<remove_reference_t<decltype(v)>>, "neighbore must be non-const");
      REQUIRE(vid == 1);
      REQUIRE(vertex_value(g, v) == "Mannheim");
    }
    {
      auto&& [vid, v] = *++i2;
      REQUIRE(vid == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
      REQUIRE(vertex_value(g, v) == "W\xc3\xbcrzburg");
    }

    static_assert(std::input_or_output_iterator<decltype(i1)>);
    using _It = graph::neighbor_iterator<G>;
    using _Se = graph::vertex_iterator_t<G>;
    bool yy   = std::sentinel_for<_Se, _It>;
    bool xx   = std::sized_sentinel_for<_Se, _It>;
    static_assert(std::sized_sentinel_for<_Se, _It> == false);
    auto _Ki =
          std::sized_sentinel_for<_Se, _It> ? std::ranges::subrange_kind::sized : std::ranges::subrange_kind::unsized;

    auto vvf = [&g](vertex_reference_t<G> uu) -> std::string& { //
      return vertex_value(g, uu);
    };
    using VVF = decltype(vvf);

    graph::neighbor_iterator<G, false, VVF> i3(g, uid, vvf);
    {
      // The following asserts are used to isolate problem with failing input_or_output_iterator concept for neighbor_iterator
      static_assert(std::movable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT movable");
      static_assert(std::default_initializable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT default_initializable");
      //static_assert(std::__detail::__is_signed_integer_like<std::iter_difference_t<decltype(i3)>>, "neighbor_iterator<G,VVF> is NOT __is_signed_integer_like");
      static_assert(std::weakly_incrementable<decltype(i3)>, "neighbor_iterator<G,VVF> is NOT weakly_incrementable");
      static_assert(std::input_or_output_iterator<decltype(i3)>,
                    "neighbor_iterator<G,VVF> is NOT an input_or_output_iterator");

      auto&& [vid, v, name] = *i3;
      auto&& x              = *i3;
      REQUIRE(vid == 1);
      REQUIRE(name == "Mannheim");
    }
    {
      auto&& [vid, v, name] = *++i3;
      REQUIRE(vid == 4);
      REQUIRE(name == "W\xc3\xbcrzburg");
    }

    //graph::views::neighbor_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const neighbor_iterator") {
    using G2 = const G;
    G2& g2   = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>, "graph must be const");

    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;

    //graph::views::neighbor_iterator<G2> i0; // default construction
    graph::neighbor_iterator<G2, false> i1(g2, uid);
    static_assert(std::forward_iterator<decltype(i1)>, "neighbor_iterator must be a forward_iterator");
    {
      auto&& [vid, v] = *i1;

      vertex_reference_t<G2> v2 = v;
      static_assert(is_const_v<remove_reference_t<decltype(v2)>>, "neighbor must be const");

      static_assert(is_const_v<decltype(vid)>, "id must be const");
      static_assert(std::is_lvalue_reference_v<decltype(v)>, "neighbor must be lvalue reference");
      static_assert(is_const_v<remove_reference_t<decltype(v)>>, "neighbor must be const");
      REQUIRE(vid == 1);
    }
    {
      auto&& [vid, uv] = *++i1;
      REQUIRE(vid == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    graph::neighbor_iterator<G2, false> i2(g2, uid);
    {
      auto&& [vid, uv] = *i2;
      static_assert(is_const_v<decltype(vid)>, "id must be const");
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>, "edge must be const");
      REQUIRE(vid == 1);
    }
    {
      auto&& [vid, uv] = *++i2;
      REQUIRE(vid == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    auto vvf  = [&g2](vertex_reference_t<G2> v) -> const std::string& { return vertex_value(g2, v); };
    using VVF = decltype(vvf);
    graph::neighbor_iterator<G2, false, VVF> i3(g2, uid, vvf);
    {
      auto&& [vid, v, name] = *i3;
      REQUIRE(vid == 1);
      REQUIRE(name == "Mannheim");
    }
    {
      auto&& [vid, v, name] = *++i3;
      REQUIRE(vid == 4);
      REQUIRE(name == "W\xc3\xbcrzburg");
    }
  }

  SECTION("non-const neighbors") {
    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;
    using view_t              = decltype(graph::views::neighbors(g, uid));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vid, v] : graph::views::neighbors(g, uid)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }

  SECTION("const neighbors") {
    using G2                  = const G;
    G2&                   g2  = g;
    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;

    using view_t = decltype(graph::views::neighbors(g2, uid));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vid, v] : graph::views::neighbors(g2, uid)) {
      ++cnt;
    }
    // descriptor's implicit conversion to id_type is evaluated, and then calling into edges(g,id) causing
    //   warning C4244: 'argument': conversion from 'graph::descriptor<_It>::id_type' to 'const unsigned int', possible loss of data
    // Consider removing edges(g,id), or refactor to use explicit conversion
    REQUIRE(cnt == size(edges(g2, u)));
  }

  SECTION("non-const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;
    size_t                cnt = 0;
    auto                  vvf = [&g](vertex_reference_t<G> v) -> std::string& { return vertex_value(g, v); };
    for (auto&& [vid, v, val] : graph::views::neighbors(g, uid, vvf)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }
  SECTION("const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    using G2                  = const G;
    G2&                   g2  = g;
    vertex_reference_t<G> u   = **frankfurt;
    vertex_id_t<G>        uid = frankfurt_id;
    auto   edge_fn            = [&g2](vertex_reference_t<G2> v) -> const std::string& { return vertex_value(g2, v); };
    size_t cnt                = 0;
    for (auto&& [vid, uv, val] : graph::views::neighbors(g2, uid, edge_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g2, u)));
  }

  SECTION("neighbors is a forward view") {
    auto nlist = neighbors(g, frankfurt_id);
    auto it1   = nlist.begin();
    using I    = decltype(it1);

    auto it2 = it1;            // copyable
    I    it3(it1);             // copy-constuctible
    auto it4 = std::move(it2); // movable
    I    it5(std::move(it3));  // move-constuctible
    I    it6;                  // default-constructible

    using I2 = std::remove_cvref_t<I>;
    REQUIRE(std::move_constructible<I2>);
    REQUIRE(std::copyable<I2>);
    REQUIRE(std::movable<I2>);
    REQUIRE(std::swappable<I2>);
    CHECK(std::is_default_constructible_v<I2>);

    CHECK(std::input_or_output_iterator<I2>);
    CHECK(std::indirectly_readable<I2>);
    CHECK(std::input_iterator<I2>);
    CHECK(std::forward_iterator<I2>);

    using Rng = decltype(nlist);
    CHECK(std::ranges::range<Rng>);
    CHECK(std::movable<Rng>);
    //CHECK(std::derived_from<Rng, std::ranges::view_base>);
    CHECK(std::ranges::enable_view<Rng>);
    CHECK(std::ranges::view<decltype(nlist)>);

    auto it8  = std::ranges::begin(nlist);
    auto it9  = std::ranges::end(nlist);
    auto empt = std::ranges::empty(nlist);
  }
}
