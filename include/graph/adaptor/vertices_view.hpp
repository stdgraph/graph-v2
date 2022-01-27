#pragma once
#include "graph/graph.hpp"

// for([ukey, u] : vertices_view(g)
//
namespace std::graph {

template <typename G>
class const_vertices_view_iterator {
public:
  using graph_type            = G;
  using vertex_key_type       = vertex_key_t<G>;
  using vertex_range_type     = vertex_range_t<remove_cvref_t<G>>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = ranges::range_value_t<vertex_range_type>;
  using vertex_reference_type = vertex_type&; // for internal use
  using vertex_pointer_type   = vertex_type*;

  using iterator_category = input_iterator_tag; // input_iterator to allow sentinal
  using value_type        = pair<const vertex_key_type, const vertex_type&>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  using shadow_value_type = pair<vertex_key_type, vertex_pointer_type>;

public:
  const_vertices_view_iterator(graph_type& g) : iter_(ranges::begin(vertices(g))) {}

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

  constexpr bool operator==(const const_vertices_view_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertices_view_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  vertex_iterator_type      iter_;
};

template <typename G>
auto vertices_view(const G& g) {
  using SR = ranges::subrange<vertex_iterator_t<const G>>;
  return SR(ranges::begin(vertices(g)), ranges::end(vertices(g)));
}


template <typename G>
auto vertices_view(G& g) {
  using SR = ranges::subrange<vertex_iterator_t<G>>;
  return SR(ranges::begin(vertices(g)), ranges::end(vertices(g)));
}

} // namespace std::graph
