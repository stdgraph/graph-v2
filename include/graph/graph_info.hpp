﻿#pragma once
#include <concepts>
#include "detail/graph_using.hpp"

namespace graph {
//
// vertex_info
// for(auto&& [uid, u]        : vertexlist(g))
// for(auto&& [uid, u, value] : vertexlist(g, [](vertex_reference_t<G> u) { return ...; } )
//
template <class VId, class V, class VV>
struct vertex_info {
  using id_type     = VId; // e.g. vertex_id_t<G>
  using vertex_type = V;   // e.g. vertex_reference_t<G>
  using value_type  = VV;  // e.g. vertex_value_t<G>

  id_type     id;
  vertex_type vertex;
  value_type  value;
};
template <class VId, class V>
struct vertex_info<VId, V, void> {
  using id_type     = VId;
  using vertex_type = V;
  using value_type  = void;

  id_type     id;
  vertex_type vertex;
};
template <class VId, class VV>
struct vertex_info<VId, void, VV> {
  using id_type     = VId;
  using vertex_type = void;
  using value_type  = VV;

  id_type    id;
  value_type value;
};
template <class VId>
struct vertex_info<VId, void, void> {
  using id_type     = VId;
  using vertex_type = void;
  using value_type  = void;

  id_type id;
};

template <class V, class VV>
struct vertex_info<void, V, VV> {
  using id_type     = void; // e.g. vertex_id_t<G>
  using vertex_type = V;   // e.g. vertex_reference_t<G>
  using value_type  = VV;  // e.g. vertex_value_t<G>

  vertex_type vertex;
  value_type  value;
};
template <class V>
struct vertex_info<void, V, void> {
  using id_type     = void; // e.g. vertex_id_t<G>
  using vertex_type = V;   // e.g. vertex_reference_t<G>
  using value_type  = void;  // e.g. vertex_value_t<G>

  vertex_type vertex;
};
template <class VV>
struct vertex_info<void, void, VV> {
  using id_type     = void; // e.g. vertex_id_t<G>
  using vertex_type = void;   // e.g. vertex_reference_t<G>
  using value_type  = VV;  // e.g. vertex_value_t<G>

  value_type  value;
};

template <class VId, class VV>
using copyable_vertex_t = vertex_info<VId, void, VV>; // {id, value}

//
// edge_info
//
// for(auto&& [target_id, uv]        : incidence(g,u))
// for(auto&& [target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
// for(auto&& [source_id, target_id, uv]        : incidence(g,u))
// for(auto&& [source_id, target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
template <class VId, bool Sourced, class E, class EV>
struct edge_info {
  using source_id_type = VId; // e.g. vertex_id_t<G> when Sourced==true, or void
  using target_id_type = VId; // e.g. vertex_id_t<G>
  using edge_type      = E;   // e.g. edge_reference_t<G> or void
  using value_type     = EV;  // e.g. edge_value_t<G> or void

  source_id_type source_id;
  target_id_type target_id;
  edge_type      edge;
  value_type     value;
};

template <class VId, class E>
struct edge_info<VId, true, E, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
  edge_type      edge;
};
template <class VId>
struct edge_info<VId, true, void, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
};
template <class VId, class EV>
struct edge_info<VId, true, void, EV> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = EV;

  source_id_type source_id;
  target_id_type target_id;
  value_type     value;
};

template <class VId, class E, class EV>
struct edge_info<VId, false, E, EV> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = EV;

  target_id_type target_id;
  edge_type      edge;
  value_type     value;
};
template <class VId, class E>
struct edge_info<VId, false, E, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = void;

  target_id_type target_id;
  edge_type      edge;
};

template <class VId, class EV>
struct edge_info<VId, false, void, EV> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = EV;

  target_id_type target_id;
  value_type     value;
};
template <class VId>
struct edge_info<VId, false, void, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = void;

  target_id_type target_id;
};

//
// targeted_edge
// for(auto&& [vid,uv,value] : edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [vid,uv]       : edges_view(g, u) )
//
//template <class VId, class E, class EV>
//using targeted_edge = edge_info<VId, false, E, EV>; // {target_id, edge, [, value]}

//
// sourced_edge
// for(auto&& [uid,vid,uv,value] : sourced_edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [uid,vid,uv]       : sourced_edges_view(g, u) )
//
//template <class VId, class V, class E, class EV>
//using sourced_edge = edge_info<VId, true, E, EV>; // {source_id, target_id, edge, [, value]}

//
// edgelist_edge
// for(auto&& [uid,vid,uv,value] : edges_view(g, [](vertex_edge_reference_t<G> g) { return ...; } )
// for(auto&& [uid,vid,uv]       : edges_view(g) )
//
template <class VId, class E, class EV>
using edgelist_edge = edge_info<VId, true, E, EV>; // {source_id, target_id [, edge] [, value]}

//
// copyable_edge_t
//
template <class VId, class EV = void>
using copyable_edge_t = edge_info<VId, true, void, EV>; // {source_id, target_id [, value]}

//
// neighbor_info (for adjacency)
//
template <class VId, bool Sourced, class V, class VV>
struct neighbor_info {
  using source_id_type = VId; // e.g. vertex_id_t<G> when Sourced==true, or void
  using target_id_type = VId; // e.g. vertex_id_t<G>
  using vertex_type    = V;   // e.g. vertex_reference_t<G> or void
  using value_type     = VV;  // e.g. vertex_value_t<G> or void

  source_id_type source_id;
  target_id_type target_id;
  vertex_type    target;
  value_type     value;
};

template <class VId, class V, class VV>
struct neighbor_info<VId, false, V, VV> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = VV;

  target_id_type target_id;
  vertex_type    target;
  value_type     value;
};

template <class VId, class V>
struct neighbor_info<VId, false, V, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = void;

  target_id_type target_id;
  vertex_type    target;
};

template <class VId, class VV>
struct neighbor_info<VId, false, void, VV> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = VV;

  target_id_type target_id;
  value_type     value;
};

template <class VId>
struct neighbor_info<VId, false, void, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = void;

  target_id_type target_id;
};

template <class VId, class V>
struct neighbor_info<VId, true, V, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
  vertex_type    target;
};

template <class VId, class VV>
struct neighbor_info<VId, true, void, VV> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = VV;

  source_id_type source_id;
  target_id_type target_id;
  value_type     value;
};

template <class VId>
struct neighbor_info<VId, true, void, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
};

//
// copyable_edge_t
//
template <class VId, class VV>                                  // For exposition only
using copyable_neighbor_t = neighbor_info<VId, true, void, VV>; // {source_id, target_id [, value]}

//
// view concepts
//
template <class T, class VId, class VV = void> // For exposition only
concept copyable_vertex = std::convertible_to<T, copyable_vertex_t<VId, VV>>;

template <class T, class VId, class EV = void> // For exposition only
concept copyable_edge = std::convertible_to<T, copyable_edge_t<VId, EV>>;

template <class T, class VId, class EV = void> // For exposition only
concept copyable_neighbor = std::convertible_to<T, copyable_neighbor_t<VId, EV>>;

//
// is_sourced<G>
//
template <class T>
inline constexpr bool is_sourced_v = false;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<edge_info<VId, true, V, VV>> = true;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<neighbor_info<VId, true, V, VV>> = true;

} // namespace graph
