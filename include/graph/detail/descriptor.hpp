#pragma once

#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_DESCRIPTOR_HPP
#  define GRAPH_DESCRIPTOR_HPP

namespace graph::detail {

/**
 * @brief An iterator that uses a descriptor (integral index or iterator) for a container.
 * 
 * An integral index is used for random-access containers, while an iterator is used for other containers.
 * 
 * A descriptor enables an abstraction that allows the same code to be used for different types of containers
 * without loss of efficiency from the underlying container. For instance, if an integral index is used
 * for a contiguous container, the code will be as efficient as if the index were used directly.
 * However, if it's used for a map (bidirectional container), the consuming code will need to do a log(n)
 * lookup each time it wants to access the associated value.
 * 
 * @tparam I Iterator type of the underlying container.
 */
template <forward_iterator I>
class _descriptor_iterator {
public:
  using inner_iterator = I;
  using this_type      = _descriptor_iterator<inner_iterator>;

  using difference_type   = iter_difference_t<inner_iterator>;
  using value_type        = conditional_t<random_access_iterator<inner_iterator>, difference_type, inner_iterator>;
  using pointer           = std::add_pointer_t<std::add_const_t<value_type>>;
  using reference         = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using iterator_category = std::forward_iterator_tag;
  using iterator_concept  = iterator_category;

  _descriptor_iterator() = default;
  explicit _descriptor_iterator(value_type descriptor) : descriptor_(descriptor) {}
  // copy & move constructors and assignment operators are default

  //
  // dereference
  //
  reference operator*() const { return descriptor_; }
  pointer   operator->() const { return &descriptor_; }

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

template <typename T>
struct is_tuple_like : std::false_type {};
template <class T, class U>
struct is_tuple_like<std::pair<T, U>> : public std::true_type {};
template <class... Args>
struct is_tuple_like<std::tuple<Args...>> : public std::true_type {};

template <typename... Args>
inline constexpr bool is_tuple_like_v = is_tuple_like<Args...>::value;


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


// (obsolete, for tests only. eventually need to remove)
template <forward_range C>
class descriptor_view_old {
public:
  //using size_type       = range_size_t<C>;
  using value_type      = descriptor_value_t<range_value_t<C>>;
  using difference_type = range_difference_t<C>;
  using id_type         = difference_type;                    // e.g. vertex_id_t
  using iterator        = _descriptor_iterator<iterator_t<C>>; //
  using descriptor_type = iter_value_t<iterator>;             // integral index or iterator, depending on container type

  descriptor_view_old() = default;
  explicit descriptor_view_old(C& c) : c_(c) {}

  auto size() const
  requires sized_range<C>
  {
    return std::ranges::size(c_);
  }

  auto begin() const {
    if constexpr (std::ranges::random_access_range<C>) {
      return iterator{static_cast<difference_type>(0)};
    } else {
      return iterator{std::ranges::begin(c_)};
    }
  }
  auto end() const {
    if constexpr (std::ranges::random_access_range<C>) {
      return iterator{static_cast<difference_type>(std::ranges::size(c_))};
    } else {
      return iterator{std::ranges::end(c_)};
    }
  }

  // Helper function to get the id of an descriptor. Actual implementation in vertex_id(g, ident)
  id_type id(descriptor_type ident) const {
    if constexpr (random_access_range<C>) {
      return ident;
    } else if constexpr (random_access_range<C>) {
      return static_cast<id_type>(std::distance(c_.begin(), ident));
    } else if constexpr (is_tuple_like_v<range_value_t<C>>) {
      return ident->first; // map
    } else {
      static_assert(
            random_access_range<C>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  // Helper function to find an descriptor by vertex id. Actual implementation in find_vertex(g, id).
  iterator find(id_type id) const {
    if constexpr (random_access_range<C>) {
      return iterator(id);
    } else if constexpr (random_access_range<C>) {
      return iterator(c_.begin() + id);
    } else if constexpr (bidirectional_range<C>) {
      return iterator(c_.find(id)); // map or set
    } else {
      static_assert(random_access_range<C>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
      return iterator();
    }
  }

private:
  C& c_;
};

template <forward_iterator I>
using _descriptor_range = subrange<_descriptor_iterator<I>>;

} // namespace graph::detail


#endif // GRAPH_DESCRIPTOR_HPP
