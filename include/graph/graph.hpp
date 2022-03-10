#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include "detail/graph_access.hpp"


#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {
// Template parameters:
// G  - Graph
// EV - Edge Value (user-defined or void)
// VV - Vertex Value (user-defined or void)
// GV - Graph Value (user-defined or void)
// ER - Edge Range
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

//
// graph concepts
//
template <class G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g, ranges::iterator_t<vertex_range_t<G>> ui) {
  { vertices(g) } -> ranges::forward_range;
  {vertex_key(g, ui)};
};

template <class G, class ER>
concept edge_range = ranges::forward_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  target_key(g, uv);
  target(g, uv);
};

template <class G, class ER>
concept sourced_edge_range =
      requires(G&& g, ranges::range_reference_t<ER> uv, vertex_key_t<G> ukey, vertex_reference_t<G> u) {
  source_key(g, uv);
  source(g, uv);
  edge_key(g, uv);
  other_key(g, uv, ukey);
  other_vertex(g, uv, u);
};

template <class G>
concept incidence_graph = ranges::range<vertex_range_t<G>> && ranges::range<vertex_edge_range_t<G>> &&
      edge_range<G, vertex_edge_range_t<G>>;
//!is_same_v<vertex_edge_range_t<G>, vertex_range_t<G>> &&
// CSR fails this condition b/c row_index & col_index are both index_vectors; common?

template <class G>
concept sourced_incidence_graph = incidence_graph<G> && sourced_edge_range<G, vertex_edge_range_t<G>>;

template <class G>
concept adjacency_matrix = false; // tag algorithms that can take advantage of matrix layout

#  ifdef FUTURE
// use other_key & other_vertex existence to identify a graph as unordered?
// overridable per graph to direct algorithm
template <class G, class ER>
using is_ordered = false_type;
template <class G, class ER>
inline constexpr bool is_ordered_v = is_ordered<G, ER>::value;

template <class G, class ER>
using is_unordered = false_type;
template <class G, class ER>
inline constexpr bool is_unordered_v = is_unordered<G, ER>::value;

template <class G, class ER>
using is_undefined_order = bool_constant<!is_ordered_v<G, ER> && !is_unordered_v<G, ER>>;
template <class G, class ER>
inline constexpr bool is_undefined_order_v = is_undefined_order_v<G, ER>;
#  endif //FUTURE

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

namespace views {
  // experimental

  //
  // vertex_view
  // for(auto&& [ukey, u, value] : vertexlist(g, [](vertex_reference_t<G> u) { return ...; } )
  // for(auto&& [ukey, u]        : vertexlist(g))
  //
  template <class VKey, class V, class VV>
  struct vertex_view {
    VKey key;
    V    vertex;
    VV   value;
  };
  template <class VKey, class V>
  struct vertex_view<VKey, V, void> {
    VKey key;
    V    vertex;
  };
  template <class VKey, class VV>
  struct vertex_view<VKey, void, VV> {
    VKey key;
    VV   value;
  };
  template <class VKey>
  struct vertex_view<VKey, void, void> {
    VKey key;
  };

  template <class VKey, class VV>
  using copyable_vertex_t = vertex_view<VKey, void, VV>; // {key, value}

  //
  // edge_view
  //
  template <class VKey, bool Sourced, class E, class EV>
  struct edge_view {
    VKey source_key;
    VKey target_key;
    E    edge;
    EV   value;
  };

  template <class VKey, class E>
  struct edge_view<VKey, true, E, void> {
    VKey source_key;
    VKey target_key;
    E    edge;
  };
  template <class VKey>
  struct edge_view<VKey, true, void, void> {
    VKey source_key;
    VKey target_key;
  };
  template <class VKey, class EV>
  struct edge_view<VKey, true, void, EV> {
    VKey source_key;
    VKey target_key;
    EV   value;
  };

  template <class VKey, class E, class EV>
  struct edge_view<VKey, false, E, EV> {
    VKey target_key;
    E    edge;
    EV   value;
  };
  template <class VKey, class E>
  struct edge_view<VKey, false, E, void> {
    VKey target_key;
    E    edge;
  };

  template <class VKey, class EV>
  struct edge_view<VKey, false, void, EV> {
    VKey target_key;
    EV   value;
  };
  template <class VKey>
  struct edge_view<VKey, false, void, void> {
    VKey target_key;
  };

  //
  // targeted_edge
  // for(auto&& [vkey,uv,value] : edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
  // for(auto&& [vkey,uv]       : edges_view(g, u) )
  //
  template <class VKey, class E, class EV>
  using targeted_edge = edge_view<VKey, false, E, EV>; // {target_key, edge, [, value]}

  //
  // sourced_edge
  // for(auto&& [ukey,vkey,uv,value] : sourced_edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
  // for(auto&& [ukey,vkey,uv]       : sourced_edges_view(g, u) )
  //
  template <class VKey, class V, class E, class EV>
  using sourced_edge = edge_view<VKey, true, E, EV>; // {source_key, target_key, edge, [, value]}

  //
  // edgelist_edge
  // for(auto&& [ukey,vkey,uv,value] : edges_view(g, [](vertex_edge_reference_t<G> g) { return ...; } )
  // for(auto&& [ukey,vkey,uv]       : edges_view(g) )
  //
  template <class VKey, class E, class EV>
  using edgelist_edge = edge_view<VKey, true, E, EV>; // {source_key, target_key [, edge] [, value]}

  //
  // copyable_edge
  //
  template <class VKey, class EV>
  using copyable_edge_t = edge_view<VKey, true, void, EV>; // {source_key, target_key [, value]}

  //
  // neighbor_view (for adjacency)
  //
  template <class VKey, bool Sourced, class V, class VV>
  struct neighbor_view {
    VKey source_key;
    VKey target_key;
    V    target;
    VV   value;
  };

  template <class VKey, class V, class VV>
  struct neighbor_view<VKey, false, V, VV> {
    VKey target_key;
    V    target;
    VV   value;
  };

  template <class VKey, class V>
  struct neighbor_view<VKey, false, V, void> {
    VKey target_key;
    V    target;
  };

  template <class VKey, class V>
  struct neighbor_view<VKey, true, V, void> {
    VKey target_key;
    V    target;
  };

  template <class VKey>
  struct neighbor_view<VKey, true, void, void> {
    VKey target_key;
  };

  //
  // view concepts
  //
  template <class T, class VKey, class VV>
  concept copyable_vertex = convertible_to<T, copyable_vertex_t<VKey, VV>>;

  template <class T, class VKey, class EV>
  concept copyable_edge = convertible_to<T, copyable_edge_t<VKey, EV>>;


} // namespace views

} // namespace std::graph

#endif //GRAPH_HPP
