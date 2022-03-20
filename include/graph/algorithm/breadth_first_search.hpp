//
//	Author: J. Phillip Ratzloff
//
// inspired by bfs_range.hpp from: NWGraph
//

#include "../graph.hpp"
#include <queue>
#include <vector>

#if !defined(GRAPH_BFS_HPP)
#  define GRAPH_BFS_HPP

namespace std::graph {

// options: depth_limit, {cancel, cancel_branch}

template <class G>
struct bfs_vertex_view {
  vertex_reference_t<G> vertex;
  vertex_reference_t<G> parent;
  vertex_id<G>          parent_id;
  vertex_id<G>          seed        = 0;
  bool                  is_path_end = false;
  size_t                depth       = 0;
};

template <class G>
struct bfs_edge_view {
  edge_reference_t<G>   edge;
  vertex_reference_t<G> parent;
  vertex_id<G>          parent_id;
  vertex_id<G>          seed         = 0;
  bool                  is_back_edge = false;
  size_t                depth        = 0;
};

//----------------------------------------------------------------------------------------
/// breadth-first search range for vertices, given a single seed vertex.
///
template <incidence_graph G, typename A = allocator<char>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class breadth_first_search_vertex_range {
public:
  // single-source BFS
  breadth_first_search_vertex_range(G& graph, vertex_id_t<G> seed, A alloc = A());

  // multi-source BFS
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  breadth_first_search_vertex_range(G& graph, const VKR& seeds, A alloc = A());

  class const_iterator {};
  class iterator : public const_iterator {};

public:
  iterator       begin();
  const_iterator begin() const;
  const_iterator cbegin() const;

  iterator       end();
  const_iterator end() const;
  const_iterator cend() const;
};


//----------------------------------------------------------------------------------------
/// breadth-first search range for edges, given a single seed vertex.
///
/// requires bi-directional edges to get last edge on a vertex
///
template <incidence_graph G, typename A = allocator<char>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class breadth_first_search_edge_range {
public:
  // single-source BFS
  breadth_first_search_edge_range(G& graph, vertex_id_t<G> seed, A alloc = A());

  // multi-source BFS
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  breadth_first_search_edge_range(G& graph, const VKR& seeds, A alloc = A());

  class const_iterator {};
  class iterator : public const_iterator {

  public:
    iterator       begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator       end();
    const_iterator end() const;
    const_iterator cend() const;
  };

} // namespace std::graph

#endif // GRAPH_BFS_HPP
