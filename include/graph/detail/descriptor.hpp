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

template <forward_iterator InnerIter, class IdT = iter_difference_t<InnerIter>>
class descriptor_iterator;

template <forward_range R, class IdT = range_difference_t<R>>
class descriptor_view;

template <forward_range R, class IdT = range_difference_t<R>>
class descriptor_subrange_view;


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

  constexpr descriptor(InnerIter front, InnerIter it) : front_(front) {
    if constexpr (integral<value_type>) {
      value_ = static_cast<value_type>(std::ranges::distance(front, it));
    } else {
      value_ = it;
    }
  }
  constexpr descriptor(InnerIter front, ptrdiff_t id) : front_(front) {
    if constexpr (integral<value_type>) {
      value_ = id;
    } else {
      value_ = front + id;
    }
  }

  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter>
  constexpr descriptor(R& r, inner_iterator it = r.begin()) : front_(r.begin()) {
    if constexpr (integral<value_type>) {
      value_ = static_cast<value_type>(it - r.begin());
    } else {
      value_ = it;
    }
  }

  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter>
  constexpr descriptor(R& r, std::ptrdiff_t id = 0) : front_(r.begin()) {
    if constexpr (integral<value_type>) {
      value_ = id;
    } else {
      value_ = front_ + id;
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
      return static_cast<id_type>(std::ranges::distance(front_, value_)); // value_ is an iterator
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
      return front_ + value_;
    } else {
      return value_;
    }
  }
  [[nodiscard]] constexpr inner_iterator get_inner_iterator() const {
    if constexpr (integral<value_type>) {
      return front_ + value_;
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

  //
  // operators ==, !=, <=>
  //
  constexpr bool operator==(const descriptor& rhs) const noexcept { return value_ == rhs.value_; }

  constexpr auto operator<=>(const descriptor& rhs) const noexcept
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
  constexpr operator id_type() const noexcept { return get_vertex_id(); }

private:
  value_type     value_ = value_type();     // index or iterator
  inner_iterator front_ = inner_iterator(); // begin of the inner range
};


//
// descriptor_iterator
//
template <forward_iterator InnerIter, class IdT>
class descriptor_iterator {
  // Types
public:
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

  using iterator_category = std::forward_iterator_tag;
  using iterator_concept  = iterator_category;

  // Construction/Destruction/Assignment
public:
  constexpr descriptor_iterator()                           = default;
  constexpr descriptor_iterator(const descriptor_iterator&) = default;
  constexpr ~descriptor_iterator() noexcept                 = default;

  constexpr explicit descriptor_iterator(const value_type& desc) : descriptor_(desc) {}

  constexpr descriptor_iterator(InnerIter front, id_type id) : descriptor_(front, id) {}
  constexpr descriptor_iterator(InnerIter front, InnerIter it) : descriptor_(front, it) {}

  template <forward_range R>
  requires is_convertible_v<iterator_t<R>, InnerIter> //
  constexpr descriptor_iterator(R& r, inner_iterator it = r.begin()) : descriptor_(r, it) {}

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
  [[nodiscard]] constexpr reference operator*() const noexcept { return this->descriptor_; }
  [[nodiscard]] constexpr pointer   operator->() const noexcept { return &*this->descriptor_; }

  //
  // operators ++
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

  //
  // operators ==, !=
  //
  template <class I>
  requires std::equality_comparable_with<I, InnerIter>
  [[nodiscard]] constexpr bool operator==(const descriptor_iterator<I, IdT>& rhs) const noexcept {
    return descriptor_ == rhs.descriptor_;
  }

  // Member variables
protected:
  mutable value_type descriptor_ = value_type();
};


//
// descriptor_view
//
template <forward_range R, class IdT>
class descriptor_view : public std::ranges::view_interface<descriptor_view<R, IdT>> {
  // Types
public:
  using this_type            = descriptor_view<R, IdT>;
  using inner_range          = subrange<iterator_t<R>, sentinel_t<R>>; // range of the underlying container
  using inner_iterator       = iterator_t<R>;
  using const_inner_iterator = const_iterator_t<R>;
  using inner_sentinel       = sentinel_t<R>;
  using const_inner_sentinel = const_sentinel_t<R>;
  using inner_value_type     = range_value_t<R>;

  using id_type    = IdT;
  using value_type = descriptor<inner_iterator, id_type>;

  using iterator = descriptor_iterator<inner_iterator, id_type>;
  using sentinel = descriptor_iterator<inner_sentinel, id_type>;

  using const_iterator = descriptor_iterator<const_inner_iterator, id_type>;
  using const_sentinel = descriptor_iterator<const_inner_sentinel, id_type>;

  // Construction/Destruction/Assignment
public:
  constexpr descriptor_view() = default;

  constexpr descriptor_view(const R& r) : inner_range_(const_cast<R&>(r)) {}

  // Properties
public:
  [[nodiscard]] auto size() const
  requires sized_range<inner_range>
  {
    return inner_range_.size();
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
      return iterator(desc(inner_range_.begin(), static_cast<id_type>(0)));
    } else {
      return iterator(desc(inner_range_.begin(), inner_range_.begin()));
    }
  }
  [[nodiscard]] constexpr sentinel end() {
    using desc      = descriptor<inner_sentinel, id_type>; // descriptor
    using desc_type = typename value_type::value_type;     // integer or iterator

    if constexpr (integral<desc_type>) {
      return sentinel(desc(inner_range_.begin(), ssize(inner_range_)));
    } else {
      return sentinel(desc(inner_range_.begin(), inner_range_.end()));
    }
  }

  [[nodiscard]] constexpr const_iterator begin() const {
    using desc      = descriptor<const_inner_iterator, id_type>; // descriptor
    using desc_type = typename desc::value_type;                 // integer or iterator
    if constexpr (integral<desc_type>) {
      return const_iterator(desc(static_cast<ptrdiff_t>(0)));
    } else {
      return const_iterator(desc(inner_range_.begin(), inner_range_.begin()));
    }
  }
  [[nodiscard]] constexpr const_sentinel end() const {
    using desc      = descriptor<const_inner_sentinel, id_type>; // descriptor
    using desc_type = typename value_type::value_type;           // integer or iterator
    if constexpr (integral<desc_type>) {
      return const_sentinel(desc(static_cast<ptrdiff_t>(inner_range_.size())));
    } else {
      return const_sentinel(desc(inner_range_.begin(), inner_range_.end()));
    }
  }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr const_sentinel cend() const noexcept { return end(); }

  // Operators
public:
  // Member variables
private:
  inner_range inner_range_;
};

template <forward_range R, class IdT>
class descriptor_subrange_view : public std::ranges::view_interface<descriptor_subrange_view<R, IdT>> {
  // Types
public:
  using this_type            = descriptor_subrange_view<R, IdT>;
  using inner_range          = subrange<iterator_t<R>, sentinel_t<R>>; // range of the underlying container
  using inner_iterator       = iterator_t<R>;
  using const_inner_iterator = const_iterator_t<R>;
  using inner_sentinel       = sentinel_t<R>;
  using const_inner_sentinel = const_sentinel_t<R>;
  using inner_value_type     = range_value_t<R>;

  using id_type    = IdT;
  using value_type = descriptor<inner_iterator, id_type>;

  using iterator = descriptor_iterator<inner_iterator, id_type>;
  using sentinel = descriptor_iterator<inner_sentinel, id_type>;

  using const_iterator = descriptor_iterator<const_inner_iterator, id_type>;
  using const_sentinel = descriptor_iterator<const_inner_sentinel, id_type>;


  // Construction/Destruction/Assignment
public:
  descriptor_subrange_view() = default;

  constexpr explicit descriptor_subrange_view(const R& r)
        : inner_range_(const_cast<R&>(r)), inner_subrange_(const_cast<R&>(r)) {}


  constexpr descriptor_subrange_view(const R& r, iterator_t<R> first, sentinel_t<R> last)
        : inner_range_(const_cast<R&>(r)), inner_subrange_(first, last) {}

  constexpr descriptor_subrange_view(const R& r, const subrange<iterator_t<R>, sentinel_t<R>>& subrng)
        : inner_range_(const_cast<R&>(r)), inner_subrange_(subrng) {}

  constexpr descriptor_subrange_view& operator=(const descriptor_subrange_view&) = default;

  // Properties
public:
  auto size() const {
    using size_type = range_size_t<R>;                  //
    if constexpr (integral<value_type>) {
      return static_cast<size_type>(*end() - *begin()); // subtract integral index
    } else {
      return static_cast<size_type>(std::ranges::distance(*begin(), *end()));
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

#  if 0
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
#  endif

private:
  inner_range inner_range_;
  inner_range inner_subrange_; // subrange of inner_range_
};
} // namespace graph

template <class R, class VId>
constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_view<R, VId>> = true;

template <class R, class VId>
constexpr bool std::ranges::enable_borrowed_range<graph::descriptor_subrange_view<R, VId>> = true;

#endif // GRAPH_DESCRIPTOR_HPP
