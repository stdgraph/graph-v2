/**
 * @file tc.hpp
 * 
 * @brief Triangle counting.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 *   Kevin Deweese
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

#ifndef GRAPH_TC_HPP
#  define GRAPH_TC_HPP

namespace graph {

/**
 * @ingroup graph_algorithms
 * @brief Find the number of triangles in a graph.
 * 
 * Complexity: O(|V|^3)
 * 
 * @tparam G          The graph type.
 *
 * @param g           The graph.
 */
template <adjacency_list G>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
size_t triangle_count(G&& g) {
  size_t N(size(vertices(g)));
  size_t triangles(0);

  for (vertex_id_t<G> uid = 0; uid < N; ++uid) {
    graph::incidence_iterator<G> i(g, uid);
    auto                         ie = end(edges(g, uid));
    while (i != ie) {
      auto&& [vid, uv] = *i;
      graph::incidence_iterator<G> j(g, vid);

      auto je = end(edges(g, vid));
      auto i2 = i;

      // Alternatively use std::set_intersection(i2, ie, j, je, counter) but this is slower
      while (i2 != ie && j != je) {
        auto&& [wid1, uw] = *i2;
        auto&& [wid2, vw] = *j;
        if (wid1 < wid2) {
          ++i2;
        } else if (wid2 < wid1) {
          ++j;
        } else {
          ++triangles;
          ++i2;
          ++j;
        }
      }
      ++i;
    }
  }
  return triangles;
}
} // namespace graph

#endif //GRAPH_TC_HPP
