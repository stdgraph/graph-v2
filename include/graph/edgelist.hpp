#pragma once

#include "detail/graph_cpo.hpp"

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

// function support
// source_id(el)
// target_id(el)
// edge_value(el)
// num_edges
// has_edge
// contains_edge(el,uid,vid)
// edge_id
//
// type support
// vertex_id_t<EL>
// edge_t<EL>
// edge_value_t<EL>
// edge_range_t<EL>
//
// edge_descriptor<VId, true, void, void> : {source_id, target_id}
// edge_descriptor<VId, true, void, EV>   : {source_id, target_id, EV}
//
// is_edge_descriptor_v<E>
//

// Target concepts for edgelist
namespace std::graph::edgelist {
// move implementations for source_id(e), target_id(e), edge_value(e) into std::graph::edgelist?
// other functions to support: num_edges(el), contains_edge(uid,vid), edge_id(e)

//
// edgelist ranges
//
template <class E> // For exposition only
concept _source_target_id = requires(E e) {
  { source_id(e) };
  { target_id(e) } -> same_as<decltype(source_id(e))>;
};
template <class E> // For exposition only
concept _index_source_target_id = requires(E e) {
  { source_id(e) } -> integral;
  { target_id(e) } -> same_as<decltype(source_id(e))>;
};
template <class E> // For exposition only
concept _has_edge_value = requires(E e) {
  { edge_value(e) };
};

template <class EL> // For exposition only
concept basic_sourced_edgelist = ranges::forward_range<EL> && _source_target_id<ranges::range_value_t<EL>>;

template <class EL> // For exposition only
concept basic_sourced_index_edgelist = ranges::forward_range<EL> && _index_source_target_id<ranges::range_value_t<EL>>;

template <class EL> // For exposition only
concept sourced_edgelist = basic_sourced_edgelist<EL> && _has_edge_value<ranges::range_value_t<EL>>;

template <class EL> // For exposition only
concept sourced_index_edgelist = basic_sourced_index_edgelist<EL> && _has_edge_value<ranges::range_value_t<EL>>;

//
// edgelist types (note that concepts don't really do anything except document expectations)
//
template <basic_sourced_edgelist EL> // For exposition only
using edge_iterator_t = ranges::iterator_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_t = ranges::range_value_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_reference_t = ranges::range_reference_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using vertex_id_t = decltype(source_id(declval<edge_t<EL>>()));

template <sourced_edgelist EL> // For exposition only
using edge_value_t = decltype(edge_value(declval<edge_t<EL>>()));

template <sourced_edgelist EL> // For exposition only
using edge_id_t = edge_descriptor<vertex_id_t<EL>, true, void, void>;

// template aliases can't be distinguished with concepts :(
//
//template <basic_adjacency_list G> // For exposition only
//using vid_t = decltype(vertex_id(declval<G>()));
//
//template <basic_sourced_edgelist EL> // For exposition only
//using vid_t = decltype(source_id(declval<edge_t<EL>>()));

} // namespace std::graph::edgelist

#endif
