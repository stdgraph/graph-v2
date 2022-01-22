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
auto vertices(G&& g) {
  return access::vertices(g);
}
namespace _fwd {
  template <typename G>
  using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));
  template <typename G>
  using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;
} // namespace _fwd

template <typename G>
auto vertices(G&& g, _fwd::vertex_reference_t<G> u) {
  return access::vertices(g,u);
}


//edges(g)
//edges(g,u)
//vertices(g,u)

//vertex_key(g,u)
//vertex_value(g,u)
//edge_key(g,uv)
//edge_value(g,uv)
//target(g,uv)
//source(g,uv)

} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
