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
template <typename G, typename ER>
using edge_t = typename ranges::range_value_t<ER>;
template <typename G, typename ER>
using edge_reference_t = typename ranges::range_reference_t<ER>;
template <typename G, typename ER>
using edge_key_t = decltype(edge_key(declval<G&&>(),
                                     declval<edge_reference_t<G, ER>>())); // e.g. pair<vertex_key_t<G>,vertex_key_t<G>>
template <typename G, typename ER>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G, ER>>()));

//
// concepts
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

template <typename G>
concept adjacencey_graph = ranges::range<vertex_range_t<G>> && ranges::range<vertex_vertex_range_t<G>> &&
      is_same_v<vertex_vertex_range_t<G>, vertex_range_t<G>> && edge_range<G, vertex_vertex_range_t<G>>;

template <typename G>
concept sourced_adjacencey_graph = adjacencey_graph<G> && sourced_edge_range<G, vertex_vertex_range_t<G>>;


} // namespace std::graph

#endif //GRAPH_HPP
