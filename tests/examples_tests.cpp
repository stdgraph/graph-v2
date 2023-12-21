#include <catch2/catch.hpp>
//#include "csv_routes.hpp"
#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include "graph/graph.hpp"
#include "graph/views/breadth_first_search.hpp"
#include "graph/container/compressed_graph.hpp"

using std::vector;
using std::string;
using std::tuple;
using std::get;
using std::declval;

using std::is_const_v;
using std::is_reference_v;
using std::is_lvalue_reference_v;
using std::is_same_v;
using std::remove_reference_t;
using std::add_lvalue_reference_t;

using std::integral;
using std::ranges::forward_range;
using std::ranges::random_access_range;

using std::graph::vertex_range_t;
using std::graph::vertex_iterator_t;
using std::graph::vertex_reference_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_reference_t;
using std::graph::vertex_id_t;
using namespace std::graph::views;

#if 0
#  if 0
template <class G1a>
auto check1(G1a&& g) {
  static_assert(!is_const_v<G1a>);
}
template <class G2b>
auto check2(G2b&& g) {
  static_assert(is_const_v<std::remove_reference_t<G2b>>);
  //static_assert(is_const_v<decltype(g)>);
  //static_assert(is_const_v<G2b>);
}
template <class G3c>
auto check3(G3c&& g) {
  static_assert(is_const_v<std::remove_reference_t<G3c>>);
  static_assert(std::is_volatile_v<std::remove_reference_t<G3c>>);
  //static_assert(is_const_v<decltype(g)>);
  //static_assert(is_const_v<G3c>);
}
void check_test() {
  using G = vector<vector<tuple<int>>>;
  {
    using G1 = G;
    G1 g;
    check1(g);
  }
  {
    using G2 = const G;
    static_assert(is_const_v<G2>);
    G2 g;
    G2& g2 = g;
    static_assert(is_const_v<decltype(g)>);
    check2(g);
    static_assert(is_const_v<std::remove_reference_t<decltype(g2)>>);
    check2(g2);
  }
  {
    using G3 = const volatile G;
    static_assert(is_const_v<G3>);
    static_assert(std::is_volatile_v<G3>);
    G3  g;
    G3& g3 = g;
    static_assert(is_const_v<decltype(g)>);
    check3(g);
    static_assert(is_const_v<std::remove_reference_t<decltype(g3)>>);
    check3(g3);
  }
}
#  endif

namespace mygraph {
using vertex_id_type = int;
using edge_type = tuple<vertex_id_type>;
using edges_type = vector<edge_type>;
using vertices_type = vector<edges_type>;
using Graph = vertices_type;

vertex_id_type vertex_id(const Graph& g, vertex_iterator_t<const Graph> it) { return static_cast<vertex_id_type>(it - std::ranges::begin(std::graph::vertices(g))); }
vertex_id_type target_id(const Graph& g, const edge_type& uv) noexcept { return get<0>(uv); }

//using Graph = vector<vector<tuple<int>>>;
//vertex_id_t<const Graph> target_id(const Graph& g, edge_reference_t<const Graph> uv) noexcept { return get<0>(uv); }
static_assert(is_same_v<edge_reference_t<const Graph>, const tuple<int>&>);
} // namespace mygraph

vector<string> actors{"Tom Cruise",        "Kevin Bacon",    "Hugo Weaving",  "Carrie-Anne Moss", "Natalie Portman",
                      "Jack Nicholson",    "Kelly McGillis", "Harrison Ford", "Sebastian Stan",   "Mila Kunis",
                      "Michelle Pfeiffer", "Keanu Reeves",   "Julia Roberts"};

mygraph::Graph costar_adjacency_list{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                                     {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                                     {1, 4}};

// in case the previous def of target_id(g,uv) isn't found...
vertex_id_t<const mygraph::Graph> target_id(const mygraph::Graph&                  g,
                                            edge_reference_t<const mygraph::Graph> uv) noexcept {
  return get<0>(uv);
}

#  if 1
TEST_CASE("const vertex-vertex-tuple types", "[compressed][bfs][example][bacon]") {
  using G     = const mygraph::Graph;
  using _UnCV = std::remove_cvref_t<G>;

  static_assert(random_access_range<vertex_range_t<G>>, "vertices range is random access");
  static_assert(is_lvalue_reference_v<vertex_range_t<G>>, "vertices range is an lvalue reference");
  static_assert(is_const_v<remove_reference_t<vertex_range_t<G>>>, "vertices ranges is const");
  static_assert(is_same_v<vertex_range_t<G>, const vector<vector<tuple<int>>>&>,
                "vertices range is a const vector<vector<tuple<int>>> reference");

  static_assert(integral<vertex_id_t<G>>, "Vertex id is integral");
  static_assert(is_lvalue_reference_v<vertex_range_t<G>>, "vertices range is an lvalue reference");
  static_assert(!is_const_v<remove_reference_t<vertex_id_t<G>>>, "Vertex id is not const");

  static_assert(forward_range<vertex_range_t<G>>, "vertex edges range is forward range");
  static_assert(std::is_lvalue_reference_v<vertex_edge_range_t<G>>, "vertex edges range is an lvalue reference");
  static_assert(is_const_v<remove_reference_t<vertex_edge_range_t<G>>>, "vertex edges range is const");
  static_assert(std::is_same_v<vertex_edge_range_t<G>, const vector<tuple<int>>&>,
                "vertex edges range is a const vertex<tuple<int>> reference");

  static_assert(std::is_lvalue_reference_v<edge_reference_t<G>>, "Edge is a reference");
  static_assert(is_const_v<remove_reference_t<edge_reference_t<G>>>, "Edge is const");
  static_assert(std::is_same_v<edge_reference_t<G>, const tuple<int>&>, "Edge is a const tuple<int> reference");
  static_assert(std::is_same_v<edge_reference_t<G>, std::add_lvalue_reference_t<const tuple<int>>>, "Edge is a const tuple<int> reference");

  static_assert(std::graph::_Edges::_Can_ref_eval<G, _UnCV>, "Can auto evaluate edges(g,u)");
  static_assert(std::graph::_Edges::_Can_id_eval<G, _UnCV>, "Can auto evaluate edges(g,uid)");

  static_assert(!std::graph::_Target_id::_Has_ref_member<G, _UnCV>, "Has no member target_id(g,uv)");
  static_assert(std::graph::_Target_id::_Has_ref_ADL<G, _UnCV>, "Has ADL target_id(g,uv)");

  static_assert(std::graph::_Target::_Can_ref_eval<G, _UnCV>, "Can auto evaluate target(g,uv)");
  //static_assert(std::is_same_v<vertex_reference_t<G>,
  //                             decltype(std::graph::target(declval<G>(), declval<edge_reference_t<G>>()))>);

  for (auto&& u : std::graph::vertices(costar_adjacency_list)) {
    for (auto&& uv : std::graph::edges(costar_adjacency_list, u)) {
      auto vid = std::graph::target_id(costar_adjacency_list, uv);
      static_assert(std::integral<decltype(vid)>);
    }
  }
}
#  endif

TEST_CASE("non-const vertex-vertex-tuple types", "[compressed][bfs][example][bacon]") {
  using G     = mygraph::Graph;
  using _UnCV = std::remove_cvref_t<G>;
  static_assert(std::ranges::random_access_range<std::graph::vertex_range_t<G>>, "vertices range is a reference");
  static_assert(std::is_same_v<vertex_range_t<G>, vector<vector<tuple<int>>>&>,
                "vertices range is a vector<vector<tuple<int>>> reference");

  static_assert(std::integral<vertex_id_t<G>>, "Vertex id is integral");

  static_assert(std::is_lvalue_reference_v<vertex_edge_range_t<G>>, "edges range is a reference");
  static_assert(std::is_same_v<vertex_edge_range_t<G>, vector<tuple<int>>&>,
                "edges range is a vertex<tuple<int>> reference");

  static_assert(std::is_lvalue_reference_v<edge_reference_t<G>>, "Edge is a reference");
  static_assert(std::is_same_v<edge_reference_t<G>, tuple<int>&>, "Edge is a tuple<int> reference");
  //static_assert(std::is_same_v<edge_reference_t<G>, std::add_lvalue_reference<tuple<int>>>, "Edge is a tuple<int> reference");

  static_assert(std::graph::_Edges::_Can_ref_eval<G, _UnCV>, "Can auto evaluate edges(g,u)");
  static_assert(std::graph::_Edges::_Can_id_eval<G, _UnCV>, "Can auto evaluate edges(g,uid)");

  static_assert(!std::graph::_Target_id::_Has_ref_member<G, _UnCV>, "Has no member target_id(g,uv)");
  static_assert(std::graph::_Target_id::_Has_ref_ADL<G, _UnCV>, "Has ADL target_id(g,uv)");

  static_assert(std::graph::_Target::_Can_ref_eval<G, _UnCV>, "Can auto evaluate target(g,uv)");
  static_assert(std::is_same_v<vertex_reference_t<G>,
                               decltype(std::graph::target(declval<G>(), declval<edge_reference_t<G>>()))>);

  for (auto&& u : std::graph::vertices(costar_adjacency_list)) {
    for (auto&& uv : std::graph::edges(costar_adjacency_list, u)) {
      auto vid = std::graph::target_id(costar_adjacency_list, uv);
      static_assert(std::integral<decltype(vid)>);
    }
  }
}


TEST_CASE("Kevin Bacon example", "[compressed][bfs][example][bacon]") {
  using G = mygraph::Graph;

  vector<int> bacon_number(size(actors));
  //auto        evf = [](edge_reference_t<G> uv) { return uv; };

  // 1 -> Kevin Bacon
  for (auto&& [uid, vid, uv] : sourced_edges_breadth_first_search(costar_adjacency_list, 1)) {
    bacon_number[vid] = bacon_number[uid] + 1;
  }

  for (int i = 0; i < size(actors); ++i) {
    std::cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
  }
}
#endif //0
