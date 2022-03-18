#pragma once
#include "graph/graph.hpp"

//
// edgelist(g,u) -> edge_view<VKey,Sourced,E,EV>:
//
// enable: for([ukey, vkey, uv]        : edgelist(g))
//         for([ukey, vkey, uv]        : edgelist(g,fn))
//         for([ukey, vkey, uv, value] : edgelist(g,ukey,vkey))
//         for([ukey, vkey, uv, value] : edgelist(g,ukey,vkey,fn))
//
namespace std::graph::views {


template <incidence_graph G, class EVF = void>
class edgelist_iterator;


template <incidence_graph G>
class edgelist_iterator_base {
  using vertex_iterator = vertex_iterator_t<G>;
  using edge_iterator   = vertex_edge_iterator_t<G>;

protected:
  /// <summary>
  /// If the current vertex is non-empty then uvi is set to begin(edges(g,*ui)).
  /// Otherwise, skip past vertices until we find one with edges, and set uvi to the first edge.
  /// If no vertices with edges are found, ui = end(vertices(g)).
  /// </summary>
  /// <param name="g">graph</param>
  /// <param name="ui">Current vertex</param>
  /// <param name="uvi">Current edge</param>
  constexpr void find_non_empty_vertex(G& g, vertex_iterator& ui, edge_iterator& uvi) noexcept {
    for (; ui != ranges::end(vertices(g)); ++ui) {
      if (!ranges::empty(edges(g, *ui))) {
        uvi = ranges::begin(edges(g, *ui));
        return;
      }
    }
  }

  /// <summary>
  /// Find the next edge. Assumes current vertex & edge iterators point to valid
  /// objects.
  /// </summary>
  /// <param name="g">Graph</param>
  /// <param name="ui">Current vertex</param>
  /// <param name="uvi">Current edge</param>
  constexpr void find_next_edge(G& g, vertex_iterator& ui, edge_iterator& uvi) noexcept {
    assert(ui != ranges::end(vertices(g)));
    assert(uvi != ranges::end(edges(g, *ui)));
    if (++uvi != ranges::end(edges(g, *ui)))
      return;
    ++ui;
    find_non_empty_vertex(g, ui, uvi);
  }
};


/// <summary>
/// Iterator for an edgelist range of edges for a vertex.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <typeparam name="EVF">Edge Value Function</typeparam>
template <incidence_graph G, class EVF>
class edgelist_iterator : public edgelist_iterator_base<G> {
public:
  using base_type = edgelist_iterator_base<G>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<G>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, true, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  edgelist_iterator(graph_type& g, vertex_iterator ui, const EVF& value_fn)
        : base_type(), g_(g), ui_(ui), uvi_(), value_fn_(&value_fn) {}
  edgelist_iterator(graph_type& g, const EVF& value_fn) : edgelist_iterator(g, ranges::begin(vertices(g)), value_fn) {
    this->find_non_empty_vertex(g_, ui_, uvi_);
  }

  constexpr edgelist_iterator()                         = default;
  constexpr edgelist_iterator(const edgelist_iterator&) = default;
  constexpr edgelist_iterator(edgelist_iterator&&)      = default;
  constexpr ~edgelist_iterator()                        = default;

  constexpr edgelist_iterator& operator=(const edgelist_iterator&) = default;
  constexpr edgelist_iterator& operator=(edgelist_iterator&&) = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type  = remove_reference_t<edge_reference_type>;
  using shadow_value_type = edge_view<vertex_key_type, true, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

public:
  constexpr reference operator*() const {
    value_ = {vertex_key(g_, ui_), target_key(g_, *uvi_), &*uvi_, invoke(*value_fn_, *uvi_)};
    return reinterpret_cast<reference>(value_);
  }

  constexpr edgelist_iterator& operator++() {
    this->find_next_edge(g_, ui_, uvi_);
    return *this;
  }
  constexpr edgelist_iterator operator++(int) const {
    edgelist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs.uvi_; }
  //constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  vertex_iterator                  ui_;
  edge_iterator                    uvi_;
  const EVF*                       value_fn_ = nullptr;

  friend bool operator==(const vertex_iterator& lhs, const edgelist_iterator& rhs) { return lhs == rhs.ui_; }
};


template <incidence_graph G>
class edgelist_iterator<G, void> : public edgelist_iterator_base<G> {
public:
  using base_type = edgelist_iterator_base<G>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<G>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, true, edge_reference_type, edge_value_type>;
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
  using shadow_value_type = edge_view<vertex_key_type, true, shadow_edge_type*, edge_value_type>;

public:
  edgelist_iterator(graph_type& g, vertex_iterator ui) : base_type(), g_(g), ui_(ui), uvi_() {
    this->find_non_empty_vertex(g_, ui_, uvi_);
  }
  edgelist_iterator(graph_type& g) : edgelist_iterator(g, ranges::begin(vertices(g))) {}

  constexpr edgelist_iterator()                         = default;
  constexpr edgelist_iterator(const edgelist_iterator&) = default;
  constexpr edgelist_iterator(edgelist_iterator&&)      = default;
  constexpr ~edgelist_iterator()                        = default;

  constexpr edgelist_iterator& operator=(const edgelist_iterator&) = default;
  constexpr edgelist_iterator& operator=(edgelist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = {vertex_key(g_, ui_), target_key(g_, *uvi_), &*uvi_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr edgelist_iterator& operator++() {
    this->find_next_edge(g_, ui_, uvi_);
    return *this;
  }
  constexpr edgelist_iterator operator++(int) const {
    edgelist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs.uvi_; }
  //constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  vertex_iterator                  ui_;
  edge_iterator                    uvi_;

  friend bool operator==(const vertex_iterator& lhs, const edgelist_iterator& rhs) { return lhs == rhs.ui_; }
};


template <class G, class EVF>
using edgelist_view = ranges::subrange<edgelist_iterator<G, EVF>, vertex_iterator_t<G>>;

namespace access {
  // ranges
  TAG_INVOKE_DEF(edgelist); // edgelist(g)                 -> edges[ukey,vkey,uv]
                            // edgelist(g,fn)              -> edges[ukey,vkey,uv,value]
                            // edgelist(g,ukey,vkey)       -> edges[ukey,vkey,uv]
                            // edgelist(g,ukey,vkey,fn)    -> edges[ukey,vkey,uv,value]

  template <class G>
  concept _has_edgelist_g_adl = incidence_graph<G> && requires(G&& g) {
    {edgelist(g)};
  };
  template <class G>
  concept _has_edgelist_g_ukey_adl = incidence_graph<G> && requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
    {edgelist(g, ukey, vkey)};
  };
  template <class G, class EVF>
  concept _has_edgelist_g_evf_adl = incidence_graph<G> && requires(G&& g, const EVF& evf) {
    {edgelist(g, evf)};
  };
  template <class G, class EVF>
  concept _has_edgelist_g_ukey_evf_adl = incidence_graph<G> &&
        requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, const EVF& evf) {
    {edgelist(g, ukey, vkey, evf)};
  };

} // namespace access

//
// edgelist(g)
// edgelist(g,ukey,vkey)
//
template <incidence_graph G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edgelist(G&& g) {
  if constexpr (access::_has_edgelist_g_adl<G>) {
    return access::edgelist(g);
  } else {
    using iterator_type = edgelist_iterator<G, void>;
    return edgelist_view<G, void>(iterator_type(g), ranges::end(vertices(g)));
  }
}

template <incidence_graph G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edgelist(G&& g, vertex_key_t<G> first, vertex_key_t<G> last) {
  assert(first <= last && static_cast<size_t>(last) <= ranges::size(vertices(g)));
  if constexpr (access::_has_edgelist_g_ukey_adl<G>)
    return access::edgelist(g, first, last);
  else {
    using iterator_type = edgelist_iterator<G, void>;
    return edgelist_view<G, void>(iterator_type(g, find_vertex(g, first)), find_vertex(g, last));
  }
}


//
// edgelist(g,u,evf)
// edgelist(g,ukey,evf)
//
template <incidence_graph G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edgelist(G&& g, const EVF& evf) {
  if constexpr (access::_has_edgelist_g_evf_adl<G, EVF>) {
    return access::edgelist(g, evf);
  } else {
    using iterator_type = edgelist_iterator<G, EVF>;
    return edgelist_view<G, EVF>(iterator_type(g, evf), ranges::end(vertices(g)));
  }
}

template <incidence_graph G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edgelist(G&& g, vertex_key_t<G> first, vertex_key_t<G> last, const EVF& evf) {
  assert(first <= last && static_cast<size_t>(last) <= ranges::size(vertices(g)));
  if constexpr (access::_has_edgelist_g_ukey_evf_adl<G, EVF>)
    return access::edgelist(g, first, last, evf);
  else {
    using iterator_type = edgelist_iterator<G, EVF>;
    return edgelist_view<G, void>(iterator_type(g, find_vertex(g, first)), find_vertex(g, last), evf);
  }
}


} // namespace std::graph::views
