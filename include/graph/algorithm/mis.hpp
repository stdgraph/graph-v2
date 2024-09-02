/**
 * @file mis.hpp
 * 
 * @brief Single-Source Shortest paths and shortest sistances algorithms using Dijkstra & 
 * Bellman-Ford algorithms.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

#ifndef GRAPH_MIS_HPP
#  define GRAPH_MIS_HPP

namespace graph {

/**
 * @ingroup graph_algorithms
 * @brief Find a maximal independent set of vertices
 * 
 * Complexity: O(|E|)
 * 
 * @tparam G          The graph type.
 * @tparam Iter       The output iterator type.
 * 
 * @param g           The graph.
 * @param mis         The output iterator.
 * @param seed        The seed which must be contained in the independent set.
 */

template <adjacency_list G, class Iter>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> && output_iterator<Iter, vertex_id_t<G>>
void maximal_independent_set(G&&            g,       // graph
                             Iter           mis,     // out: maximal independent set
                             vertex_id_t<G> seed = 0 // seed vtx
) {
  size_t N(size(vertices(g)));
  assert(seed < N && seed >= 0);

  std::vector<bool> removed_vertices(g.size());
  *mis++                 = seed;
  removed_vertices[seed] = true;
  for (auto&& [vid, v] : views::incidence(g, seed)) {
    removed_vertices[vid] = true;
  }

  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (!removed_vertices[uid]) {
      *mis++ = uid;
      for (auto&& [vid, v] : views::incidence(g, uid)) {
        removed_vertices[vid] = true;
      }
    }
  }
}

} // namespace graph

#endif //GRAPH_MIS_HPP
