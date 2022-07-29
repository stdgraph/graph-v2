#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include "detail/graph_access.hpp"


#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {
// Template parameters:
// G   - Graph
// GV  - Graph Value (user-defined or void)
//
// V   - Vertex type
// VId - Vertex Id type
// VV  - Vertex Value (user-defined or void)
// VR  - Vertex Range
// VI  - Vertex Iterator
// VVF - Vertex Value Function: vvf(u) -> value
//
// E   - Edge type
// EV  - Edge Value (user-defined or void)
// ER  - Edge Range
// EVF - Edge Value Function: evf(uv) -> value

// Parameters:
// g       - graph reference
// u,v,x,y - vertex references
// uid,vid - vertex ids
// ui,vi   - vertex iterators
// vvf     - vertex value function: vvf(u) -> value
//
// uv      - edge reference
// uvi     - edge_iterator (use std::optional?)
// evf     - edge value function: evf(uv) -> value


/// <summary>
/// Override for an edge type where source and target are unordered
/// For instance, given:
///      vertex_iterator_t<G> ui = ...;
///      for(auto&& uv : edges(g,*ui)) ...
/// if(source_id(g,u) != vertex_id(ui)) then target_id(g,u) == vertex_id(ui)
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

//
// graph concepts
//
template <class G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g, vertex_iterator_t<G> ui) {
  { vertices(g) } -> ranges::forward_range;
  vertex_id(g, ui);
};

template <class G, class E>
concept targeted_edge = requires(G&& g, E& uv) {
  target_id(g, uv);
  target(g, uv);
};

template <class G, class E>
concept sourced_edge = requires(G&& g, E& uv) {
  source_id(g, uv);
  source(g, uv);
};
template <class G, class E>
struct is_sourced_edge : public integral_constant<bool, sourced_edge<G, E>> {};
template <class G, class E>
inline constexpr bool is_sourced_edge_v = is_sourced_edge<G, E>::value;


template <class G>
concept incidence_graph = vertex_range<G> && targeted_edge<G, edge_t<G>> && requires(
      G&& g, vertex_reference_t<G> u, vertex_id_t<G> uid, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  { edges(g, u) } -> ranges::forward_range;
  { edges(g, uid) } -> ranges::forward_range;
};
// !is_same_v<vertex_range_t<G>, vertex_edge_range_t<G>>
//      CSR fails this condition b/c row_index & col_index are both index_vectors; common?

template <class G>
concept sourced_incidence_graph = incidence_graph<G> && sourced_edge<G, edge_t<G>> &&
      requires(G&& g, edge_reference_t<G> uv) {
  edge_id(g, uv);
#  ifdef ENABLE_OTHER_FNC
  other_id(g, uv, uid);
  other_vertex(g, uv, u);
#  endif
};

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
concept has_find_vertex = requires(G&& g, vertex_id_t<G> uid) {
  { find_vertex(g, uid) } -> forward_iterator;
};

template <class G>
concept has_find_vertex_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
  { find_vertex_edge(g, u, vid) } -> forward_iterator;
  { find_vertex_edge(g, uid, vid) } -> forward_iterator;
};

template <class G>
concept has_contains_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  { contains_edge(g, uid, vid) } -> convertible_to<bool>;
};


/// <summary>
/// Override for a graph type where source_id and target_id are unordered
/// on an edge so views and algorithms know to choose the correct target
/// based on where they came from.
///
/// An unordered edge implies sourced_edge<G> is true so that an algorithm can
/// decide if source_id(g,uv) or target_id(g,uv) is the true target, based on
/// where the algorithm came from.
///
/// If a graph container implementation has a run-time property of ordered or
/// unordered (e.g. it can't be determined at compile-time) then unordered_edge<G,E>
/// should be true_type. The only consequence is that an additional if is done to
/// check whether source_id or target_id is used for the target in this library. 
/// The container implementation can still preserve its implementation of order,
/// assuming it always includes a source_id on the edge.
/// 
/// Example:
///  namespace my_namespace {
///      template <class T>
///      class my_graph { ... };
///      template class< T>
///      class my_edge { int src_id; int tgt_id; ... };
///  }
///  namespace std::graph {
///     template<class T>
///     struct define_unordered_edge<my_namespace::my_graph<T>, my_namespace::my_edge<T>> : public true_type {};
///  }
/// </summary>
/// <typeparam name="G">Graph</typeparam>
///

template <class E>
struct define_unordered_edge : public false_type {}; // specialized for graph container edge

template <class G, class E>
struct is_unordered_edge : public conjunction<define_unordered_edge<E>, is_sourced_edge<G, E>> {};

template <class G, class E>
inline constexpr bool is_unordered_edge_v = is_unordered_edge<G, E>::value;

template <class G, class E>
concept unordered_edge = is_unordered_edge_v<G, E>;

//
// is_ordered_edge, ordered_edge
//
template <class G, class E>
struct is_ordered_edge : public negation<is_unordered_edge<G,E>> {};

template <class G, class E>
inline constexpr bool is_ordered_edge_v = is_ordered_edge<G, E>::value;

template <class G, class E>
concept ordered_edge = is_ordered_edge_v<G, E>;


} // namespace std::graph

#endif //GRAPH_HPP
