#pragma once

#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_DESCRIPTOR_HPP
#  define GRAPH_DESCRIPTOR_HPP

namespace graph {
// Descriptor Definition
//
// A descriptor is an abstract reference to a vertex or edge in a graph container.
//
// Practically, a descriptor is an integral index for contiguous and random-access containers and an iterator for other containers.
//
// The following features are supported:
// 1. The default is descriptor_type = conditional_t<random_access_range<range_type>, range_difference_t<R>, iterator_t<R>>;

// Descriptor Change Notes
//
// Implications in the change for descriptors:
// 1. The vertex functions in the Graph Container Interface will replace vertex id and vertex reference with a vertex descriptor,
//    and edge functions will replace edge reference with vertex descriptor.
// 2. The second implication is that the number of concepts, views and functions will shrink because of this.
// 3. It will become easier to write generic algorithms without specializations for vertices in random-access vs. bidirectional
//    containers.
// 4. It becomes much harder to detect if uv.target_id(g) is defined, so member functions may not be auto-detected by CPOs.
//

// Questions
// 1. How to provide the raw vertex & edge ranges? (non-descriptor ranges)
//     a. special ranges? (e.g. raw_vertices_range(g), raw_edges_range(g,u))
//

// "Concepts" for descriptors
// 1. descriptor_iterator
// 2. descriptor_value: index or iterator
//
// 3. descriptor_view | descriptor_subrange_view
//     a. size()
//     b. begin(), end()
//     c. id(descriptor) -> vertex_id_t
//     d. find(id) ->
//
// 5. inner_iterator: iterator of the underlying container
// 6. inner_value_type: vertex or edge value
// 7. inner_id_type: index or key

// Future Tasks
// 1. Implement descriptore_subrange_view & use with compressed_graph
// 2. Consolidate use of _Is_basic_id_adj & _Is_tuple_id_adj to use descriptor in CPOs (one definition)
//
// NOTES
// 1. descriptor_iterator[] always returns a reference to the iterators's value, not the nth descriptor in the range.
//    because there is no such object. This will cause unexpected behavior if callers rely on it.
// 2. graph.vertices() is assumed to return a native value, so the CPO does descriptor_view(vertices(g)).
//    Conversely, a free function vertices(g) is assumed to be an overridden function and return a descriptor_view
//    already.
// 3. vertex_reference_t<G> won't work when a temporary descriptor is passed to it (e.g. vertex_value(g, *find_vertex(g,uid)))
//    because only const values can be passed to it.
// 4. Use of descripotr<I> for public friend functions for dynamic_graph is required instead of graph type aliases because
//    types aren't fully formed yet. This is awkward. Need to find a better solution. Options to investigate:
//        a. Define them as free functions at the end of the file.
//        b. Use member functions that the CPOs can find naturally.
//

// This is a limited form of tuple-like described in https://en.cppreference.com/w/cpp/utility/tuple/tuple-like.
// It only detects pair<T0,T1> and tuple<T0,T1,...>. It doesn't handle array or subrange.
//
template <typename T>
struct _is_tuple_like : std::false_type {};
template <class T, class U>
struct _is_tuple_like<pair<T, U>> : public std::true_type {};
template <class... Args>
struct _is_tuple_like<tuple<Args...>> : public std::true_type {};

template <typename... Args>
inline constexpr bool _is_tuple_like_v = _is_tuple_like<Args...>::value;

// vertex range type                          desc value_type   vertex_id_type    inner_value_type      target_id_type        edge_value_type
// =========================================  ================  ================  ====================  ====================  ================================
// vector<vector<int>>                        VId               VId               vector<int>           int                   void
// vector<vector<tuple<int, double, float>>>  VId               VId               vector<int>           int                   double (not tuple<double,float>)
// vector<map<int, double>>                   VId               VId               map<int, double>      int                   double
// vector<set<int>>                           VId               VId               set<int>              int                   void
// deque<deque<int>>                          VId               VId               deque<int>            int                   void
// deque<map<int, double>>                    VId               VId               map<int, double>      int                   double
// map<int,vector<int>>                       iterator          int               vector<int>           int                   void
// vertex<int>                                VId               VId               int                   n/a				      n/a    (e.g. CSR)
//
// edge range type                            desc value_type   inner_id_type     inner_value_type            target_id_type        edge_value_type
// =========================================  ================  ================  ==========================  ====================  ================================
// vector<int>                                VId               VId               int                         int                   void
// vector<tuple<int, double, float>>          VId               VId               tuple<int, double, float>   int                   double (not tuple<double,float>)
// map<int, double>                           iterator          int               pair<int, double>           int                   double
// set<int>                                   iterator          int               int                         int                   void
//
// inner_id_type is only useful for the edge range for adjacency_matrix
//
// Because the vertex range can be a simple vertex<int> for CSR we can't assume that the edge range is part of the
// vertex range. However, that is a common and useful use-case and can be used as a default.
//
// target_id(g,uv) requires the owning range to be stored with the descriptor so we have the context of the edges
// range.

template <forward_iterator InnerIter>
class descriptor;

template <forward_iterator InnerIter>
class descriptor_iterator;

template <forward_range R>
class descriptor_subrange_view_impl;


/**
 * @brief A descriptor used for a vertex or edge
 * 
 * This class provides an abstraction for a vertex or edge in a graph that hides the use of an index or an iterator.
 * This helps create generic algorithms that work with both random-access and non-random-access containers.
 * 
 * It's tempting to break this into separate index_descriptor and iterator_descriptor classes. If that were done,
 * the iterator_descriptor class would not require the begin_ member variable. However, this would remove the 
 * convenience of the vertex_index() function, which requires it.
 * 
 * @tparam InnerIter The iterator for the inner (raw) type that's being referenced.
 */
template <forward_iterator InnerIter>
class descriptor {
public:
  using inner_iterator   = InnerIter;
  using inner_value_type = iter_value_t<inner_iterator>;
  // preserve inner value constness based on inner_iterator
  using inner_reference = conditional_t<std::is_const_v<remove_reference_t<decltype(*declval<inner_iterator>())>>,
                                        std::add_lvalue_reference_t<std::add_const_t<inner_value_type>>,
                                        std::add_lvalue_reference_t<inner_value_type>>;
  using inner_pointer   = conditional_t<std::is_const_v<remove_reference_t<decltype(*declval<inner_iterator>())>>,
                                      std::add_pointer_t<std::add_const_t<inner_value_type>>,
                                      std::add_pointer_t<inner_value_type>>;

  // Determine if this an index-based or iterator-based descriptor
  using id_type    = ptrdiff_t; // signed integer type for adding to, or subtracting from, an iterator
  using value_type = conditional_t<random_access_iterator<inner_iterator>, id_type, inner_iterator>;

  // Honor the const/non-const contract for the value type
  using pointer       = std::add_pointer_t<value_type>;
  using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

  using reference       = std::add_lvalue_reference_t<value_type>;
  using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

  using difference_type = std::iter_difference_t<inner_iterator>;

  constexpr descriptor()                  = default;
  constexpr descriptor(const descriptor&) = default;
  constexpr descriptor(descriptor&&)      = default;
  constexpr ~descriptor() noexcept        = default;

  constexpr descriptor(InnerIter first, InnerIter it) : begin_(first) {
    if constexpr (integral<value_type>) {
      value_ = static_cast<value_type>(std::ranges::distance(first, it));
    } else {
      value_ = it;
    }
  }
  constexpr descriptor(InnerIter first, ptrdiff_t id) : begin_(first) {
    if constexpr (integral<value_type>) {
      value_ = id;
    } else {
      value_ = first + id;
    }
  }

  // for testing
  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter>
  constexpr descriptor(R& r, inner_iterator it /*= r.begin()*/) : begin_(r.begin()) {
    //it = r.begin();
    if constexpr (integral<value_type>) {
      value_ = static_cast<value_type>(it - r.begin());
    } else {
      value_ = it;
    }
  }

  // for testing
  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter>
  constexpr descriptor(R& r, std::ptrdiff_t id = 0) : begin_(r.begin()) {
    if constexpr (integral<value_type>) {
      value_ = id;
    } else {
      value_ = begin_;
      std::advance(value_, ptrdiff_t(id));
    }
  }

#  if 0
  // The following is intended to assign an iterator to a const_iterator, but it generates many errors I haven't had time to resolve.
  template <class InnerIter2>
  requires convertible_to<InnerIter2, InnerIter>
  constexpr descriptor& operator=(const descriptor<InnerIter2>& rhs) {
    if (&rhs != this) {
      begin_ = rhs.begin_;
      value_ = rhs.value_;
    }
    return *this;
  }
#  else
  constexpr descriptor& operator=(const descriptor&) = default;
#  endif
  constexpr descriptor& operator=(descriptor&&) = default;

  // Properies
public:
  /**
   * @brief Get the descriptor value.
   * @return The descriptor value, either an index or an iterator.
   */
  constexpr value_type& value() const noexcept { return value_; }

  constexpr inner_iterator get_inner_iterator() const {
    if constexpr (integral<value_type>) {
      return begin_ + value_;
    } else {
      return value_;
    }
  }

  /**
   * @brief Get a reference to the inner value referenced by the descriptor.
   * @return The inner value referenced by the descriptor.
   */
  constexpr inner_reference inner_value() const noexcept {
    if constexpr (integral<value_type>) {
      return *(begin_ + value_);
    } else {
      return *value_;
    }
  }

  /**
   * @brief Get the vertex id for a vertex descriptor.
   * 
   * @return id_type with a value of the vertex id. This is always a value type because it may be calculated
   *         rather than stored. For example, the vertex id for a vector is the index, but for a map it is the key.
   */
  [[nodiscard]] constexpr id_type vertex_index() const
  requires(integral<value_type> || random_access_iterator<inner_iterator> || _is_tuple_like_v<inner_value_type>)
  {
    if constexpr (integral<value_type>) {
      return value_;
    } else if constexpr (random_access_iterator<inner_iterator>) {
      return static_cast<id_type>(std::ranges::distance(begin_, value_)); // value_ is an iterator
    } else if constexpr (_is_tuple_like_v<inner_value_type>) {
      return std::get<0>(*value_);                                        // e.g., pair::first used for map
    } else {
      static_assert(
            random_access_iterator<inner_iterator>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  /**
   * @brief Get the target id for an edge descriptor.
   * 
   * If the inner value is a tuple or pair, the first element is used as the target id. Examples:
   *    vector<pair<int,double>>
   *    list<tuple<int,double>>
   *    map<int,double>
   * Otherwise, the value itself is used as the target id. In some cases this is OK (e.g. set<int>).
   * In more complex cases, the target id may be a struct and the caller needs to determine what to do
   * with it by defining the target_id() for their graph container.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return target id.
   */
  constexpr const auto edge_target_id() const {
    if constexpr (_is_tuple_like_v<inner_value_type>) {
      return std::get<0>(inner_value()); // e.g., pair::first used for map, or vector<tuple<int,double>>
    } else {
      return inner_value();              // default to the value itself, e.g. set<int>
    }
  }

  // Operators
public:
  //
  // dereference
  //

  // Note: range concept requirement: decltype(*descriptor) == value_type&
  [[nodiscard]] constexpr reference       operator*() noexcept { return value_; }
  [[nodiscard]] constexpr const_reference operator*() const noexcept { return value_; }

  [[nodiscard]] constexpr pointer       operator->() noexcept { return &value_; }
  [[nodiscard]] constexpr const_pointer operator->() const noexcept { return &value_; }

  //
  // operator ++ += +
  //
  constexpr descriptor& operator++() {
    ++value_;
    return *this;
  }
  constexpr descriptor operator++(int) {
    descriptor tmp = *this;
    ++value_;
    return tmp;
  }

  constexpr descriptor& operator+=(iter_difference_t<inner_iterator> n)
  requires random_access_iterator<inner_iterator>
  {
    value_ += n;
    return *this;
  }
  constexpr descriptor operator+(iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    descriptor tmp = *this;
    tmp += n;
    return tmp;
  }

  //
  // operator -- -= -
  //
  constexpr descriptor& operator--()
  requires bidirectional_iterator<inner_iterator>
  {
    --value_;
    return *this;
  }
  constexpr descriptor operator--(int)
  requires bidirectional_iterator<inner_iterator>
  {
    descriptor tmp = *this;
    --value_;
    return tmp;
  }

  constexpr descriptor& operator-=(iter_difference_t<inner_iterator> n)
  requires random_access_iterator<inner_iterator>
  {
    value_ -= n;
    return *this;
  }
  constexpr descriptor operator-(iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    descriptor tmp = *this;
    tmp -= n;
    return tmp;
  }

  template <class InnerIter2>
  constexpr iter_difference_t<inner_iterator> operator-(const descriptor<InnerIter2>& rhs) const
  requires random_access_iterator<inner_iterator>
  {
    return value_ - rhs.value_;
  }

  //
  // operator []
  //
  constexpr inner_reference operator[](iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    return value_[n];
  }

  //
  // operators ==, <=>
  //
  constexpr bool operator==(const descriptor& rhs) const noexcept { //
    return value_ == rhs.value_;
  }

  template <forward_iterator InnerIter2>
  requires std::equality_comparable_with<InnerIter, InnerIter2>
  constexpr bool operator==(const descriptor<InnerIter2>& rhs) const noexcept {
    return value_ == rhs.value_;
  }

  // for testing; useful in general?
  template <forward_iterator InnerIter2>
  requires std::equality_comparable_with<InnerIter, InnerIter2>
  constexpr bool operator==(const InnerIter2& rhs) const noexcept {
    return value_ == rhs;
  }

  constexpr auto operator<=>(const descriptor& rhs) const noexcept
  requires std::integral<value_type> || std::random_access_iterator<inner_iterator>
  { //
    return value_ <=> rhs.value_;
  }

  template <random_access_iterator InnerIter2>
  requires std::three_way_comparable_with<InnerIter, InnerIter2> &&
           (std::integral<value_type> || random_access_iterator<inner_iterator>)
  constexpr auto operator<=>(const descriptor<InnerIter2>& rhs) const noexcept {
    return value_ <=> rhs.value_;
  }

  /**
   * @brief Cast to the vertex id type.
   * 
   * This is a convenience function to allow the descriptor to be used as a vertex id for the
   * outer (vertex) range, where operator[] is often used for indexing.
   */
  constexpr operator id_type() const noexcept
  requires(integral<value_type> || random_access_iterator<inner_iterator> || _is_tuple_like_v<inner_value_type>)
  {
    return vertex_index();
  }

private:
  value_type     value_ = value_type();     // index or iterator
  inner_iterator begin_ = inner_iterator(); // begin of the inner range
};


//
// descriptor_iterator
//
template <forward_iterator InnerIter>
class descriptor_iterator {
  // Types
public:
  using this_type = descriptor_iterator<InnerIter>;

  using inner_iterator   = InnerIter;
  using inner_value_type = iter_value_t<inner_iterator>; //
  using descriptor_type  = descriptor<inner_iterator>;

  using value_type = descriptor_type; // descriptor
  using id_type    = typename value_type::id_type;

  using pointer       = std::add_pointer_t<value_type>;
  using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

  //using reference = std::add_lvalue_reference_t<value_type>;
  using reference = conditional_t<std::is_const_v<remove_reference_t<decltype(*declval<inner_iterator>())>>,
                                  std::add_lvalue_reference_t<std::add_const_t<value_type>>,
                                  std::add_lvalue_reference_t<value_type>>;

  //using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using difference_type = std::iter_difference_t<InnerIter>;

  using iterator_category = typename InnerIter::iterator_category;
  using iterator_concept =
        conditional_t<contiguous_iterator<inner_iterator>, std::contiguous_iterator_tag, iterator_category>;

  // Construction/Destruction/Assignment
public:
  constexpr descriptor_iterator()                           = default;
  constexpr descriptor_iterator(const descriptor_iterator&) = default;
  constexpr ~descriptor_iterator() noexcept                 = default;

  constexpr explicit descriptor_iterator(const value_type& desc) : descriptor_(desc) {}

  constexpr descriptor_iterator(InnerIter front, id_type id) : descriptor_(front, id) {}
  constexpr descriptor_iterator(InnerIter front, InnerIter it) : descriptor_(front, it) {}

  // for testing
  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter> //
  constexpr descriptor_iterator(R& r, inner_iterator it /*= r.begin()*/) : descriptor_(r, it) {}

  // for testing
  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter> //
  constexpr descriptor_iterator(R& r, ptrdiff_t id = 0) : descriptor_(r, id) {}

  constexpr descriptor_iterator& operator=(const descriptor_iterator&) = default;

  // Properties
public:
  // Operations
public:
  // Operators
public:
  [[nodiscard]] constexpr reference operator*() const noexcept { return descriptor_; }
  [[nodiscard]] constexpr pointer   operator->() const noexcept { return &descriptor_; }

  //
  // operators ++ += +
  //
  constexpr descriptor_iterator& operator++() {
    ++this->descriptor_;
    return *this;
  }
  constexpr descriptor_iterator operator++(int) {
    descriptor_iterator tmp = *this;
    ++this->descriptor_;
    return tmp;
  }

  constexpr descriptor_iterator& operator+=(iter_difference_t<inner_iterator> n)
  requires random_access_iterator<inner_iterator>
  {
    this->descriptor_ += n;
    return *this;
  }
  constexpr descriptor_iterator operator+(iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    descriptor_iterator tmp = *this;
    tmp += n;
    return tmp;
  }

  friend constexpr descriptor_iterator operator+(iter_difference_t<inner_iterator> n, const descriptor_iterator& it)
  requires random_access_iterator<inner_iterator>
  {
    descriptor_iterator<InnerIter> tmp = it;
    tmp += n;
    return tmp;
  }


  //
  // operators -- -= -
  //
  constexpr descriptor_iterator& operator--()
  requires bidirectional_iterator<InnerIter>
  {
    --this->descriptor_;
    return *this;
  }
  constexpr descriptor_iterator operator--(int)
  requires bidirectional_iterator<InnerIter>
  {
    descriptor_iterator tmp = *this;
    --this->descriptor_;
    return tmp;
  }

  constexpr descriptor_iterator& operator-=(iter_difference_t<inner_iterator> n)
  requires random_access_iterator<inner_iterator>
  {
    this->descriptor_ -= n;
    return *this;
  }
  constexpr descriptor_iterator operator-(iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    descriptor_iterator tmp = *this;
    tmp -= n;
    return tmp;
  }

  template <forward_iterator InnerIter2>
  constexpr iter_difference_t<inner_iterator> operator-(InnerIter2 rhs) const
  requires random_access_iterator<inner_iterator>
  {
    return this->descriptor_ - rhs.descriptor_;
  }

  //
  // operators []
  //
  constexpr reference operator[](iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    // do not use
    // This exists to satisfy the random_access_iterator concept only.
    // It would be preferable to return a reference to the inner_value, but that violates the concept.
    assert(false);
    return descriptor_; // ideally would be: return descriptor_[n], but that's a non-existant object.
  }


  //
  // operators ==, !=, <=>
  //
  [[nodiscard]] constexpr bool operator==(const descriptor_iterator& rhs) const noexcept {
    return descriptor_ == rhs.descriptor_;
  }
  template <forward_iterator InnerIter2>
  requires std::equality_comparable_with<InnerIter, InnerIter2>
  [[nodiscard]] constexpr bool operator==(const descriptor_iterator<InnerIter2>& rhs) const noexcept {
    return descriptor_ == rhs.descriptor_;
  }

  constexpr auto operator<=>(const descriptor_iterator& rhs) const noexcept
  requires random_access_iterator<inner_iterator>
  {
    return descriptor_ <=> rhs.descriptor_;
  }

  template <forward_iterator InnerIter2>
  constexpr auto operator<=>(const descriptor_iterator<InnerIter2>& rhs) const noexcept
  requires random_access_iterator<inner_iterator>
  {
    return descriptor_ <=> rhs.descriptor_;
  }

  // Member variables
protected:
  mutable value_type descriptor_ = value_type();
};


//
// descriptor_view
//
//template <forward_iterator I, std::sentinel_for<I> S = I>
//[[nodiscard]] constexpr auto descriptor_view(I first, S last) {
//  using id_type = ptrdiff_t;
//  if constexpr (random_access_iterator<I>) {
//    return std::ranges::subrange(descriptor_iterator(first, id_type(0)),
//                                 descriptor_iterator(first, id_type(last - first)));
//  } else {
//    return std::ranges::subrange(descriptor_iterator(first, first), descriptor_iterator(first, last));
//  }
//}


template <forward_range R>
[[nodiscard]] constexpr auto descriptor_view(R&& r) {
  using id_type = ptrdiff_t;
  if constexpr (random_access_range<R>) {
    return std::ranges::subrange(descriptor_iterator(begin(r), id_type(0)),
                                 descriptor_iterator(begin(r), id_type(ssize(r))));
  } else {
    return std::ranges::subrange(descriptor_iterator(begin(r), begin(r)), descriptor_iterator(begin(r), end(r)));
  }
}

template <forward_range R>
using descriptor_view_t = decltype(descriptor_view(std::declval<R>()));

template <forward_range R>
bool is_end(const descriptor_view_t<R>& r, const descriptor<iterator_t<R>>& desc) {
  if constexpr (integral<typename descriptor<iterator_t<R>>::value_type>) {
    return *desc == std::ranges::ssize(r);
  } else {
    return *desc == std::ranges::end(r);
  }
}

//
// descriptor_subrange_view
//
//template <forward_range R1>
//[[nodiscard]] constexpr auto descriptor_subrange_view(R1&& rng, R1&& subrng) {
//  // Is subrng truly a subrange in rng?
//  if constexpr (random_access_range<R1>) {
//    assert(std::ranges::begin(rng) <= std::ranges::begin(subrng) && std::ranges::end(subrng) <= std::ranges::end(rng));
//  }
//
//    // beginning of rng is used to calculate the id correctly with the subrng iterator
//  return std::ranges::subrange(descriptor_iterator(rng.begin(), subrng.begin()),
//                               descriptor_iterator(rng.begin(), subrng.end()));
//}

template <forward_range R1, forward_range R2>
requires std::equality_comparable_with<iterator_t<R1>, iterator_t<R2>>
[[nodiscard]] constexpr auto descriptor_subrange_view(R1&& rng, R2&& subrng) {
  // Is subrng truly a subrange in rng?
  if constexpr (random_access_range<R1>) {
    assert(std::ranges::begin(rng) <= std::ranges::begin(subrng) && std::ranges::end(subrng) <= std::ranges::end(rng));
  }

  // beginning of rng is used to calculate the id correctly with the subrng iterator
  return std::ranges::subrange(descriptor_iterator(rng.begin(), subrng.begin()),
                               descriptor_iterator(rng.begin(), subrng.end()));
}

//template <forward_range R>
//[[nodiscard]] constexpr auto descriptor_subrange_view(R&& rng, pair<ptrdiff_t, ptrdiff_t> subrng) {
//  // Is subrng truly a subrange in rng?
//  if constexpr (random_access_range<R>) {
//    //assert(std::ranges::begin(rng) <= std::ranges::begin(subrng) && std::ranges::end(subrng) <= std::ranges::end(rng));
//  }
//
//  // beginning of rng is used to calculate the id correctly with the subrng iterator
//  return std::ranges::subrange(descriptor_iterator(rng.begin(), subrng.first),
//                               descriptor_iterator(rng.begin(), subrng.second));
//}

template <forward_range R>
using descriptor_subrange_view_t = decltype(descriptor_subrange_view(std::declval<R>(), std::declval<R>()));


/**
 * @brief Is a range a descripor_view or descriptor_subrange_view?
 */
template <typename R>
concept descriptor_range = forward_range<R> &&
                           requires()
{
  typename iterator_t<R>::descriptor_type; // successful for both descriptor_view and descriptor_subrange_view
};

/**
 * @brief Create a descriptor_view from a range.
 * 
 * If R is already a descriptor range, it is returned as-is. Otherwise, a descriptor_view is created from the range.
 * 
 * @tparam R Range type
 * @param r Range
 * @return descriptor_view(r) or r, if it is already a descriptor range.
 */
template <typename R>
auto make_descriptor_view(R&& r) -> decltype(auto) {
  if constexpr (descriptor_range<R>) {
    return forward<R&&>(r);
  } else {
    return descriptor_view(forward<R&&>(r));
  }
}

// Graph container must return descriptor_subrange_view
//template <typename R>
//auto make_descriptor_subrange_view(R&& r, R&& sr) -> decltype(auto) {
//  if constexpr (descriptor_range<R>) {
//    return forward<R&&>(r);
//  } else {
//    return descriptor_view(forward<R&&>(r));
//  }
//}

} // namespace graph

#endif // GRAPH_DESCRIPTOR_HPP
