#pragma once

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include <vector>

namespace std::graph {

template <incidence_graph G>
struct reaches {
  vertex_key_t<G> from;
  vertex_key_t<G> to;
};

/// Transitive closure returns all vertices that can be reached from a source vertex,
/// for all source vertices. This algorithm specializes on a dense graph using
/// Warshall's algorithm. Complexity is O(n^3).
//
// clang-format off
template <incidence_graph G, typename OutIter, typename A = allocator<bool>>
  requires ranges::random_access_range<vertex_range_t<G>> && 
           integral<vertex_key_t<G>> && 
           output_iterator<OutIter, reaches<G>>
           //&& directed<G>
// clang-format on
constexpr void warshall_transitive_closure(G& g, OutIter result_iter, A alloc = A()) {
  using views::vertexlist;
  using views::incidence;
  const size_t      V = ranges::size(g);
  std::vector<bool> reach(V * V, alloc); // adjacency matrix bitmap

  // transform edges into adjacency matrix bitmap
  for (auto&& [ukey, u] : vertexlist(g))
    for (auto&& [vkey, uv] : incidence(g, ukey))
      reach[ukey * V + vkey] = true;

  // evaluate transitive closure
  for (auto&& [kkey, k] : vertexlist(g))
    for (auto&& [ukey, u] : vertexlist(g))
      for (auto&& [vkey, v] : vertexlist(g))
        reach[ukey * V] = reach[ukey * V + vkey] || (reach[ukey * V + kkey] && reach[kkey * V + vkey]);

  // output results
  for (auto&& [ukey, u] : vertexlist(g))
    for (auto&& [vkey, v] : vertexlist(g))
      if (reach[ukey * V + vkey])
        *result_iter = {ukey, vkey};
}

} // namespace std::graph
