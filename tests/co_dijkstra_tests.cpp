#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"
#include "graph/container/dynamic_graph.hpp"

#define TEST_OPTION_OUTPUT (1)
#define TEST_OPTION_GEN (2)
#define TEST_OPTION_TEST (3)
#define TEST_OPTION TEST_OPTION_OUTPUT

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
using std::graph::experimental::dijkstra_events;

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
using std::graph::experimental::co_dijkstra;
using std::graph::experimental::bfs_vertex_value_t;
using std::graph::experimental::bfs_edge_value_t;
using std::graph::experimental::shortest_path_invalid_distance;
using std::graph::experimental::init_shortest_paths;

using routes_vol_graph_traits = std::graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

using Distance     = double;
using Distances    = std::vector<Distance>;
using Predecessors = std::vector<vertex_id_t<routes_vol_graph_type>>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

TEST_CASE("co_dijstra_clrs test", "[dynamic][dijkstra][bfs][vertex][coroutine]") {
  init_console();

  using G  = routes_vol_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  Distances    distances(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  init_shortest_paths(distances, predecessors);

  SECTION("co_dijkstra fnc vertices") {
    auto distance = [&g](edge_reference_t<G> uv) -> double { return edge_value(g, uv); };
    for (auto bfs = co_dijkstra(g, dijkstra_events::discover_vertex, frankfurt_id, predecessors, distances, distance);
         bfs;) {
      auto&& [event, payload] = bfs();
      switch (event) {
      case dijkstra_events::discover_vertex: {
        auto&& [uid, u, km] = get<bfs_vertex_value_t<G, double>>(payload); // or get<1>(payload);
        cout << "[" << uid << "] " << vertex_value(g, u) << " " << km << "km" << endl;
      } break;
      default: break;
      }
    }
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("co_dikstra output") {

    //vertices_breadth_first_search_view<G, void> bfs(g, frankfurt_id);

    //int cnt = 0;
    //cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (seed)" << endl;
    //for (auto&& [uid, u] : bfs) {
    //  //ostream_indenter indent(static_cast<int>(bfs.depth()));
    //  //cout << indent << '[' << uid << "] " << vertex_value(g, u) << endl;
    //  cout << '[' << uid << "] " << vertex_value(g, u) << endl;
    //  ++cnt;
    //}
    //REQUIRE(cnt == 9);
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
}

#if 0
using std::graph::fibonacci_sequence;
using std::graph::fib_seq;

TEST_CASE("co fib test", "[fibonacci][coroutine]") {
  try {
    auto gen = fibonacci_sequence(10); // max 94 before uint64_t overflows

    for (int j = 0; gen; ++j)
      std::cout << "fib(" << j << ")=" << gen() << '\n';
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << '\n';
  } catch (...) {
    std::cerr << "Unknown exception.\n";
  }
}

TEST_CASE("co fib class test", "[fibonacci][coroutine]") {
  try {
    fib_seq fs(10);
    auto    gen = fs(); // max 94 before uint64_t overflows

    for (int j = 0; gen; ++j)
      std::cout << "fib(" << j << ")=" << gen() << '\n';
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << '\n';
  } catch (...) {
    std::cerr << "Unknown exception.\n";
  }
}
#endif //0
