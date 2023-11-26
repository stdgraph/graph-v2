#pragma once

namespace std::graph {
//
// vertex_descriptor
// for(auto&& [uid, u]        : vertexlist(g))
// for(auto&& [uid, u, value] : vertexlist(g, [](vertex_reference_t<G> u) { return ...; } )
//
template <class VId, class V, class VV>
struct vertex_descriptor {
  VId id;
  V   vertex;
  VV  value;
};
template <class VId, class V>
struct vertex_descriptor<VId, V, void> {
  VId id;
  V   vertex;
};
template <class VId, class VV>
struct vertex_descriptor<VId, void, VV> {
  VId id;
  VV  value;
};
template <class VId>
struct vertex_descriptor<VId, void, void> {
  VId id;
};

template <class VId, class VV>
using copyable_vertex_t = vertex_descriptor<VId, void, VV>; // {id, value}

//
// edge_descriptor
//
// for(auto&& [target_id, uv]        : incidence(g,u))
// for(auto&& [target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
// for(auto&& [source_id, target_id, uv]        : incidence(g,u))
// for(auto&& [source_id, target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
template <class VId, bool Sourced, class E, class EV>
struct edge_descriptor {
  VId source_id;
  VId target_id;
  E   edge;
  EV  value;
};

template <class VId, class E>
struct edge_descriptor<VId, true, E, void> {
  VId source_id;
  VId target_id;
  E   edge;
};
template <class VId>
struct edge_descriptor<VId, true, void, void> {
  VId source_id;
  VId target_id;
};
template <class VId, class EV>
struct edge_descriptor<VId, true, void, EV> {
  VId source_id;
  VId target_id;
  EV  value;
};

template <class VId, class E, class EV>
struct edge_descriptor<VId, false, E, EV> {
  VId target_id;
  E   edge;
  EV  value;
};
template <class VId, class E>
struct edge_descriptor<VId, false, E, void> {
  VId target_id;
  E   edge;
};

template <class VId, class EV>
struct edge_descriptor<VId, false, void, EV> {
  VId target_id;
  EV  value;
};
template <class VId>
struct edge_descriptor<VId, false, void, void> {
  VId target_id;
};

//
// targeted_edge
// for(auto&& [vid,uv,value] : edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [vid,uv]       : edges_view(g, u) )
//
//template <class VId, class E, class EV>
//using targeted_edge = edge_descriptor<VId, false, E, EV>; // {target_id, edge, [, value]}

//
// sourced_edge
// for(auto&& [uid,vid,uv,value] : sourced_edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [uid,vid,uv]       : sourced_edges_view(g, u) )
//
//template <class VId, class V, class E, class EV>
//using sourced_edge = edge_descriptor<VId, true, E, EV>; // {source_id, target_id, edge, [, value]}

//
// edgelist_edge
// for(auto&& [uid,vid,uv,value] : edges_view(g, [](vertex_edge_reference_t<G> g) { return ...; } )
// for(auto&& [uid,vid,uv]       : edges_view(g) )
//
template <class VId, class E, class EV>
using edgelist_edge = edge_descriptor<VId, true, E, EV>; // {source_id, target_id [, edge] [, value]}

//
// copyable_edge_t
//
template <class VId, class EV>
using copyable_edge_t = edge_descriptor<VId, true, void, EV>; // {source_id, target_id [, value]}

//
// neighbor_descriptor (for adjacency)
//
template <class VId, bool Sourced, class V, class VV>
struct neighbor_descriptor {
  VId source_id;
  VId target_id;
  V   target;
  VV  value;
};

template <class VId, class V, class VV>
struct neighbor_descriptor<VId, false, V, VV> {
  VId target_id;
  V   target;
  VV  value;
};

template <class VId, class V>
struct neighbor_descriptor<VId, false, V, void> {
  VId target_id;
  V   target;
};

template <class VId, class VV>
struct neighbor_descriptor<VId, false, void, VV> {
  VId target_id;
  VV  value;
};

template <class VId>
struct neighbor_descriptor<VId, false, void, void> {
  VId target_id;
};

template <class VId, class V>
struct neighbor_descriptor<VId, true, V, void> {
  VId source_id;
  VId target_id;
  V   target;
};

template <class VId, class VV>
struct neighbor_descriptor<VId, true, void, VV> {
  VId source_id;
  VId target_id;
  VV  value;
};

template <class VId>
struct neighbor_descriptor<VId, true, void, void> {
  VId source_id;
  VId target_id;
};

//
// view concepts
//
template <class T, class VId, class VV>
concept copyable_vertex = convertible_to<T, copyable_vertex_t<VId, VV>>;

template <class T, class VId, class EV>
concept copyable_edge = convertible_to<T, copyable_edge_t<VId, EV>>;

//
// is_sourced<G>
//
template <class T>
inline constexpr bool is_sourced_v = false;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<edge_descriptor<VId, true, V, VV>> = true;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<neighbor_descriptor<VId, true, V, VV>> = true;

} // namespace std::graph
