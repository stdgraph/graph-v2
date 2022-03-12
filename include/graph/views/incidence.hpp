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


template <class G, bool Sourced, class EVFResult, class Derived>
class incidence_iterator_base;

template <class G, bool Sourced = false, class EVF = void>
class incidence_iterator;


template <class G, bool Sourced, class EVF, class Derived>
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
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
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
  using shadow_value_type = edge_view<vertex_key_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

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

protected:
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;
};

template <class G, bool Sourced, class Derived>
class incidence_iterator_base<G, Sourced, void, Derived> {
public: // types
  using derived_iterator = Derived;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
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

protected:
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;
};

template <class G, bool Sourced, class EVF>
class incidence_iterator : public incidence_iterator_base<G, Sourced, EVF, incidence_iterator<G, Sourced, EVF>> {
public:
  using base_type       = incidence_iterator_base<G, Sourced, EVF, incidence_iterator<G, Sourced, EVF>>;
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
  incidence_iterator(graph_type& g, edge_iterator iter, const EVF& value_fn)
        : base_type(g, iter), value_fn_(&value_fn) {}
  incidence_iterator(graph_type& g, vertex_type& u, const EVF& value_fn)
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
      value_ = {source_key(*g_.value, *iter_), target_key(*g_, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
    else
      value_ = {target_key(*g_.value, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
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
  incidence_iterator(graph_type& g, edge_iterator iter) : base_type(g, iter) {}
  incidence_iterator(graph_type& g, vertex_type& u) : incidence_iterator(g, ranges::begin(edges(g, u))) {}

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
      value_ = {source_key(*g_.value, *iter_), target_key(*g_, *iter_), &*iter_};
    else {
      value_ = {target_key(*g_.value, *iter_), &*iter_}; // "illegal indirection" when this is used in MSVC
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

template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_reference_t<G> u, const EVF& evf) {
  using iter_type     = incidence_iterator<G, false, EVF>;
  using sentinal_type = typename iter_type::edge_iterator;

  static_assert(input_or_output_iterator<iter_type>);

  using SR = ranges::subrange<iter_type, sentinal_type>;

  auto first = iter_type(g, ranges::begin(edges(g, u)), evf);
  auto last  = sentinal_type(ranges::end(edges(g, u)));
  return SR(first, last);
}

template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey, const EVF& evf) { return incidence(g, *find_vertex(g, ukey), evf); }



} // namespace std::graph::views
