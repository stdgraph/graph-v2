#pragma once
#include "graph/graph.hpp"
#include "views_utility.hpp"

//
// neighbors(g,u) -> neighbor_view<VKey,Sourced,E,EV>:
//
// enable: for([vkey, uv]              : neighbors(g,ukey))
//         for([vkey, uv, value]       : neighbors(g,ukey,fn))
//         for([ukey, vkey, uv]        : sourced_neighbors(g,ukey))
//         for([ukey, vkey, uv, value] : sourced_neighbors(g,ukey,fn))
//
namespace std::graph::views {


template <class G, bool Sourced = false, class VVF = void>
class neighbor_iterator;


/// <summary>
/// Iterator for an neighbors range of edges for a vertex.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <typeparam name="VVF">Edge Value Function</typeparam>
template <class G, bool Sourced, class VVF>
class neighbor_iterator
      : public source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)>;

  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = invoke_result_t<VVF, vertex_reference_type>;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_view<const vertex_key_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter, const VVF& value_fn)
        : base_type(vertex_key(g, ui)), g_(g), iter_(iter), value_fn_(&value_fn) {}
  neighbor_iterator(graph_type& g, vertex_key_type ukey, const VVF& value_fn)
        : base_type(ukey), g_(g), iter_(ranges::begin(edges(g, ukey))), value_fn_(&value_fn) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&) = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type =
        neighbor_view<vertex_key_type, Sourced, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (undirected_incidence_graph<G>) {
      static_assert(sourced_incidence_graph<G>);
      if (target_key(g_, *iter_) != this->source_vertex_key()) {
        value_.source_key = source_key(g_.*iter_);
        value_.target_key = target_key(g_, *iter_);
        value_.target     = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
      } else {
        value_.source_key = target_key(g_.*iter_);
        value_.target_key = source_key(g_, *iter_);
        value_.target     = const_cast<shadow_vertex_type*>(&source(g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_incidence_graph<G>) {
        value_.source_key = source_key(g_, *iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = this->source_vertex_key();
        value_.target_key = target_key(g_, *iter_);
      }
      value_.target = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    } else {
      value_.target_key = target_key(g_, *iter_);
      value_.target     = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
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


template <class G, bool Sourced>
class neighbor_iterator<G, Sourced, void>
      : public source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)>;

  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = void;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_view<const vertex_key_type, Sourced, vertex_reference_type, vertex_value_type>;
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
  using shadow_value_type  = neighbor_view<vertex_key_type, Sourced, shadow_vertex_type*, vertex_value_type>;

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter)
        : base_type(vertex_key(g, ui)), g_(g), iter_(iter) {}
  neighbor_iterator(graph_type& g, vertex_key_type ukey)
        : base_type(ukey), g_(g), iter_(ranges::begin(edges(g, ukey))) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&) = default;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (undirected_incidence_graph<G>) {
      static_assert(sourced_incidence_graph<G>);
      if (target_key(g_, *iter_) != this->source_vertex_key()) {
        value_.source_key = source_key(g_.*iter_);
        value_.target_key = target_key(g_, *iter_);
        value_.target     = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
      } else {
        value_.source_key = target_key(g_.*iter_);
        value_.target_key = source_key(g_, *iter_);
        value_.target     = const_cast<shadow_vertex_type*>(&source(g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_incidence_graph<G>) {
        value_.source_key = source_key(g_, *iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = this->source_vertex_key();
        value_.target_key = target_key(g_, *iter_);
      }
      value_.target = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    } else {
      value_.target_key = target_key(g_, *iter_);
      value_.target     = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
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


template <class G, bool Sourced, class VVF>
using neighbors_view = ranges::subrange<neighbor_iterator<G, Sourced, VVF>, vertex_edge_iterator_t<G>>;

namespace access {
  // ranges
  TAG_INVOKE_DEF(neighbors); // neighbors(g,ukey)            -> edges[vkey,uv]
                             // neighbors(g,ukey,fn)         -> edges[vkey,uv,value]

  template <class G>
  concept _has_neighbors_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {neighbors(g, ukey)};
  };
  template <class G, class VVF>
  concept _has_neighbors_g_ukey_evf_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
    {neighbors(g, ukey, vvf)};
  };

  TAG_INVOKE_DEF(sourced_neighbors); // sourced_neighbors(g,ukey)    -> edges[ukey,vkey,uv]
                                     // sourced_neighbors(g,ukey,fn) -> edges[ukey,vkey,uv,value]

  template <class G>
  concept _has_sourced_neighbors_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {sourced_neighbors(g, ukey)};
  };
  template <class G, class VVF>
  concept _has_sourced_neighbors_g_ukey_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
    {sourced_neighbors(g, ukey, vvf)};
  };

} // namespace access

//
// neighbors(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_neighbors_g_ukey_adl<G>)
    return access::neighbors(g, ukey);
  else {
    using iterator_type = neighbor_iterator<G, false, void>;
    return neighbors_view<G, false, void>(iterator_type(g, ukey), ranges::end(edges(g, ukey)));
  }
}


//
// neighbors(g,ukey,vvf)
//
template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
  if constexpr (access::_has_neighbors_g_ukey_evf_adl<G, VVF>)
    return access::neighbors(g, ukey, vvf);
  else {
    using iterator_type = neighbor_iterator<G, false, VVF>;
    return neighbors_view<G, false, VVF>(iterator_type(g, ukey, vvf), ranges::end(edges(g, ukey)));
  }
}

//
// sourced_neighbors(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_sourced_neighbors_g_ukey_adl<G>)
    return access::sourced_neighbors(g, ukey);
  else {
    using iterator_type = neighbor_iterator<G, true, void>;
    return neighbors_view<G, true, void>(iterator_type(g, ukey), ranges::end(edges(g, ukey)));
  }
}


//
// sourced_neighbors(g,ukey,vvf)
//
template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
  if constexpr (access::_has_sourced_neighbors_g_ukey_evf_adl<G, VVF>)
    return access::sourced_neighbors(g, ukey, vvf);
  else {
    using iterator_type = neighbor_iterator<G, true, VVF>;
    return neighbors_view<G, true, VVF>(iterator_type(g, ukey, vvf), ranges::end(edges(g, ukey)));
  }
}


} // namespace std::graph::views
