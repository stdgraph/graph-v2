#pragma once

#include "graph.hpp"

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

// This file implements the interface for an edgelist (el).
//
// An edgelist is a range of edges where source_id(e) and target_id(e) are property
// functions that can be called on an edge (value type of the range).
//
// An optional edge_value(e) property can also be used if a value is defined for
// the edgelist. Use the has_edge_value<EL> concept to determine if it defined.
//
// The concepts, types and property functions mirror definitions for edges and
// and edge for an adjacency list.
//

// edgelist concepts
// -----------------------------------------------------------------------------
// basic_sourced_edgelist<EL>
// basic_sourced_index_edgelist<EL>
// has_edge_value<EL>
//
// Type aliases
// -----------------------------------------------------------------------------
// edge_range_t<EL>     = EL
// edge_iterator_t<EL>  = range_iterator_t<EL>
// edge_t<EL>           = range_value_t<EL>
// edge_reference_t<EL> = range_value_t<EL>
// edge_value_t<EL>     = decltype(edge_value(e)) (optional)
// vertex_id_t<EL>      = decltype(source_id(e))
//
// edgelist functions
// -----------------------------------------------------------------------------
// num_edges(el)  (todo)
// has_edge(el)  (todo)
// contains_edge(el,uid,vid)  (todo)
//
// edge functions
// -----------------------------------------------------------------------------
// source_id(e)
// target_id(e)
// edge_value(e)
//
// Edge definitions supported without overrides
// -----------------------------------------------------------------------------
// The standard implementation supports two edge types with support for
// source_id(e) and target_id(e) to return their respective values, and an
// optional edge_value(e) if the edge has a value (shown following). The
// functions can be overridden for user-defined edge types.
//
//  pair<T,T>
//  tuple<T,T>
//  tuple<T,T,EV,...>
//
//  edge_descriptor<VId, true, void, void> : {source_id, target_id}
//  edge_descriptor<VId, true, void, EV>   : {source_id, target_id, EV}
//
//  edge_descriptor<VId, true, E&, void>   : {source_id, target_id, edge}
//  edge_descriptor<VId, true, E&, EV>     : {source_id, target_id, edge, EV}
//
// Naming conventions
// -----------------------------------------------------------------------------
// Type     Variable    Description
// -------- ----------- --------------------------------------------------------
// EL       el          EdgeList
// E        e           Edge on an edgelist
// EV       val         Edge Value
//

// merge implementation into graph with single namespace?
// Issues:
//  1.  name conflict with edgelist view? No: basic_sourced_edgelist vs. views::edgelist.
//  2.  template aliases can't be distinguished by concepts
//  3.  vertex_id_t definition for adjlist and edgelist have be done in separate locations

namespace graph {


namespace edgelist {
  //
  // edgelist concepts
  //
  template <class EL>                                           // For exposition only
  concept basic_sourced_edgelist = input_range<EL> &&           //
                                   !range<range_value_t<EL>> && // distinguish from adjacency list
                                   requires(range_value_t<EL> e) {
                                     { source_id(e) };
                                     { target_id(e) } -> same_as<decltype(source_id(e))>;
                                   };

  template <class EL>                                                  // For exposition only
  concept basic_sourced_index_edgelist = basic_sourced_edgelist<EL> && //
                                         requires(range_value_t<EL> e) {
                                           { source_id(e) } -> integral;
                                           { target_id(e) } -> integral; // this is redundant, but makes it clear
                                         };

  // (non-basic concepts imply inclusion of an edge reference which doesn't make much sense)

  template <class EL>                                    // For exposition only
  concept has_edge_value = basic_sourced_edgelist<EL> && //
                           requires(range_value_t<EL> e) {
                             { edge_value(e) };
                           };

  template <class EL>
  struct is_directed : public std::false_type {}; // specialized for graph container

  template <class EL>
  inline constexpr bool is_directed_v = is_directed<EL>::value;


  //
  // edgelist types (note that concepts don't really do anything except document expectations)
  //
  template <basic_sourced_edgelist EL> // For exposition only
  using edge_range_t = EL;

  template <basic_sourced_edgelist EL> // For exposition only
  using edge_iterator_t = iterator_t<edge_range_t<EL>>;

  template <basic_sourced_edgelist EL> // For exposition only
  using edge_t = range_value_t<edge_range_t<EL>>;

  template <basic_sourced_edgelist EL> // For exposition only
  using edge_reference_t = range_reference_t<edge_range_t<EL>>;

  template <basic_sourced_edgelist EL> // For exposition only
  using edge_value_t = decltype(edge_value(declval<edge_t<edge_range_t<EL>>>()));

  template <basic_sourced_edgelist EL> // For exposition only
  using vertex_id_t = decltype(source_id(declval<edge_t<edge_range_t<EL>>>()));


  // template aliases can't be distinguished with concepts :(
  //
  //template <basic_adjacency_list G> // For exposition only
  //using vid_t = decltype(vertex_id(declval<G>()));
  //
  //template <basic_sourced_edgelist EL> // For exposition only
  //using vid_t = decltype(source_id(declval<edge_t<EL>>()));

} // namespace edgelist
} // namespace graph

#endif
