#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/breadth_first_search.hpp"
#include "graph/container/dynamic_graph.hpp"

#define TEST_OPTION_OUTPUT (1)
#define TEST_OPTION_GEN (2)
#define TEST_OPTION_TEST (3)
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_value_t;
using std::graph::vertex_edge_range_t;
using std::graph::vertex_reference_t;
using std::graph::edge_t;
using std::graph::edge_value_t;
using std::graph::edge_reference_t;

using std::graph::graph_value;
using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_id;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::degree;
using std::graph::find_vertex;
using std::graph::find_vertex_edge;

using std::graph::vertices_breadth_first_search_view;
using std::graph::views::vertices_breadth_first_search;

using std::graph::edges_breadth_first_search_view;
using std::graph::views::edges_breadth_first_search;
using std::graph::views::sourced_edges_breadth_first_search;

using routes_vol_graph_traits = std::graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}


//template <typename _Iter>
//concept io_iterator = requires(_Iter __i) {
//  { *__i } -> std::__detail::__can_reference;
//}
//&&std::weakly_incrementable<_Iter>;

TEST_CASE("vertices_breadth_first_search_view test", "[dynamic][bfs][vertex]") {
  init_console();

  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("vertices_breadth_first_search_view is an input view") {
    vertices_breadth_first_search_view<G, void> bfs(g, frankfurt_id);
    auto                                        it1 = bfs.begin();
    using I                                         = decltype(it1);

    auto it2 = it1;            // copyable
    I    it3(it1);             // copy-constuctible
    auto it4 = std::move(it2); // movable
    I    it5(std::move(it3));  // move-constuctible
    I    it6;                  // default-constructible

    using I2 = std::remove_cvref_t<I>;
    static_assert(std::move_constructible<I2>);
    static_assert(std::copyable<I2>);
    static_assert(std::movable<I2>);
    static_assert(std::swappable<I2>);
    static_assert(std::is_default_constructible_v<I2>);

    //static_assert(std::input_iterator<I2>);
    static_assert(std::input_or_output_iterator<I2>);
    static_assert(std::indirectly_readable<I2>);
    static_assert(std::input_iterator<I2>);

    using Rng = decltype(bfs);
    static_assert(std::is_constructible_v<Rng>);
    static_assert(std::ranges::range<Rng>);
    static_assert(std::movable<Rng>);
    static_assert(std::derived_from<Rng, std::ranges::view_base>);
    static_assert(std::ranges::enable_view<Rng>);
    static_assert(std::ranges::view<decltype(bfs)>);

    auto it8  = std::ranges::begin(bfs);
    auto it9  = std::ranges::end(bfs);
    auto n    = std::ranges::size(bfs);
    auto empt = std::ranges::empty(bfs);
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("vertices_breadth_first_search_view output") {
    vertices_breadth_first_search_view<G, void> bfs(g, frankfurt_id);

    int cnt = 0;
    cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, u] : bfs) {
      //ostream_indenter indent(static_cast<int>(bfs.depth()));
      //cout << indent << '[' << uid << "] " << vertex_value(g, u) << endl;
      cout << '[' << uid << "] " << vertex_value(g, u) << endl;
      ++cnt;
    }
    REQUIRE(cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
          [1] Mannheim
          [2] Würzburg
          [3] Kassel
            [4] Karlsruhe
            [5] Nürnberg
            [6] Erfurt
            [7] München
              [8] Augsburg
              [9] Stuttgart
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("vertices_breadth_first_search_view generate content test") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl
         //<< indent << "vertices_breadth_first_search_view bfs(g, frankfurt_id);" << endl
         << indent << "auto bfs      = vertices_breadth_first_search(g, frankfurt_id)" << endl
         << indent << "auto city     = bfs.begin();" << endl
         << indent << "int  city_cnt = 0;" << endl;

    int city_cnt = 0;
    for (auto&& [uid, u] : vertices_breadth_first_search(g, frankfurt_id)) {
      ++city_cnt;
      cout << endl << indent << "if(city != bfs.end()) {\n";
      ++indent;
      cout << indent << "auto&& [uid, u] = *city;\n"
           << indent << "REQUIRE(" << uid << " == uid);\n"
           << indent << "REQUIRE(\"" << vertex_value(g, u) << "\" == vertex_value(g, u));\n"
           << indent << "++city_cnt;\n"
           << indent << "++city;\n";
      --indent;
      cout << indent << "}\n";
    }
    cout << endl << indent << "REQUIRE(" << city_cnt << " == city_cnt);" << endl << endl;
  }
#elif TEST_OPTION == TEST_OPTION_TEST

  SECTION("vertices_breadth_first_search_view test content") {
    auto bfs      = vertices_breadth_first_search(g, frankfurt_id);
    auto city     = bfs.begin();
    int  city_cnt = 0;

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(1 == uid);
      REQUIRE("Mannheim" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(4 == uid);
      REQUIRE("Würzburg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(6 == uid);
      REQUIRE("Kassel" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(2 == uid);
      REQUIRE("Karlsruhe" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(5 == uid);
      REQUIRE("Nürnberg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(7 == uid);
      REQUIRE("Erfurt" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(8 == uid);
      REQUIRE("München" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(3 == uid);
      REQUIRE("Augsburg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(9 == uid);
      REQUIRE("Stuttgart" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    REQUIRE(9 == city_cnt);
  }
#endif

  SECTION("vertices_breadth_first_search_view with vertex value function") {
    int  city_cnt = 0;
    auto vvf      = [&g](vertex_reference_t<G> u) { return vertex_value(g, u); };
    vertices_breadth_first_search_view<G, decltype(vvf)> bfs(g, frankfurt_id, vvf);
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, u, city_name] : bfs) {
      //ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << city_name << endl;
      //cout << "[" << uid << "] " << city_name << endl;
      ++city_cnt;
    }
    REQUIRE(9 == city_cnt);
  }

  SECTION("vertices_breadth_first_search_view can do cancel_all") {
    int                                   city_cnt = 0;
    vertices_breadth_first_search_view<G> bfs(g, frankfurt_id);
    for (auto&& [uid, u] : bfs) {
      ++city_cnt;
      if (uid == 2) // Karlsruhe
        bfs.cancel(std::graph::cancel_search::cancel_all);
    }
    REQUIRE(4 == city_cnt);
  }
  SECTION("vertices_breadth_first_search_view can do cancel_branch #1") {
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    int                                   city_cnt = 0;
    vertices_breadth_first_search_view<G> bfs(g, frankfurt_id);
    for (auto&& [uid, u] : bfs) {
      //ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << vertex_value(g,u) << endl;
      //cout << "[" << uid << "] " << vertex_value(g,u) << endl;
      ++city_cnt;
      if (uid == 4) // Wurbzurg
        bfs.cancel(std::graph::cancel_search::cancel_branch);
    }
    REQUIRE(6 == city_cnt);
  }
}

TEST_CASE("vertices_breadth_first_search test", "[dynamic][bfs][vertex]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("vertices_breadth_first_search_view is an input view") {
    auto bfs = vertices_breadth_first_search(g, frankfurt_id);
    auto it1 = bfs.begin();
    using I  = decltype(it1);

    auto it2 = it1;            // copyable
    I    it3(it1);             // copy-constuctible
    auto it4 = std::move(it2); // movable
    I    it5(std::move(it3));  // move-constuctible
    I    it6;                  // default-constructible

    using I2 = std::remove_cvref_t<I>;
    static_assert(std::move_constructible<I2>);
    static_assert(std::copyable<I2>);
    static_assert(std::movable<I2>);
    static_assert(std::swappable<I2>);
    static_assert(std::is_default_constructible_v<I2>);

    //static_assert(std::input_iterator<I2>);
    static_assert(std::input_or_output_iterator<I2>);
    static_assert(std::indirectly_readable<I2>);
    static_assert(std::input_iterator<I2>);

    using Rng = decltype(bfs);
    static_assert(std::is_constructible_v<Rng>);
    static_assert(std::ranges::range<Rng>);
    static_assert(std::movable<Rng>);
    static_assert(std::derived_from<Rng, std::ranges::view_base>);
    static_assert(std::ranges::enable_view<Rng>);
    static_assert(std::ranges::view<decltype(bfs)>);

    auto it8  = std::ranges::begin(bfs);
    auto it9  = std::ranges::end(bfs);
    auto n    = std::ranges::size(bfs);
    auto empt = std::ranges::empty(bfs);
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("vertices_breadth_first_search output") {
    auto bfs = vertices_breadth_first_search(g, frankfurt_id);

    int cnt = 0;
    cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, u] : bfs) {
      //ostream_indenter indent(static_cast<int>(bfs.depth()));
      cout << '[' << uid << "] " << vertex_value(g, u) << endl;
      ++cnt;
    }
    REQUIRE(cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
        [1] Mannheim
        [4] Würzburg
        [6] Kassel
        [2] Karlsruhe
        [5] Nürnberg
        [7] Erfurt
        [8] München
        [3] Augsburg    
        [9] Stuttgart
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("vertices_breadth_first_search generate content test") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl
         << indent << "auto bfs      = vertices_breadth_first_search(g, frankfurt_id);" << endl
         << indent << "auto city     = bfs.begin();" << endl
         << indent << "int  city_cnt = 0;" << endl;

    int city_cnt = 0;
    for (auto&& [uid, u] : vertices_breadth_first_search(g, frankfurt_id)) {
      ++city_cnt;
      cout << endl << indent << "if(city != bfs.end()) {\n";
      ++indent;
      cout << indent << "auto&& [uid, u] = *city;\n"
           << indent << "REQUIRE(" << uid << " == uid);\n"
           << indent << "REQUIRE(\"" << vertex_value(g, u) << "\" == vertex_value(g, u));\n"
           << indent << "++city_cnt;\n"
           << indent << "++city;\n";
      --indent;
      cout << indent << "}\n";
    }
    cout << endl << indent << "REQUIRE(" << city_cnt << " == city_cnt);" << endl << endl;
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("vertices_breadth_first_search test content") {

    auto bfs      = vertices_breadth_first_search(g, frankfurt_id);
    auto city     = bfs.begin();
    int  city_cnt = 0;

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(1 == uid);
      REQUIRE("Mannheim" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(4 == uid);
      REQUIRE("Würzburg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(6 == uid);
      REQUIRE("Kassel" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(2 == uid);
      REQUIRE("Karlsruhe" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(5 == uid);
      REQUIRE("Nürnberg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(7 == uid);
      REQUIRE("Erfurt" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(8 == uid);
      REQUIRE("München" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(3 == uid);
      REQUIRE("Augsburg" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    if (city != bfs.end()) {
      auto&& [uid, u] = *city;
      REQUIRE(9 == uid);
      REQUIRE("Stuttgart" == vertex_value(g, u));
      ++city_cnt;
      ++city;
    }

    REQUIRE(9 == city_cnt);
  }
#endif

  SECTION("vertices_breadth_first_search with vertex value function") {
    int  city_cnt = 0;
    auto vvf      = [&g](vertex_reference_t<G> u) { return vertex_value(g, u); };
    auto bfs      = vertices_breadth_first_search(g, frankfurt_id, vvf);
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, u, city_name] : bfs) {
      //ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << city_name << endl;
      ++city_cnt;
    }
    REQUIRE(9 == city_cnt);
  }

  SECTION("vertices_breadth_first_search can do cancel_all") {
    int  city_cnt = 0;
    auto bfs      = vertices_breadth_first_search(g, frankfurt_id);
    for (auto&& [uid, u] : bfs) {
      ++city_cnt;
      if (uid == 2) // Karlsruhe
        bfs.cancel(std::graph::cancel_search::cancel_all);
    }
    REQUIRE(4 == city_cnt);
  }
  SECTION("vertices_depth_first_search can do cancel_branch #1") {
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    int  city_cnt = 0;
    auto bfs      = vertices_breadth_first_search(g, frankfurt_id);
    for (auto&& [uid, u] : bfs) {
      ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << vertex_value(g,u) << endl;
      ++city_cnt;
      if (uid == 4) // Wurzburg
        bfs.cancel(std::graph::cancel_search::cancel_branch);
    }
    REQUIRE(6 == city_cnt);
  }
}

TEST_CASE("edges_breadth_first_search_view test", "[dynamic][bfs][edge]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("edges_breadth_first_search_view is an input view") {
    edges_breadth_first_search_view<G> bfs(g, frankfurt_id);
    auto                               it1 = bfs.begin();
    using I                                = decltype(it1);

    auto it2 = it1;            // copyable
    I    it3(it1);             // copy-constuctible
    auto it4 = std::move(it2); // movable
    I    it5(std::move(it3));  // move-constuctible
    //I    it6;                  // default-constructible

    using I2 = std::remove_cvref_t<I>;
    static_assert(std::move_constructible<I2>);
    static_assert(std::movable<I2>);
    static_assert(std::copyable<I2>);
    static_assert(std::swappable<I2>);
    static_assert(std::is_default_constructible_v<I2>);

    static_assert(std::input_or_output_iterator<I2>);
    static_assert(std::indirectly_readable<I2>);
    static_assert(std::input_iterator<I2>);

    using Rng = decltype(bfs);
    static_assert(std::ranges::range<Rng>);
    static_assert(std::movable<Rng>);
    static_assert(std::derived_from<Rng, std::ranges::view_base>);
    static_assert(std::ranges::enable_view<Rng>);
    static_assert(std::ranges::view<decltype(bfs)>);

    auto it8  = std::ranges::begin(bfs);
    auto it9  = std::ranges::end(bfs);
    auto n    = std::ranges::size(bfs);
    auto empt = std::ranges::empty(bfs);
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("edges_breadth_first_search_view output") {
    edges_breadth_first_search_view<G> bfs(g, frankfurt_id);
    int                                cnt = 0;
    cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [vid, uv] : bfs) {
      //ostream_indenter indent(static_cast<int>(bfs.depth()));
      cout << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g, uv) << "km" << endl;
      ++cnt;
    }
    REQUIRE(cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
          --> [1] Mannheim 85km
          --> [4] Würzburg 217km
          --> [6] Kassel 173km
          --> [2] Karlsruhe 80km
          --> [5] Nürnberg 103km
          --> [7] Erfurt 186km
          --> [8] München 502km
          --> [3] Augsburg 250km
          --> [9] Stuttgart 183km
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("edges_breadth_first_search_view generate content test") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl
         //<< indent << "edges_breadth_first_search_view bfs(g, frankfurt_id);" << endl
         << indent << "auto bfs      = edges_breadth_first_search(g, frankfurt_id);" << endl
         << indent << "auto route    = bfs.begin();" << endl
         << indent << "int  city_cnt = 0;" << endl;

    int city_cnt = 0;
    for (auto&& [vid, uv] : edges_breadth_first_search(g, frankfurt_id)) {
      ++city_cnt;
      cout << endl << indent << "if(route != bfs.end()) {\n";
      ++indent;
      cout << indent << "auto&& [vid, uv] = *route;\n"
           << indent << "REQUIRE(" << vid << " == vid);\n"
           << indent << "REQUIRE(" << edge_value(g, uv) << " == edge_value(g, uv));\n"
           << indent << "REQUIRE(\"" << vertex_value(g, target(g, uv)) << "\" == vertex_value(g, target(g, uv)));\n"
           << indent << "++city_cnt;\n"
           << indent << "++route;\n";
      --indent;
      cout << indent << "}\n";
    }
    cout << endl << indent << "REQUIRE(" << city_cnt << " == city_cnt);" << endl << endl;
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("edges_breadth_first_search_view test content") {
    auto bfs      = edges_breadth_first_search(g, frankfurt_id);
    auto route    = bfs.begin();
    int  city_cnt = 0;

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(1 == vid);
      REQUIRE(85 == edge_value(g, uv));
      REQUIRE("Mannheim" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(4 == vid);
      REQUIRE(217 == edge_value(g, uv));
      REQUIRE("Würzburg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(6 == vid);
      REQUIRE(173 == edge_value(g, uv));
      REQUIRE("Kassel" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(2 == vid);
      REQUIRE(80 == edge_value(g, uv));
      REQUIRE("Karlsruhe" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(5 == vid);
      REQUIRE(103 == edge_value(g, uv));
      REQUIRE("Nürnberg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(7 == vid);
      REQUIRE(186 == edge_value(g, uv));
      REQUIRE("Erfurt" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(8 == vid);
      REQUIRE(502 == edge_value(g, uv));
      REQUIRE("München" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(3 == vid);
      REQUIRE(250 == edge_value(g, uv));
      REQUIRE("Augsburg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(9 == vid);
      REQUIRE(183 == edge_value(g, uv));
      REQUIRE("Stuttgart" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    REQUIRE(9 == city_cnt);
  }
#endif
  SECTION("edges_breadth_first_search_view with edge value function") {
    auto                                              evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    edges_breadth_first_search_view<G, decltype(evf)> bfs(g, frankfurt_id, evf);
    int                                               city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [vid, uv, km] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      //cout << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      ++city_cnt;
    }

    REQUIRE(city_cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
          --> [1] Mannheim 85km
          --> [4] Würzburg 217km
          --> [6] Kassel 173km
          --> [2] Karlsruhe 80km
          --> [5] Nürnberg 103km
          --> [7] Erfurt 186km
          --> [8] München 502km
          --> [3] Augsburg 250km
          --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search_view with no-EVF and Sourced") {
    edges_breadth_first_search_view<G, void, true> bfs(g, frankfurt_id);
    int                                            city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, vid, uv] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g,uv) << "km" << endl;
      //cout << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g,uv) << "km" << endl;
      ++city_cnt;
    }
    REQUIRE(city_cnt == 9);
    /*
    [0] Frankfürt (seed)
      [0] --> [1] Mannheim 85km
      [0] --> [4] Würzburg 217km
      [0] --> [6] Kassel 173km
      [1] --> [2] Karlsruhe 80km
      [4] --> [5] Nürnberg 103km
      [4] --> [7] Erfurt 186km
      [6] --> [8] München 502km
      [2] --> [3] Augsburg 250km
      [5] --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search_view with EVF and Sourced") {
    auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    edges_breadth_first_search_view<G, decltype(evf), true> bfs(g, frankfurt_id, evf);
    int                                                     city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, vid, uv, km] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      //cout << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      ++city_cnt;
    }
    REQUIRE(city_cnt == 9);
    /*
    [0] Frankfürt (seed)
      [0] --> [1] Mannheim 85km
      [0] --> [4] Würzburg 217km
      [0] --> [6] Kassel 173km
      [1] --> [2] Karlsruhe 80km
      [4] --> [5] Nürnberg 103km
      [4] --> [7] Erfurt 186km
      [6] --> [8] München 502km
      [2] --> [3] Augsburg 250km
      [5] --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search_view can do cancel_all") {
    int                                city_cnt = 0;
    edges_breadth_first_search_view<G> bfs(g, frankfurt_id);
    for (auto&& [vid, uv] : bfs) {
      ++city_cnt;
      if (vid == 2) // Karlsruhe
        bfs.cancel(std::graph::cancel_search::cancel_all);
    }
    REQUIRE(4 == city_cnt);
  }
  SECTION("edges_breadth_first_search_view can do cancel_branch") {
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    int                                city_cnt = 0;
    edges_breadth_first_search_view<G> bfs(g, frankfurt_id);
    for (auto&& [vid, uv] : bfs) {
      //ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << vertex_value(g,u) << endl;
      ++city_cnt;
      if (vid == 4) // Wurzburg
        bfs.cancel(std::graph::cancel_search::cancel_branch);
    }
    REQUIRE(6 == city_cnt);
  }
}

TEST_CASE("edges_breadth_first_search test", "[dynamic][bfs][edge]") {
  init_console();
  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  SECTION("edges_breadth_first_search is an input view") {
    auto bfs = edges_breadth_first_search(g, frankfurt_id);
    auto it1 = bfs.begin();
    using I  = decltype(it1);

    auto it2 = it1;            // copyable
    I    it3(it1);             // copy-constuctible
    auto it4 = std::move(it2); // movable
    I    it5(std::move(it3));  // move-constuctible
    //I    it6;                  // default-constructible

    using I2 = std::remove_cvref_t<I>;
    static_assert(std::move_constructible<I2>);
    static_assert(std::movable<I2>);
    static_assert(std::copyable<I2>);
    static_assert(std::swappable<I2>);
    static_assert(std::is_default_constructible_v<I2>);

    static_assert(std::input_or_output_iterator<I2>);
    static_assert(std::indirectly_readable<I2>);
    static_assert(std::input_iterator<I2>);

    using Rng = decltype(bfs);
    static_assert(std::ranges::range<Rng>);
    static_assert(std::movable<Rng>);
    static_assert(std::derived_from<Rng, std::ranges::view_base>);
    static_assert(std::ranges::enable_view<Rng>);
    static_assert(std::ranges::view<decltype(bfs)>);

    auto it8  = std::ranges::begin(bfs);
    auto it9  = std::ranges::end(bfs);
    auto n    = std::ranges::size(bfs);
    auto empt = std::ranges::empty(bfs);
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("edges_breadth_first_search output") {
    auto bfs = edges_breadth_first_search(g, frankfurt_id);
    int  cnt = 0;
    cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [vid, uv] : bfs) {
      //ostream_indenter indent(static_cast<int>(bfs.depth()));
      cout << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g, uv) << "km" << endl;
      ++cnt;
    }
    REQUIRE(cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
          --> [1] Mannheim 85km
          --> [4] Würzburg 217km
          --> [6] Kassel 173km
          --> [2] Karlsruhe 80km
          --> [5] Nürnberg 103km
          --> [7] Erfurt 186km
          --> [8] München 502km
          --> [3] Augsburg 250km
          --> [9] Stuttgart 183km
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("edges_breadth_first_search generate content test") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl
         //<< indent << "edges_breadth_first_search_view bfs(g, frankfurt_id);" << endl
         << indent << "auto bfs = edges_breadth_first_search(g, frankfurt_id);" << endl
         << indent << "auto route    = bfs.begin();" << endl
         << indent << "int  city_cnt = 0;" << endl;

    int city_cnt = 0;
    for (auto&& [vid, uv] : edges_breadth_first_search(g, frankfurt_id)) {
      ++city_cnt;
      cout << endl << indent << "if(route != bfs.end()) {\n";
      ++indent;
      cout << indent << "auto&& [vid, uv] = *route;\n"
           << indent << "REQUIRE(" << vid << " == vid);\n"
           << indent << "REQUIRE(" << edge_value(g, uv) << " == edge_value(g, uv));\n"
           << indent << "REQUIRE(\"" << vertex_value(g, target(g, uv)) << "\" == vertex_value(g, target(g, uv)));\n"
           << indent << "++city_cnt;\n"
           << indent << "++route;\n";
      --indent;
      cout << indent << "}\n";
    }
    cout << endl << indent << "REQUIRE(" << city_cnt << " == city_cnt);" << endl << endl;
  }
#elif TEST_OPTION_TEST
  SECTION("edges_breadth_first_search test content") {
    auto bfs      = edges_breadth_first_search(g, frankfurt_id);
    auto route    = bfs.begin();
    int  city_cnt = 0;

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(1 == vid);
      REQUIRE(85 == edge_value(g, uv));
      REQUIRE("Mannheim" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(4 == vid);
      REQUIRE(217 == edge_value(g, uv));
      REQUIRE("Würzburg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(6 == vid);
      REQUIRE(173 == edge_value(g, uv));
      REQUIRE("Kassel" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(2 == vid);
      REQUIRE(80 == edge_value(g, uv));
      REQUIRE("Karlsruhe" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(5 == vid);
      REQUIRE(103 == edge_value(g, uv));
      REQUIRE("Nürnberg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(7 == vid);
      REQUIRE(186 == edge_value(g, uv));
      REQUIRE("Erfurt" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(8 == vid);
      REQUIRE(502 == edge_value(g, uv));
      REQUIRE("München" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(3 == vid);
      REQUIRE(250 == edge_value(g, uv));
      REQUIRE("Augsburg" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    if (route != bfs.end()) {
      auto&& [vid, uv] = *route;
      REQUIRE(9 == vid);
      REQUIRE(183 == edge_value(g, uv));
      REQUIRE("Stuttgart" == vertex_value(g, target(g, uv)));
      ++city_cnt;
      ++route;
    }

    REQUIRE(9 == city_cnt);
  }
#endif

  SECTION("edges_breadth_first_search with edge value function") {
    auto evf      = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    auto bfs      = edges_breadth_first_search(g, frankfurt_id, evf);
    int  city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [vid, uv, km] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      //cout << "--> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      ++city_cnt;
    }
    REQUIRE(city_cnt == 9);
    /* Output:
        [0] Frankfürt (seed)
          --> [1] Mannheim 85km
          --> [4] Würzburg 217km
          --> [6] Kassel 173km
          --> [2] Karlsruhe 80km
          --> [5] Nürnberg 103km
          --> [7] Erfurt 186km
          --> [8] München 502km
          --> [3] Augsburg 250km
          --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search with no-EVF and Sourced") {
    auto bfs      = sourced_edges_breadth_first_search(g, frankfurt_id);
    int  city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, vid, uv] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g,uv) << "km" << endl;
      //cout << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << edge_value(g,uv) << "km" << endl;
      ++city_cnt;
    }
    REQUIRE(city_cnt == 9);
    /*
    [0] Frankfürt (seed)
      [0] --> [1] Mannheim 85km
      [0] --> [4] Würzburg 217km
      [0] --> [6] Kassel 173km
      [1] --> [2] Karlsruhe 80km
      [4] --> [5] Nürnberg 103km
      [4] --> [7] Erfurt 186km
      [6] --> [8] München 502km
      [2] --> [3] Augsburg 250km
      [5] --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search with EVF and Sourced") {
    auto evf      = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    auto bfs      = sourced_edges_breadth_first_search(g, frankfurt_id, evf);
    int  city_cnt = 0;
    //cout << "\n[" << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    for (auto&& [uid, vid, uv, km] : bfs) {
      //ostream_indenter indent(bfs.size());
      //cout << indent << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      //cout << "[" << uid << "] --> [" << vid << "] " << vertex_value(g, target(g, uv)) << ' ' << km << "km" << endl;
      ++city_cnt;
    }
    REQUIRE(city_cnt == 9);
    /*
    [0] Frankfürt (seed)
      [0] --> [1] Mannheim 85km
      [0] --> [4] Würzburg 217km
      [0] --> [6] Kassel 173km
      [1] --> [2] Karlsruhe 80km
      [4] --> [5] Nürnberg 103km
      [4] --> [7] Erfurt 186km
      [6] --> [8] München 502km
      [2] --> [3] Augsburg 250km
      [5] --> [9] Stuttgart 183km
    */
  }

  SECTION("edges_breadth_first_search can do cancel_all") {
    int  city_cnt = 0;
    auto bfs      = edges_breadth_first_search(g, frankfurt_id);
    for (auto&& [vid, uv] : bfs) {
      ++city_cnt;
      if (vid == 2) // Karlsruhe
        bfs.cancel(std::graph::cancel_search::cancel_all);
    }
    REQUIRE(4 == city_cnt);
  }
  SECTION("edges_breadth_first_search_view can do cancel_branch") {
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    auto bfs      = edges_breadth_first_search(g, frankfurt_id);
    int  city_cnt = 0;
    for (auto&& [vid, uv] : bfs) {
      //ostream_indenter indent(size(bfs));
      //cout << indent << "[" << uid << "] " << vertex_value(g,u) << endl;
      ++city_cnt;
      if (vid == 4) // Wurzburg
        bfs.cancel(std::graph::cancel_search::cancel_branch);
    }
    REQUIRE(6 == city_cnt);
  }
}
