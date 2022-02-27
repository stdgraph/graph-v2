#pragma once
#include "graph/graph.hpp"

//
// adjacency_view(g,u) -> pair<VKey,vertex_t<G>>:
//
// enable: for([vkey, v] : adjacency_view(g,u)
//
namespace std::graph::views {

template <class G>
class const_adjacency_iterator;
template <class G>
class adjacency_iterator;


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto adjacency_view(const G& g, vertex_reference_t<const G> u) {
  using vertex_type   = remove_cvref_t<decltype(u)>;
  using iter_type     = const_adjacency_iterator<const G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  auto last  = sentinal_type(ranges::end(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  return SR(first, last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto adjacency_view(G& g, vertex_reference_t<G> u) {
  using iter_type     = adjacency_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)));
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}


template <class G>
class const_adjacency_iterator {
public:
  using graph_type = remove_cvref_t<G>;

  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_type     = vertex_t<graph_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = pair<const vertex_key_type, const vertex_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  using shadow_value_type = pair<vertex_key_type, vertex_type*>;

public:
  const_adjacency_iterator(const graph_type& g, edge_iterator iter) : g_(&const_cast<graph_type&>(g)), iter_(iter) {}
  const_adjacency_iterator(const graph_type& g, const vertex_type& u)
        : const_adjacency_iterator(g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u)))) {}

  constexpr const_adjacency_iterator()                                = default;
  constexpr const_adjacency_iterator(const const_adjacency_iterator&) = default;
  constexpr const_adjacency_iterator(const_adjacency_iterator&&)      = default;
  constexpr ~const_adjacency_iterator()                               = default;

  constexpr const_adjacency_iterator& operator=(const const_adjacency_iterator&) = default;
  constexpr const_adjacency_iterator& operator=(const_adjacency_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{target_key(*g_, *iter_), &target(*g_, *iter_)};
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_adjacency_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr const_adjacency_iterator operator++(int) const {
    const_adjacency_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_adjacency_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const adjacency_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  graph_type*               g_     = nullptr;
  edge_iterator             iter_;

  friend bool operator==(const edge_iterator& lhs, const const_adjacency_iterator& rhs) { return lhs == rhs.iter_; }
};

template <class G>
class adjacency_iterator : public const_adjacency_iterator<G> {
public:
  using base_type = const_adjacency_iterator<G>;

  using graph_type = G;

  using vertex_key_type = vertex_key_t<G>;
  using vertex_type     = vertex_t<G>;

  using edge_range    = vertex_edge_range_t<remove_cvref_t<G>>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = pair<const vertex_key_type, vertex_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = value_type*;
  using reference         = value_type&;
  using const_reference   = value_type&;

protected:
  using shadow_value_type = pair<vertex_key_type, vertex_type*>;
  using base_type::value_;
  using base_type::g_;
  using base_type::iter_;

public:
  adjacency_iterator(graph_type& g, edge_iterator iter) : base_type(g, iter) {}
  adjacency_iterator(graph_type& g, vertex_type& u) : base_type(g, u) {}

  constexpr adjacency_iterator()                          = default;
  constexpr adjacency_iterator(const adjacency_iterator&) = default;
  constexpr adjacency_iterator(adjacency_iterator&&)      = default;
  constexpr ~adjacency_iterator()                         = default;

  constexpr adjacency_iterator& operator=(const adjacency_iterator&) = default;
  constexpr adjacency_iterator& operator=(adjacency_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{target_key(*g_, *iter_), &target(*g_, *iter_)};
    return reinterpret_cast<reference>(value_);
  }

  constexpr adjacency_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr adjacency_iterator operator++(int) const {
    adjacency_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const adjacency_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const adjacency_iterator& rhs) const { return iter_ == rhs; }

protected:
  friend bool operator==(const edge_iterator& lhs, const adjacency_iterator& rhs) { return lhs == rhs.iter_; }
};

} // namespace std::graph::views
