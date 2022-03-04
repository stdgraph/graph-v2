#pragma once
#include "graph/graph.hpp"

//
// vertexlist(g) -> vertex_iterator_view<vertex_key_t<G>, vertex_reference_t<G>>:
//
// enable: for(auto&& [ukey, u] : vertexlist(g))
//
namespace std::graph::views {


template <class G, class VVF>
class vertexlist_iterator_base {
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
class const_vertexlist_iterator : public vertexlist_iterator_base<G, VVF> {
public:
  using base_type            = vertexlist_iterator_base<G, VVF>;
  using graph_type           = remove_cvref_t<G>;
  using vertex_key_type      = vertex_key_t<G>;
  using vertex_range_type    = vertex_range_t<graph_type>;
  using vertex_iterator_type = ranges::iterator_t<vertex_range_type>;
  using vertex_type          = ranges::range_value_t<vertex_range_type>;
  using vertex_value_func    = typename base_type::vertex_value_type;
  using vertex_value_type    = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_view<vertex_key_t<const graph_type>, const vertex_type&, const vertex_value_type>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_value_type = vertex_view<vertex_key_t<graph_type>, vertex_type*, vertex_value_type*>;

public:
  const_vertexlist_iterator(const graph_type& g, VVF value_fn)
        : base_type(value_fn), iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  const_vertexlist_iterator(VVF value_fn, vertex_iterator_type iter, vertex_key_type start_at = 0)
        : base_type(value_fn), value_(start_at, nullptr), iter_(iter) {}

  constexpr const_vertexlist_iterator()                                 = default;
  constexpr const_vertexlist_iterator(const const_vertexlist_iterator&) = default;
  constexpr const_vertexlist_iterator(const_vertexlist_iterator&&)      = default;
  constexpr ~const_vertexlist_iterator()                                = default;

  constexpr const_vertexlist_iterator& operator=(const const_vertexlist_iterator&) = default;
  constexpr const_vertexlist_iterator& operator=(const_vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    value_.value  = this->value_fn_(*iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.key;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr const_vertexlist_iterator operator++(int) const {
    const_vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const const_vertexlist_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};

template <class G>
class const_vertexlist_iterator<G, void> : public vertexlist_iterator_base<G, void> {
public:
  using base_type            = vertexlist_iterator_base<G, void>;
  using graph_type           = remove_cvref_t<G>;
  using vertex_key_type      = vertex_key_t<G>;
  using vertex_range_type    = vertex_range_t<graph_type>;
  using vertex_iterator_type = ranges::iterator_t<vertex_range_type>;
  using vertex_type          = ranges::range_value_t<vertex_range_type>;
  using vertex_value_func    = typename base_type::vertex_value_type;
  using vertex_value_type    = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  //using value_type        = vertex_iterator_view<G, void>;
  using value_type      = vertex_view<vertex_key_t<const graph_type>, const vertex_type&, const vertex_value_type>;
  using difference_type = ranges::range_difference_t<vertex_range_type>;
  using pointer         = const value_type*;
  using const_pointer   = const value_type*;
  using reference       = const value_type&;
  using const_reference = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_value_type = vertex_view<vertex_key_t<graph_type>, vertex_type*, vertex_value_type*>;

public:
  const_vertexlist_iterator(const graph_type& g)
        : base_type(), iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  const_vertexlist_iterator(vertex_iterator_type iter, vertex_key_type start_at = 0)
        : base_type(), value_(start_at, nullptr, nullptr), iter_(iter) {}

  constexpr const_vertexlist_iterator()                                 = default;
  constexpr const_vertexlist_iterator(const const_vertexlist_iterator&) = default;
  constexpr const_vertexlist_iterator(const_vertexlist_iterator&&)      = default;
  constexpr ~const_vertexlist_iterator()                                = default;

  constexpr const_vertexlist_iterator& operator=(const const_vertexlist_iterator&) = default;
  constexpr const_vertexlist_iterator& operator=(const_vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    value_.value  = this->value_fn_(*iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.key;
    // leave value_.second (vertex) as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr const_vertexlist_iterator operator++(int) const {
    const_vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const const_vertexlist_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};


template <class G, class VVF>
class vertexlist_iterator : public const_vertexlist_iterator<G, VVF> {
public:
  using base_type            = const_vertexlist_iterator<G, VVF>;
  using graph_type           = remove_cvref_t<G>;
  using vertex_key_type      = vertex_key_t<G>;
  using vertex_range_type    = vertex_range_t<remove_cvref_t<G>>;
  using vertex_iterator_type = ranges::iterator_t<vertex_range_type>;
  using vertex_type          = ranges::range_value_t<vertex_range_type>;
  using vertex_value_func    = typename base_type::vertex_value_type;
  using vertex_value_type    = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  //using value_type        = vertex_iterator_view<G, void>;
  using value_type      = vertex_view<vertex_key_t<graph_type>, vertex_type&, vertex_value_type>;
  using difference_type = ranges::range_difference_t<vertex_range_type>;
  using pointer         = value_type*;
  using const_pointer   = value_type*;
  using reference       = value_type&;
  using const_reference = value_type&;

protected:
  //using shadow_value_type = _shadow_vertex_iterator_view<G, void>;
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_value_type = vertex_view<vertex_key_t<graph_type>, vertex_type*, vertex_value_type>;
  using base_type::value_;
  using base_type::iter_;

public:
  vertexlist_iterator(graph_type& g, VVF value_fn) : base_type(g, value_fn) {}
  vertexlist_iterator(VVF value_fn, vertex_iterator_type iter, vertex_key_type start_at = 0)
        : base_type(value_fn, iter, start_at) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
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
  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <class G>
class vertexlist_iterator<G, void> : public const_vertexlist_iterator<G, void> {
public:
  using base_type            = const_vertexlist_iterator<G, void>;
  using graph_type           = remove_cvref_t<G>;
  using vertex_key_type      = vertex_key_t<G>;
  using vertex_range_type    = vertex_range_t<remove_cvref_t<G>>;
  using vertex_iterator_type = ranges::iterator_t<vertex_range_type>;
  using vertex_type          = ranges::range_value_t<vertex_range_type>;
  using vertex_value_func    = typename base_type::vertex_value_type;
  using vertex_value_type    = typename base_type::vertex_value_type;

  using iterator_category = forward_iterator_tag;
  //using value_type        = vertex_iterator_view<G, void>;
  using value_type      = vertex_view<vertex_key_t<graph_type>, vertex_type&, vertex_value_type>;
  using difference_type = ranges::range_difference_t<vertex_range_type>;
  using pointer         = value_type*;
  using const_pointer   = value_type*;
  using reference       = value_type&;
  using const_reference = value_type&;

protected:
  //using shadow_value_type = _shadow_vertex_iterator_view<G, void>;
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_value_type = vertex_view<vertex_key_t<graph_type>, vertex_type*, vertex_value_type>;
  using base_type::value_;
  using base_type::iter_;

public:
  vertexlist_iterator(graph_type& g) : base_type(g) {}
  vertexlist_iterator(vertex_iterator_type iter, vertex_key_type start_at = 0) : base_type(iter, start_at) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
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
  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <class G, class VV = void>
class vertexlist_view {};


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(const G& g) {
  using iter_type     = const_vertexlist_iterator<const G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(const_cast<G&>(g))));
  auto last  = sentinal_type(ranges::end(vertices(const_cast<G&>(g))));
  return SR(first, last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g) {
  using iter_type     = vertexlist_iterator<G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(ranges::begin(vertices(g)));
  auto last  = sentinal_type(ranges::end(vertices(g)));
  return SR(first, last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(const G& g, vertex_iterator_t<const G> first, vertex_iterator_t<const G> last) {
  using iter_type          = const_vertexlist_iterator<const G, void>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using SR                 = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return SR(iter_type(first, start_at), last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last) {
  using iter_type          = const_vertexlist_iterator<G, void>;
  using sentinal_type      = typename iter_type::vertex_iterator_type;
  using SR                 = ranges::subrange<iter_type, sentinal_type>;
  vertex_key_t<G> start_at = static_cast<vertex_key_t<G>>(first - begin(vertices(g)));
  return SR(iter_type(first, start_at), last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto
vertexlist(const G& g, vertex_iterator_t<const G> first, vertex_iterator_t<const G> last, vertex_key_t<G> start_at) {
  using iter_type     = const_vertexlist_iterator<const G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;
  return SR(iter_type(first, start_at), last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto vertexlist(G& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, vertex_key_t<G> start_at) {
  using iter_type     = const_vertexlist_iterator<G, void>;
  using sentinal_type = typename iter_type::vertex_iterator_type;
  using SR            = ranges::subrange<iter_type, sentinal_type>;
  return SR(iter_type(first, start_at), last);
}


} // namespace std::graph::views
