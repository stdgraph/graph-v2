//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//
//  Make sure various combinations of graphs can compile

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wunused-result"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4834) // warning C4834: discarding return value of function with 'nodiscard' attribute
#  pragma warning(disable : 4267) // warning C4267: 'argument': conversion from 'size_t' to 'int', possible loss of data
#endif

#include <list>
#include <vector>

//#include "bfs_edge_range.hpp"
#include "graph/graph.hpp"
#include "graph/views/breadth_first_search.hpp"

#include "imdb-graph.hpp"
#include "karate-graph.hpp"
#include "ospf-graph.hpp"
#include "spice-graph.hpp"
#include "utilities.hpp"

#include <vector>

int main() {

  /**
   * Karate is only represented as index edge list and index adjacency list
   */
  std::vector<std::vector<size_t>> G(34);
  push_back_plain_fill(karate_index_edge_list, G, false, 0);
  static_assert(std::graph::adjacency_list<decltype(G)>);

  std::vector<std::list<std::tuple<size_t>>> H(34);
  push_back_plain_fill(karate_index_edge_list, H, false, 0);
  push_back_fill(karate_index_edge_list, H, false, 0);

  /**
   * Other graphs have vertices and edges tables
   */
  auto a = make_plain_graph(ospf_vertices, ospf_edges);
  auto b = make_property_graph(ospf_vertices, ospf_edges);
  auto c = make_index_graph(ospf_vertices, ospf_edges);

  auto d = make_plain_graph<decltype(ospf_vertices), decltype(ospf_edges), std::vector<std::list<size_t>>>(
        ospf_vertices, ospf_edges, true);

  auto e = make_index_graph<decltype(ospf_vertices), decltype(ospf_edges),
                            std::vector<std::vector<std::tuple<size_t, size_t>>>>(ospf_vertices, ospf_edges, true);

  auto [f, g] = make_plain_bipartite_graphs<>(movies, actors, movies_actors);
  auto h      = make_plain_bipartite_graph(movies, actors, movies_actors, 0);
  auto i      = make_plain_bipartite_graph(movies, actors, movies_actors, 1);

  auto [j, k] = make_plain_bipartite_graphs<decltype(movies), decltype(actors), decltype(movies_actors),
                                            std::vector<std::list<size_t>>>(movies, actors, movies_actors);
  auto l      = make_plain_bipartite_graph<decltype(movies), decltype(actors), decltype(movies_actors),
                                           std::vector<std::list<size_t>>>(movies, actors, movies_actors, 0);
  auto m      = make_plain_bipartite_graph<decltype(movies), decltype(actors), decltype(movies_actors),
                                           std::vector<std::list<size_t>>>(movies, actors, movies_actors, 1);

  auto n = make_plain_graph<decltype(spice_vertices), decltype(spice_edges), std::vector<std::list<int>>>(
        spice_vertices, spice_edges);
  auto o = make_plain_graph<decltype(spice_vertices), decltype(spice_edges_values), std::vector<std::list<int>>>(
        spice_vertices, spice_edges_values);
  auto p = make_index_graph(spice_vertices, spice_edges);
  auto q = make_index_graph(spice_vertices, spice_edges_values);
  auto r = make_property_graph(spice_vertices, spice_edges);
  auto s = make_property_graph(spice_vertices, spice_edges_values);

  //bfs_edge_range(n, 1);
  std::graph::views::edges_breadth_first_search(n, 1);

  //bfs_edge_range(o, 1);
  std::graph::views::edges_breadth_first_search(o, 1);

  //bfs_edge_range(p, 1);
  std::graph::views::edges_breadth_first_search(p, 1);

  //bfs_edge_range(q, 0);
  std::graph::views::edges_breadth_first_search(q, 0);

  return 0;
}

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
