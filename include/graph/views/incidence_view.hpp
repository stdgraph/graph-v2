#pragma once
#include "graph/graph.hpp"

//
// edges_view(g,u) -> pair<vertex_key_t<G>,vertex_edge_t<G>&>:
//
// enable: for([vkey, uv] : edges_view(g,u))
//
namespace std::graph::views {

template <class G>
class const_incidence_iterator;
template <class G>
class incidence_iterator;

template <class G>
class const_incidence_iterator;

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(const G& g, vertex_reference_t<const G> u) {
  using vertex_type   = remove_cvref_t<decltype(u)>;
  using iter_type     = const_incidence_iterator<const G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  auto last  = sentinal_type(ranges::end(edges(const_cast<G&>(g), const_cast<vertex_type&>(u))));
  return SR(first, last);
}


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(G& g, vertex_reference_t<G> u) {
  using iter_type     = incidence_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)));
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(const G& g, vertex_key_t<const G> ukey) { return edges_view(g, *find_vertex(g, ukey)); }

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto edges_view(G& g, vertex_key_t<G> ukey) { return edges_view(g, *find_vertex(g, ukey)); }


template <class G, class Projection>
class incidence_iterator_base {
public:
  using graph_type      = remove_cvref_t<G>;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using projection_fn   = Projection;
  using projection_type = decltype(Projection(declval<edge_type>()));

  using value_type = targeted_edge<const vertex_key_type, const edge_type&, projection_type>;

protected:
  using shadow_value_type = targeted_edge<vertex_key_type, edge_type*, projection_type>;

protected:
  incidence_iterator_base(const graph_type& g, edge_iterator iter, const projection_fn& projection)
        : g_(&const_cast<graph_type&>(g)), iter_(iter), projection_(projection) {}
  incidence_iterator_base(const graph_type& g, const vertex_type& u, const projection_fn& projection)
        : incidence_iterator_base(
                g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u))), projection) {}

  incidence_iterator_base()                                      = default;
  incidence_iterator_base(const incidence_iterator_base&) = default;
  incidence_iterator_base(incidence_iterator_base&&)      = default;
  ~incidence_iterator_base()                                     = default;

  incidence_iterator_base& operator=(const incidence_iterator_base&) = default;
  incidence_iterator_base& operator=(incidence_iterator_base&&) = default;

  void set_value() { value_ = shadow_value_type{target_key(*g_, *iter_), &*iter_, projection_(*iter_)}; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr, projection_type());
  graph_type*               g_     = nullptr;
  edge_iterator             iter_;
  projection_fn             projection_;
};

template <class G>
class incidence_iterator_base<G, void> {
public:
  using graph_type      = remove_cvref_t<G>;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using projection_type = void;

  using value_type = targeted_edge<const vertex_key_type, const edge_type&, projection_type>;

protected:
  using shadow_value_type = targeted_edge<vertex_key_type, edge_type*, void>;

protected:
  incidence_iterator_base(const graph_type& g, edge_iterator iter)
        : g_(&const_cast<graph_type&>(g)), iter_(iter) {}
  incidence_iterator_base(const graph_type& g, const vertex_type& u)
        : incidence_iterator_base(
                g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u)))) {}

  incidence_iterator_base()                                      = default;
  incidence_iterator_base(const incidence_iterator_base&) = default;
  incidence_iterator_base(incidence_iterator_base&&)      = default;
  ~incidence_iterator_base()                                     = default;

  incidence_iterator_base& operator=(const incidence_iterator_base&) = default;
  incidence_iterator_base& operator=(incidence_iterator_base&&) = default;

  void set_value() { value_ = shadow_value_type{target_key(*g_, *iter_), &*iter_}; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  graph_type*               g_     = nullptr;
  edge_iterator             iter_;
};

template <class G>
class const_incidence_iterator {
public:
  using graph_type = remove_cvref_t<G>;

  using vertex_key_type = vertex_key_t<graph_type>;
  using vertex_type     = vertex_t<graph_type>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = targeted_edge<const vertex_key_type, const edge_type&, void>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  using shadow_value_type = pair<vertex_key_type, edge_type*>;

public:
  const_incidence_iterator(const graph_type& g, edge_iterator iter)
        : g_(&const_cast<graph_type&>(g)), iter_(iter) {}
  const_incidence_iterator(const graph_type& g, const vertex_type& u)
        : const_incidence_iterator(
                g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u)))) {}

  constexpr const_incidence_iterator()                                       = default;
  constexpr const_incidence_iterator(const const_incidence_iterator&) = default;
  constexpr const_incidence_iterator(const_incidence_iterator&&)      = default;
  constexpr ~const_incidence_iterator()                                      = default;

  constexpr const_incidence_iterator& operator=(const const_incidence_iterator&) = default;
  constexpr const_incidence_iterator& operator=(const_incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{target_key(*g_, *iter_), &*iter_};
    return reinterpret_cast<reference>(value_);
  }

  constexpr const_incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr const_incidence_iterator operator++(int) const {
    const_incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const const_incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

protected:
  mutable shadow_value_type value_ = shadow_value_type(vertex_key_type(), nullptr);
  graph_type*               g_     = nullptr;
  edge_iterator             iter_;

  friend bool operator==(const edge_iterator& lhs, const const_incidence_iterator& rhs) {
    return lhs == rhs.iter_;
  }
};

template <class G>
class incidence_iterator : public const_incidence_iterator<G> {
public:
  using base_type = const_incidence_iterator<G>;

  using graph_type = G;

  using vertex_key_type = vertex_key_t<G>;
  using vertex_type     = vertex_t<G>;

  using edge_range    = vertex_edge_range_t<remove_cvref_t<G>>;
  using edge_iterator = ranges::iterator_t<edge_range>;
  using edge_type     = ranges::range_value_t<edge_range>;

  using iterator_category = forward_iterator_tag;
  using value_type        = targeted_edge<const vertex_key_type, edge_type&, void>;
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
  incidence_iterator(graph_type& g, edge_iterator iter) : base_type(g, iter) {}
  incidence_iterator(graph_type& g, vertex_type& u) : base_type(g, u) {}

  constexpr incidence_iterator()                                 = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                                = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    value_ = shadow_value_type{target_key(*g_, *iter_), &*iter_};
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

protected:
  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};

} // namespace std::graph::views
