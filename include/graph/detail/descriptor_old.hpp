#pragma once

#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_DESCRIPTOR_OLD_HPP
#  define GRAPH_DESCRIPTOR_OLD_HPP

namespace graph {

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
// Practically, a descriptor is an integral index for contiguous and random-access containers and an iterator for other containers.
//
// Rename vertex_descriptor to vertex_info, etc.
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

template <typename T>
struct is_tuple_like : std::false_type {};
template <class T, class U>
struct is_tuple_like<pair<T, U>> : public std::true_type {};
template <class... Args>
struct is_tuple_like<tuple<Args...>> : public std::true_type {};

template <typename... Args>
inline constexpr bool is_tuple_like_v = is_tuple_like<Args...>::value;


template <typename T>
struct tuple_tail {
  using type = T;
};
template <typename T, typename... Rest>
struct tuple_tail<tuple<T, Rest...>> {
  using type = tuple<Rest...>;
};
template <typename T, typename U>
struct tuple_tail<pair<T, U>> {
  using type = U;
};
template <typename T>
using tuple_tail_t = typename tuple_tail<T>::type;


#  if 0
template <typename I>
struct edge_descriptor_traits {
  using inner_iterator  = I;
  using difference_type = iter_difference_t<inner_iterator>;
  using value_type      = conditional_t<random_access_iterator<inner_iterator>, difference_type, inner_iterator>;

  // container                  inner_id_type       inner_target_type       inner_value_type
  // =========================  ================    ====================    ====================
  // vector<int>                difference_type     int                     int
  // vector<pair<int, double>>  difference_type     int                     pair<int,double>
  // deque<int>                 difference_type     int                     int
  // deque<pair<int, double>>   difference_type     int                     pair<int,double>
  // map<int, double>           int                 int                     pair<int,double>
  // set<int>                   int                 int                     int
  // list<int>                  int                 int                     int
  // list<tuple<int, double>>   int                 int                     tuple<int,double>
  // list<pair<int, double>>    int                 int                     pair<int,double>
  //

  using inner_value_type = iter_value_t<I>;

  using inner_id_type = conditional_t<
        random_access_iterator<inner_iterator>, //
        difference_type,                        //
        conditional_t<is_tuple_like_v<iter_value_t<I>>, tuple_element_t<0, iter_value_t<I>>, iter_value_t<I>>>;
};
#  endif //0

// vertex range type                          inner_id_type     inner_value_type      target_id_type        edge_value_type
// =========================================  ================  ====================  ====================  ================================
// vector<vector<int>>                        difference_type   vector<int>           int                   void
// vector<vector<tuple<int, double, float>>>  difference_type   vector<int>           int                   double (not tuple<double,float>)
// vector<map<int, double>>                   difference_type   map<int, double>      int                   double
// vector<set<int>>                           difference_type   set<int>              int                   void
// deque<deque<int>>                          difference_type   deque<int>            int                   void
// deque<map<int, double>>                    difference_type   map<int, double>      int                   double
// map<int,vector<int>>                       int               vector<int>           int                   void
// vertex<int>                                difference_type   int                   n/a				    n/a    (e.g. CSR)
//
// edge range type                            inner_id_type     inner_value_type            target_id_type        edge_value_type
// =========================================  ================  ==========================  ====================  ================================
// vector<int>                                difference_type   int                         int                   void
// vector<tuple<int, double, float>>          difference_type   tuple<int, double, float>   int                   double (not tuple<double,float>)
// map<int, double>                           void (iterator)   pair<int, double>           int                   double
// set<int>                                   void (iterator)   int                         int                   void
// 
// inner_id_type is only useful for adjacency_matrix
//
// Because the vertex range can be a simple vertex<int> for CSR we can't assume that the edge range is part of the
// vertex range. However, that is a common and useful use-case and can be used as a default.
//

template <typename I>
struct _descriptor_traits {
  using inner_iterator  = I;
  using difference_type = iter_difference_t<inner_iterator>;
  using value_type      = conditional_t<random_access_iterator<inner_iterator>, difference_type, inner_iterator>;

  using inner_value_type = iter_value_t<I>;

  // The type used to lookup an element in the container.
  using inner_id_type =
        conditional_t<random_access_iterator<inner_iterator>, //
                      difference_type,                        //
                      conditional_t<is_tuple_like_v<inner_value_type>, tuple_element_t<0, inner_value_type>, void>>;

  //using inner_target_id_type =
  //      conditional_t<is_tuple_like_v<inner_value_type>, tuple_element_t<0, inner_value_type>, inner_value_type>;
};


/**
 * @brief An iterator that uses a descriptor (integral index or iterator) for a container.
 * 
 * An integral index is used for random-access containers, while an iterator is used for other containers.
 * 
 * A descriptor enables an abstraction that allows the same code to be used for different types of containers
 * without loss of efficiency from the underlying container. For instance, if an integral index is used
 * for a contiguous container, the code will be as efficient as if the index were used directly.
 * If it's used for a map (bidirectional container), the iterator will be dereferenced to get the associated
 * value, avoiding a log(n) lookup if the id were to be used.
 * 
 * @tparam I Iterator type of the underlying container.
 */
template <forward_iterator I, class Traits = _descriptor_traits<I>>
class _descriptor_iterator {
public:
  using this_type = _descriptor_iterator<I>;
  using traits    = Traits;

  using inner_iterator   = I;
  using inner_id_type    = typename traits::inner_id_type;
  using inner_value_type = typename traits::inner_value_type;

  using difference_type   = typename traits::difference_type;
  using value_type        = typename traits::value_type;
  using pointer           = std::add_pointer_t<std::add_const_t<value_type>>;
  using reference         = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using iterator_category = std::forward_iterator_tag;
  using iterator_concept  = iterator_category;

  _descriptor_iterator() = default;
  explicit _descriptor_iterator(value_type descriptor) : descriptor_(descriptor) {}
  // copy & move constructors and assignment operators are default

  template <forward_range R>
  requires convertible_to<iterator_t<R>, inner_iterator>
  explicit _descriptor_iterator(R& r, inner_iterator iter) {
    if constexpr (integral<value_type>) {
      descriptor_ = static_cast<difference_type>(std::distance(std::ranges::begin(r), iter));
    } else {
      descriptor_ = iter;
    }
  }

  //
  // dereference
  //
  reference operator*() const noexcept { return descriptor_; }
  pointer   operator->() const noexcept { return &descriptor_; }

  //
  // operators ++
  //
  _descriptor_iterator& operator++() {
    ++descriptor_;
    return *this;
  }
  _descriptor_iterator operator++(int) {
    _descriptor_iterator tmp = *this;
    ++descriptor_;
    return tmp;
  }

  //
  // operators ==, !=
  //
  auto operator==(const _descriptor_iterator& rhs) const { return descriptor_ == rhs.descriptor_; }

private:
  value_type descriptor_ = value_type(); // integral index or iterator, depending on container type
};

template <class T>
struct descriptor_value {
  using type = T;
};
template <class T, class U>
struct descriptor_value<std::pair<T, U>> {
  using type = U;
};
template <class T, class U, class... Args>
struct descriptor_value<std::tuple<T, U, Args...>> {
  using type = U;
};
template <class T>
using descriptor_value_t = typename descriptor_value<T>::type;


// Helper to detect if T has a member function named size
template <typename T>
class _has_size {
private:
  // Check if the expression T().size() is valid
  template <typename U>
  static auto check(int) -> decltype(std::declval<U>().size(), std::true_type());

  // Fallback if the above is not valid
  template <typename>
  static std::false_type check(...);

public:
  // Result is true if the first check is valid, false otherwise
  static constexpr bool value = decltype(check<T>(0))::value;
};

// Helper variable template for easier usage
template <typename T>
inline constexpr bool has_size_v = _has_size<T>::value;


// Helper to detect if T has a member function named size
template <typename T>
class _ident_type {
private:
  // Check if the expression T().size() is valid
  template <typename U>
  static auto check(int) -> decltype(std::ranges::size(declval<U>()));

  // Fallback if the above is not valid
  template <typename U>
  static iterator_t<U> check(...);

public:
  // Result is true if the first check is valid, false otherwise
  using type = decltype(check<T>(0));
};

// Helper variable template for easier usage
template <typename T>
using _ident_t = _ident_type<T>::type;


template <forward_range C>
class descriptor_view : public std::ranges::view_interface<descriptor_view<C>> {
public:
  //using size_type       = range_size_t<C>;
  using inner_iterator = iterator_t<C>;                        // iterator of the underlying container
  using iterator       = _descriptor_iterator<inner_iterator>; //

  using value_type      = descriptor_value_t<range_value_t<C>>;
  using difference_type = iter_difference_t<iterator>;
  using id_type         = difference_type;        // e.g. vertex_id_t
  using descriptor_type = iter_value_t<iterator>; // integral index or iterator, depending on container type

  descriptor_view() = default;
  constexpr explicit descriptor_view(C&& c) : c_(c) {}

  auto size() const
  requires sized_range<C>
  {
    return std::ranges::size(c_);
  }

  iterator begin() const { return std::ranges::begin(c_); }
  iterator end() const { return std::ranges::end(c_); }

  /**
   * @brief Get the vertex id for a descriptor.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return vertex id.
   */
  id_type id(descriptor_type& desc) const {
    if constexpr (integral<descriptor_type>) {
      return desc;
    } else if constexpr (random_access_range<C>) {
      return static_cast<id_type>(std::distance(c_.begin(), desc));
    } else if constexpr (is_tuple_like_v<range_value_t<C>>) {
      return std::get<0>(*desc); // e.g., pair::first used for map
    } else {
      static_assert(
            random_access_range<C>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  /**
   * @brief Find an element in the container, given an id.
   * 
   * This assumes that the full range of id's in the container is [0, size(c_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param id The id to search for.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(id_type id) const {
    if constexpr (integral<descriptor_type>) {
      return iterator(id);
    } else if constexpr (random_access_range<C>) {
      return iterator(c_.begin() + id);
    } else if constexpr (bidirectional_range<C>) {
      return iterator(c_.find(id)); // map or set
    } else {
      static_assert(random_access_range<C>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return end();
  }

private:
  reference_wrapper<C> c_;
};

template <forward_range C>
class descriptor_subrange_view : public std::ranges::view_interface<descriptor_subrange_view<C>> {
public:
  //using size_type       = range_size_t<C>;
  using inner_iterator = iterator_t<C>;                        // iterator of the underlying container
  using iterator       = _descriptor_iterator<inner_iterator>; //

  using value_type      = descriptor_value_t<range_value_t<C>>;
  using difference_type = iter_difference_t<iterator>;
  using id_type         = difference_type;        // e.g. vertex_id_t
  using descriptor_type = iter_value_t<iterator>; // integral index or iterator, depending on container type

  descriptor_subrange_view() = default;
  constexpr explicit descriptor_subrange_view(C&& c)
        : c_(c), first_(c, std::ranges::begin(c)), last_(c, std::ranges::end(c)) {}

  auto size() const
  requires sized_range<C>
  {
    return std::ranges::size(c_);
  }

  iterator begin() const { return first_; }
  iterator end() const { return last_; }

  /**
   * @brief Get the vertex id for a descriptor.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return vertex id.
   */
  id_type id(descriptor_type& desc) const {
    if constexpr (integral<descriptor_type>) {
      return desc;
    } else if constexpr (random_access_range<C>) {
      return static_cast<id_type>(std::distance(c_.begin(), desc));
    } else if constexpr (is_tuple_like_v<range_value_t<C>>) {
      return std::get<0>(*desc); // e.g., pair::first used for map
    } else {
      static_assert(
            random_access_range<C>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  /**
   * @brief Find an element in the container, given an id.
   * 
   * This assumes that the full range of id's in the container is [0, size(c_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param id The id to search for.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(id_type id) const {
    if constexpr (integral<descriptor_type>) {
      return iterator(id);
    } else if constexpr (random_access_range<C>) {
      return iterator(c_.begin() + id);
    } else if constexpr (bidirectional_range<C>) {
      return iterator(c_.find(id)); // map or set
    } else {
      static_assert(random_access_range<C>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return last_;
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
  iterator subrange_find(id_type id) const {
    if constexpr (integral<descriptor_type>) {
      if (id >= *first_ && id < *last_) {
        return iterator(c_, c_.begin() + id);
      }
    } else if constexpr (random_access_range<C>) {
      if ((id >= *first_ - c_.begin()) && (id < *last_ - c_.begin())) {
        return iterator(c_.begin() + id);
      }
    } else if constexpr (bidirectional_range<C>) {
      return iterator(c_.find(id)); // map or set
    } else {
      static_assert(random_access_range<C>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return last_;
  }

private:
  reference_wrapper<C> c_;

  // first_ and last_ may be a sub-range of the container (e.g. for a CSR).
  iterator first_;
  iterator last_;
};


#  if 0
template <forward_range R>
using _descriptor_view = subrange<_descriptor_iterator<iterator_t<R>>, _descriptor_iterator<iterator_t<R>>>;

template <forward_range R>
auto to_descriptor_view(R&& r) {
  using dview  = _descriptor_view<R>;
  using diter  = iterator_t<dview>;
  using dvalue = range_value_t<dview>;
  if constexpr (integral<dvalue>) {
    return dview{diter(0), diter(static_cast<dvalue>(size(r)))};
  } else {
    return dview{diter(std::ranges::begin(r)), diter(std::ranges::end(r))};
  }
}

// The concrete value of a descriptor is the underlying type the descriptor points to.

// value_type of the inner iterator of the descriptor_view
template <forward_range DR>
using _concrete_value_type = iter_value_t<typename iterator_t<DR>::inner_iterator>;

/**
 * @brief Returns a reference to the concrete value of a descriptor.
 * 
 * @tparam DV Descriptor value_type.
 * @tparam R  Underlying range type of values referenced by the descriptor.
 * 
 * @param r      Underlying range of concrete values.
 * @param dvalue Descriptor value.
 * 
 * @return Reference to the concrete value.
 */
template <forward_range R, class DV>
auto& _concrete_value(R&& r, DV&& dvalue) {
  if constexpr (integral<DV>) {
    return r[dvalue]; // could also be *(r.begin() + dvalue), or r.begin()[dvalue]
  } else {
    return *dvalue;
  }
}
#  endif

} // namespace graph


#endif // GRAPH_DESCRIPTOR_OLD_HPP
