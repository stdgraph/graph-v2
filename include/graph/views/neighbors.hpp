#pragma once
#include "graph/graph.hpp"

//
// neighbors(g,u) -> neighbor_view<VKey,Sourced,E,EV>:
//
// enable: for([vkey, uv]        : neighbors(g,u))
//         for([vkey, uv, value] : neighbors(g,u,fn))
//         for([ukey, vkey, uv]        : sourced_neighbors(g,u))
//         for([ukey, vkey, uv, value] : sourced_neighbors(g,u,fn))
//
namespace std::graph::views {


template <class G, bool Sourced = false, class VVF = void>
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class neighbor_iterator;


/// <summary>
/// Iterator for an neighbors range of edges for a vertex.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <typeparam name="VVF">Edge Value Function</typeparam>
template <class G, bool Sourced, class VVF>
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class neighbor_iterator {
public:
  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = invoke_result_t<VVF, vertex_reference_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;
  //using edge_reference_type = edge_reference_t<graph_type>;
  //using edge_value_type     = invoke_result_t<VVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_view<const vertex_key_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  neighbor_iterator(graph_type& g, edge_iterator iter, const VVF& value_fn)
        : g_(g), iter_(iter), value_fn_(&value_fn) {}
  neighbor_iterator(graph_type& g, vertex_reference_type u, const VVF& value_fn)
        : neighbor_iterator(g, ranges::begin(edges(g, u)), value_fn) {}

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
    // const in this functions signature causes the const version of target(), which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter
    shadow_vertex_type* v = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(g_, *iter_), target_key(g_, *iter_), v, invoke(*value_fn_, *v)};
    else
      value_ = {target_key(g_, *iter_), v, invoke(*value_fn_, *v)};
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
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class neighbor_iterator<G, Sourced, void> {
public:
  using graph_type            = G;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = void;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;
  //using edge_reference_type = edge_reference_t<graph_type>;
  //using edge_value_type     = void;

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
  neighbor_iterator(graph_type& g, edge_iterator iter) : g_(g), iter_(iter) {}
  neighbor_iterator(graph_type& g, vertex_reference_type u) : neighbor_iterator(g, ranges::begin(edges(g, u))) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&) = default;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes the const version of target(), which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter
    shadow_vertex_type* v = const_cast<shadow_vertex_type*>(&target(g_, *iter_));
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(g_, *iter_), target_key(g_, *iter_), v};
    else {
      value_ = {target_key(g_, *iter_), v};
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
  TAG_INVOKE_DEF(neighbors); // neighbors(g,u)               -> edges[vkey,uv]
                             // neighbors(g,ukey)            -> edges[vkey,uv]
                             // neighbors(g,u,   fn)         -> edges[vkey,uv,value]
                             // neighbors(g,ukey,fn)         -> edges[vkey,uv,value]

  template <class G>
  concept _has_neighbors_g_u_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u) {
    {neighbors(g, u)};
  };
  template <class G>
  concept _has_neighbors_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {neighbors(g, ukey)};
  };
  template <class G, class VVF>
  concept _has_neighbors_g_u_evf_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u, const VVF& vvf) {
    {neighbors(g, u, vvf)};
  };
  template <class G, class VVF>
  concept _has_neighbors_g_ukey_evf_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
    {neighbors(g, ukey, vvf)};
  };

  TAG_INVOKE_DEF(sourced_neighbors); // sourced_neighbors(g,u)       -> edges[ukey,vkey,uv]
                                     // sourced_neighbors(g,ukey)    -> edges[ukey,vkey,uv]
                                     // sourced_neighbors(g,u,   fn) -> edges[ukey,vkey,uv,value]
                                     // sourced_neighbors(g,ukey,fn) -> edges[ukey,vkey,uv,value]

  template <class G>
  concept _has_sourced_neighbors_g_u_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u) {
    {sourced_neighbors(g, u)};
  };
  template <class G>
  concept _has_sourced_neighbors_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {sourced_neighbors(g, ukey)};
  };
  template <class G, class VVF>
  concept _has_sourced_neighbors_g_u_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_reference_t<G> u, const VVF& vvf) {
    {sourced_neighbors(g, u, vvf)};
  };
  template <class G, class VVF>
  concept _has_sourced_neighbors_g_ukey_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
    {sourced_neighbors(g, ukey, vvf)};
  };

} // namespace access

//
// neighbors(g,u)
// neighbors(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_reference_t<G> u) {
  if constexpr (access::_has_neighbors_g_u_adl<G>) {
    return access::neighbors(g, u);
  } else {
    using iterator_type = neighbor_iterator<G, false, void>;
    return neighbors_view<G, false, void>(iterator_type(g, ranges::begin(edges(g, u))), ranges::end(edges(g, u)));
  }
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_neighbors_g_ukey_adl<G>)
    return access::neighbors(g, ukey);
  else
    return neighbors(g, *find_vertex(g, ukey));
}


//
// neighbors(g,u,vvf)
// neighbors(g,ukey,vvf)
//
template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_reference_t<G> u, const VVF& vvf) {
  if constexpr (access::_has_neighbors_g_u_evf_adl<G, VVF>) {
    return access::neighbors(g, u, vvf);
  } else {
    using iterator_type = neighbor_iterator<G, false, VVF>;
    return neighbors_view<G, false, VVF>(iterator_type(g, ranges::begin(edges(g, u)), vvf), ranges::end(edges(g, u)));
  }
}

template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto neighbors(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
  if constexpr (access::_has_neighbors_g_ukey_evf_adl<G, VVF>)
    return access::neighbors(g, ukey, vvf);
  else
    return neighbors(g, *find_vertex(g, ukey), vvf);
}


//
// sourced_neighbors(g,u)
// sourced_neighbors(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_reference_t<G> u) {
  if constexpr (access::_has_sourced_neighbors_g_u_adl<G>)
    return access::sourced_neighbors(g, u);
  else {
    using iterator_type = neighbor_iterator<G, true, void>;
    return neighbors_view<G, true, void>(iterator_type(g, ranges::begin(edges(g, u))), ranges::end(edges(g, u)));
  }
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_sourced_neighbors_g_ukey_adl<G>)
    return access::sourced_neighbors(g, ukey);
  else
    return sourced_neighbors(g, *find_vertex(g, ukey));
}


//
// sourced_neighbors(g,u,vvf)
// sourced_neighbors(g,ukey,vvf)
//
template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_reference_t<G> u, const VVF& vvf) {
  if constexpr (access::_has_sourced_neighbors_g_u_evf_adl<G, VVF>) {
    return access::sourced_neighbors(g, u, vvf);
  } else {
    using iterator_type = neighbor_iterator<G, true, VVF>;
    return neighbors_view<G, true, VVF>(iterator_type(g, ranges::begin(edges(g, u)), vvf), ranges::end(edges(g, u)));
  }
}

template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_neighbors(G&& g, vertex_key_t<G> ukey, const VVF& vvf) {
  if constexpr (access::_has_sourced_neighbors_g_ukey_evf_adl<G, VVF>)
    return access::sourced_neighbors(g, ukey, vvf);
  else
    return sourced_neighbors(g, *find_vertex(g, ukey), vvf);
}


} // namespace std::graph::views
