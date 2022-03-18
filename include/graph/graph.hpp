#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include "detail/graph_access.hpp"


#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {
// Template parameters:
// G    - Graph
// GV   - Graph Value (user-defined or void)
// V    - Vertex type
// VKey - Vertex Key type
// VV   - Vertex Value (user-defined or void)
// VR   - Vertex Range
// VVF  - Vertex Value Function: vvf(u) -> value
// E    - Edge type
// EV   - Edge Value (user-defined or void)
// ER   - Edge Range
// EVF  - Edge Value Function: evf(uv) -> value
//
// Parameters:
// g         - graph reference
// u,v,x,y   - vertex references
// ukey,vkey - vertex keys
// ui,vi     - vertex iterators
// uv        - edge reference
// uvi       - edge_iterator (use std::optional?)

// edge value types
template <class G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));

/// <summary>
/// Override for an edge type where source and target are unordered
/// For instance, given:
///      vertex_iterator_t<G> ui = ...;
///      for(auto&& uv : edges(g,*ui)) ...
/// if(source_key(g,u) != vertex_key(ui)) then target_key(g,u) == vertex_key(ui)
///
/// Example:
///  namespace my_namespace {
///      template<class X>
///      class my_graph { ... };
///  }
///  namespace std::graph {
///     template<class X>
///     inline constexpr bool is_undirected_edge_v<edge_t<my_namespace::my_graph<X>>> = true;
///  }
/// </summary>
/// <typeparam name="E">The edge type with unordered source and target</typeparam>
template <class E>
inline constexpr bool is_undirected_edge_v = false;

/// <summary>
/// Override for a graph type where edges are defined densely in a matrix to allow for
/// optimized algorithms can take advantage of the memory layout.
///
/// Example:
///  namespace my_namespace {
///      template<class X>
///      class my_graph { ... };
///  }
///  namespace std::graph {
///     template<class X>
///     inline constexpr bool is_adjacency_matrix_v<my_namespace::my_graph<X>> = true;
///  }
/// </summary>
/// <typeparam name="G"></typeparam>
template <class G>
inline constexpr bool is_adjacency_matrix_v = false;

//
// graph concepts
//
template <class G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g, vertex_iterator_t<G> ui) {
  { vertices(g) } -> ranges::forward_range;
  vertex_key(g, ui);
};

template <class G, class ER>
concept targeted_edge = ranges::forward_range<ER> &&
      requires(G&& g, ranges::range_reference_t<ER> uv) {
  target_key(g, uv);
  target(g, uv);
};

template <class G, class ER>
concept sourced_edge =
      requires(G&& g, ranges::range_reference_t<ER> uv) {
  source_key(g, uv);
  source(g, uv);
};

template <class G>
concept incidence_graph = vertex_range<G> && targeted_edge<G, vertex_edge_range_t<G>> && requires(
      G&& g, vertex_reference_t<G> u, vertex_key_t<G> ukey, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  { edges(g, u) } -> ranges::forward_range;
  { edges(g, ukey) } -> ranges::forward_range;
};
// !is_same_v<vertex_range_t<G>, vertex_edge_range_t<G>>
//      CSR fails this condition b/c row_index & col_index are both index_vectors; common?

template <class G>
concept sourced_incidence_graph = incidence_graph<G> && sourced_edge<G, vertex_edge_range_t<G>> &&
      requires(G&& g, edge_reference_t<G> uv) {
  edge_key(g, uv);
#  ifdef ENABLE_OTHER_FNC
  other_key(g, uv, ukey);
  other_vertex(g, uv, u);
#  endif
};

template <class G>
concept undirected_incidence_graph = sourced_incidence_graph<G> && is_undirected_edge_v<edge_t<G>>;

template <class G>
concept directed_incidence_graph = !undirected_incidence_graph<G>;

template <class G>
concept adjacency_matrix = is_adjacency_matrix_v<G>;

//
// property concepts
//
template <class G>
concept has_degree = requires(G&& g, vertex_reference_t<G> u) {
  {degree(g, u)};
};

//
// find/contains concepts
//
template <class G>
concept has_find_vertex = requires(G&& g, vertex_key_t<G> ukey) {
  { find_vertex(g, ukey) } -> forward_iterator;
};

template <class G>
concept has_find_vertex_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
  { find_vertex_edge(g, u, vkey) } -> forward_iterator;
  { find_vertex_edge(g, ukey, vkey) } -> forward_iterator;
};

template <class G>
concept has_contains_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  { contains_edge(g, ukey, vkey) } -> convertible_to<bool>;
};

} // namespace std::graph

#endif //GRAPH_HPP
