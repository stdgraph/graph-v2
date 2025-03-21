﻿#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/container/compressed_graph.hpp"
#include <concepts>

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using graph::vertex_t;
using graph::vertex_id_t;
using graph::vertex_value_t;
using graph::vertex_reference_t;
using graph::vertex_edge_range_t;
using graph::edge_t;
using graph::edge_value_t;

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
using graph::views::vertexlist;

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

TEST_CASE("vertexlist test", "[csr][vertexlist]") {

  init_console();

  using G = routes_compressed_graph_type;
  auto g  = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
  // name_order_policy::source_order_found gives best output with least overlap for germany routes

  const auto frankfurt    = find_frankfurt(g);
  const auto frankfurt_id = find_frankfurt_id(g);

  SECTION("non-const vertexlist_iterator") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    graph::vertexlist_iterator<G> i0; // default construction
    graph::vertexlist_iterator<G> i1(g);
    static_assert(std::forward_iterator<decltype(i1)>, "vertexlist_iterator must be a forward_iterator");
    {
      auto&& [uid, u] = *i1;
      static_assert(is_const_v<decltype(uid)>, "vertex id must be const");
      static_assert(!is_const_v<remove_reference_t<decltype(u)>>, "vertex must be non-const");
      REQUIRE(uid == 0);
    }
    {
      auto&& [uid, u] = *++i1;
      REQUIRE(uid == 1);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    graph::vertexlist_iterator<G> i2(g);
    {
      auto&& [uid, u] = *i2;
      static_assert(is_const_v<decltype(uid)>, "vertex id must be const");
      static_assert(std::is_lvalue_reference_v<decltype(u)>, "vertex must be lvalue reference");
      static_assert(!is_const_v<remove_reference_t<decltype(u)>>, "vertex must be non-const");
      REQUIRE(uid == 0);
    }
    {
      auto&& [uid, u] = *++i2;
      REQUIRE(uid == 1);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    static_assert(std::input_or_output_iterator<decltype(i1)>);
    using _It = graph::vertexlist_iterator<G>;
    using _Se = graph::vertex_iterator_t<G>;
    bool yy   = std::sentinel_for<_Se, _It>;
    bool xx   = std::sized_sentinel_for<_Se, _It>;
    static_assert(std::sized_sentinel_for<_Se, _It> == false);
    auto _Ki =
          std::sized_sentinel_for<_Se, _It> ? std::ranges::subrange_kind::sized : std::ranges::subrange_kind::unsized;

    auto vvf  = [&g](vertex_t<G>& u) -> std::string& { return vertex_value(g, u); };
    using VVF = decltype(vvf);
    graph::vertexlist_iterator<G, VVF> i3(g, vvf, begin(vertices(g)));
    {
      // The following asserts are used to isolate problem with failing input_or_output_iterator concept for vertexlist_iterator
      static_assert(std::movable<decltype(i3)>, "vertexlist_iterator<G,VVF> is NOT movable");
      static_assert(std::default_initializable<decltype(i3)>,
                    "vertexlist_iterator<G,VVF> is NOT default_initializable");
      //static_assert(std::__detail::__is_signed_integer_like<std::iter_difference_t<decltype(i3)>>, "vertexlist_iterator<G,VVF> is NOT __is_signed_integer_like");
      static_assert(std::weakly_incrementable<decltype(i3)>, "vertexlist_iterator<G,VVF> is NOT weakly_incrementable");
      static_assert(std::input_or_output_iterator<decltype(i3)>,
                    "vertexlist_iterator<G,VVF> is NOT an input_or_output_iterator");

      auto&& [uid, u, name] = *i3;
      REQUIRE(uid == 0);
      REQUIRE(name == "Frankf\xC3\xBCrt");
    }
    {
      auto&& [uid, u, name] = *++i3;
      REQUIRE(uid == 1);
      REQUIRE(name == "Mannheim");
    }

    //graph::views::vertexlist_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const vertexlist_iterator") {
    using G2 = const G;
    G2& g2   = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>, "graph must be const");

    //graph::views::vertexlist_iterator<G2> i0; // default construction
    graph::vertexlist_iterator<G2> i1(g2);
    static_assert(std::forward_iterator<decltype(i1)>, "vertexlist_iterator must be a forward_iterator");
    {
      auto&& [uid, u] = *i1;

      static_assert(is_const_v<decltype(uid)>, "id must be const");
      static_assert(std::is_lvalue_reference_v<decltype(u)>, "vertex must be lvalue reference");
      static_assert(is_const_v<remove_reference_t<decltype(u)>>, "vertex must be const");
      REQUIRE(uid == 0);
    }
    {
      auto&& [uid, u] = *++i1;
      REQUIRE(uid == 1);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    graph::vertexlist_iterator<G2> i2(g2);
    {
      auto&& [uid, u] = *i2;
      static_assert(is_const_v<decltype(uid)>, "id must be const");
      static_assert(is_const_v<remove_reference_t<decltype(u)>>, "vertex must be const");
      REQUIRE(uid == 0);
    }
    {
      auto&& [uid, u] = *++i2;
      REQUIRE(uid == 1);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    auto vvf  = [&g](vertex_t<G>& u) -> std::string& { return vertex_value(g, u); };
    using VVF = decltype(vvf);
    graph::vertexlist_iterator<G, VVF> i3(g, vvf, begin(vertices(g)));
    {
      auto&& [uid, u, name] = *i3;
      REQUIRE(uid == 0);
      REQUIRE(name == "Frankf\xC3\xBCrt");
    }
    {
      auto&& [uid, u, name] = *++i3;
      REQUIRE(uid == 1);
      REQUIRE(name == "Mannheim");
    }
  }

  SECTION("non-const vertexlist") {
    using view_t = decltype(graph::views::vertexlist(g));
    static_assert(forward_range<view_t>, "vertexlist(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [uid, u] : graph::views::vertexlist(g)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    cnt = 0;
    for (auto&& [uid, u] : graph::views::vertexlist(g, vertices(g))) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }

  SECTION("const vertexlist") {
    using G2     = const G;
    G2& g2       = g;
    using view_t = decltype(graph::views::vertexlist(g2));
    static_assert(forward_range<view_t>, "vertexlist(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [uid, u] : graph::views::vertexlist(g2)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    cnt = 0;
    for (auto&& [uid, u] : graph::views::vertexlist(g2, vertices(g2))) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }

  SECTION("non-const vertexlist with vertex_fn") {
    // Note: must include trailing return type on lambda
    size_t cnt       = 0;
    auto   vertex_fn = [&g](vertex_reference_t<G> u) -> std::string& { return vertex_value(g, u); };
    for (auto&& [uid, u, val] : graph::views::vertexlist(g, vertex_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));

    cnt = 0;
    for (auto&& [uid, u, val] : graph::views::vertexlist(g, vertices(g), vertex_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g)));
  }
  SECTION("const vertexlist with vertex_fn") {
    // Note: must include trailing return type on lambda
    using G2         = const G;
    G2&    g2        = g;
    size_t cnt       = 0;
    auto   vertex_fn = [&g2](vertex_reference_t<G2> u) -> const std::string& { return vertex_value(g2, u); };
    for (auto&& [uid, u, val] : graph::views::vertexlist(g2, vertex_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(vertices(g2)));
  }

  SECTION("vertexlist is a forward view") {
    auto vlist = vertexlist(g);
    auto it1   = vlist.begin();
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

    using Rng = decltype(vlist);
    CHECK(std::ranges::range<Rng>);
    CHECK(std::movable<Rng>);
    //CHECK(std::derived_from<Rng, std::ranges::view_base>);
    CHECK(std::ranges::enable_view<Rng>);
    CHECK(std::ranges::view<decltype(vlist)>);

    auto it8  = std::ranges::begin(vlist);
    auto it9  = std::ranges::end(vlist);
    auto empt = std::ranges::empty(vlist);
  }
}
