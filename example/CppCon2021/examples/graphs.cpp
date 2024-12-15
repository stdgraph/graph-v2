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
#include <iostream>
#include <iomanip>

//#include "bfs_edge_range.hpp"
#include "graph/graph.hpp"
#include "graph/views/breadth_first_search.hpp"

#include "imdb-graph.hpp"
#include "karate-graph.hpp"
#include "ospf-graph.hpp"
#include "spice-graph.hpp"
#include "utilities.hpp"

int main() {

  /**
   * Karate is only represented as index edge list and index adjacency list
   */
  std::vector<std::vector<size_t>> G(34);
  push_back_plain_fill(karate_index_edge_list, G, false, 0);
  static_assert(graph::adjacency_list<decltype(G)>);
  std::cout << "Karate adjacency list:\n";
  std::cout << "size = " << G.size() << std::endl;
  for (size_t uid = 0; uid < G.size(); ++uid) {
    std::cout << std::setw(3) << uid << ":";
    for (auto&& vid : G[uid]) {
      std::cout << " " << vid;
    }
    std::cout << std::endl;
  }

  std::vector<std::list<std::tuple<size_t>>> H(34);
  push_back_plain_fill(karate_index_edge_list, H, false, 0);
  std::cout << "\nKarate (edge_list plain fill):\n";
  std::cout << "size = " << H.size() << std::endl;
  for (size_t uid = 0; uid < H.size(); ++uid) {
    std::cout << std::setw(3) << uid << ":";
    for (auto&& [vid] : H[uid]) {
      std::cout << " " << vid;
    }
    std::cout << std::endl;
  }


  push_back_fill(karate_index_edge_list, H, false, 0);
  std::cout << "\nKarate (edge_list fill...adding more):\n";
  std::cout << "size = " << H.size() << std::endl;
  for (size_t uid = 0; uid < H.size(); ++uid) {
    std::cout << std::setw(3) << uid << ":";
    for (auto&& [vid] : H[uid]) {
      std::cout << " " << vid;
    }
    std::cout << std::endl;
  }

  //----------------------------------------------------------------------------

  /**
   * Other graphs have vertices and edges tables
   */
  auto a = make_plain_graph(ospf_vertices, ospf_edges);
  std::cout << "\nOSPF plain graph:\n";
  std::cout << "size = " << a.size() << std::endl;
  for (size_t uid = 0; uid < a.size(); ++uid) {
    std::cout << std::setw(3) << ospf_vertices[uid] << ":";
    for (auto&& vid : a[uid]) {
      std::cout << " " << ospf_vertices[vid];
    }
    std::cout << std::endl;
  }

  auto b = make_property_graph(ospf_vertices, ospf_edges);
  std::cout << "\nOSPF property graph:\n";
  std::cout << "size = " << b.size() << std::endl;
  for (size_t uid = 0; uid < b.size(); ++uid) {
    std::cout << std::setw(3) << ospf_vertices[uid] << ":";
    for (auto&& [vid, val] : b[uid]) {
      std::cout << " " << ospf_vertices[vid] << ":" << val;
    }
    std::cout << std::endl;
  }

  auto c = make_index_graph(ospf_vertices, ospf_edges);
  std::cout << "\nOSPF index graph:\n";
  std::cout << "size = " << c.size() << std::endl;
  for (size_t uid = 0; uid < c.size(); ++uid) {
    std::cout << std::setw(3) << ospf_vertices[uid] << ":";
    for (auto&& [vid, val] : c[uid]) {
      std::cout << " " << ospf_vertices[vid] << ":" << std::get<2>(ospf_edges[val]);
    }
    std::cout << std::endl;
  }

  auto d = make_plain_graph<decltype(ospf_vertices), decltype(ospf_edges), std::vector<std::list<size_t>>>(
        ospf_vertices, ospf_edges, true);
  std::cout << "\nOSPF plain graph (vector of lists):\n";
  std::cout << "size = " << d.size() << std::endl;
  for (size_t uid = 0; uid < d.size(); ++uid) {
    std::cout << std::setw(3) << ospf_vertices[uid] << ":";
    for (auto&& vid : d[uid]) {
      std::cout << " " << ospf_vertices[vid];
    }
    std::cout << std::endl;
  }

  auto e = make_index_graph<decltype(ospf_vertices), decltype(ospf_edges),
                            std::vector<std::vector<std::tuple<size_t, size_t>>>>(ospf_vertices, ospf_edges, true);
  std::cout << "\nOSPF index graph (vector of vector of tuples):\n";
  std::cout << "size = " << e.size() << std::endl;
  for (size_t uid = 0; uid < e.size(); ++uid) {
    std::cout << std::setw(3) << ospf_vertices[uid] << ":";
    for (auto&& [vid, val] : e[uid]) {
      std::cout << " " << ospf_vertices[vid] << ":" << std::get<2>(ospf_edges[val]);
    }
    std::cout << std::endl;
  }

  //----------------------------------------------------------------------------

  auto [f, g] = make_plain_bipartite_graphs<>(movies, actors, movies_actors);
  auto h      = make_plain_bipartite_graph(movies, actors, movies_actors, 0);
  auto i      = make_plain_bipartite_graph(movies, actors, movies_actors, 1);
  std::cout << "\nMovies-actors plain bipartite graphs\n";
  std::cout << "index 0: " << f.size() << "==" << h.size() << std::endl;
  for (size_t uid = 0; uid < f.size(); ++uid) {
    std::cout << std::setw(20) << movies[uid] << ": |";
    for (auto&& vid : f[uid]) {
      std::cout << actors[vid] << "|";
    }
    std::cout << std::endl;
  }
  std::cout << "index 1: " << g.size() << "==" << i.size() << std::endl;
  for (size_t uid = 0; uid < g.size(); ++uid) {
    std::cout << std::setw(20) << actors[uid] << ": |";
    for (auto&& vid : g[uid]) {
      std::cout << movies[vid] << "|";
    }
    std::cout << std::endl;
  }

  auto [j, k] = make_plain_bipartite_graphs<decltype(movies), decltype(actors), decltype(movies_actors),
                                            std::vector<std::list<size_t>>>(movies, actors, movies_actors);
  auto l      = make_plain_bipartite_graph<decltype(movies), decltype(actors), decltype(movies_actors),
                                           std::vector<std::list<size_t>>>(movies, actors, movies_actors, 0);
  auto m      = make_plain_bipartite_graph<decltype(movies), decltype(actors), decltype(movies_actors),
                                           std::vector<std::list<size_t>>>(movies, actors, movies_actors, 1);
  std::cout << "\nMovies-actors plain bipartite graphs (vector of lists)\n";
  std::cout << "index 0: " << j.size() << "==" << l.size() << std::endl;
  for (size_t uid = 0; uid < j.size(); ++uid) {
    std::cout << std::setw(20) << movies[uid] << ": |";
    for (auto&& vid : j[uid]) {
      std::cout << actors[vid] << "|";
    }
    std::cout << std::endl;
  }
  std::cout << "index 1: " << k.size() << "==" << m.size() << std::endl;
  for (size_t uid = 0; uid < k.size(); ++uid) {
    std::cout << std::setw(20) << actors[uid] << ": |";
    for (auto&& vid : k[uid]) {
      std::cout << movies[vid] << "|";
    }
    std::cout << std::endl;
  }

  //----------------------------------------------------------------------------

  auto n = make_plain_graph<decltype(spice_vertices), decltype(spice_edges), std::vector<std::list<int>>>(
        spice_vertices, spice_edges);
  auto o = make_plain_graph<decltype(spice_vertices), decltype(spice_edges_values), std::vector<std::list<int>>>(
        spice_vertices, spice_edges_values);
  auto p = make_index_graph(spice_vertices, spice_edges);
  auto q = make_index_graph(spice_vertices, spice_edges_values);
  auto r = make_property_graph(spice_vertices, spice_edges);
  auto s = make_property_graph(spice_vertices, spice_edges_values);

  std::cout << "\nSpice property graph (using edges+values)\n";
  std::cout << "Size: " << s.size() << std::endl;
  for (size_t uid = 0; uid < s.size(); ++uid) {
    std::cout << std::setw(4) << spice_vertices[uid] << ": |";
    for (auto&& [vid, comp, val] : s[uid]) {
      std::cout << std::setw(3) << spice_vertices[vid] << ":" << comp << "/" << val << "|";
    }
    std::cout << std::endl;
  }


  //bfs_edge_range(n, 1);
  graph::views::edges_breadth_first_search(n, 1);

  //bfs_edge_range(o, 1);
  graph::views::edges_breadth_first_search(o, 1);

  //bfs_edge_range(p, 1);
  graph::views::edges_breadth_first_search(p, 1);

  //bfs_edge_range(q, 0);
  graph::views::edges_breadth_first_search(q, 0);

  return 0;
}

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
