#pragma once

#ifndef GRAPH_IDENTIFIER_HPP
#  define GRAPH_IDENTIFIER_HPP

#  include <iterator>
#  include <concepts>
#  include <type_traits>

namespace graph::detail {
// Range concepts & types
using std::ranges::contiguous_range;
using std::ranges::random_access_range;
using std::ranges::bidirectional_range;
using std::ranges::forward_range;
using std::ranges::sized_range;

using std::ranges::range_size_t;
using std::ranges::range_value_t;
using std::ranges::range_difference_t;
using std::ranges::iterator_t;

using std::ranges::subrange;

// Iterator concepts & types
using std::contiguous_iterator;
using std::random_access_iterator;
using std::bidirectional_iterator;
using std::forward_iterator;

using std::contiguous_iterator_tag;

using std::iter_difference_t;
using std::iter_value_t;
using std::iter_reference_t;


// Other concepts and types
using std::integral;

using std::conditional_t;
using std::declval;


/**
 * @brief An iterator that uses an identifier (integral index or iterator) for a container.
 * 
 * An integral index is used for contiguous containers, while an iterator is used for other containers.
 * 
 * This creates an abstraction that allows the same code to be used for different types of containers
 * without loss of efficiency from the underlying container. For instance, if an integral index is used
 * for a contiguous container, the code will be as efficient as if the index were used directly.
 * However, if it's used for a map (bidirectional container), the consuming code will need to do a log(n)
 * lookup each time it wants to access the associated value.
 * 
 * An integral index could be used for random access containers, but it is not as efficient as an iterator
 * when accessing the value. Consider deque, for example.
 * 
 * Values are returned by value, not by reference.
 * 
 * @tparam I Iterator type of the underlying container.
 */
template <forward_iterator I>
class identifier_iterator {
public:
  using inner_iterator = I;

  using difference_type   = iter_difference_t<inner_iterator>;
  using value_type        = conditional_t<contiguous_iterator<inner_iterator>, difference_type, inner_iterator>;
  using pointer           = std::add_pointer_t<std::add_const_t<value_type>>;
  using reference         = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using iterator_category = typename inner_iterator::iterator_category;
  using iterator_concept  = conditional_t<contiguous_iterator<I>, contiguous_iterator_tag, iterator_category>;

  identifier_iterator() = default;
  explicit identifier_iterator(value_type identifier) : identifier_(identifier) {}

  //
  // dereference
  //
  reference operator*() const { return identifier_; }
  pointer   operator->() const { return &identifier_; }

  //
  // operators ++, +=, +
  //
  identifier_iterator& operator++() {
    ++identifier_;
    return *this;
  }
  identifier_iterator operator++(int) {
    identifier_iterator tmp = *this;
    ++identifier_;
    return tmp;
  }
  identifier_iterator& operator+=(difference_type rhs)
  requires random_access_iterator<inner_iterator>
  {
    identifier_ += rhs;
    return *this;
  }
  identifier_iterator operator+(difference_type rhs) const
  requires random_access_iterator<inner_iterator>
  {
    identifier_iterator tmp = *this;
    tmp += rhs;
    return tmp;
  }
  friend identifier_iterator operator+(difference_type lhs, identifier_iterator rhs)
  requires random_access_iterator<inner_iterator>
  {
    return rhs + lhs;
  }

  //
  // operators --, -=, -
  //
  identifier_iterator& operator--()
  requires bidirectional_iterator<inner_iterator>
  {
    --identifier_;
    return *this;
  }
  identifier_iterator operator--(int)
  requires bidirectional_iterator<inner_iterator>
  {
    identifier_iterator tmp = *this;
    --identifier_;
    return tmp;
  }
  identifier_iterator& operator-=(difference_type rhs)
  requires random_access_iterator<inner_iterator>
  {
    identifier_ -= rhs;
    return *this;
  }
  identifier_iterator operator-(difference_type rhs) const
  requires random_access_iterator<inner_iterator>
  {
    identifier_iterator tmp = *this;
    tmp -= rhs;
    return tmp;
  }
  difference_type operator-(identifier_iterator rhs) const
  requires random_access_iterator<inner_iterator>
  {
    return identifier_ - rhs.identifier_;
  }

  //
  // operators ==, <=>
  //
  auto operator==(const identifier_iterator& rhs) const { return identifier_ == rhs.identifier_; }

  auto operator<=>(const identifier_iterator& rhs) const
  requires random_access_iterator<inner_iterator>
  {
    return identifier_ <=> rhs.identifier_;
  }

  //
  // operator []
  //
  // This is added to satisfy the random_access_iterator concept but it should not be used.
  //
  reference operator[](difference_type n) const
  requires random_access_iterator<inner_iterator>
  {
    if constexpr (std::is_integral_v<value_type>) {
      assert(false); // this should not be used
      // this will dereference the integral index as if it were an iterator and likely cause a crash
      return *(*this + n);
    } else {
      return *(*this + n);
    }
  }

private:
  value_type identifier_ = value_type(); // integral index or iterator, depending on container type
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
struct identifier_value {
  using type = T;
};
template <class T, class U>
struct identifier_value<std::pair<T, U>> {
  using type = U;
};
template <class T, class U, class... Args>
struct identifier_value<std::tuple<T, U, Args...>> {
  using type = U;
};
template <class T>
using identifier_value_t = typename identifier_value<T>::type;


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
class identifier_view {
public:
  //using size_type       = range_size_t<C>;
  using value_type      = identifier_value_t<range_value_t<C>>;
  using difference_type = range_difference_t<C>;
  using id_type         = difference_type;                                                    // e.g. vertex_id_t
  using identifier_type = conditional_t<contiguous_range<C>, range_size_t<C>, iterator_t<C>>; // _ident_t<C>; //
  using iterator        = identifier_iterator<iterator_t<C>>;

  identifier_view() = default;
  explicit identifier_view(C& c) : c_(c) {}

  auto size() const
  requires sized_range<C>
  {
    return std::ranges::size(c_);
  }

  //value_type& operator[](identifier_type i) {
  //  if constexpr (contiguous_range<C>) {
  //    return c_[i];
  //  } else if constexpr (random_access_range<C>) {
  //    return *i;
  //  } else {
  //    return i->second; // map
  //  }
  //}
  const value_type& operator[](identifier_type i) const {
    if constexpr (contiguous_range<C>) {
      return c_[i];
    } else if constexpr (is_tuple_like_v<range_value_t<C>>) {
      return i->second; // map
    } else {
      return *i;
    }
  }

  auto begin() const {
    if constexpr (std::ranges::contiguous_range<C>) {
      return iterator{static_cast<difference_type>(0)};
    } else {
      return iterator{std::ranges::begin(c_)};
    }
  }
  auto end() const {
    if constexpr (std::ranges::contiguous_range<C>) {
      return iterator{static_cast<difference_type>(std::ranges::size(c_))};
    } else {
      return iterator{std::ranges::end(c_)};
    }
  }

  id_type id(identifier_type ident) const {
    if constexpr (contiguous_range<C>) {
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

  iterator find(id_type id) const {
    if constexpr (contiguous_range<C>) {
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

} // namespace graph::detail


#endif // GRAPH_IDENTIFIER_HPP
