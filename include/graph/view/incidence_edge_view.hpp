#pragma once
#include "graph/graph.hpp"

//
// edges_view(g,u):
//
// enable: for([vkey, uv] : edges_view(g,u)
//
// range returned is an input_range, which is a requirement of subrange.
// forward_range would also be reasonable if subrange allowed it.
//
namespace std::graph {

template <typename G>
class const_vertex_edge_view_iterator;
template <typename G>
class vertex_edge_view_iterator;


template <typename G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(const G& g, vertex_reference_t<const G> u) {
  using iter_type     = const_vertex_edge_view_iterator<const G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(const_cast<G&>(g))));
  auto last  = sentinal_type(ranges::end(vertices(const_cast<G&>(g))));
  return SR(first, last);
}


template <typename G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(G& g, vertex_reference_t<G> u) {
  using iter_type     = vertex_edge_view_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(g)));
  auto last  = sentinal_type(ranges::end(vertices(g)));
  return SR(first, last);
}


template <typename G>
class const_vertex_edge_view_iterator {
public:
  using graph_type = G;

  using vertex_key_type   = vertex_key_t<G>;
  using vertex_value_type = vertex_value_t<G>;

  using edge_range    = vertex_edge_range_t<remove_cvref_t<G>>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = input_iterator_tag; // input_iterator to allow sentinal
  using value_type        = pair<const vertex_key_type, const edge_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  using shadow_value_type = pair<vertex_key_type, edge_type*>;

public:
  const_vertex_edge_view_iterator(const graph_type& g, edge_iterator iter) : g_(&g), iter_(iter) {}
  const_vertex_edge_view_iterator(const graph_type& g, const vertex_value_type& u)
        : const_vertex_edge_view_iterator(
                g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_value_type&>(u)))) {}

  constexpr const_vertex_edge_view_iterator()                                       = default;
  constexpr const_vertex_edge_view_iterator(const const_vertex_edge_view_iterator&) = default;
  constexpr const_vertex_edge_view_iterator(const_vertex_edge_view_iterator&&)      = default;
  constexpr ~const_vertex_edge_view_iterator()                                      = default;

  constexpr const_vertex_edge_view_iterator& operator=(const const_vertex_edge_view_iterator&) = default;
  constexpr const_vertex_edge_view_iterator& operator=(const_vertex_edge_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_        = edge_type{target_key(*g_, *iter_), &*iter_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_vertex_edge_view_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr const_vertex_edge_view_iterator operator++(int) const {
    const_vertex_edge_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_vertex_edge_view_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertex_edge_view_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  graph_type*               g_     = nullptr;
  edge_iterator             iter_;

  friend bool operator==(const edge_iterator& lhs, const const_vertex_edge_view_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};

template <typename G>
class vertex_edge_view_iterator : public const_vertex_edge_view_iterator<G> {
public:
  using base_type = const_vertex_edge_view_iterator<G>;

  using graph_type = G;

  using vertex_key_type   = vertex_key_t<G>;
  using vertex_value_type = vertex_value_t<G>;

  using edge_range    = vertex_edge_range_t<remove_cvref_t<G>>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = input_iterator_tag; // input_iterator to allow sentinal
  using value_type        = pair<const vertex_key_type, edge_type&>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = value_type*;
  using reference         = value_type&;
  using const_reference   = value_type&;

protected:
  using shadow_value_type = pair<vertex_key_type, edge_type*>;
  using base_type::value_;
  using base_type::g_;
  using base_type::iter_;

public:
  vertex_edge_view_iterator(graph_type& g, edge_iterator iter) : base_type(g, iter) {}
  vertex_edge_view_iterator(graph_type& g, vertex_value_type& u) : base_type(g, u) {}

  constexpr vertex_edge_view_iterator()                                 = default;
  constexpr vertex_edge_view_iterator(const vertex_edge_view_iterator&) = default;
  constexpr vertex_edge_view_iterator(vertex_edge_view_iterator&&)      = default;
  constexpr ~vertex_edge_view_iterator()                                = default;

  constexpr vertex_edge_view_iterator& operator=(const vertex_edge_view_iterator&) = default;
  constexpr vertex_edge_view_iterator& operator=(vertex_edge_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = edge_type{target_key(*g_, *iter_), &*iter_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertex_edge_view_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr vertex_edge_view_iterator operator++(int) const {
    vertex_edge_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertex_edge_view_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertex_edge_view_iterator& rhs) const { return iter_ == rhs; }

protected:
  friend bool operator==(const edge_iterator& lhs, const vertex_edge_view_iterator& rhs) { return lhs == rhs.iter_; }
};

} // namespace std::graph
