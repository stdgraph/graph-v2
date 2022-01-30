#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

// Tags defined in access to avoid conflicts with function names in std::graph,
// allowing customization for default behavior.
//
// graphs must use tags like std::graph::access::vertex_key_fn_t when defining 
// CPO specialization.
//
namespace access {
  // ranges
  TAG_INVOKE_DEF(vertices); // vertices(g) -> [graph vertices], vertices(g,u) -> [adjacency edges]
  TAG_INVOKE_DEF(edges);    // edges(g) -> [graph edges], edges(g,u) -> [incidence edges]

  // graph value
  TAG_INVOKE_DEF(graph_value); // graph_value(g) -> GV&

  // vertex values
  TAG_INVOKE_DEF(vertex_key);   // vertex_key(g,ui) -> VKey
  TAG_INVOKE_DEF(vertex_value); // vertex_value(g,u) -> VV&
  TAG_INVOKE_DEF(degree);       // degree(g,u) -> VKey

  // edge values
  TAG_INVOKE_DEF(target_key); // target_key(g,uv) -> VKey
  TAG_INVOKE_DEF(target);     // target(g,uv) -> v
  TAG_INVOKE_DEF(edge_value); // edge_value(g,uv) -> EV&

  // +sourced edge values (only available when source_key is on the edge)
  TAG_INVOKE_DEF(source_key);   // source_key(g,uv) -> VKey
  TAG_INVOKE_DEF(source);       // source(g,uv) -> v
  TAG_INVOKE_DEF(edge_key);     // edge_key(g,uv) -> pair<VKey,VKey>
  TAG_INVOKE_DEF(other_key);    // other_key(g,uv,xkey) -> VKey (ukey or vkey)
  TAG_INVOKE_DEF(other_vertex); // other_vertex(g,uv,x) -> y (u or v)

  // find
  TAG_INVOKE_DEF(find_vertex);        // find_vertex(g,ukey) -> ui
  TAG_INVOKE_DEF(find_vertex_edge);   // find_vertex_edge(g,u,vkey) -> uvi
  TAG_INVOKE_DEF(find_vertex_vertex); // find_vertex_vertex(g,u,vkey) -> uvi
  TAG_INVOKE_DEF(find_edge);          // find_edge(g,ukey,vkey) -> uvi
  TAG_INVOKE_DEF(contains_edge);      // contains_edge(g,ukey,vkey) -> bool
} // namespace access

//
// Vertex range & directly related types
//
template <typename G>
auto&& vertices(G&& g) {
  return access::vertices(g);
}

template <typename G>
using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));
template <typename G>
using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G&&>>;
template <typename G>
using vertex_t = ranges::range_value_t<vertex_range_t<G>>;
template <typename G>
using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;

//
// Vertex-edge range (incidence) & directly related types
//
template <typename G>
auto&& edges(G&& g, vertex_reference_t<G> u) {
  return access::edges(g, u);
}

template <typename G>
using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_edge_iterator_t = ranges::iterator_t<vertex_edge_range_t<G>>;

//
// Vertex-vertex range (adjacency) & directly related types
//
namespace access {
  template <typename G>
  concept _has_adj_vertices_adl = requires(G&& g, vertex_reference_t<G> u) {
    {vertices(g, u)};
  };
} // namespace access

template <typename G>
requires access::_has_adj_vertices_adl<G>
auto&& vertices(G&& g, vertex_reference_t<G> u) { return access::vertices(g, u); }

template <typename G>
using vertex_vertex_range_t = decltype(vertices(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <typename G>
using vertex_vertex_iterator_t = ranges::iterator_t<vertex_vertex_range_t<G>>;


//
// Edges range & directly related types
//

// edges(g)
//
template <typename G>
auto&& edges(G&& g) {
  return access::edges(g);
}
template <typename G>
using edge_range_t = decltype(edges(declval<G&&>()));
template <typename G>
using edge_iterator_t = ranges::iterator_t<edge_range_t<G>>;


//
// Vertex properties
//

//vertex_key(g,ui)
//
template <typename G>
auto vertex_key(G&& g, vertex_iterator_t<G> ui) {
  return access::vertex_key(g, ui);
}
template <typename G>
using vertex_key_t = decltype(vertex_key(declval<G&&>(), declval<vertex_iterator_t<G>>()));


//vertex_value(g,u)
//
template <typename G>
auto&& vertex_value(G&& g, vertex_reference_t<G> u) {
  return access::vertex_value(g, u);
}
template <typename G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

// degree - number of outgoing edges (e.g. neighbors)
//
namespace access {
  template <typename G>
  concept _has_degree_adl = requires(G&& g, vertex_reference_t<G> u) {
    {degree(g, u)};
  };
} // namespace access

template <typename G>
requires access::_has_degree_adl<G> || ranges::sized_range<vertex_edge_range_t<G>>
auto degree(G&& g, vertex_reference_t<G> u) {
  if constexpr (access::_has_degree_adl<G>)
    return access::degree(g, u);
  else if constexpr (ranges::sized_range<vertex_edge_range_t<G>>) {
    return ranges::size(edges(g, u));
  }
}


//
// Edge properties
//

//target_key(g,uv)
//
template <typename G>
auto target_key(G&& g, const ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::target_key(g, uv);
}
template <typename G>
auto target_key(G&& g, const ranges::range_reference_t<vertex_vertex_range_t<G>> uv) {
  return access::target_key(g, uv);
}

//target(g,uv)
//
template <typename G>
auto&& target(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::target(g, uv);
}
template <typename G>
auto&& target(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv) {
  return access::target(g, uv);
}

//edge_value(g,uv)
//
template <typename G>
auto&& edge_value(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::edge_value(g, uv);
}

//
// Sourced Edge properties (when source_key(g,uv) is defined)
//

// source_key
//
template <typename G>
auto source_key(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::source_key(g, uv);
}
template <typename G>
auto source_key(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv) {
  return access::source_key(g, uv);
}

//source(g,uv)
//
template <typename G>
auto&& source(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::source(g, uv);
}
template <typename G>
auto&& source(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv) {
  return access::source(g, uv);
}

// edge_key(g,uv)
template <typename G>
auto edge_key(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
  return access::edge_key(g, uv);
}
template <typename G>
auto edge_key(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv) {
  return access::edge_key(g, uv);
}

// other_key
//
template <typename G>
auto other_key(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv, vertex_key_t<G> ukey) {
  return access::other_key(g, uv, ukey);
}
template <typename G>
auto other_key(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv, vertex_key_t<G> ukey) {
  return access::other_key(g, uv, ukey);
}

// other_vertex
//
template <typename G>
auto&& other_vertex(G&& g, ranges::range_reference_t<vertex_edge_range_t<G>> uv, vertex_reference_t<G> u) {
  return access::other_vertex(g, uv, u);
}
template <typename G>
auto&& other_vertex(G&& g, ranges::range_reference_t<vertex_vertex_range_t<G>> uv, vertex_reference_t<G> u) {
  return access::other_vertex(g, uv, u);
}


// find_vertex_edge
//
template <typename G>
auto find_vertex_edge(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  return access::find_vertex_edge(g, ukey, vkey);
}

// find_vertex_vertex
//
template <typename G>
auto find_vertex_vertex(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  return access::find_vertex_vertex(g, ukey, vkey);
}


// find_edge
//
template <typename G>
auto find_edge(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  return access::find_edge(g, ukey, vkey);
}

// contains_edge
//
template <typename G>
auto contains_edge(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  return access::contains_edge(g, ukey, vkey);
}


// graph_value
//
template <typename G>
auto&& graph_value(G&& g) {
  return access::graph_value(g);
}
template <typename G>
using graph_value_t = decltype(graph_value(declval<G&&>()));


} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
