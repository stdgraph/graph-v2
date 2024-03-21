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

enum class dijkstra_events : int {
  initialize_vertex,
  discover_vertex,
  examine_vertex,
  examine_edge,
  edge_relaxed,
  edge_not_relaxed,
  finish_vertex
};

constexpr dijkstra_events& operator&=(dijkstra_events& lhs, dijkstra_events rhs) noexcept {
  lhs = static_cast<dijkstra_events>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
constexpr dijkstra_events  operator&(dijkstra_events lhs, dijkstra_events rhs) noexcept { return (lhs &= rhs); }
constexpr dijkstra_events& operator|=(dijkstra_events& lhs, dijkstra_events rhs) noexcept {
  lhs = static_cast<dijkstra_events>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
constexpr dijkstra_events operator|(dijkstra_events lhs, dijkstra_events rhs) noexcept { return (lhs |= rhs); }


template<class G>
class co_dijkstra_clrs {
public:
  co_dijkstra_clrs() = default;
  co_dijkstra_clrs(const G& g, vertex_id_t<G> seed) : g_(g) {}

public:
public:
public:
public:

private:
  reference_wrapper<G> g_;
  vertex_id_t<G>       seed_{};
};

}

#endif // GRAPH_CO_DIJKSTRA_CLRS_HPP
