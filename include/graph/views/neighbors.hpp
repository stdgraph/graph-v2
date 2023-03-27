#pragma once
#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"

//
// neighbors(g,u) -> neighbor_descriptor<VId,false,E,EV> -> {target_id, vertex& [,value]}
//
// given:    auto vvf = [&g](vertex_reference_t<G> v) { return vertex_value(g,v); }
//
// examples: for([vid, v]             : neighbors(g,uid))
//           for([vid, v, value]      : neighbors(g,uid,vvf))
//
// Since u is passed to incidence(), there's no need to include Sourced versions of
// incidence().
//
namespace std::graph {


template <adjacency_list G, bool Sourced = false, class VVF = void>
class neighbor_iterator;


/**
 * @brief Iterator for an neighbors range of edges for a vertex.
 *
 * @tparam G    Graph type
 * @tparam VVF  Edge Value Function type
*/
template <adjacency_list G, bool Sourced, class VVF>
class neighbor_iterator
      : public source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G, edge_t<G>>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G, edge_t<G>>)>;

  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = invoke_result_t<VVF, vertex_reference_type>;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_descriptor<const vertex_id_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter, const VVF& value_fn)
        : base_type(vertex_id(g, ui)), g_(g), iter_(iter), value_fn_(&value_fn) {}
  neighbor_iterator(graph_type& g, vertex_id_type uid, const VVF& value_fn)
        : base_type(uid), g_(g), iter_(ranges::begin(edges(g, uid))), value_fn_(&value_fn) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&)      = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type =
        neighbor_descriptor<vertex_id_type, Sourced, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (unordered_edge<G, edge_type>) {
      static_assert(sourced_adjacency_list<G>);
      if (target_id(g_, *iter_) != this->source_vertex_id()) {
        value_.source_id = source_id(g_.*iter_);
        value_.target_id = target_id(g_, *iter_);
        value_.target    = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
      } else {
        value_.source_id = target_id(g_.*iter_);
        value_.target_id = source_id(g_, *iter_);
        value_.target    = const_cast<shadow_vertex_type*>(&source(g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.source_id = source_id(g_, *iter_);
        value_.target_id = target_id(g_, *iter_);
      } else {
        value_.source_id = this->source_vertex_id();
        value_.target_id = target_id(g_, *iter_);
      }
      value_.target = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    } else {
      value_.target_id = target_id(g_, *iter_);
      value_.target    = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    }
    value_.value = invoke(*value_fn_, *value_.target); // 'value' undeclared identifier (.value not in struct?)
    return reinterpret_cast<reference>(value_);
  }

  constexpr neighbor_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr neighbor_iterator operator++(int) const {
    neighbor_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;
  const VVF*                       value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const neighbor_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, bool Sourced>
class neighbor_iterator<G, Sourced, void>
      : public source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G, edge_t<G>>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G, edge_t<G>>)>;

  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = void;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_descriptor<const vertex_id_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type  = neighbor_descriptor<vertex_id_type, Sourced, shadow_vertex_type*, vertex_value_type>;

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter)
        : base_type(vertex_id(g, ui)), g_(g), iter_(iter) {}
  neighbor_iterator(graph_type& g, vertex_id_type uid) : base_type(uid), g_(g), iter_(ranges::begin(edges(g, uid))) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (unordered_edge<G, edge_type>) {
      static_assert(sourced_adjacency_list<G>);
      if (target_id(g_, *iter_) != this->source_vertex_id()) {
        value_.source_id = source_id(g_.*iter_);
        value_.target_id = target_id(g_, *iter_);
        value_.target    = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
      } else {
        value_.source_id = target_id(g_.*iter_);
        value_.target_id = source_id(g_, *iter_);
        value_.target    = const_cast<shadow_vertex_type*>(&source(g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.source_id = source_id(g_, *iter_);
        value_.target_id = target_id(g_, *iter_);
      } else {
        value_.source_id = this->source_vertex_id();
        value_.target_id = target_id(g_, *iter_);
      }
      value_.target = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    } else {
      value_.target_id = target_id(g_, *iter_);
      value_.target    = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    }
    return reinterpret_cast<reference>(value_);
  }

  constexpr neighbor_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr neighbor_iterator operator++(int) const {
    neighbor_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;

  friend bool operator==(const edge_iterator& lhs, const neighbor_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, bool Sourced, class VVF>
using neighbors_view = ranges::subrange<neighbor_iterator<G, Sourced, VVF>, vertex_edge_iterator_t<G>>;
} // namespace std::graph

namespace std::graph::tag_invoke {
// ranges
TAG_INVOKE_DEF(neighbors); // neighbors(g,uid)            -> edges[vid,uv]
                           // neighbors(g,uid,fn)         -> edges[vid,uv,value]

template <class G>
concept _has_neighbors_g_uid_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> uid) {
                                                        { neighbors(g, uid) };
                                                      };
template <class G, class VVF>
concept _has_neighbors_g_uid_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> uid, const VVF& vvf) {
                                                            { neighbors(g, uid, vvf) };
                                                          };
} // namespace std::graph::tag_invoke

namespace std::graph::views {
//
// neighbors(g,uid)
//
template <adjacency_list G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_id_t<G> uid) {
  if constexpr (std::graph::tag_invoke::_has_neighbors_g_uid_adl<G>)
    return std::graph::tag_invoke::neighbors(g, uid);
  else {
    using iterator_type = neighbor_iterator<G, false, void>;
    return neighbors_view<G, false, void>(iterator_type(g, uid), ranges::end(edges(g, uid)));
  }
}


//
// neighbors(g,uid,vvf)
//
template <adjacency_list G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_id_t<G> uid, const VVF& vvf) {
  if constexpr (std::graph::tag_invoke::_has_neighbors_g_uid_evf_adl<G, VVF>)
    return std::graph::tag_invoke::neighbors(g, uid, vvf);
  else {
    using iterator_type = neighbor_iterator<G, false, VVF>;
    return neighbors_view<G, false, VVF>(iterator_type(g, uid, vvf), ranges::end(edges(g, uid)));
  }
}
} // namespace std::graph::views
