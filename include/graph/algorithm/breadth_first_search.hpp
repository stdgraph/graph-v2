/**
 * @file breadth_first_search.hpp
 * 
 * @brief Single-Source & multi-source breadth-first search.
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

#include <vector>
#include <queue>
#include <ranges>
#include <limits>
#include <fmt/format.h>

#ifndef GRAPH_BREADTH_FIRST_SEARCH_HPP
#  define GRAPH_BREADTH_FIRST_SEARCH_HPP

namespace graph {

using vertex_id_type = int;
using Graph          = std::vector<std::vector<vertex_id_type>>;
using Predecessors   = std::vector<vertex_id_type>;
using Source         = vertex_id_type;
using Sources        = std::vector<Source>;
using Distance       = int;
using Distances      = std::vector<Distance>;

class dfs_visitor_base {
  // Types
public:
  // Visitor Functions
public:
  // vertex visitor functions
  void on_initialize_vertex(const vertex_id_type& uid) {}
  void on_discover_vertex(const vertex_id_type& uid) {}
  void on_examine_vertex(const vertex_id_type& uid) {}
  void on_finish_vertex(const vertex_id_type& uid) {}

  // edge visitor functions
  void on_examine_edge(const vertex_id_type& uid) {}
};

/**
 * @ingroup graph_algorithms
 * @brief Returns the largest value used to represent an infinite distance.
 * 
 * @return The largest possible distance value.
*/
inline Distance bfs_infinite_distance() { return std::numeric_limits<Distance>::max(); }

/**
 * @ingroup graph_algorithms
 * @brief Returns a distance value of zero.
 * 
 * @return A value of zero distance.
*/
inline Distance bfs_zero() { return 0; }

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance values to the infinite value.
 * 
 * @param distances The range of distance values to initialize.
*/
inline void init_bfs(Distances& distances) {
  for (size_t i = 0; i < distances.size(); ++i) {
    distances[i] = bfs_infinite_distance();
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance and predecessor values for bfs.
 * 
 * @param distances The range of distance values to initialize. Each value will be assigned the infinite distance.
 * @param predecessors The range of predecessors to initialize. Each value will be assigned the vertex id.
*/
inline void init_bfs(Distances& distances, Predecessors& predecessors) {
  init_bfs(distances);
  for (size_t i = 0; i < predecessors.size(); ++i) {
    predecessors[i] = i;
  }
}

/**
 * @brief Single-source breadth-first search.
 * 
 * @param g            The graph.
 * @param source	   The source vertex.
 * @param predecessors The predecessor vertex for g[uid].
 */

// C++
inline void
breadth_first_search(const Graph& g, const Source& source, Distances& distances, Predecessors& predecessors) {}

// C
inline void breadth_first_search(const Graph*  g,
                                 const Source  source,
                                 const size_t  distances_len,
                                 Distances*    distances,
                                 const size_t  predecessors_len,
                                 Predecessors* predecessors) {}


} // namespace graph

#endif // GRAPH_BREADTH_FIRST_SEARCH_HPP
