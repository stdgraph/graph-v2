/**
 * @file topological_sort.hpp
 * 
 * @brief Single-Source & multi-source topological sort search.
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

#ifndef GRAPH_TOPO_SORT_ALGO_HPP
#  define GRAPH_TOPO_SORT_ALGO_HPP

namespace graph {

using vertex_id_type = int;
using Graph          = std::vector<std::vector<vertex_id_type>>;
using Predecessors   = std::vector<vertex_id_type>;
using Source         = vertex_id_type;
using Sources        = std::vector<Source>;
using Distance       = int;
using Distances      = std::vector<Distance>;

class topological_sort_visitor_base {
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
inline Distance topological_sort_infinite_distance() { return std::numeric_limits<Distance>::max(); }

/**
 * @ingroup graph_algorithms
 * @brief Returns a distance value of zero.
 * 
 * @return A value of zero distance.
*/
inline Distance topological_sort_zero() { return 0; }

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance values to the infinite value.
 * 
 * @param distances The range of distance values to initialize.
*/
inline void init_topological_sort(Distances& distances) {
  for (size_t i = 0; i < distances.size(); ++i) {
    distances[i] = topological_sort_infinite_distance();
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance and predecessor values for topological sort.
 * 
 * @param distances The range of distance values to initialize. Each value will be assigned the infinite distance.
 * @param predecessors The range of predecessors to initialize. Each value will be assigned the vertex id.
*/
inline void init_topological_sort(Distances& distances, Predecessors& predecessors) {
  init_topological_sort(distances);
  for (size_t i = 0; i < predecessors.size(); ++i) {
    predecessors[i] = static_cast<range_value_t<Predecessors>>(i);
  }
}

/**
 * @brief Single-source topological sort.
 * 
 * @param g            The graph.
 * @param source	   The source vertex.
 * @param predecessors The predecessor vertex for g[uid].
 */

// C++
inline void topological_sort(const Graph& g, const Source& source, Distances& distances, Predecessors& predecessors) {}

// C
inline void topological_sort(const Graph*  g,
                             const Source  source,
                             const size_t  distances_len,
                             Distances*    distances,
                             const size_t  predecessors_len,
                             Predecessors* predecessors) {
  // std::queue & std::priority_queue
  // https://en.cppreference.com/w/cpp/container
}


} // namespace graph

#endif // GRAPH_TOPO_SORT_ALGO_HPP
