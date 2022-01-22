#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

// vertices(g)  : default: g->vertices(), g
// vertices(g,u): default: u.vertices(g)
//
namespace _detail {
  TAG_INVOKE_DEF(vertices);
}
template<typename G>
auto vertices(G&& g) {
  return _detail::vertices(g);
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
