#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/co_bfs.hpp"

#include <variant>
#include <queue>
#include <algorithm>

#ifndef GRAPH_CO_DIJKSTRA_CLRS_HPP
#  define GRAPH_CO_DIJKSTRA_CLRS_HPP

namespace std::graph {

enum class dijkstra_event : int {
  initialize_vertex,
  discover_vertex,
  examine_vertex,
  examine_edge,
  edge_relaxed,
  edge_not_relaxed,
  finish_vertex
};



}

#endif // GRAPH_CO_DIJKSTRA_CLRS_HPP
