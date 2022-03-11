#pragma once
#include "graph/graph.hpp"

//
// incidence(g,u) -> edge_view<VKey,Sourced,E,EV>:
//
// enable: for([vkey, uv]        : incidence(g,u))
//         for([vkey, uv, value] : incidence(g,u,fn))
//         for([ukey, vkey, uv]        : sourced_incidence(g,u))
//         for([ukey, vkey, uv, value] : sourced_incidence(g,u,fn))
//
namespace std::graph::views {


#if 1


template <class G, bool Sourced, class EVFResult, class Derived>
class incidence_iterator_base;

template <class G, bool Sourced = false, class EVF = void>
class incidence_iterator;


template <class G, bool Sourced, class EVFResult, class Derived>
class incidence_iterator_base {
public: // types
  using derived_iterator = Derived;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = EVFResult;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_type&, edge_value_type>;
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
  using shadow_value_type = edge_view<vertex_key_type, Sourced, shadow_edge_type*, edge_value_type>;

  incidence_iterator_base(graph_type& g, edge_iterator iter) : g_(g), iter_(iter) {}
  incidence_iterator_base(graph_type& g, vertex_type& u) : incidence_iterator_base(g, ranges::begin(edges(g, u))) {}

  incidence_iterator_base()                               = default;
  incidence_iterator_base(const incidence_iterator_base&) = default;
  incidence_iterator_base(incidence_iterator_base&&)      = default;
  ~incidence_iterator_base()                              = default;

  incidence_iterator_base& operator=(const incidence_iterator_base&) = default;
  incidence_iterator_base& operator=(incidence_iterator_base&&) = default;

public:
  //constexpr incidence_iterator& operator++() {
  //  ++iter_;
  //  return *this;
  //}
  //constexpr incidence_iterator operator++(int) const {
  //  incidence_iterator tmp(*this);
  //  ++*this;
  //  return tmp;
  //}

  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

protected:
  //friend bool operator==(const edge_iterator& lhs, const derived_iterator& rhs) { return lhs == rhs.iter_; }

#  if 0
  void set_value() const {
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_.source_key = source_key(*g_, *iter_);
    value_.target_key = target_key(*g_, *iter_);
    value_.edge       = &*iter_;
    // derived class assigns value_.value when it exists
  }
#  endif


protected:
  mutable shadow_value_type value_ = {};
  ref_to_ptr<graph_type>    g_;
  edge_iterator             iter_;
};

template <class G, bool Sourced, class EVF>
class incidence_iterator
      : public incidence_iterator_base<G,
                                       Sourced,
                                       invoke_result<EVF, edge_reference_t<G>>,
                                       incidence_iterator<G, Sourced, EVF>> {
public:
  using base_type       = incidence_iterator_base<G,
                                            Sourced,
                                            invoke_result<EVF, edge_reference_t<G>>,
                                            incidence_iterator<G, Sourced, EVF>>;
  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = typename base_type::edge_value_type;

  using iterator_category = typename base_type::iterator_category;
  using value_type        = typename base_type::value_type;
  using difference_type   = typename base_type::difference_type;
  using pointer           = typename base_type::pointer;
  using const_pointer     = typename base_type::pointer;
  using reference         = typename base_type::reference;
  using const_reference   = typename base_type::const_reference;
  using rvalue_reference  = typename base_type::rvalue_reference;

public:
  incidence_iterator(const graph_type& g, edge_iterator iter, const EVF& value_fn)
        : base_type(g, iter), value_fn_(&value_fn) {}
  incidence_iterator(const graph_type& g, const vertex_type& u, const EVF& value_fn)
        : incidence_iterator(g, ranges::begin(edges(g, u)), value_fn) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    //base_type::set_value();
    //value_.value = invoke(*value_fn_, *iter_);
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(*g_, *iter_), target_key(*g_, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
    else
      value_ = {target_key(*g_, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
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

private: // member variables
  using base_type::value_;
  using base_type::g_;
  using base_type::iter_;
  const EVF* value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};

template <class G, bool Sourced>
class incidence_iterator<G, Sourced, void>
      : public incidence_iterator_base<G, Sourced, void, incidence_iterator<G, Sourced, void>> {
public:
  using base_type       = incidence_iterator_base<G, Sourced, void, incidence_iterator<G, Sourced, void>>;
  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = typename base_type::edge_value_type;

  using iterator_category = typename base_type::iterator_category;
  using value_type        = typename base_type::value_type;
  using difference_type   = typename base_type::difference_type;
  using pointer           = typename base_type::pointer;
  using const_pointer     = typename base_type::pointer;
  using reference         = typename base_type::reference;
  using const_reference   = typename base_type::const_reference;
  using rvalue_reference  = typename base_type::rvalue_reference;

public:
  incidence_iterator(const graph_type& g, edge_iterator iter) : base_type(g, iter) {}
  incidence_iterator(const graph_type& g, const vertex_type& u) : incidence_iterator(g, ranges::begin(edges(g, u))) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    //base_type::set_value();
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(*g_, *iter_), target_key(*g_, *iter_), &*iter_};
    else {
      value_.target_key = target_key(*g_.value, *iter_);
      value_.edge       = &*iter_;
      //value_            = {target_key(*g_, *iter_), &*iter_}; // "illegal indirection" when this is used in MSVC
    }
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

private: // member variables
  using base_type::value_;
  using base_type::g_;
  using base_type::iter_;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_reference_t<G> u) {
  using iter_type     = incidence_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;

  static_assert(input_or_output_iterator<iter_type>);

  using SR = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)));
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey) { return incidence(g, *find_vertex(g, ukey)); }


#else
template <class G>
class const_incidence_iterator;
template <class G>
class incidence_iterator;

template <class G>
class const_incidence_iterator;

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

  incidence_iterator_base()                               = default;
  incidence_iterator_base(const incidence_iterator_base&) = default;
  incidence_iterator_base(incidence_iterator_base&&)      = default;
  ~incidence_iterator_base()                              = default;

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
  incidence_iterator_base(const graph_type& g, edge_iterator iter) : g_(&const_cast<graph_type&>(g)), iter_(iter) {}
  incidence_iterator_base(const graph_type& g, const vertex_type& u)
        : incidence_iterator_base(g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u)))) {}

  incidence_iterator_base()                               = default;
  incidence_iterator_base(const incidence_iterator_base&) = default;
  incidence_iterator_base(incidence_iterator_base&&)      = default;
  ~incidence_iterator_base()                              = default;

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
  const_incidence_iterator(const graph_type& g, edge_iterator iter) : g_(&const_cast<graph_type&>(g)), iter_(iter) {}
  const_incidence_iterator(const graph_type& g, const vertex_type& u)
        : const_incidence_iterator(g, ranges::begin(edges(const_cast<graph_type&>(g), const_cast<vertex_type&>(u)))) {}

  constexpr const_incidence_iterator()                                = default;
  constexpr const_incidence_iterator(const const_incidence_iterator&) = default;
  constexpr const_incidence_iterator(const_incidence_iterator&&)      = default;
  constexpr ~const_incidence_iterator()                               = default;

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

  friend bool operator==(const edge_iterator& lhs, const const_incidence_iterator& rhs) { return lhs == rhs.iter_; }
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

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

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


template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(const G& g, vertex_reference_t<const G> u) {
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
constexpr auto incidence(G& g, vertex_reference_t<G> u) {
  using iter_type     = incidence_iterator<G>;
  using sentinal_type = typename iter_type::edge_iterator;
  using SR            = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)));
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(const G& g, vertex_key_t<const G> ukey) { return incidence(g, *find_vertex(g, ukey)); }

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G& g, vertex_key_t<G> ukey) { return incidence(g, *find_vertex(g, ukey)); }
#endif

} // namespace std::graph::views
