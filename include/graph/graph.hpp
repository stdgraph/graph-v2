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

// edge range & iterator types
template <typename G>
using edge_range_t = decltype(edges(declval<G&&>()));
template <typename G>
using edge_iterator_t = ranges::iterator_t<edge_range_t<G>>;

template <typename G>
using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_iterator_t<G>>()));
template <typename G>
using vertex_edge_iterator_t = ranges::iterator_t<vertex_edge_range_t<G>>;

template <typename G>
using vertex_vertex_range_t = decltype(vertices(declval<G&&>(), declval<vertex_iterator_t<G>>()));
template <typename G>
using vertex_vertex_iterator_t = ranges::iterator_t<vertex_vertex_range_t<G>>;

// Value types
template <typename G>
using graph_value_t = decltype(graph_value(declval<G&&>()));

template <typename G>
using vertex_t = ranges::range_value_t<vertex_range_t<G>>;
template <typename G>
using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G&&>>;
template <typename G>
using vertex_key_t = decltype(vertex_key(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

template <typename G, typename ER>
using edge_t = typename ranges::range_value_t<ER>;
template <typename G, typename ER>
using edge_reference_t = ranges::range_reference_t<ER>;
template <typename G, typename ER>
using edge_key_t = decltype(edge_key(declval<G&&>(), declval<edge_reference_t<ER>>())); // e.g. pair<vertex_key_t<G>,vertex_key_t<G>>
template <typename G, typename ER>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<ER>>()));


template <typename G>
concept incidence_graph = std::ranges::range<vertex_range_t<G>> && std::ranges::range<vertex_edge_range_t<G>>;

template <typename G>
concept adjacencey_graph = std::ranges::range<vertex_range_t<G>> && std::ranges::range<vertex_vertex_range_t<G>>;


} // namespace std::graph

#endif //GRAPH_HPP
