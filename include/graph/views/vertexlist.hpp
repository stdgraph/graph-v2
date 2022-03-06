#pragma once
#include "graph/graph.hpp"

//
// vertexlist(g) -> [key, vertex& [,value]]
//
// enable: for(auto&& [ukey, u]      : vertexlist(g))
//    and: for(auto&& [ukey, u, val] : vertexlist(g,vertex_value)
//
namespace std::graph::views {

template <class G, class VVF = void>
class vertexlist_iterator;


template <class G, class VVF>
class vertexlist_iterator_base : public input_iterator_tag {
public:
  using graph_type        = G;
  using vertex_type       = vertex_t<graph_type>;
  using vertex_value_func = VVF;
  using vertex_value_type = decltype(vertex_value_func(declval<vertex_type>()));

public:
  vertexlist_iterator_base(VVF value_fn) : value_fn_(value_fn) {}

  vertexlist_iterator_base()                                = default;
  vertexlist_iterator_base(vertexlist_iterator_base const&) = default;
  vertexlist_iterator_base(vertexlist_iterator_base&&)      = default;
  ~vertexlist_iterator_base()                               = default;

  vertexlist_iterator_base& operator=(const vertexlist_iterator_base&) = default;
  vertexlist_iterator_base& operator=(vertexlist_iterator_base&&) = default;

protected:
  vertex_value_func value_fn_;
};

template <class G>
class vertexlist_iterator_base<G, void> {
public:
  using graph_type        = G;
  using vertex_type       = vertex_t<graph_type>;
  using vertex_value_func = void;
  using vertex_value_type = void;
};


template <class G, class VVF>
class vertexlist_iterator : public vertexlist_iterator_base<G, VVF> {
public:
  using graph_type            = G;
  using base_type             = vertexlist_iterator_base<graph_type, VVF>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = ranges::range_reference_t<vertex_range_type>;
  using vertex_value_func     = typename base_type::vertex_value_type;
  using vertex_value_type     = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_view<const vertex_key_t<graph_type>, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type  = vertex_view<vertex_key_t<graph_type>, shadow_vertex_type*, vertex_value_type*>;

public:
  vertexlist_iterator(graph_type& g, VVF value_fn) 
        : base_type(value_fn), iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  vertexlist_iterator(VVF value_fn, vertex_iterator_type iter, vertex_key_type start_at = 0)
        : base_type(value_fn), value_{start_at, nullptr, nullptr}, iter_(iter) {}

  vertexlist_iterator(graph_type& g) : base_type(), iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  vertexlist_iterator(vertex_iterator_type iter,
                      vertex_key_type      start_at = 0) requires(is_void_v<VVF> || is_default_constructible_v<VVF>)
        : base_type(), value_{start_at, nullptr}, iter_(iter) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.value = this->value_fn_(*iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.key;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};

template <class G>
class vertexlist_iterator<G, void> : public vertexlist_iterator_base<G, void> {
public:
  using graph_type            = G;
  using base_type             = vertexlist_iterator_base<graph_type, void>;
  using vertex_key_type       = vertex_key_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = ranges::range_reference_t<vertex_range_type>;
  using vertex_value_func     = typename base_type::vertex_value_type;
  using vertex_value_type     = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_view<const vertex_key_t<graph_type>, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type  = vertex_view<vertex_key_t<graph_type>, shadow_vertex_type*, void>;

public:
  vertexlist_iterator(graph_type& g) : base_type(), iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  vertexlist_iterator(vertex_iterator_type iter, vertex_key_type start_at = 0)
        : base_type(), value_{start_at, nullptr}, iter_(iter) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.value = this->value_fn_(*iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.key;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ to get value_.vertex when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


//
// vertexlist(g [,proj])
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g) {
  using iter_type     = vertexlist_iterator<G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using rng           = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(g)));
  auto last  = sentinal_type(ranges::end(vertices(g)));
  return rng(move(first), move(last));
}

template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, VVF proj) {
  using iter_type     = vertexlist_iterator<G, VVF>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using rng           = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(g)), proj);
  auto last  = sentinal_type(ranges::end(vertices(g)));
  return rng(move(first), move(last));
}


//
// vertexlist(g, first, last [,proj])
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last) {
  using iter_type          = vertexlist_iterator<G, void>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using rng                = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return rng(iter_type(first, start_at), move(last));
}

template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, VVF proj) {
  using iter_type          = vertexlist_iterator<G, VVF>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using rng                = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return rng(iter_type(first, start_at), move(last), proj);
}


//
// vertexlist(g,first,last,start_at [,proj])
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, vertex_key_t<G> start_at) {
  using iter_type     = vertexlist_iterator<G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using rng           = ranges::subrange<iter_type, sentinal_type>;
  return rng(iter_type(first, start_at), last);
}

template <class G, class VVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto
vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, vertex_key_t<G> start_at, VVF proj) {
  using iter_type     = vertexlist_iterator<G, VVF>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using rng           = ranges::subrange<iter_type, sentinal_type>;
  return rng(iter_type(first, start_at), last, proj);
}


} // namespace std::graph::views
