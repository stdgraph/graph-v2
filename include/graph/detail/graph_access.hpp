#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

// vertices(g)  : default: g->vertices(), g
// vertices(g,u): default: u.vertices(g)
//
namespace access {
  TAG_INVOKE_DEF(vertices);
}

template <typename G>
auto&& vertices(G&& g) {
  return access::vertices(g);
}
namespace _fwd {
  template <typename G>
  using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));
  template <typename G>
  using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G&&>>;
  template <typename G>
  using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;
  template <typename G>
  using vertex_vertex_range_t = decltype(vertices(declval<G&&>(), declval<vertex_reference_t<G>>()));
} // namespace _fwd
template <typename G>
auto&& vertices(G&& g, _fwd::vertex_reference_t<G> u) {
  return access::vertices(g, u);
}

//vertex_key(g,u) - only available when vertices in contiguous memory
//
namespace access {
  TAG_INVOKE_DEF(vertex_key);
}
template <typename G>
auto vertex_key(G&& g, _fwd::vertex_reference_t<G> u) {
  return access::vertex_key(g, u);
}
namespace _fwd {
  template <typename G>
  using vertex_key_t = decltype(vertex_key(declval<G&&>(), declval<vertex_reference_t<G>>()));
}

//vertex_value(g,u)
//
namespace access {
  TAG_INVOKE_DEF(vertex_value);
}
template <typename G>
auto&& vertex_value(G&& g, _fwd::vertex_reference_t<G> u) {
  return access::vertex_value(g, u);
}


//edges(g)
//edges(g,u)
//
namespace access {
  TAG_INVOKE_DEF(edges);
}
template <typename G>
auto&& edges(G&& g) {
  return access::edges(g);
}
template <typename G>
auto&& edges(G&& g, _fwd::vertex_reference_t<G> u) {
  return access::edges(g, u);
}
namespace _fwd {
  template <typename G>
  using edge_range_t = decltype(edges(declval<G&&>()));
  template <typename G>
  using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_reference_t<G>>()));
} // namespace _fwd


//edge_key(g,uv)
//
namespace access {
  TAG_INVOKE_DEF(edge_key);
}
template <typename G>
auto edge_key(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::edge_key(g, uv);
}
template <typename G>
auto edge_key(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv) {
  return access::edge_key(g, uv);
}

//edge_value(g,uv)
//
namespace access {
  TAG_INVOKE_DEF(edge_value);
}
template <typename G>
auto&& edge_value(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::edge_value(g, uv);
}

//target_key(g,uv)
//
namespace access {
  TAG_INVOKE_DEF(target_key);
}
template <typename G>
auto target_key(G&& g, const ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::target_key(g, uv);
}
template <typename G>
auto target_key(G&& g, const ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv) {
  return access::target_key(g, uv);
}

//target(g,uv)
//
namespace access {
  TAG_INVOKE_DEF(target);
}
template <typename G>
auto&& target(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::target(g, uv);
}
template <typename G>
auto&& target(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv) {
  return access::target(g, uv);
}

//source(g,uv)
//
namespace access {
  TAG_INVOKE_DEF(source);
}
template <typename G>
auto&& source(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::source(g, uv);
}
template <typename G>
auto&& source(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv) {
  return access::source(g, uv);
}

// source_key
//
namespace access {
  TAG_INVOKE_DEF(source_key);
}
template <typename G>
auto source_key(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv) {
  return access::source_key(g, uv);
}
template <typename G>
auto source_key(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv) {
  return access::source_key(g, uv);
}

// other_vertex_key
//
namespace access {
  TAG_INVOKE_DEF(other_vertex_key);
}
template <typename G>
auto other_vertex_key(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv, _fwd::vertex_key_t<G> ukey) {
  return access::other_vertex_key(g, uv, ukey);
}
template <typename G>
auto other_vertex_key(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv, _fwd::vertex_key_t<G> ukey) {
  return access::other_vertex_key(g, uv, ukey);
}

// other_vertex
//
namespace access {
  TAG_INVOKE_DEF(other_vertex);
}
template <typename G>
auto&& other_vertex(G&& g, ranges::range_reference_t<_fwd::vertex_edge_range_t<G>> uv, _fwd::vertex_reference_t<G> u) {
  return access::other_vertex(g, uv, u);
}
template <typename G>
auto&&
other_vertex(G&& g, ranges::range_reference_t<_fwd::vertex_vertex_range_t<G>> uv, _fwd::vertex_reference_t<G> u) {
  return access::other_vertex(g, uv, u);
}


// find_vertex_edge
//
namespace access {
  TAG_INVOKE_DEF(find_vertex_edge);
}
template <typename G>
auto find_vertex_edge(G&& g, _fwd::vertex_key_t<G> ukey, _fwd::vertex_key_t<G> vkey) {
  return access::find_vertex_edge(g, ukey, vkey);
}

// find_vertex_vertex
//
namespace access {
  TAG_INVOKE_DEF(find_vertex_vertex);
}
template <typename G>
auto find_vertex_vertex(G&& g, _fwd::vertex_key_t<G> ukey, _fwd::vertex_key_t<G> vkey) {
  return access::find_vertex_vertex(g, ukey, vkey);
}


// find_edge
//
namespace access {
  TAG_INVOKE_DEF(find_edge);
}
template <typename G>
auto find_edge(G&& g, _fwd::vertex_key_t<G> ukey, _fwd::vertex_key_t<G> vkey) {
  return access::find_edge(g, ukey, vkey);
}

// contains_edge
//
namespace access {
  TAG_INVOKE_DEF(contains_edge);
}
template <typename G>
auto contains_edge(G&& g, _fwd::vertex_key_t<G> ukey, _fwd::vertex_key_t<G> vkey) {
  return access::contains_edge(g, ukey, vkey);
}


// degree - number of outgoing edges (e.g. neighbors)
//
namespace access {
  TAG_INVOKE_DEF(degree);

  template <typename G>
  concept _has_degree_adl = requires(G&& g, _fwd::vertex_key_t<G> ukey) {
    {degree(g, ukey)};
  };

  template <typename G>
  concept _has_edges_size_adl = requires(G&& g, _fwd::vertex_reference_t<G> u) {
    {ranges::size(edges(g, u))};
  };
} // namespace access
template <typename G>
requires access::_has_degree_adl<G> || access::_has_edges_size_adl<G>
auto degree(G&& g, _fwd::vertex_key_t<G> ukey) {
  if constexpr (access::_has_degree_adl<G>)
    return access::degree(g, ukey);
  else if constexpr (access::_has_edges_size_adl<G>) {
    return ranges::size(edges(g, *(ranges::begin(vertices(g)) + ukey)));
  }
}

// graph_value
//
namespace access {
  TAG_INVOKE_DEF(graph_value);
}
template <typename G>
auto&& graph_value(G&& g) {
  return access::graph_value(g);
}


} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
