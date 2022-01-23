#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include "detail/graph_access.hpp"


#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {

// Vertex range & iterator types
template <typename G>
using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));
template <typename G>
using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G&&>>;

// Vertex value types
template <typename G>
using vertex_t = ranges::range_value_t<vertex_range_t<G>>;
template <typename G>
using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;
template <typename G>
using vertex_key_t = decltype(vertex_key(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

// edge range & iterator types
template <typename G>
using edge_range_t = decltype(edges(declval<G&&>()));
template <typename G>
using edge_iterator_t = ranges::iterator_t<edge_range_t<G>>;

template <typename G>
using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_edge_iterator_t = ranges::iterator_t<vertex_edge_range_t<G>>;

template <typename G>
using vertex_vertex_range_t = decltype(vertices(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_vertex_iterator_t = ranges::iterator_t<vertex_vertex_range_t<G>>;

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

// graph value types
template <typename G>
using graph_value_t = decltype(graph_value(declval<G&&>()));


//
// concepts
//
template <typename G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g) {
  { vertices(g) } -> ranges::forward_range;
};

template <typename G, typename ER>
concept edge_range = ranges::forward_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  target(g, uv);
  target_key(g, uv);
};
template <typename G, typename ER>
concept sourced_edge_range =
      requires(G&& g, ranges::range_reference_t<ER> uv, vertex_key_t<G> ukey, vertex_reference_t<G> u) {
  source(g, uv);
  source_key(g, uv);
  edge_key(g, uv);
  other_vertex_key(g, uv, ukey);
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
