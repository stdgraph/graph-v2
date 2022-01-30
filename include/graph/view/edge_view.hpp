#pragma once
#include "graph/graph.hpp"
#include <tuple>

//
// edges_view(g):
//
// enable: for([ukey, vkey, uv] : edges_view(g)
//
namespace std::graph {

template <typename G>
class const_edge_view_iterator;
template <typename G>
class edge_view_iterator;


template <typename G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(const G& g) {
  using vertex_type   = remove_cvref_t<decltype(u)>;
  using iter_type     = const_edge_view_iterator<const G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  auto last  = sentinal_type(ranges::end(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  return SR(first, last);
}


template <typename G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(G& g) {
  using iter_type     = edge_view_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)));
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}


template <typename G>
class const_edge_view_iterator {
public:
  using graph_type = remove_cvref_t<G>;

  using vertex_range    = vertex_range_t<graph_type>;
  using vertex_iterator = ranges::iterator_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_type     = vertex_t<graph_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = tuple<const vertex_key_type, const vertex_key_type, const edge_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  using shadow_value_type = pair<vertex_key_type, vertex_key_type, edge_type*>;

public:
  const_edge_view_iterator(const graph_type& g, vertex_iterator vtx_iter, edge_iterator edg_iter)
        : g_(&const_cast<graph_type&>(g)), vtx_iter_(vtx_iter), edg_iter_(edg_iter) {
    find_first_non_empty();
  }
  const_edge_view_iterator(const graph_type& g, vertex_iterator vtx_iter)
        : const_edge_view_iterator(
                g, vtx_iter, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(*vtx_iter)))) {}

  constexpr const_edge_view_iterator()                                = default;
  constexpr const_edge_view_iterator(const const_edge_view_iterator&) = default;
  constexpr const_edge_view_iterator(const_edge_view_iterator&&)      = default;
  constexpr ~const_edge_view_iterator()                               = default;

  constexpr const_edge_view_iterator& operator=(const const_edge_view_iterator&) = default;
  constexpr const_edge_view_iterator& operator=(const_edge_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{vertex_key(g,vtx_iter_), target_key(*g_, *edg_iter_), &*edg_iter_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_edge_view_iterator& operator++() {
    next();
    return *this;
  }
  constexpr const_edge_view_iterator operator++(int) const {
    const_edge_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_edge_view_iterator& rhs) const { return vtx_iter_ == rhs.vtx_iter_ && edg_iter_ == rhs.edg_iter_; }
  //constexpr bool operator==(const edge_view_iterator& rhs) const { return edg_iter_ == rhs; }
protected:
  void find_first_non_empty() {
    // skip past any vertices with no incidence edges
    for (; vtx_iter != ranges::end(vertices(g)); ++vtx_iter) {
      if (!ranges::empty(edges(g, *vtx_iter))) {
        edg_iter_ = ranges::begin(edges(g, *vtx_iter));
        return;
      }
    }
  }
  void next() {
    assert(vtx_iter_ != ranges::end(vertices(g)));
    assert(edg_iter_ != ranges::end(edges(g_, *vtx_iter)));
    if (++edg_iter != ranges::end(edges(g_, *vtx_iter)))
      return;
    ++vtx_iter;
    find_first_non_empty();
  }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), vertex_key_type(), nullptr);
  graph_type*               g_     = nullptr;
  vertex_iterator           vtx_iter_;
  edge_iterator             edg_iter_;

  //friend bool operator==(const edge_iterator& lhs, const const_edge_view_iterator& rhs) { return lhs == rhs.edg_iter_; }
};

template <typename G>
class edge_view_iterator : public const_edge_view_iterator<G> {
public:
  using base_type = const_edge_view_iterator<G>;

  using graph_type = G;

  using vertex_range    = vertex_range_t<G>;
  using vertex_iterator = ranges::iterator_t<G>;
  using vertex_key_type = vertex_key_t<G>;
  using vertex_type     = vertex_t<G>;

  using edge_range    = vertex_edge_range_t<remove_cvref_t<G>>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = tuple<const vertex_key_type, const vertex_key_type, edge_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = value_type*;
  using reference         = value_type&;
  using const_reference   = value_type&;

protected:
  using shadow_value_type = tuple<vertex_key_type, vertex_key_type, edge_type*>;
  using base_type::value_;
  using base_type::g_;
  using base_type::vtx_iter_;
  using base_type::edg_iter_;
  using base_type::next;

public:
  edge_view_iterator(graph_type& g, vertex_iterator vtx_iter, edge_iterator edg_iter) : base_type(g, vtx_iter, edg_iter) {}
  edge_view_iterator(graph_type& g, itertex_iterator vtx_itr) : base_type(g, vtx_iter) {}

  constexpr edge_view_iterator()                          = default;
  constexpr edge_view_iterator(const edge_view_iterator&) = default;
  constexpr edge_view_iterator(edge_view_iterator&&)      = default;
  constexpr ~edge_view_iterator()                         = default;

  constexpr edge_view_iterator& operator=(const edge_view_iterator&) = default;
  constexpr edge_view_iterator& operator=(edge_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{target_key(*g_, *edg_iter_), &*edg_iter_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr edge_view_iterator& operator++() {
    next();
    return *this;
  }
  constexpr edge_view_iterator operator++(int) const {
    edge_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edge_view_iterator& rhs) const {
    return vtx_iter_ == rhs.vtx_iter_ && edg_iter_ == rhs.edg_iter_;
  }
  //constexpr bool operator==(const edge_view_iterator& rhs) const { return edg_iter_ == rhs; }

protected:
  //friend bool operator==(const edge_iterator& lhs, const edge_view_iterator& rhs) { return lhs == rhs.edg_iter_; }
};

} // namespace std::graph
