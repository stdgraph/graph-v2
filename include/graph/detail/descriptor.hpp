#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_DESCRIPTOR_HPP
#  define GRAPH_DESCRIPTOR_HPP

#  define ENABLE_CONST_ITERATOR 0

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

template <forward_iterator InnerIter, class IdT = iter_difference_t<InnerIter>>
class descriptor;

template <forward_iterator InnerIter, integral IdT = iter_difference_t<InnerIter>>
class descriptor_iterator;

//template <forward_iterator I, std::sentinel_for<I> S = I, class IdT = iter_difference_t<I>>
//class descriptor_view;
//
//template <forward_iterator I, std::sentinel_for<I> S = I, class IdT = iter_difference_t<I>>
//class descriptor_subrange_view;


// class descriptor(id, first)
// class descriptor(iterator)
// class descriptor(iterator, first)

//
// descriptor
//
template <forward_iterator InnerIter, class IdT>
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

  using id_type    = IdT;
  using value_type = conditional_t<random_access_iterator<inner_iterator>, ptrdiff_t, inner_iterator>;

  using pointer         = std::add_pointer_t<value_type>;
  using reference       = std::add_lvalue_reference_t<value_type>;
  using difference_type = std::iter_difference_t<inner_iterator>;

  constexpr descriptor()                  = default;
  constexpr descriptor(const descriptor&) = default;
  constexpr ~descriptor() noexcept        = default;

  //constexpr descriptor(id_type id, inner_iterator begin);
  //constexpr descriptor(inner_iterator it, inner_iterator begin);
  //constexpr descriptor(inner_iterator it);

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
  constexpr descriptor(R& r, inner_iterator it = r.begin()) : begin_(r.begin()) {
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

  constexpr descriptor& operator=(const descriptor&) = default;

  // Properies
public:
  constexpr const value_type& value() const noexcept { return value_; }

  /**
   * @brief Get the vertex id for a descriptor on an outer range.
   * 
   * @return id_type with a value of the vertex id. This is always a value type because it may be calculated
   *         rather than stored. For example, the vertex id for a vector is the index, but for a map it is the key.
   */
  [[nodiscard]] constexpr id_type get_vertex_id() const
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
   * @brief Get the target id for a descriptor in the owning range.
   * 
   * If the inner value is a tuple or pair, the first element is used as the target id. Examples:
   *    vector<pair<int,double>>
   *    list<tuple<int,double>>
   *    map<int,double>
   * Otherwise, the value itself is used as the target id. In some cases this is OK (e.g. set<int>).
   * In more complex cases, the target id may be a struct and the caller needs to determine what to do
   * with it.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return target id.
   */
  constexpr const id_type get_target_id() const {
    if constexpr (_is_tuple_like_v<inner_value_type>) {
      return std::get<0>(*get_inner_iterator()); // e.g., pair::first used for map, or vector<tuple<int,double>>
    } else {
      return *get_inner_iterator();              // default to the value itself, e.g. set<int>
    }
  }

private:
  /**
   * @brief Get an iterator to the element in the underlying container.
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return An iterator to the underlying container.
   */
  [[nodiscard]] constexpr inner_iterator get_inner_iterator() {
    if constexpr (integral<value_type>) {
      return begin_ + value_;
    } else {
      return value_;
    }
  }
  [[nodiscard]] constexpr inner_iterator get_inner_iterator() const {
    if constexpr (integral<value_type>) {
      return begin_ + value_;
    } else {
      return value_;
    }
  }

public:
  //
  // dereference
  //
  [[nodiscard]] constexpr inner_reference operator*() const noexcept { return *get_inner_iterator(); }
  [[nodiscard]] constexpr inner_pointer   operator->() const noexcept { return &*get_inner_iterator(); }

  //
  // operator ++
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
  constexpr iter_difference_t<inner_iterator> operator-(const descriptor<InnerIter2, IdT>& rhs) const
  requires random_access_iterator<inner_iterator>
  {
    return value_ - rhs.value_;
  }

  constexpr inner_reference operator[](iter_difference_t<inner_iterator> n) const
  requires random_access_iterator<inner_iterator>
  {
    return value_[n];
  }

  //
  // operators ==, !=, <=>
  //
  template <forward_iterator InnerIter2>
  constexpr bool operator==(const descriptor<InnerIter2, IdT>& rhs) const noexcept {
    return value_ == rhs.value_;
  }
  // for testing; useful in general?
  template <forward_iterator InnerIter2>
  constexpr bool operator==(const InnerIter2& rhs) const noexcept 
  requires std::equality_comparable_with<inner_iterator, InnerIter2>
  {
    return value_ == rhs;
  }

  template <random_access_iterator InnerIter2>
  constexpr auto operator<=>(const descriptor<InnerIter2, IdT>& rhs) const noexcept
  requires random_access_iterator<inner_iterator>
  {
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
    return get_vertex_id();
  }

private:
  value_type     value_ = value_type();     // index or iterator
  inner_iterator begin_ = inner_iterator(); // begin of the inner range
};


//
// descriptor_iterator
//
template <forward_iterator InnerIter, integral IdT>
class descriptor_iterator {
  // Types
public:
  using this_type        = descriptor_iterator<InnerIter, IdT>;

  using inner_iterator   = InnerIter;
  using inner_value_type = iter_value_t<inner_iterator>; //

  using id_type    = IdT;
  using value_type = descriptor<inner_iterator, id_type>; // descriptor

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
  constexpr descriptor_iterator(R& r, inner_iterator it = r.begin()) : descriptor_(r, it) {}

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
  [[nodiscard]] constexpr pointer   operator->() const noexcept { return &*descriptor_; }

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
    descriptor_iterator<InnerIter, IdT> tmp = it;
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
    return descriptor_[n];
  }


  //
  // operators ==, !=, <=>
  //
  template <class InnerIter2>
  //requires std::equality_comparable_with<InnerIter, InnerIter2>
  [[nodiscard]] constexpr bool operator==(const descriptor_iterator<InnerIter2, IdT>& rhs) const noexcept {
    return descriptor_ == rhs.descriptor_;
  }

  template <class InnerIter2>
  constexpr auto operator<=>(const descriptor_iterator<InnerIter2, IdT>& rhs) const noexcept
  requires random_access_iterator<inner_iterator>
  {
    return descriptor_ <=> rhs.descriptor_;
  }

  // Member variables
protected:
  mutable value_type descriptor_ = value_type();
};


template <forward_iterator I, std::sentinel_for<I> S = I, class IdT = iter_difference_t<I>>
[[nodiscard]] constexpr auto descriptor_view(I first, S last) {
  if constexpr (random_access_iterator<I>) {
    return std::ranges::subrange(descriptor_iterator(first, IdT(0)), descriptor_iterator(first, IdT(last - first)));
  } else {
    return std::ranges::subrange(descriptor_iterator(first, first), descriptor_iterator(first, last));
  }
}

template <forward_range R, class IdT = range_difference_t<R>>
[[nodiscard]] constexpr auto descriptor_view(R&& r) {
  if constexpr (random_access_range<R>) {
    return std::ranges::subrange(descriptor_iterator(begin(r), IdT(0)), descriptor_iterator(begin(r), IdT(size(r))));
  } else {
    return std::ranges::subrange(descriptor_iterator(begin(r), begin(r)), descriptor_iterator(begin(r), end(r)));
  }
}

template <forward_range R, class IdT = range_difference_t<R>>
using descriptor_view_t = decltype(descriptor_view(std::declval<R>()));

//} // namespace views


#  if 0
//
// descriptor_view
//
template <forward_iterator I, std::sentinel_for<I> S, class IdT>
class descriptor_view : public std::ranges::view_interface<descriptor_view<I, S, IdT>> {
  // Types
public:
  using this_type        = descriptor_view<I, I, IdT>;
  using inner_range      = subrange<I, S>; // range of the underlying container
  using inner_iterator   = I;
  using inner_sentinel   = S;
  using inner_value_type = iter_value_t<I>;

  using id_type    = IdT;
  using value_type = descriptor<inner_iterator, id_type>;

  using iterator = descriptor_iterator<inner_iterator, id_type>;
  using sentinel = descriptor_iterator<inner_sentinel, id_type>;

  // Construction/Destruction/Assignment
public:
  constexpr descriptor_view() = default;

  constexpr descriptor_view(inner_iterator first, inner_iterator last) : inner_range_(first, last) {}

  template <class R>
  constexpr explicit descriptor_view(R&& r) : descriptor_view(r.begin(), r.end()) {}

  // Properties
public:
  [[nodiscard]] auto size() const
  requires sized_range<inner_range>
  {
    return inner_range_.size();
  }

  [[nodiscard]] constexpr inner_range& get_inner_range() const noexcept { return inner_range_; }

  template <class I>
  constexpr const auto get_vertex_id(const descriptor<I, id_type>& desc) const {
    return desc.get_vertex_id();
  }

  template <class I>
  constexpr const auto get_target_id(const descriptor<I, id_type>& desc) const {
    return desc.get_target_id();
  }

  // Operations
public:
  [[nodiscard]] constexpr iterator begin() const {
    using desc      = descriptor<inner_iterator, id_type>; // descriptor
    using desc_type = typename desc::value_type;           // integer or iterator

    if constexpr (integral<desc_type>) {
      return iterator(desc(inner_range_.begin(), static_cast<id_type>(0)));
    } else {
      return iterator(desc(inner_range_.begin(), inner_range_.begin()));
    }
  }
  [[nodiscard]] constexpr sentinel end() const {
    using desc      = descriptor<inner_sentinel, id_type>; // descriptor
    using desc_type = typename value_type::value_type;     // integer or iterator

    if constexpr (integral<desc_type>) {
      return sentinel(desc(inner_range_.begin(), ssize(inner_range_)));
    } else {
      return sentinel(desc(inner_range_.begin(), inner_range_.end()));
    }
  }

  // Operators
public:
  // Member variables
private:
  mutable inner_range inner_range_;
};

template <forward_iterator I, std::sentinel_for<I> S, class IdT>
class descriptor_subrange_view : public std::ranges::view_interface<descriptor_subrange_view<I, S, IdT>> {
  // Types
public:
  using this_type        = descriptor_subrange_view<I, S, IdT>;
  using inner_range      = subrange<I, S>; // range of the underlying container
  using inner_iterator   = I;
  using inner_sentinel   = S;
  using inner_value_type = iter_value_t<I>;

  using id_type    = IdT;
  using value_type = descriptor<inner_iterator, id_type>;

  using iterator = descriptor_iterator<inner_iterator, id_type>;
  using sentinel = descriptor_iterator<inner_sentinel, id_type>;

  // Construction/Destruction/Assignment
public:
  constexpr descriptor_subrange_view() = default;

  constexpr descriptor_subrange_view(inner_iterator first, inner_sentinel last)
        : inner_range_(first, last), inner_subrange_(first, last) {}

  constexpr descriptor_subrange_view(inner_iterator first,
                                     inner_sentinel last,
                                     inner_iterator subfirst,
                                     inner_sentinel sublast)
        : inner_range_(first, last), inner_subrange_(subfirst, sublast) {}


  template <class R>
  constexpr explicit descriptor_subrange_view(const R& r) : descriptor_subrange_view(begin(r), end(r)) {}

  template <class R>
  constexpr descriptor_subrange_view(R& r, iterator_t<R> subfirst, sentinel_t<R> sublast)
        : descriptor_subrange_view(begin(r), end(r), subfirst, sublast) {}

  template <class R>
  constexpr descriptor_subrange_view(R& r, R& subrng)
        : descriptor_subrange_view(begin(r), end(r), begin(subrng), end(subrng)) {}

  constexpr descriptor_subrange_view& operator=(const descriptor_subrange_view&) = default;

  // Properties
public:
  auto size() const {
    if constexpr (integral<value_type>) {
      return static_cast<size_t>(*end() - *begin()); // subtract integral index
    } else {
      return static_cast<size_t>(std::ranges::distance(*begin(), *end()));
    }
  }

  [[nodiscard]] constexpr inner_range&       get_inner_range() noexcept { return inner_range_; }
  [[nodiscard]] constexpr const inner_range& get_inner_range() const noexcept { return inner_range_; }

  template <class I>
  constexpr const auto get_vertex_id(const descriptor<I, id_type>& desc) const {
    return desc.get_vertex_id();
  }

  template <class I>
  constexpr const auto get_target_id(const descriptor<I, id_type>& desc) const {
    return desc.get_target_id();
  }


  // Operations
public:
  [[nodiscard]] constexpr iterator begin() {
    using desc      = descriptor<inner_iterator, id_type>; // descriptor
    using desc_type = typename desc::value_type;           // integer or iterator

    if constexpr (integral<desc_type>) {
      return iterator(desc(inner_subrange_, static_cast<id_type>(inner_subrange_.begin() - inner_range_.begin())));
    } else {
      return iterator(desc(inner_subrange_, inner_subrange_.begin()));
    }
  }
  [[nodiscard]] constexpr sentinel end() {
    using desc      = descriptor<inner_sentinel, id_type>; // descriptor
    using desc_type = typename value_type::value_type;     // integer or iterator

    if constexpr (integral<desc_type>) {
      return sentinel(desc(inner_subrange_, static_cast<id_type>(inner_subrange_.end() - inner_range_.begin())));
    } else {
      return sentinel(desc(inner_subrange_, inner_subrange_.end()));
    }
  }

#    if 0
  /**
   * @brief Find an element in the descriptor container, given a descriptor.
   * 
   * This assumes that the full range of id's in the container is [0, size(r_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param id The id to search for.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return iterator(desc);
    } else if constexpr (random_access_range<R>) {
      return iterator(inner_range_.begin() + get_vertex_id(desc));
    } else if constexpr (bidirectional_range<R>) {
      return iterator(inner_range_.find(get_vertex_id(desc))); // map or set
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return end();
  }

  /**
   * @brief Find an element in a container, given an id, constrained to the range [first_, last_).
   * 
   * The id must be in the range [first_, last_) of the container. If it isn't, the iterator will be equal to end()
   * which is also last_.
   * 
   * Note: The first/last constraint is really for edges in a CSR. Vertices in a CSR and edges in vertex<vertex<int>>
   * will include all elements in the container. Specialization for different conditions could reduce the number of
   * constraints.
   * 
   * @param id The id to search for. It must be in the range [first_, last_) of the container.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator subrange_find(const id_type& id) const {
    if constexpr (integral<value_type>) {
      assert(id >= *begin() && id < *end());
      return iterator(r_, r_.begin() + id);
    } else if constexpr (random_access_range<R>) {
      assert((id >= *inner_subrange_.begin() - inner_range_.begin()) &&
             (id < *inner_subrange_.end() - inner_range_.begin()));
      return iterator(*this, inner_range_.begin() + id);
    } else if constexpr (bidirectional_range<R>) {
      auto it = r_.find(id);
      if (it != r_.end()) {
        return iterator(it);
      }
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return end();
  }
#    endif

private:
  inner_range inner_range_;
  inner_range inner_subrange_; // subrange of inner_range_
};
#  endif //0

} // namespace graph

//template <class R, class VId>
//constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_view<R, VId>> = true;
//
//template <class R, class VId>
//constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_subrange_view<R, VId>> = true;

//template <class I, class S, class VId>
//constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_view<I, S, VId>> = true;

//template <class R, class VId>
//constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_view2<R, VId>> = true;

#endif // GRAPH_DESCRIPTOR_HPP
