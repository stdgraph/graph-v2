#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"
#include <stack>
#include <vector>
#include <functional>
#include <variant>

#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Woverloaded-virtual"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <boost/cobalt/main.hpp>

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif


#ifndef GRAPH_CO_BFS_HPP
#  define GRAPH_CO_BFS_HPP

namespace std::graph {

enum class bfs_event : int {
  none              = 0, // useful?
  initialize_vertex = 0x0001,
  examine_vertex    = 0x0002,
  examine_edge      = 0x0004,
  discover_vertex   = 0x0008,
  edge_relaxed      = 0x0010,
  edge_not_relaxed  = 0x0020,
  finish_vertex     = 0x0040,

  vertex_default = discover_vertex, // useful?
  edge_default   = examine_edge     // useful?
};

enum class bfs_features : int {
  edge_basic     = 0x0000,
  edge_reference = 0x0001,

  edge_unsourced = 0x0000,
  edge_sourced   = 0x0002,

  vertex_basic     = 0x0000,
  vertex_reference = 0x0001
};

template <class VId, bool Sourced, class E, class EV, class V, class VV>
using bfs_descriptor = std::variant<       //
      vertex_descriptor<VId, V, VV>,       //
      edge_descriptor<VId, Sourced, E, EV> //
      >;
// bfs_descriptor --> traverse_descriptor? (include neighbor_descriptor?)

// using bfs_desc = bfs_descriptor<...>;
//
// The following functions would be overloaded to access all possible values of the variant.
// source_id(bfs_desc);
// target_id(bfs_desc);
// edge(bfs_desc);
// edge_value(bfs_desc);
// vertex_id(bfs_desc);
// vertex(bfs_desc);
// vertex_value(bfs_desc);

// Additional values for bfs_descriptor? (e.g. distance, color, predecessor, etc.)
// An alternative could be a pointer to an internal structure that holds these values, and the functions would be overloaded to access them.
// That would be a much more versatile solution that goes beyond the existing descriptors.
// Structured bindings would no longer be necessary. (Could we retain them for our uses? Maybe retain them for existing views?)

} // namespace std::graph

#endif // GRAPH_CO_BFS_HPP
