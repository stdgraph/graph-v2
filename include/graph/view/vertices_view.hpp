#pragma once
#include "graph/graph.hpp"

//
// vertices_view(g) -> pair<vertex_key_t<G>, vertex_reference_t<G>>:
//
// enable: for(auto&& [ukey, u] : vertices_view(g))
//
namespace std::graph::view {

template <class G>
class const_vertices_view_iterator;
template <class G>
class vertices_view_iterator;


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertices_view(const G& g) {
  using iter_type     = const_vertices_view_iterator<const G>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(const_cast<G&>(g))));
  auto last  = sentinal_type(ranges::end(vertices(const_cast<G&>(g))));
  return SR(first, last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertices_view(G& g) {
  using iter_type     = vertices_view_iterator<G>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(g)));
  auto last  = sentinal_type(ranges::end(vertices(g)));
  return SR(first, last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertices_view(const G& g, vertex_iterator_t<const G> first, vertex_iterator_t<const G> last) {
  using iter_type          = const_vertices_view_iterator<const G>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using SR                 = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return SR(iter_type(first, start_at), last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertices_view(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last) {
  using iter_type          = const_vertices_view_iterator<G>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using SR                 = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return SR(iter_type(first, start_at), last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto
vertices_view(const G& g, vertex_iterator_t<const G> first, vertex_iterator_t<const G> last, vertex_key_t<G> start_at) {
  using iter_type     = const_vertices_view_iterator<const G>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;
  return SR(iter_type(first, start_at), last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertices_view(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, vertex_key_t<G> start_at) {
  using iter_type     = const_vertices_view_iterator<G>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;
  return SR(iter_type(first, start_at), last);
}


template <class G>
class const_vertices_view_iterator {
public:
  using graph_type            = G;
  using vertex_key_type       = vertex_key_t<G>;
  using vertex_range_type     = vertex_range_t<remove_cvref_t<G>>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = ranges::range_value_t<vertex_range_type>;
  using vertex_reference_type = vertex_type&; // for internal use
  using vertex_pointer_type   = vertex_type*;

  using iterator_category = forward_iterator_tag;
  using value_type        = pair<const vertex_key_type, const vertex_type&>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  using shadow_value_type = pair<vertex_key_type, vertex_pointer_type>;

public:
  const_vertices_view_iterator(const graph_type& g) : iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  const_vertices_view_iterator(vertex_iterator_type iter, vertex_key_type start_at = 0)
        : value_(start_at, nullptr), iter_(iter) {}

  constexpr const_vertices_view_iterator()                                    = default;
  constexpr const_vertices_view_iterator(const const_vertices_view_iterator&) = default;
  constexpr const_vertices_view_iterator(const_vertices_view_iterator&&)      = default;
  constexpr ~const_vertices_view_iterator()                                   = default;

  constexpr const_vertices_view_iterator& operator=(const const_vertices_view_iterator&) = default;
  constexpr const_vertices_view_iterator& operator=(const_vertices_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.second = &*iter_;
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_vertices_view_iterator& operator++() {
    ++iter_;
    ++value_.first;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr const_vertices_view_iterator operator++(int) const {
    const_vertices_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_vertices_view_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertices_view_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const const_vertices_view_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};

template <class G>
class vertices_view_iterator : public const_vertices_view_iterator<G> {
public:
  using base_type             = const_vertices_view_iterator<G>;
  using graph_type            = G;
  using vertex_key_type       = vertex_key_t<G>;
  using vertex_range_type     = vertex_range_t<remove_cvref_t<G>>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = ranges::range_value_t<vertex_range_type>;
  using vertex_reference_type = vertex_type&; // for internal use
  using vertex_pointer_type   = vertex_type*;

  using iterator_category = forward_iterator_tag;
  using value_type        = pair<const vertex_key_type, vertex_type&>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = value_type*;
  using reference         = value_type&;
  using const_reference   = value_type&;

protected:
  using shadow_value_type = pair<vertex_key_type, vertex_pointer_type>;
  using base_type::value_;
  using base_type::iter_;

public:
  vertices_view_iterator(graph_type& g) : base_type(g) {}
  vertices_view_iterator(vertex_iterator_type iter, vertex_key_type start_at = 0) : base_type(iter, start_at) {}

  constexpr vertices_view_iterator()                              = default;
  constexpr vertices_view_iterator(const vertices_view_iterator&) = default;
  constexpr vertices_view_iterator(vertices_view_iterator&&)      = default;
  constexpr ~vertices_view_iterator()                             = default;

  constexpr vertices_view_iterator& operator=(const vertices_view_iterator&) = default;
  constexpr vertices_view_iterator& operator=(vertices_view_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.second = &*iter_;
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertices_view_iterator& operator++() {
    ++iter_;
    ++value_.first;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr vertices_view_iterator operator++(int) const {
    vertices_view_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertices_view_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertices_view_iterator& rhs) const { return iter_ == rhs; }

protected:
  friend bool operator==(const vertex_iterator_type& lhs, const vertices_view_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};

} // namespace std::graph::view
