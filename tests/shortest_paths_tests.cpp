#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/vertexlist.hpp"
#include <fmt/format.h>
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
using std::vector;

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::add_lvalue_reference_t;
using std::invoke_result_t;
using std::assignable_from;
using std::less;
using std::plus;
using std::is_arithmetic_v;

using std::graph::index_adjacency_list;
using std::graph::edge_weight_function;
using std::graph::basic_edge_weight_function;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;
using std::graph::edge_value_t;

using std::graph::vertices;
using std::graph::find_vertex;
using std::graph::vertex_value;
using std::graph::edges;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::num_vertices;
using std::graph::vertex_reference_t;
using std::graph::edge_reference_t;

using std::graph::views::vertexlist;

using std::graph::shortest_path_invalid_distance;
using std::graph::init_shortest_paths;
using std::graph::dijkstra_shortest_paths;
using std::graph::dijkstra_shortest_distances;
using std::graph::dijkstra_visitor_base;

using routes_volf_graph_traits = std::graph::container::vofl_graph_traits<double, std::string>;
using routes_volf_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_volf_graph_traits>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

using Distance     = double;
using Distances    = vector<Distance>;
using Predecessors = vector<vertex_id_t<routes_volf_graph_type>>;

// Walk the predecessors and generate a comma-separated string with "[uid] city_name" for each entry
template <class G>
auto to_string(G&& g, const Predecessors& predecessors, vertex_id_t<G> uid, vertex_id_t<G> source) {
  assert(num_vertices(g) == size(predecessors));
  std::string pred;
  for (; uid != source; uid = predecessors[uid]) {
    vertex_id_t<G>        pid  = predecessors[uid];
    vertex_reference_t<G> pref = *find_vertex(g, pid);
    if (pred.empty()) {
      pred += fmt::format("[{}]{}", pid, vertex_value(g, pref));
    } else {
      pred += fmt::format(", [{}]{}", pid, vertex_value(g, pref));
    }
  }
  return pred;
}

// Walk the predecessors and generate a vector with only the predecessor vertex ids
template <class G>
Predecessors to_vector(G&& g, const Predecessors& predecessors, vertex_id_t<G> uid, vertex_id_t<G> source) {
  Predecessors pred;
  for (; uid != source; uid = predecessors[uid]) {
    pred.push_back(predecessors[uid]);
  }
  return pred; // only predecessors are included
}

// Output a vector of predecessor ids as a comma-separated list
auto to_string(const Predecessors& predecessors) {
  std::string pred;
  for (auto& pid : predecessors) {
    if (pred.empty()) {
      pred += fmt::format("{}", pid);
    } else {
      pred += fmt::format(",{}", pid);
    }
  }
  return pred;
}

TEST_CASE("Dijkstra's Common Shortest Segments", "[csv][vofl][shortest][segments][dijkstra][common]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  Distances    distance(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  init_shortest_paths(distance, predecessors);
  auto weight = [](edge_reference_t<G> uv) -> double { return 1.0; };

#if 0
  //using V = std::graph::dijkstra_visitor_base<G>;
  //static_assert(std::graph::dijkstra_visitor<G, V>, "Visitor doesn't match dijkstra_visitor requirements");
#endif

  dijkstra_shortest_paths(g, frankfurt_id, distance, predecessors);

  SECTION("types") {
    //auto weight         = [](edge_reference_t<G> uv) -> double { return 1.0; };
    using WF            = decltype(weight);
    using DistanceValue = double;
    using Plus          = invoke_result_t<plus<DistanceValue>, DistanceValue, invoke_result_t<WF, edge_reference_t<G>>>;

    static_assert(index_adjacency_list<G>, "G isn't an index_adjacency_list");

    static_assert(is_arithmetic_v<DistanceValue>, "Distance isn't numeric");
    static_assert(std::strict_weak_order<std::less<DistanceValue>, DistanceValue, DistanceValue>,
                  "Distance not stict weak order");
    static_assert(assignable_from<add_lvalue_reference_t<DistanceValue>, Plus>,
                  "Distance not assignable from Weight Function 2");
    static_assert(assignable_from<
                        add_lvalue_reference_t<DistanceValue>,
                        invoke_result_t<plus<DistanceValue>, DistanceValue, invoke_result_t<WF, edge_reference_t<G>>>>,
                  "Distance not assignable from Weight Function");
    static_assert(basic_edge_weight_function<G, WF, DistanceValue, less<DistanceValue>, plus<DistanceValue>>,
                  "Invalid basic edge weight function");

    static_assert(is_arithmetic_v<invoke_result_t<WF, edge_reference_t<G>>>, "WF doesn't return an arithmetic value");
    static_assert(edge_weight_function<G, WF, DistanceValue>, "Invalid edge weight function");
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("Dijkstra's Shortest Segments output") {
    cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (source)" << endl;
    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      vertex_reference_t<G> pred = *find_vertex(g, predecessors[uid]);
      cout << '[' << uid << "] " << city_name << ", segments=" << distance[uid]
           << ", predecessors=" << to_string(g, predecessors, uid, frankfurt_id) << endl;
    }

    /* Output:
        [2] Frankfürt (seed)
        [0] Augsburg, segments=3, predecessors=[3]Karlsruhe, [5]Mannheim, [2]Frankfürt
        [1] Erfurt, segments=2, predecessors=[9]Würzburg, [2]Frankfürt
        [2] Frankfürt, segments=0, predecessors=
        [3] Karlsruhe, segments=2, predecessors=[5]Mannheim, [2]Frankfürt
        [4] Kassel, segments=1, predecessors=[2]Frankfürt
        [5] Mannheim, segments=1, predecessors=[2]Frankfürt
        [6] München, segments=2, predecessors=[4]Kassel, [2]Frankfürt
        [7] Nürnberg, segments=2, predecessors=[9]Würzburg, [2]Frankfürt
        [8] Stuttgart, segments=3, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [9] Würzburg, segments=1, predecessors=[2]Frankfürt
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("Dijkstra's Shortest Segments generate") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl << indent << "for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {" << endl;
    ++indent;
    cout << indent << "switch (uid) {" << endl;

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      cout << indent << "case " << uid << ": {" << endl;
      ++indent;
      {
        cout << indent << "REQUIRE(\"" << city_name << "\" == vertex_value(g, u));" << endl
             << indent << "REQUIRE(" << distance[uid] << " == distance[uid]);" << endl;

        std::string expect = to_string(to_vector(g, predecessors, uid, frankfurt_id));
        cout << indent << "REQUIRE(Predecessors{" << expect << "} == to_vector(g, predecessors, uid, frankfurt_id));"
             << endl;
        //cout << indent << "} break;" << endl; // case
      }
      --indent;
      cout << indent << "} break;" << endl; // case
    }

    cout << indent << "}" << endl; // switch
    --indent;
    cout << indent << "}" << endl; // for
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("Dijkstra's Shortest Segments test content") {

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      switch (uid) {
      case 0: {
        REQUIRE("Augsburg" == vertex_value(g, u));
        REQUIRE(3 == distance[uid]);
        REQUIRE(Predecessors{3, 5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 1: {
        REQUIRE("Erfurt" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 2: {
        REQUIRE("Frankfürt" == vertex_value(g, u));
        REQUIRE(0 == distance[uid]);
        REQUIRE(Predecessors{} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 3: {
        REQUIRE("Karlsruhe" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 4: {
        REQUIRE("Kassel" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 5: {
        REQUIRE("Mannheim" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 6: {
        REQUIRE("München" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{4, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 7: {
        REQUIRE("Nürnberg" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 8: {
        REQUIRE("Stuttgart" == vertex_value(g, u));
        REQUIRE(3 == distance[uid]);
        REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 9: {
        REQUIRE("Würzburg" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      }
    }
  }
#endif
}

TEST_CASE("Dijkstra's Common Shortest Paths", "[csv][vofl][shortest][paths][dijkstra][common]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  vector<double>         distance(size(vertices(g)));
  vector<vertex_id_t<G>> predecessors(size(vertices(g)));
  init_shortest_paths(distance, predecessors);
  auto weight = [&g](edge_reference_t<G> uv) -> double { return edge_value(g, uv); };

  dijkstra_shortest_paths(g, frankfurt_id, distance, predecessors, weight);

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("Dijkstra's Shortest Paths output") {
    cout << endl << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (source)" << endl;
    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      vertex_reference_t<G> pred = *find_vertex(g, predecessors[uid]);
      cout << '[' << uid << "] " << city_name << ", distance=" << distance[uid]
           << ", predecessors=" << to_string(g, predecessors, uid, frankfurt_id) << endl;
    }

    /* Output:
        [2] Frankfürt (source)
        [0] Augsburg, distance=415, predecessors=[3]Karlsruhe, [5]Mannheim, [2]Frankfürt
        [1] Erfurt, distance=403, predecessors=[9]Würzburg, [2]Frankfürt
        [2] Frankfürt, distance=0, predecessors=
        [3] Karlsruhe, distance=165, predecessors=[5]Mannheim, [2]Frankfürt
        [4] Kassel, distance=173, predecessors=[2]Frankfürt
        [5] Mannheim, distance=85, predecessors=[2]Frankfürt
        [6] München, distance=487, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [7] Nürnberg, distance=320, predecessors=[9]Würzburg, [2]Frankfürt
        [8] Stuttgart, distance=503, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [9] Würzburg, distance=217, predecessors=[2]Frankfürt
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("Dijkstra's Shortest Paths generate") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl << indent << "for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {" << endl;
    ++indent;
    cout << indent << "switch (uid) {" << endl;

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      cout << indent << "case " << uid << ": {" << endl;
      ++indent;
      {
        cout << indent << "REQUIRE(\"" << city_name << "\" == vertex_value(g, u));" << endl
             << indent << "REQUIRE(" << distance[uid] << " == distance[uid]);" << endl;

        std::string expect = to_string(to_vector(g, predecessors, uid, frankfurt_id));
        cout << indent << "REQUIRE(Predecessors{" << expect << "} == to_vector(g, predecessors, uid, frankfurt_id));"
             << endl;
        //cout << indent << "} break;" << endl; // case
      }
      --indent;
      cout << indent << "} break;" << endl; // case
    }

    cout << indent << "}" << endl; // switch
    --indent;
    cout << indent << "}" << endl; // for
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
    switch (uid) {
    case 0: {
      REQUIRE("Augsburg" == vertex_value(g, u));
      REQUIRE(415 == distance[uid]);
      REQUIRE(Predecessors{3, 5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 1: {
      REQUIRE("Erfurt" == vertex_value(g, u));
      REQUIRE(403 == distance[uid]);
      REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 2: {
      REQUIRE("Frankfürt" == vertex_value(g, u));
      REQUIRE(0 == distance[uid]);
      REQUIRE(Predecessors{} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 3: {
      REQUIRE("Karlsruhe" == vertex_value(g, u));
      REQUIRE(165 == distance[uid]);
      REQUIRE(Predecessors{5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 4: {
      REQUIRE("Kassel" == vertex_value(g, u));
      REQUIRE(173 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 5: {
      REQUIRE("Mannheim" == vertex_value(g, u));
      REQUIRE(85 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 6: {
      REQUIRE("München" == vertex_value(g, u));
      REQUIRE(487 == distance[uid]);
      REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 7: {
      REQUIRE("Nürnberg" == vertex_value(g, u));
      REQUIRE(320 == distance[uid]);
      REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 8: {
      REQUIRE("Stuttgart" == vertex_value(g, u));
      REQUIRE(503 == distance[uid]);
      REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 9: {
      REQUIRE("Würzburg" == vertex_value(g, u));
      REQUIRE(217 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    }
  }
#endif
}

TEST_CASE("Dijkstra's Common Shortest Distances", "[csv][vofl][shortest][distances][dijkstra][common]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  vector<double> distance(size(vertices(g)));
  init_shortest_paths(distance);
  auto weight = [&g](edge_reference_t<G> uv) -> double { return edge_value(g, uv); };

  // This test case just tests that these will compile without error. The distances will be the same as before.
  dijkstra_shortest_distances(g, frankfurt_id, distance);
  dijkstra_shortest_distances(g, frankfurt_id, distance, weight);
}

TEST_CASE("Dijkstra's General Shortest Segments", "[csv][vofl][shortest][segments][dijkstra][general]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  Distances    distance(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  init_shortest_paths(distance, predecessors);
  auto weight  = [](edge_reference_t<G> uv) -> double { return 1.0; };
  auto visitor = dijkstra_visitor_base<G>();

#if 0
  using Visitor = decltype(visitor);
  auto&                           u       = *find_vertex(g, frankfurt_id);
  Visitor::vertex_desc_type       u_desc  = {frankfurt_id, u};
  auto&                           uv      = *begin(edges(g, u));
  Visitor::sourced_edge_desc_type uv_desc = {frankfurt_id, target_id(g, uv), uv};

  static_assert(std::graph::dijkstra_visitor<G, decltype(visitor)>, "visitor is not a dijkstra_visitor");
#endif
  dijkstra_shortest_paths(g, frankfurt_id, distance, predecessors, weight, visitor, std::less<Distance>(),
                          std::plus<Distance>());

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("Dijkstra's Shortest Segments output") {
    cout << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (source)" << endl;
    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      vertex_reference_t<G> pred = *find_vertex(g, predecessors[uid]);
      cout << '[' << uid << "] " << city_name << ", segments=" << distance[uid]
           << ", predecessors=" << to_string(g, predecessors, uid, frankfurt_id) << endl;
    }

    /* Output:
        [2] Frankfürt (seed)
        [0] Augsburg, segments=3, predecessors=[3]Karlsruhe, [5]Mannheim, [2]Frankfürt
        [1] Erfurt, segments=2, predecessors=[9]Würzburg, [2]Frankfürt
        [2] Frankfürt, segments=0, predecessors=
        [3] Karlsruhe, segments=2, predecessors=[5]Mannheim, [2]Frankfürt
        [4] Kassel, segments=1, predecessors=[2]Frankfürt
        [5] Mannheim, segments=1, predecessors=[2]Frankfürt
        [6] München, segments=2, predecessors=[4]Kassel, [2]Frankfürt
        [7] Nürnberg, segments=2, predecessors=[9]Würzburg, [2]Frankfürt
        [8] Stuttgart, segments=3, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [9] Würzburg, segments=1, predecessors=[2]Frankfürt
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("Dijkstra's Shortest Segments generate") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl << indent << "for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {" << endl;
    ++indent;
    cout << indent << "switch (uid) {" << endl;

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      cout << indent << "case " << uid << ": {" << endl;
      ++indent;
      {
        cout << indent << "REQUIRE(\"" << city_name << "\" == vertex_value(g, u));" << endl
             << indent << "REQUIRE(" << distance[uid] << " == distance[uid]);" << endl;

        std::string expect = to_string(to_vector(g, predecessors, uid, frankfurt_id));
        cout << indent << "REQUIRE(Predecessors{" << expect << "} == to_vector(g, predecessors, uid, frankfurt_id));"
             << endl;
        //cout << indent << "} break;" << endl; // case
      }
      --indent;
      cout << indent << "} break;" << endl; // case
    }

    cout << indent << "}" << endl; // switch
    --indent;
    cout << indent << "}" << endl; // for
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("Dijkstra's Shortest Segments test content") {

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      switch (uid) {
      case 0: {
        REQUIRE("Augsburg" == vertex_value(g, u));
        REQUIRE(3 == distance[uid]);
        REQUIRE(Predecessors{3, 5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 1: {
        REQUIRE("Erfurt" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 2: {
        REQUIRE("Frankfürt" == vertex_value(g, u));
        REQUIRE(0 == distance[uid]);
        REQUIRE(Predecessors{} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 3: {
        REQUIRE("Karlsruhe" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 4: {
        REQUIRE("Kassel" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 5: {
        REQUIRE("Mannheim" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 6: {
        REQUIRE("München" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{4, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 7: {
        REQUIRE("Nürnberg" == vertex_value(g, u));
        REQUIRE(2 == distance[uid]);
        REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 8: {
        REQUIRE("Stuttgart" == vertex_value(g, u));
        REQUIRE(3 == distance[uid]);
        REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      case 9: {
        REQUIRE("Würzburg" == vertex_value(g, u));
        REQUIRE(1 == distance[uid]);
        REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
      } break;
      }
    }
  }
#endif
}

TEST_CASE("Dijkstra's General Shortest Paths", "[csv][vofl][shortest][paths][dijkstra][general]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  vector<double>         distance(size(vertices(g)));
  vector<vertex_id_t<G>> predecessors(size(vertices(g)));
  init_shortest_paths(distance, predecessors);
  auto weight  = [&g](edge_reference_t<G> uv) -> double { return edge_value(g, uv); };
  auto visitor = dijkstra_visitor_base<G>();

  dijkstra_shortest_paths(g, frankfurt_id, distance, predecessors, weight, visitor, std::less<Distance>(),
                          std::plus<Distance>());

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("Dijkstra's Shortest Paths output") {
    cout << endl << '[' << frankfurt_id << "] " << vertex_value(g, **frankfurt) << " (source)" << endl;
    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      vertex_reference_t<G> pred = *find_vertex(g, predecessors[uid]);
      cout << '[' << uid << "] " << city_name << ", distance=" << distance[uid]
           << ", predecessors=" << to_string(g, predecessors, uid, frankfurt_id) << endl;
    }

    /* Output:
        [2] Frankfürt (source)
        [0] Augsburg, distance=415, predecessors=[3]Karlsruhe, [5]Mannheim, [2]Frankfürt
        [1] Erfurt, distance=403, predecessors=[9]Würzburg, [2]Frankfürt
        [2] Frankfürt, distance=0, predecessors=
        [3] Karlsruhe, distance=165, predecessors=[5]Mannheim, [2]Frankfürt
        [4] Kassel, distance=173, predecessors=[2]Frankfürt
        [5] Mannheim, distance=85, predecessors=[2]Frankfürt
        [6] München, distance=487, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [7] Nürnberg, distance=320, predecessors=[9]Würzburg, [2]Frankfürt
        [8] Stuttgart, distance=503, predecessors=[7]Nürnberg, [9]Würzburg, [2]Frankfürt
        [9] Würzburg, distance=217, predecessors=[2]Frankfürt
    */
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("Dijkstra's Shortest Paths generate") {
    using namespace std::graph;
    using std::cout;
    using std::endl;
    ostream_indenter indent;

    cout << endl << indent << "for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {" << endl;
    ++indent;
    cout << indent << "switch (uid) {" << endl;

    for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
      cout << indent << "case " << uid << ": {" << endl;
      ++indent;
      {
        cout << indent << "REQUIRE(\"" << city_name << "\" == vertex_value(g, u));" << endl
             << indent << "REQUIRE(" << distance[uid] << " == distance[uid]);" << endl;

        std::string expect = to_string(to_vector(g, predecessors, uid, frankfurt_id));
        cout << indent << "REQUIRE(Predecessors{" << expect << "} == to_vector(g, predecessors, uid, frankfurt_id));"
             << endl;
        //cout << indent << "} break;" << endl; // case
      }
      --indent;
      cout << indent << "} break;" << endl; // case
    }

    cout << indent << "}" << endl; // switch
    --indent;
    cout << indent << "}" << endl; // for
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  for (auto&& [uid, u, city_name] : vertexlist(g, vname)) {
    switch (uid) {
    case 0: {
      REQUIRE("Augsburg" == vertex_value(g, u));
      REQUIRE(415 == distance[uid]);
      REQUIRE(Predecessors{3, 5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 1: {
      REQUIRE("Erfurt" == vertex_value(g, u));
      REQUIRE(403 == distance[uid]);
      REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 2: {
      REQUIRE("Frankfürt" == vertex_value(g, u));
      REQUIRE(0 == distance[uid]);
      REQUIRE(Predecessors{} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 3: {
      REQUIRE("Karlsruhe" == vertex_value(g, u));
      REQUIRE(165 == distance[uid]);
      REQUIRE(Predecessors{5, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 4: {
      REQUIRE("Kassel" == vertex_value(g, u));
      REQUIRE(173 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 5: {
      REQUIRE("Mannheim" == vertex_value(g, u));
      REQUIRE(85 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 6: {
      REQUIRE("München" == vertex_value(g, u));
      REQUIRE(487 == distance[uid]);
      REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 7: {
      REQUIRE("Nürnberg" == vertex_value(g, u));
      REQUIRE(320 == distance[uid]);
      REQUIRE(Predecessors{9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 8: {
      REQUIRE("Stuttgart" == vertex_value(g, u));
      REQUIRE(503 == distance[uid]);
      REQUIRE(Predecessors{7, 9, 2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    case 9: {
      REQUIRE("Würzburg" == vertex_value(g, u));
      REQUIRE(217 == distance[uid]);
      REQUIRE(Predecessors{2} == to_vector(g, predecessors, uid, frankfurt_id));
    } break;
    }
  }
#endif
}

TEST_CASE("Dijkstra's General Shortest Distances", "[csv][vofl][shortest][distances][dijkstra][general]") {
  init_console();
  using G                     = routes_volf_graph_type;
  auto&&         g            = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
  auto           frankfurt    = find_frankfurt(g);
  vertex_id_t<G> frankfurt_id = find_frankfurt_id(g);
  auto           vname        = [&](vertex_reference_t<G> u) { return vertex_value(g, u); };

  vector<double> distance(size(vertices(g)));
  init_shortest_paths(distance);
  auto weight  = [&g](edge_reference_t<G> uv) -> double { return edge_value(g, uv); };
  auto visitor = dijkstra_visitor_base<G>();

  // This test case just tests that these will compile without error. The distances will be the same as before.
  //dijkstra_shortest_distances(g, frankfurt_id, distance, std::less<Distance>(), std::plus<Distance>());
  dijkstra_shortest_distances(g, frankfurt_id, distance, weight, visitor, std::less<Distance>(), std::plus<Distance>());
}
