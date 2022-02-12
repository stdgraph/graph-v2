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
// Minimal requirements for a graph with random_access vertices(g)
//  vertices(g), edges(g,u), target_key(g,u)
//     vertex_key(g,ui) to have vertex_key_t<G> be something other than size_t
//  as needed by algorithm(s): edge_value(g,uv), vertex_value(g,uv), graph_value(g)
//
// For a sourced_graph the minimal requirements also add
//  source_key(g,uv)
//
namespace access {
  // ranges
  TAG_INVOKE_DEF(vertices); // vertices(g) -> [graph vertices], vertices(g,u) -> [adjacency edges]
  TAG_INVOKE_DEF(edges);    // edges(g,u) -> [incidence edges]

  // graph value
  TAG_INVOKE_DEF(graph_value); // graph_value(g) -> GV&

  // vertex values
  TAG_INVOKE_DEF(vertex_key);   // vertex_key(g,ui) -> VKey
                                // default = ui - begin(vertices(g)) for random_access_iterator<ui>
  TAG_INVOKE_DEF(vertex_value); // vertex_value(g,u) -> VV&
  TAG_INVOKE_DEF(degree);       // degree(g,u) -> VKey
                                // default = size(edges(g,u))

  // edge values
  TAG_INVOKE_DEF(target_key); // target_key(g,uv) -> VKey
  TAG_INVOKE_DEF(target);     // target(g,uv) -> v
                              // default = *(begin(g,vertices(g)) + target_key(g,uv))
                              // for random_access_range<vertices(g)> and integral<target_key(g,uv))
                              // uv can be from edges(g,u) or vertices(g,u)
  TAG_INVOKE_DEF(edge_value); // edge_value(g,uv) -> EV&

  // +sourced edge values (only available when source_key is on the edge)
  TAG_INVOKE_DEF(source_key);   // source_key(g,uv) -> VKey
  TAG_INVOKE_DEF(source);       // source(g,uv) -> u
                                // default = *(begin(g,vertices(g)) + source_key(g,uv))
                                // for random_access_range<vertices(g)> and integral<source_key(g,uv))
                                // uv can be from edges(g,u) or vertices(g,u)
  TAG_INVOKE_DEF(edge_key);     // edge_key(g,uv) -> pair<VKey,VKey>
                                // default = pair(source_key(g,uv),target_key(g,uv))
  TAG_INVOKE_DEF(other_key);    // other_key(g,uv,xkey) -> VKey (ukey or vkey)
                                // default = xkey != target_key(g,uv) ? target_key(g,uv) : source_key(g,uv)
  TAG_INVOKE_DEF(other_vertex); // other_vertex(g,uv,x) -> y (u or v)
                                // default = x != &target(g,uv) ? target(g,uv) : source(g,uv)

  // find
  TAG_INVOKE_DEF(find_vertex); // find_vertex(g,ukey) -> ui
                               // default = begin(vertices(g)) + ukey, for random_access_range<vertex_range_t<G>>
  TAG_INVOKE_DEF(
        find_vertex_edge); // find_vertex_edge(g,u,vkey) -> uvi; default = find(edges(g,u), [](uv) {target_id(g,uv)==vkey;}
        // find_vertex_edge(g,ukey,vkey) -> uvi; default = find_vertex_edge(g,*find_vertex(g,ukey),vkey)
  TAG_INVOKE_DEF(contains_edge); // contains_edge(g,ukey,vkey) -> bool
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
template <typename G>
using edge_t = typename ranges::range_value_t<vertex_edge_range_t<G>>;
template <typename G>
using edge_reference_t = ranges::range_reference_t<vertex_edge_range_t<G>>;

template <typename G>
using edge_key_t = decltype(edge_key(declval<G&&>(),
                                     declval<edge_reference_t<G>>())); // e.g. pair<vertex_key_t<G>,vertex_key_t<G>>

//
// Edges range & directly related types
//

//
// Vertex properties
//

//vertex_key(g,ui)
//
namespace access {
  template <typename G>
  concept _has_vertex_key_adl = requires(G&& g, vertex_iterator_t<G> ui) {
    {vertex_key(g, ui)};
  };
} // namespace access
template <typename G>
requires access::_has_vertex_key_adl<G> || random_access_iterator<vertex_iterator_t<G>>
auto vertex_key(G&& g, vertex_iterator_t<G> ui) {
  if constexpr (access::_has_vertex_key_adl<G>)
    return access::vertex_key(g, ui);
  else if constexpr (random_access_iterator<vertex_iterator_t<G>>)
    return ui - ranges::begin(vertices(g));
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
auto target_key(G&& g, edge_reference_t<const G> uv) {
  return access::target_key(g, uv);
}

//target(g,uv)
//
namespace access {
  template <typename G, typename ER>
  concept _has_target_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {target(g, uv)};
  };
} // namespace access
template <typename G, typename ER>
concept _can_eval_target = ranges::random_access_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  { target_key(g, uv) } -> integral;
};

template <typename G>
requires access::_has_target_adl<G, vertex_edge_range_t<G>> || _can_eval_target<G, vertex_edge_range_t<G>>
auto&& target(G&& g, edge_reference_t<G> uv) {
  if constexpr (access::_has_target_adl<G, vertex_edge_range_t<G>>)
    return access::target(g, uv);
  else if constexpr (_can_eval_target<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + target_key(g, uv));
}

//edge_value(g,uv)
//
template <typename G>
auto&& edge_value(G&& g, edge_reference_t<G> uv) {
  return access::edge_value(g, uv);
}

//
// Sourced Edge properties (when source_key(g,uv) is defined)
//

// source_key
//
template <typename G>
auto source_key(G&& g, edge_reference_t<G> uv) {
  return access::source_key(g, uv);
}

//source(g,uv)
//
namespace access {
  template <typename G, typename ER>
  concept _has_source_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {source(g, uv)};
  };
} // namespace access
template <typename G, typename ER>
concept _can_eval_source = ranges::random_access_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  { source_key(g, uv) } -> integral;
};

template <typename G>
requires access::_has_source_adl<G, vertex_edge_range_t<G>> || _can_eval_source<G, vertex_edge_range_t<G>>
auto&& source(G&& g, edge_reference_t<G> uv) {
  if constexpr (access::_has_source_adl<G, vertex_edge_range_t<G>>)
    return access::source(g, uv);
  else if constexpr (_can_eval_source<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + source_key(g, uv));
}

// edge_key(g,uv)
//
namespace access {
  template <typename G, typename ER>
  concept _has_edge_key_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {edge_key(g, uv)};
  };
} // namespace access
template <typename G, typename ER>
concept _can_eval_edge_key = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target_key(g, uv)};
  {source_key(g, uv)};
};

template <typename G>
requires access::_has_edge_key_adl<G, vertex_edge_range_t<G>> || _can_eval_edge_key<G, vertex_edge_range_t<G>>
auto edge_key(G&& g, edge_reference_t<G> uv) {
  if constexpr (access::_has_edge_key_adl<G, vertex_edge_range_t<G>>)
    return access::edge_key(g, uv);
  else if constexpr (_can_eval_edge_key<G, vertex_edge_range_t<G>>)
    return pair(source_key(g, uv), target_key(g, uv));
}

// other_key
//
namespace access {
  template <typename G, typename ER>
  concept _has_other_key_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {other_key(g, uv)};
  };
} // namespace access
template <typename G, typename ER>
concept _can_eval_other_key = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target_key(g, uv)};
  {source_key(g, uv)};
};

template <typename G>
requires access::_has_other_key_adl<G, vertex_edge_range_t<G>> || _can_eval_other_key<G, vertex_edge_range_t<G>>
auto other_key(G&& g, edge_reference_t<G> uv, vertex_key_t<G> xkey) {
  if constexpr (access::_has_other_key_adl<G, vertex_edge_range_t<G>>)
    return access::other_key(g, uv, xkey);
  else if constexpr (_can_eval_other_key<G, vertex_edge_range_t<G>>)
    return xkey != target_key(g, uv) ? target_key(g, uv) : source_key(g, uv);
}

// other_vertex
//
namespace access {
  template <typename G, typename ER>
  concept _has_other_vertex_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {other_vertex(g, uv)};
  };
} // namespace access
template <typename G, typename ER>
concept _can_eval_other_vertex = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target(g, uv)};
  {source(g, uv)};
};

template <typename G>
requires access::_has_other_vertex_adl<G, vertex_edge_range_t<G>> || _can_eval_other_vertex<G, vertex_edge_range_t<G>>
auto&& other_vertex(G&& g, edge_reference_t<G> uv, vertex_reference_t<G> x) {
  if constexpr (access::_has_other_vertex_adl<G, vertex_edge_range_t<G>>)
    return access::other_vertex(g, uv, x);
  else if constexpr (_can_eval_other_vertex<G, vertex_edge_range_t<G>>)
    return &x != &target(g, uv) ? target(g, uv) : source(g, uv);
}

// find_vertex
//
namespace access {
  template <typename G>
  concept _has_find_vertex_adl = requires(G&& g, vertex_key_t<G> ukey) {
    {find_vertex(g, ukey)};
  };
} // namespace access

template <typename G>
requires access::_has_find_vertex_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_find_vertex_adl<G>)
    return access::find_vertex(g, ukey);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    return begin(vertices(g)) + ukey;
}

// find_vertex_edge
//
namespace access {
  template <typename G>
  concept _has_find_vertex_edge_adl =
        requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
    {find_vertex_edge(g, u, vkey)};
  };
  template <typename G>
  concept _has_find_vertex_key_edge_adl =
        requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
    {find_vertex_edge(g, ukey, vkey)};
  };
} // namespace access

template <typename G>
auto find_vertex_edge(G&& g, vertex_reference_t<G> u, vertex_key_t<G> vkey) {
  if constexpr (access::_has_find_vertex_edge_adl<G>)
    return access::find_vertex_edge(g, u, vkey);
  else
    return ranges::find(edges(g, u), [&g, &vkey](auto&& uv) { return target_key(g, uv) == vkey; });
}

template <typename G>
requires access::_has_find_vertex_key_edge_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex_edge(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  if constexpr (access::_has_find_vertex_key_edge_adl<G>)
    return access::find_vertex_edge(g, ukey, vkey);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    find_vertex_edge(g, *(begin(vertices(g)) + ukey), vkey);
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
