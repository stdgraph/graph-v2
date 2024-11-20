/**
 * @file depth_first_search.hpp
 * 
 * @brief Single-Source & multi-source depth-first search.
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

#ifndef GRAPH_DFS_ALGORITHM_HPP
#  define GRAPH_DFS_ALGORITHM_HPP

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
inline Distance dfs_infinite_distance() { return std::numeric_limits<Distance>::max(); }

/**
 * @ingroup graph_algorithms
 * @brief Returns a distance value of zero.
 * 
 * @return A value of zero distance.
*/
inline Distance dfs_zero() { return 0; }

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance values to the infinite value.
 * 
 * @param distances The range of distance values to initialize.
*/
inline void init_dfs(Distances& distances) {
  for (size_t i = 0; i < distances.size(); ++i) {
    distances[i] = dfs_infinite_distance();
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance and predecessor values for dfs.
 * 
 * @param distances The range of distance values to initialize. Each value will be assigned the infinite distance.
 * @param predecessors The range of predecessors to initialize. Each value will be assigned the vertex id.
*/
inline void init_dfs(Distances& distances, Predecessors& predecessors) {
  init_dfs(distances);
  for (size_t i = 0; i < predecessors.size(); ++i) {
    predecessors[i] = static_cast<range_value_t<Predecessors>>(i);
  }
}

/**
 * @brief Single-source depth-first search.
 * 
 * @param g            The graph.
 * @param source	   The source vertex.
 * @param predecessors The predecessor vertex for g[uid].
 */

// C++
inline void depth_first_search(const Graph& g, const Source& source, Distances& distances, Predecessors& predecessors) {
}

// C
inline void depth_first_search(const Graph*  g,
                               const Source  source,
                               const size_t  distances_len,
                               Distances*    distances,
                               const size_t  predecessors_len,
                               Predecessors* predecessors) {
  // std::stack
  // https://en.cppreference.com/w/cpp/container
}


} // namespace graph

#endif // GRAPH_DFS_ALGORITHM_HPP
