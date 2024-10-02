#pragma once

#ifndef GRAPH_IDENTIFIER_HPP
#  define GRAPH_IDENTIFIER_HPP

#  include <iterator>
#  include <concepts>
#  include <type_traits>

namespace graph::detail {
// Type traits
using std::conditional_t;

// Concepts
using std::integral;
using std::contiguous_iterator;
using std::random_access_iterator;
using std::bidirectional_iterator;
using std::forward_iterator;

// Iterator types
using std::iterator_traits;
using std::contiguous_iterator_tag;

// Range types
using std::ranges::range_size_t;
using std::ranges::iterator_t;


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

  using difference_type   = typename iterator_traits<inner_iterator>::difference_type;
  using value_type        = conditional_t<contiguous_iterator<inner_iterator>, difference_type, inner_iterator>;
  using pointer           = std::add_pointer_t<std::add_const_t<value_type>>;
  using reference         = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using iterator_category = typename iterator_traits<inner_iterator>::iterator_category;
  using iterator_concept  = conditional_t<contiguous_iterator<I>, contiguous_iterator_tag, typename iterator_traits<inner_iterator>::iterator_category>;

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

} // namespace graph::detail


#endif // GRAPH_IDENTIFIER_HPP
