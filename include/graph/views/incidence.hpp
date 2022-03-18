#pragma once
#include "graph/graph.hpp"
#include "views_utility.hpp"

//
// incidence(g,u) -> edge_view<VKey,Sourced,E,EV>:
//
// enable: for([vkey, uv]        : incidence(g,ukey))
//         for([vkey, uv, value] : incidence(g,ukey,fn))
//
//         for([ukey, vkey, uv]        : sourced_incidence(g,ukey))
//         for([ukey, vkey, uv, value] : sourced_incidence(g,ukey,fn))
//
namespace std::graph::views {


template <incidence_graph G, bool Sourced = false, class EVF = void>
class incidence_iterator;

/// <summary>
/// Iterator for an incidence range of edges for a vertex.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <typeparam name="EVF">Edge Value Function</typeparam>
template <incidence_graph G, bool Sourced, class EVF>
class incidence_iterator
      : source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  incidence_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter, const EVF& value_fn)
        : base_type(vertex_key(g, ui)), g_(g), iter_(iter), value_fn_(&value_fn) {}
  incidence_iterator(graph_type& g, vertex_key_type ukey, const EVF& value_fn)
        : base_type(ukey), g_(g), iter_(ranges::begin(edges(g, ukey))), value_fn_(&value_fn) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type = remove_reference_t<edge_reference_type>;
  using shadow_value_type =
        edge_view<vertex_key_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

public:
  constexpr reference operator*() const {
    if constexpr (undirected_incidence_graph<G>) {
      if (target_key(g_, *iter_) != this->source_vertex_key()) {
        value_.source_key = source_key(g_, *iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = target_key(g_, *iter_);
        value_.target_key = source_key(g_, *iter_);
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_incidence_graph<G>) {
        value_.source_key = source_key(g_, *iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = this->source_vertex_key();
        value_.target_key = target_key(g_, *iter_);
      }
    } else {
      value_.target_key = target_key(g_, *iter_);
    }
    value_.edge  = &*iter_;
    value_.value = invoke(*value_fn_, *iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;
  const EVF*                       value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};


template <incidence_graph G, bool Sourced>
class incidence_iterator<G, Sourced, void>
      : public source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)> {
public:
  using base_type = source_vertex<G, ((Sourced && !sourced_incidence_graph<G>) || undirected_incidence_graph<G>)>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type  = remove_reference_t<edge_reference_type>;
  using shadow_value_type = edge_view<vertex_key_type, Sourced, shadow_edge_type*, edge_value_type>;

public:
  incidence_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter)
        : base_type(vertex_key(g, ui)), g_(g), iter_(iter) {}
  incidence_iterator(graph_type& g, vertex_key_type ukey)
        : base_type(ukey), g_(g), iter_(ranges::begin(edges(g, ukey))) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    if constexpr (undirected_incidence_graph<G>) {
      static_assert(sourced_incidence_graph<G>);
      if (target_key(g_, *iter_) != this->source_vertex_key()) {
        value_.source_key = source_key(g_.*iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = target_key(g_.*iter_);
        value_.target_key = source_key(g_, *iter_);
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_incidence_graph<G>) {
        value_.source_key = source_key(g_, *iter_);
        value_.target_key = target_key(g_, *iter_);
      } else {
        value_.source_key = this->source_vertex_key();
        value_.target_key = target_key(g_, *iter_);
      }
    } else {
      value_.target_key = target_key(g_, *iter_);
    }
    value_.edge = &*iter_;
    return reinterpret_cast<reference>(value_);
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};


template <class G, bool Sourced, class EVF>
using incidence_view = ranges::subrange<incidence_iterator<G, Sourced, EVF>, vertex_edge_iterator_t<G>>;

namespace access {
  // ranges
  TAG_INVOKE_DEF(incidence); // incidence(g,ukey)            -> edges[vkey,v]
                             // incidence(g,ukey,fn)         -> edges[vkey,v,value]

  template <class G>
  concept _has_incidence_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {incidence(g, ukey)};
  };
  template <class G, class EVF>
  concept _has_incidence_g_ukey_evf_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
    {incidence(g, ukey, evf)};
  };

  TAG_INVOKE_DEF(sourced_incidence); // sourced_incidence(g,ukey)    -> edges[ukey,vkey,v]
                                     // sourced_incidence(g,ukey,fn) -> edges[ukey,vkey,v,value]

  template <class G>
  concept _has_sourced_incidence_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {sourced_incidence(g, ukey)};
  };
  template <class G, class EVF>
  concept _has_sourced_incidence_g_ukey_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
    {sourced_incidence(g, ukey, evf)};
  };

} // namespace access

//
// incidence(g,u)
// incidence(g,ukey)
//
template <incidence_graph G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_incidence_g_ukey_adl<G>)
    return access::incidence(g, ukey);
  else
    return incidence_view<G, false, void>(incidence_iterator<G, false, void>(g, ukey), ranges::end(edges(g, ukey)));
}


//
// incidence(g,ukey,evf)
//
template <incidence_graph G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
  if constexpr (access::_has_incidence_g_ukey_evf_adl<G, EVF>)
    return access::incidence(g, ukey, evf);
  else
    return incidence_view<G, false, EVF>(incidence_iterator<G, false, EVF>(g, ukey, evf), ranges::end(edges(g, ukey)));
}


//
// sourced_incidence(g,u)
// sourced_incidence(g,ukey)
//
template <incidence_graph G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_sourced_incidence_g_ukey_adl<G>)
    return access::sourced_incidence(g, ukey);
  else
    return incidence_view<G, true, void>(incidence_iterator<G, true, void>(g, ukey), ranges::end(edges(g, ukey)));
}


//
// sourced_incidence(g,ukey,evf)
//
template <incidence_graph G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
  if constexpr (access::_has_sourced_incidence_g_ukey_evf_adl<G, EVF>)
    return access::sourced_incidence(g, ukey, evf);
  else
    return incidence_view<G, true, EVF>(incidence_iterator<G, true, EVF>(g, ukey, evf), ranges::end(edges(g, ukey)));
}


} // namespace std::graph::views
