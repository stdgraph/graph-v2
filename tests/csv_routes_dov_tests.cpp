#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/dijkstra_clrs.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/container/dynamic_graph.hpp"
#include <deque>
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
using std::graph::vertex_id_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;
using std::graph::edge_reference_t;
using std::graph::edge_value_t;

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

template <typename EV = void, typename VV = void, typename GV = void, typename VId = uint32_t, bool Sourced = false>
struct dov_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = std::graph::container::dynamic_edge<EV, VV, GV, VId, Sourced, dov_graph_traits>;
  using vertex_type = std::graph::container::dynamic_vertex<EV, VV, GV, VId, Sourced, dov_graph_traits>;
  using graph_type  = std::graph::container::dynamic_graph<EV, VV, GV, VId, Sourced, dov_graph_traits>;

  using vertices_type = std::deque<vertex_type>;
  using edges_type    = std::vector<edge_type>;
};


using routes_dov_graph_traits = dov_graph_traits<double, std::string, std::string>;
using routes_dov_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_dov_graph_traits>;

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}


TEST_CASE("Germany routes CSV+dov dijkstra_clrs", "[csv][dov][germany][dijkstra][clrs]") {
  init_console();
  using G  = routes_dov_graph_type;
  auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);
  auto weight       = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };

  std::vector<std::graph::vertex_id_t<G>> predecessors(size(g));

  // Remark(Andrew): edge_value_t should be a value
  std::vector<std::remove_reference_t<edge_value_t<G>>> distances(size(g));

  std::graph::dijkstra_clrs(g, frankfurt_id, distances, predecessors);
  std::graph::dijkstra_clrs(g, frankfurt_id, distances, predecessors, weight);
}

TEST_CASE("Dynamic graph dov test", "[dov][capabilities]") {
  using G = routes_dov_graph_type; // use it because it's easy

  // This is the type the initializer_list is expecting:
  //using init_edge_value = std::graph::views::copyable_edge_t<routes_dov_graph_traits::vertex_id_type,
  //                                                           routes_dov_graph_traits::edge_value_type>;

  // Define the graph. It's the same as the germany routes using source_order_found
  G g = {{0, 1, 85.0},  {0, 4, 217.0}, {0, 6, 173.0}, {1, 2, 80.0},  {2, 3, 250.0}, {3, 8, 84.0},
         {4, 5, 103.0}, {4, 7, 186.0}, {5, 8, 167.0}, {5, 9, 183.0}, {6, 8, 502.0}};

  using init_vertex_value             = std::graph::copyable_vertex_t<vertex_id_t<G>, std::string>;
  std::vector<std::string_view> names = {"Frankfürt", "Mannheim", "Karlsruhe", "Augsburg", "Würzburg",
                                         "Nürnberg",  "Kassel",   "Erfurt",    "München",  "Stuttgart"};
  g.load_vertices(names, [&names](std::string_view& nm) {
    auto uid = static_cast<vertex_id_t<G>>(&nm - names.data());
    return init_vertex_value{uid, std::string(nm)};
  });

  graph_value(g) = "Germany Routes";

  SECTION("metadata") {
    // Do a simple check
    REQUIRE(10 == std::ranges::size(vertices(g)));
    size_t edge_cnt = 0, edge_cnt2 = 0;
    double total_dist = 0;
    for (vertex_id_t<G> uid = 0; auto&& u : vertices(g)) {
      for (auto&& uv : edges(g, u)) {
        ++edge_cnt;
        total_dist += edge_value(g, uv);
      }
      for (auto&& uv : edges(g, uid)) {
        ++edge_cnt2;
      }
      ++uid;
    }
    REQUIRE(edge_cnt == 11);
    REQUIRE(edge_cnt2 == 11);
    REQUIRE(total_dist == 2030.0);
  }

  SECTION("functions") {
    using id_type = uint32_t;
    auto&& d      = graph_value(g);

    auto uit = std::ranges::begin(vertices(g)) + 2;
    auto id  = vertex_id(g, uit);
    REQUIRE(id == 2);
    REQUIRE(std::is_same_v<id_type, decltype(id)>);
    auto& uval = vertex_value(g, *uit);
    REQUIRE(std::is_same_v<std::string&, decltype(uval)>);
    REQUIRE("Karlsruhe" == uval);
    auto deg = degree(g, *uit);
    REQUIRE(1 == deg);

    auto& uu = edges(g, *uit);
    REQUIRE(1 == std::ranges::size(uu));
    auto& uv = *std::ranges::begin(uu);
    REQUIRE(3 == target_id(g, uv));
    REQUIRE(250.0 == edge_value(g, uv));
    auto& v = target(g, uv);
    REQUIRE(vertex_value(g, v) == "Augsburg");

    auto vit = find_vertex(g, 4);
    REQUIRE(4 == vit - std::ranges::begin(vertices(g)));
    auto uvit = find_vertex_edge(g, *vit, 7);
    REQUIRE(edge_value(g, *uvit) == 186.0);
  }

  SECTION("const functions") {
    const G& g2   = g;
    using id_type = uint32_t;
    auto&& d      = graph_value(g2);

    auto uit = std::ranges::begin(vertices(g2)) + 2;
    auto id  = vertex_id(g2, uit);
    REQUIRE(id == 2);
    REQUIRE(std::is_same_v<id_type, decltype(id)>);
    auto& uval = vertex_value(g2, *uit);
    REQUIRE(std::is_same_v<const std::string&, decltype(uval)>);
    REQUIRE("Karlsruhe" == uval);
    auto deg = degree(g2, *uit);
    REQUIRE(1 == deg);

    auto& uu = edges(g2, *uit);
    REQUIRE(1 == std::ranges::size(uu));
    auto& uv = *std::ranges::begin(uu);
    REQUIRE(3 == target_id(g2, uv));
    REQUIRE(250.0 == edge_value(g2, uv));
    auto& v = target(g2, uv);
    REQUIRE(vertex_value(g2, v) == "Augsburg");

    auto vit = find_vertex(g2, 4);
    REQUIRE(4 == vit - std::ranges::begin(vertices(g2)));
    auto uvit = find_vertex_edge(g2, *vit, 7);
    REQUIRE(edge_value(g2, *uvit) == 186.0);
  }
};

TEST_CASE("Germany routes CSV+dov test", "[csv][dov][germany]") {
  init_console();
  using G  = routes_dov_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

  auto frankfurt    = find_frankfurt(g);
  auto frankfurt_id = find_frankfurt_id(g);

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  SECTION("metadata") {
    REQUIRE(10 == std::ranges::size(vertices(g)));
    size_t total_edge_cnt = 0;
    double total_dist     = 0;
    for (auto&& u : vertices(g)) {
      size_t edge_cnt = 0;
      for (auto&& uv : edges(g, u)) {
        ++edge_cnt; // forward_list doesn't have size()
        total_dist += edge_value(g, uv);
      }
      total_edge_cnt += edge_cnt;
#if 0
      auto&& ee = (edges(g, u));
      int    x  = 0;
#else
      static_assert(std::ranges::sized_range<vertex_edge_range_t<G>>); // begin(r), end(r), size(r)?
      if constexpr (std::ranges::sized_range<vertex_edge_range_t<G>>) {
        REQUIRE(edge_cnt == std::graph::degree(g, u));
      }
#endif
    }
    REQUIRE(total_edge_cnt == 11);
    REQUIRE(total_dist == 2030.0);
  }

  SECTION("content") {
    std::string_view test_name = "Germany Routes using deque+vector";
#if TEST_OPTION == TEST_OPTION_OUTPUT
    cout << "\n" << test_name << "\n---------------------------------" << endl << routes_graph(g) << endl;
    int x = 0; // results are identifcal with csv_routes_csr_tests

               //Germany Routes using deque+vector
    //---------------------------------
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
    auto           ui  = begin(vertices(g));
    vertex_id_t<G> uid = 0;
    if (ui != end(vertices(g))) {
      REQUIRE(0 == uid);
      REQUIRE("Frankf\xc3\xbcrt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(1 == target_id(g, *uvi));
      REQUIRE("Mannheim" == vertex_value(g, target(g, *uvi)));
      REQUIRE(85 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(4 == target_id(g, *uvi));
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(217 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(6 == target_id(g, *uvi));
      REQUIRE("Kassel" == vertex_value(g, target(g, *uvi)));
      REQUIRE(173 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(3 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(1 == ++uid);
      REQUIRE("Mannheim" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(2 == target_id(g, *uvi));
      REQUIRE("Karlsruhe" == vertex_value(g, target(g, *uvi)));
      REQUIRE(80 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(2 == ++uid);
      REQUIRE("Karlsruhe" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(3 == target_id(g, *uvi));
      REQUIRE("Augsburg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(250 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(3 == ++uid);
      REQUIRE("Augsburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_id(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(84 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(4 == ++uid);
      REQUIRE("W\xc3\xbcrzburg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(5 == target_id(g, *uvi));
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, target(g, *uvi)));
      REQUIRE(103 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(7 == target_id(g, *uvi));
      REQUIRE("Erfurt" == vertex_value(g, target(g, *uvi)));
      REQUIRE(186 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(5 == ++uid);
      REQUIRE("N\xc3\xbcrnberg" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_id(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(167 == edge_value(g, *uvi));
      ++uv_cnt;

      ++uvi;
      REQUIRE(9 == target_id(g, *uvi));
      REQUIRE("Stuttgart" == vertex_value(g, target(g, *uvi)));
      REQUIRE(183 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(2 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(6 == ++uid);
      REQUIRE("Kassel" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;
      REQUIRE(8 == target_id(g, *uvi));
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, target(g, *uvi)));
      REQUIRE(502 == edge_value(g, *uvi));
      ++uv_cnt;

      REQUIRE(1 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(7 == ++uid);
      REQUIRE("Erfurt" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(8 == ++uid);
      REQUIRE("M\xc3\xbcnchen" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }
    if (++ui != end(vertices(g))) {
      REQUIRE(9 == ++uid);
      REQUIRE("Stuttgart" == vertex_value(g, *ui));

      auto   uvi    = begin(edges(g, *ui));
      size_t uv_cnt = 0;

      REQUIRE(0 == uv_cnt);
    }

    REQUIRE(10 == size(vertices(g))); // all vertices visited?
#endif
  }
}
