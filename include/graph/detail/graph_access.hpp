#pragma once

// "other" functions not needed if we can swap source/target in views when needed for undirected_incidence_graph?
//#define ENABLE_OTHER_FNC

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

// Tags defined in access to avoid conflicts with function names in std::graph,
// allowing customization for default behavior.
//
// graphs must use tags like std::graph::tag_invoke::vertex_id_fn_t when defining
// CPO specialization.
//
// Minimal requirements for a graph with random_access vertices(g)
//  vertices(g), edges(g,u), target_id(g,u)
//     vertex_id(g,ui) to have vertex_id_t<G> be something other than size_t
//  as needed by algorithm(s): edge_value(g,uv), vertex_value(g,uv), graph_value(g)
//
// For a sourced_graph the minimal requirements also add
//  source_id(g,uv)
//
namespace tag_invoke {
  // ranges
  TAG_INVOKE_DEF(vertices); // vertices(g) -> [graph vertices]

  TAG_INVOKE_DEF(edges); // edges(g,u) -> [incidence edges]
                         // edges(g,uid) -> [incidence edges]

  // graph value
  TAG_INVOKE_DEF(graph_value); // graph_value(g) -> GV&

  // vertex values
  TAG_INVOKE_DEF(vertex_id); // vertex_id(g,ui) -> VId
                             // default = ui - begin(vertices(g)) for random_access_iterator<ui>

  TAG_INVOKE_DEF(vertex_value); // vertex_value(g,u) -> VV&

  TAG_INVOKE_DEF(degree); // degree(g,u) -> VId
                          // default = size(edges(g,u))

  // edge values
  TAG_INVOKE_DEF(target_id); // target_id(g,uv) -> VId

  TAG_INVOKE_DEF(target); // target(g,uv) -> v
                          // default = *(begin(g,vertices(g)) + target_id(g,uv))
                          // for random_access_range<vertices(g)> and integral<target_id(g,uv))
                          // uv can be from edges(g,u) or vertices(g,u)

  TAG_INVOKE_DEF(edge_value); // edge_value(g,uv) -> EV&

  // +sourced edge values (only available when source_id is on the edge)
  TAG_INVOKE_DEF(source_id); // source_id(g,uv) -> VId

  TAG_INVOKE_DEF(source); // source(g,uv) -> u
                          // default = *(begin(g,vertices(g)) + source_id(g,uv))
                          // for random_access_range<vertices(g)> and integral<source_id(g,uv))
                          // uv can be from edges(g,u) or vertices(g,u)

  TAG_INVOKE_DEF(edge_id); // edge_id(g,uv) -> pair<VId,VId>
                           // default = pair(source_id(g,uv),target_id(g,uv))

#  ifdef ENABLE_OTHER_FNC
  TAG_INVOKE_DEF(other_id); // other_id(g,uv,xid) -> VId (uid or vid)
                            // default = xid != target_id(g,uv) ? target_id(g,uv) : source_id(g,uv)

  TAG_INVOKE_DEF(other_vertex); // other_vertex(g,uv,x) -> y (u or v)
                                // default = x != &target(g,uv) ? target(g,uv) : source(g,uv)
#  endif

  // find
  TAG_INVOKE_DEF(find_vertex); // find_vertex(g,uid) -> ui
                               // default = begin(vertices(g)) + uid, for random_access_range<vertex_range_t<G>>

  TAG_INVOKE_DEF(find_vertex_edge); // find_vertex_edge(g,u,vid) -> uvi
                                    //        default = find(edges(g,u), [](uv) {target_id(g,uv)==vid;}
                                    // find_vertex_edge(g,uid,vid) -> uvi
                                    //        default = find_vertex_edge(g,*find_vertex(g,uid),vid)

  TAG_INVOKE_DEF(contains_edge); // contains_edge(g,uid,vid) -> bool
} // namespace tag_invoke

// Additional functions to consider for future
//  reserve_vertices(g,n) - noop if n/a
//  reserve_edges(g,n)    - noop if n/a
//
//  load_graph(g,erng,vrng,eproj,vproj)
//

//
// Vertex range & directly related types
//
template <class G>
auto vertices(G&& g) -> decltype(tag_invoke::vertices(g)) {
  return tag_invoke::vertices(g);
}

template <class G>
using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));
template <class G>
using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G&&>>;

template <class G>
using vertex_t = ranges::range_value_t<vertex_range_t<G>>;
template <class G>
using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;

//vertex_id(g,ui)
//
namespace tag_invoke {
  template <class G>
  concept _has_vertex_id_adl = requires(G&& g, vertex_iterator_t<G> ui) {
    {vertex_id(g, ui)};
  };
} // namespace tag_invoke
template <class G>
requires tag_invoke::_has_vertex_id_adl<G> || random_access_iterator<vertex_iterator_t<G>>
auto vertex_id(G&& g, vertex_iterator_t<G> ui) {
  if constexpr (tag_invoke::_has_vertex_id_adl<G>)
    return tag_invoke::vertex_id(g, ui);
  else if constexpr (random_access_iterator<vertex_iterator_t<G>>)
    return ui - ranges::begin(vertices(g));
}

template <class G>
using vertex_id_t = decltype(vertex_id(declval<G&&>(), declval<vertex_iterator_t<G>>()));


// find_vertex
//
namespace tag_invoke {
  template <class G>
  concept _has_find_vertex_adl = requires(G&& g, vertex_id_t<G> uid) {
    {find_vertex(g, uid)};
  };
} // namespace tag_invoke

template <class G>
requires tag_invoke::_has_find_vertex_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex(G&& g, vertex_id_t<G> uid) {
  if constexpr (tag_invoke::_has_find_vertex_adl<G>)
    return tag_invoke::find_vertex(g, uid);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    return begin(vertices(g)) + uid;
}

//vertex_value(g,u)
//
template <class G>
auto vertex_value(G&& g, vertex_reference_t<G> u) -> decltype(tag_invoke::vertex_value(g, u)) {
  return tag_invoke::vertex_value(g, u);
}
template <class G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

//
// Vertex-edge range (incidence range) & directly related types
//
namespace tag_invoke {
  template <class G>
  concept _has_edges_vtxref_adl = requires(G&& g, vertex_reference_t<G> u) {
    {edges(g, u)};
  };

  template <class G>
  concept _has_edges_vtxid_adl = requires(G&& g, vertex_id_t<G> uid) {
    {edges(g, uid)};
  };
} // namespace tag_invoke

template <class G>
requires tag_invoke::_has_edges_vtxref_adl<G>
auto edges(G&& g, vertex_reference_t<G> u) -> decltype(tag_invoke::edges(g, u)) {
  return tag_invoke::edges(g, u); // graph author must define
}
template <class G>
auto edges(G&& g, vertex_id_t<G> uid) -> decltype(tag_invoke::edges(g, uid)) {
  if constexpr (tag_invoke::_has_edges_vtxid_adl<G>)
    return tag_invoke::edges(g, uid);
  else
    return edges(g, *find_vertex(g, uid));
}

template <class G>
using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_reference_t<G>>()));
template <class G>
using vertex_edge_iterator_t = ranges::iterator_t<vertex_edge_range_t<G>>;
template <class G>
using edge_t = typename ranges::range_value_t<vertex_edge_range_t<G>>;
template <class G>
using edge_reference_t = ranges::range_reference_t<vertex_edge_range_t<G>>;

template <class G>
using edge_id_t = decltype(edge_id(declval<G&&>(),
                                   declval<edge_reference_t<G>>())); // e.g. pair<vertex_id_t<G>,vertex_id_t<G>>

//
// Edges range & directly related types
//

//
// Vertex properties
//

// degree - number of outgoing edges (e.g. neighbors)
//
namespace tag_invoke {
  template <class G>
  concept _has_degree_adl = requires(G&& g, vertex_reference_t<G> u) {
    {degree(g, u)};
  };
} // namespace tag_invoke

template <class G>
requires tag_invoke::_has_degree_adl<G> || ranges::sized_range<vertex_edge_range_t<G>>
auto degree(G&& g, vertex_reference_t<G> u) {
  if constexpr (tag_invoke::_has_degree_adl<G>)
    return tag_invoke::degree(g, u);
  else if constexpr (ranges::sized_range<vertex_edge_range_t<G>>) {
    return ranges::size(edges(g, u));
  }
}

//
// Edge properties
//

//target_id(g,uv)
//
template <class G>
auto target_id(G&& g, edge_reference_t<const G> uv) -> decltype(tag_invoke::target_id(g, uv)) {
  return tag_invoke::target_id(g, uv);
}

//target(g,uv)
//
namespace tag_invoke {
  template <class G, class ER>
  concept _has_target_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {target(g, uv)};
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_target = ranges::random_access_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  { target_id(g, uv) } -> integral;
};

template <class G>
requires tag_invoke::_has_target_adl<G, vertex_edge_range_t<G>> || _can_eval_target<G, vertex_edge_range_t<G>>
auto&& target(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_target_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::target(g, uv);
  else if constexpr (_can_eval_target<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + target_id(g, uv));
}

//
// Sourced Edge properties (when source_id(g,uv) is defined)
//

// source_id
//
template <class G>
auto source_id(G&& g, edge_reference_t<G> uv) -> decltype(tag_invoke::source_id(g, uv)) {
  return tag_invoke::source_id(g, uv);
}

//source(g,uv)
//
namespace tag_invoke {
  template <class G, class ER>
  concept _has_source_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {source(g, uv)};
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_source = ranges::random_access_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  { source_id(g, uv) } -> integral;
};

template <class G>
requires tag_invoke::_has_source_adl<G, vertex_edge_range_t<G>> || _can_eval_source<G, vertex_edge_range_t<G>>
auto&& source(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_source_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::source(g, uv);
  else if constexpr (_can_eval_source<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + source_id(g, uv));
}

// edge_id(g,uv)
//
namespace tag_invoke {
  template <class G, class ER>
  concept _has_edge_id_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {edge_id(g, uv)};
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_edge_id = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target_id(g, uv)};
  {source_id(g, uv)};
};

template <class G>
requires tag_invoke::_has_edge_id_adl<G, vertex_edge_range_t<G>> || _can_eval_edge_id<G, vertex_edge_range_t<G>>
auto edge_id(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_edge_id_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::edge_id(g, uv);
  else if constexpr (_can_eval_edge_id<G, vertex_edge_range_t<G>>)
    return pair(source_id(g, uv), target_id(g, uv));
}

//edge_value(g,uv)
//
template <class G>
auto edge_value(G&& g, edge_reference_t<G> uv) -> decltype(tag_invoke::edge_value(g, uv)) {
  return tag_invoke::edge_value(g, uv);
}

// edge value types
template <class G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));


// find_vertex_edge
//
namespace tag_invoke {
  template <class G>
  concept _has_find_vertex_edge_adl = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
    {find_vertex_edge(g, u, vid)};
  };
  template <class G>
  concept _has_find_vertex_id_edge_adl =
        requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
    {find_vertex_edge(g, uid, vid)};
  };
} // namespace tag_invoke

template <class G>
auto find_vertex_edge(G&& g, vertex_reference_t<G> u, vertex_id_t<G> vid) {
  if constexpr (tag_invoke::_has_find_vertex_edge_adl<G>)
    return tag_invoke::find_vertex_edge(g, u, vid);
  else
    return ranges::find_if(edges(g, u), [&g, &vid](auto&& uv) { return target_id(g, uv) == vid; });
}

template <class G>
requires tag_invoke::_has_find_vertex_id_edge_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex_edge(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  if constexpr (tag_invoke::_has_find_vertex_id_edge_adl<G>)
    return tag_invoke::find_vertex_edge(g, uid, vid);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    find_vertex_edge(g, *(begin(vertices(g)) + uid), vid);
}

// contains_edge
//
template <class G>
auto contains_edge(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  return tag_invoke::contains_edge(g, uid, vid);
}


// graph_value
//
template <class G>
auto&& graph_value(G&& g) {
  return tag_invoke::graph_value(g);
}
template <class G>
using graph_value_t = decltype(graph_value(declval<G&&>()));


#  ifdef ENABLE_OTHER_FNC
// other_id
//
namespace tag_invoke {
  template <class G, class ER>
  concept _has_other_id_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {other_id(g, uv)};
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_other_id = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target_id(g, uv)};
  {source_id(g, uv)};
};

template <class G>
requires tag_invoke::_has_other_id_adl<G, vertex_edge_range_t<G>> || _can_eval_other_id<G, vertex_edge_range_t<G>>
auto other_id(G&& g, edge_reference_t<G> uv, vertex_id_t<G> xid) {
  if constexpr (tag_invoke::_has_other_id_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::other_id(g, uv, xid);
  else if constexpr (_can_eval_other_id<G, vertex_edge_range_t<G>>)
    return xid != target_id(g, uv) ? target_id(g, uv) : source_id(g, uv);
}

// other_vertex
//
namespace tag_invoke {
  template <class G, class ER>
  concept _has_other_vertex_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    {other_vertex(g, uv)};
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_other_vertex = requires(G&& g, ranges::range_reference_t<ER> uv) {
  {target(g, uv)};
  {source(g, uv)};
};

template <class G>
requires tag_invoke::_has_other_vertex_adl<G, vertex_edge_range_t<G>> || _can_eval_other_vertex<G, vertex_edge_range_t<G>>
auto&& other_vertex(G&& g, edge_reference_t<G> uv, vertex_reference_t<G> x) {
  if constexpr (tag_invoke::_has_other_vertex_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::other_vertex(g, uv, x);
  else if constexpr (_can_eval_other_vertex<G, vertex_edge_range_t<G>>)
    return &x != &target(g, uv) ? target(g, uv) : source(g, uv);
}
#  endif //ENABLE_OTHER_FNC


} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
