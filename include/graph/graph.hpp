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
template <typename G>
using edge_t = typename ranges::range_value_t<vertex_edge_range_t<G>>;
template <typename G>
using edge_reference_t = typename ranges::range_reference_t<vertex_edge_range_t<G>>;
template <typename G, typename ER>
using edge_key_t = decltype(edge_key(declval<G&&>(),
                                     declval<edge_reference_t<G>>())); // e.g. pair<vertex_key_t<G>,vertex_key_t<G>>
template <typename G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));

//
// graph concepts
//
template <typename G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g, ranges::iterator_t<vertex_range_t<G>> ui) {
  { vertices(g) } -> ranges::forward_range;
  {vertex_key(g, ui)};
};

template <typename G, typename ER>
concept edge_range = ranges::forward_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  target_key(g, uv);
  target(g, uv);
};

template <typename G, typename ER>
concept sourced_edge_range =
      requires(G&& g, ranges::range_reference_t<ER> uv, vertex_key_t<G> ukey, vertex_reference_t<G> u) {
  source_key(g, uv);
  source(g, uv);
  edge_key(g, uv);
  other_key(g, uv, ukey);
  other_vertex(g, uv, u);
};

template <typename G>
concept incidence_graph = ranges::range<vertex_range_t<G>> && ranges::range<vertex_edge_range_t<G>> &&
                          !is_same_v<vertex_edge_range_t<G>, vertex_range_t<G>> &&
                          edge_range<G, vertex_edge_range_t<G>>;
template <typename G>
concept sourced_incidence_graph = incidence_graph<G> && sourced_edge_range<G, vertex_edge_range_t<G>>;

#  ifdef OBSOLETE
template <typename G>
concept adjacencey_graph = ranges::range<vertex_range_t<G>> && ranges::range<vertex_vertex_range_t<G>> &&
      is_same_v<vertex_vertex_range_t<G>, vertex_range_t<G>> && edge_range<G, vertex_vertex_range_t<G>>;

template <typename G>
concept sourced_adjacencey_graph = adjacencey_graph<G> && sourced_edge_range<G, vertex_vertex_range_t<G>>;
#  endif //OBSOLETE

template <typename G>
concept adjacency_matrix = false; // tag algorithms that can take advantage of matrix layout

#  ifdef FUTURE
// use other_key & other_vertex existence to identify a graph as unordered?
// overridable per graph to direct algorithm
template <typename G, typename ER>
using is_ordered = false_type;
template <typename G, typename ER>
inline constexpr bool is_ordered_v = is_ordered<G, ER>::value;

template <typename G, typename ER>
using is_unordered = false_type;
template <typename G, typename ER>
inline constexpr bool is_unordered_v = is_unordered<G, ER>::value;

template <typename G, typename ER>
using is_undefined_order = bool_constant<!is_ordered_v<G, ER> && !is_unordered_v<G, ER>>;
template <typename G, typename ER>
inline constexpr bool is_undefined_order_v = is_undefined_order_v<G, ER>;
#  endif //FUTURE

//
// property concepts
//
template <typename G>
concept has_degree = requires(G&& g, vertex_reference_t<G> u) {
  {degree(g, u)};
};

template <typename G>
concept has_graph_value = semiregular<graph_value_t<G>>;

template <typename G>
concept has_vertex_value = semiregular<vertex_value_t<G>>;

template <typename G, typename EI>
concept has_edge_value = semiregular<edge_value_t<G, EI>>;

//
// find/contains concepts
//
template <typename G>
concept has_find_vertex = requires(G&& g, vertex_key_t<G> ukey) {
  { find_vertex(g, ukey) } -> forward_iterator;
};

template <typename G>
concept has_find_vertex_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
  { find_vertex_edge(g, u, vkey) } -> forward_iterator;
  { find_vertex_edge(g, ukey, vkey) } -> forward_iterator;
};

template <typename G>
concept has_find_vertex_vertex = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
  { find_vertex_vertex(g, u, vkey) } -> forward_iterator;
  { find_vertex_vertex(g, ukey, vkey) } -> forward_iterator;
};

template <typename G>
concept has_find_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  { find_edge(g, ukey, vkey) } -> forward_iterator;
};

template <typename G>
concept has_contains_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  { contains_edge(g, ukey, vkey) } -> convertible_to<bool>;
};

namespace view {
  // experimental

  //
  // vertex
  //
  template <typename VKey, typename V, typename VV>
  struct vertex {
    VKey key;
    V&   vertex;
    VV&  value;
  };
  template <typename VKey, typename V>
  struct vertex<VKey, V, void> {
    VKey key;
    V&   vertex;
  };

  //
  // targeted_edge
  //
  template <typename VKey, typename V, typename E, typename EV>
  struct targeted_edge {
    VKey target_key;
    E&   edge;
    EV&  value;
  };
  template <typename VKey, typename V, typename E>
  struct targeted_edge<VKey, V, E, void> {
    VKey target_key;
    E&   edge;
  };

  //
  // sourced_edge
  //
  template <typename VKey, typename V, typename E, typename EV>
  struct sourced_edge {
    VKey source_key;
    VKey target_key;
    E&   edge;
    EV&  value;
  };
  template <typename VKey, typename V, typename E>
  struct sourced_edge<VKey, V, E, void> {
    VKey source_key;
    VKey target_key;
    E&   edge;
  };

  //
  // neighbor
  //
  template <typename VKey, typename V, typename VV>
  struct neighbor {
    VKey target_key;
    V&   target;
    VV&  target_value;
  };
  template <typename VKey, typename V>
  struct neighbor<VKey, V, void> {
    VKey target_key;
    V&   target;
  };

  //
  // edgelist_edge
  //
  template <typename VKey, typename E, typename EV>
  struct edgelist_edge {
    VKey source_key;
    VKey target_key;
    E&   edge;
    EV&  value;
  };
  template <typename VKey, typename E>
  struct edgelist_edge<VKey, E, void> {
    VKey source_key;
    VKey target_key;
    E&   edge;
  };
} // namespace view

} // namespace std::graph

#endif //GRAPH_HPP
