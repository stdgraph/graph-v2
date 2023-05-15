#pragma once

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include <vector>

namespace std::graph {

template <adjacency_list G>
struct reaches {
  vertex_id_t<G> from;
  vertex_id_t<G> to;
};

/**
 * @ingroup graph_algorithms
 * @brief Warshall's Transitive Closure algorithm to find all reachable vertices from
 *        source vertices.
 * 
 * Transitive closure returns all vertices that can be reached from a source vertex,
 * for all source vertices. This algorithm specializes on a dense graph using
 * Warshall's algorithm. Complexity is O(n^3).
 */
// clang-format off
template <adjacency_list G, typename OutIter, typename Alloc = allocator<bool>>
  requires ranges::random_access_range<vertex_range_t<G>> && 
           integral<vertex_id_t<G>> && 
           output_iterator<OutIter, reaches<G>>
           //&& directed<G>
// clang-format on
constexpr void warshall_transitive_closure(G& g, OutIter result_iter, Alloc alloc = Alloc()) {
  using views::vertexlist;
  using views::incidence;
  const size_t      V = ranges::size(g);
  std::vector<bool> reach(V * V, alloc); // adjacency matrix bitmap

  // transform edges into adjacency matrix bitmap
  for (auto&& [uid, u] : vertexlist(g))
    for (auto&& [vid, uv] : incidence(g, uid))
      reach[uid * V + vid] = true;

  // evaluate transitive closure
  for (auto&& [kid, k] : vertexlist(g))
    for (auto&& [uid, u] : vertexlist(g))
      for (auto&& [vid, v] : vertexlist(g))
        reach[uid * V] = reach[uid * V + vid] || (reach[uid * V + kid] && reach[kid * V + vid]);

  // output results
  for (auto&& [uid, u] : vertexlist(g))
    for (auto&& [vid, v] : vertexlist(g))
      if (reach[uid * V + vid])
        *result_iter = {uid, vid};
}

} // namespace std::graph
