#pragma once
#include <utility>
#include <concepts>
#include <ranges>
#include <tuple>
#include <functional>
#include <optional>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include <memory>

namespace graph {
// Containers are not included to avoid potential conflicts: vector, list, string, etc.

// common types
using std::pair;
using std::tuple;
using std::tuple_element_t;
using std::tuple_size_v;
using std::reference_wrapper;
using std::forward_iterator_tag;
using std::declval;
using std::allocator;
using std::allocator_traits;
using std::unique_ptr;
using std::shared_ptr;

using std::iter_difference_t;
using std::iter_value_t;

// functional types
using std::plus;
using std::less;
using std::greater;
using std::identity;
using std::function;
using std::invoke;

// optional types
using std::optional;
using std::conditional_t;

// type traits
using std::is_arithmetic_v;
using std::is_convertible_v;
using std::is_same_v;
using std::is_invocable_v;
using std::is_void_v;
using std::is_lvalue_reference_v;
using std::is_signed_v;

using std::remove_cv_t;
using std::remove_const_t;
using std::remove_volatile_t;
using std::remove_reference_t;
using std::remove_pointer_t;
using std::remove_cvref_t;
using std::invoke_result_t;
using std::decay_t;

using std::false_type;
using std::true_type;

// concepts
using std::same_as;
using std::convertible_to;
using std::integral;
using std::invocable;
using std::regular_invocable;
using std::predicate;

// range concepts
using std::ranges::range;
using std::ranges::sized_range;
using std::ranges::input_range;
using std::ranges::output_range;
using std::ranges::forward_range;
using std::ranges::bidirectional_range;
using std::ranges::random_access_range;
using std::ranges::contiguous_range;
using std::ranges::common_range;

// iterator concepts
using std::input_iterator;
using std::output_iterator;
using std::forward_iterator;
using std::bidirectional_iterator;
using std::random_access_iterator;
using std::contiguous_iterator;

// range types
using std::ranges::subrange;

using std::ranges::iterator_t;
using std::ranges::sentinel_t;
//using std::ranges::const_iterator_t; // C++23
//using std::ranges::const_sentinel_t; // C++23

using std::ranges::range_difference_t;
using std::ranges::range_size_t;
using std::ranges::range_value_t;

using std::ranges::range_reference_t;
using std::ranges::range_rvalue_reference_t;
//using std::ranges::range_const_reference_t;  // C++23
//using std::ranges::range_common_reference_t; // not in gcc-13 for C++20

// utility functions
using std::move;
using std::forward;
using std::ranges::begin;
using std::ranges::end;
using std::ranges::size;
using std::ranges::ssize;
using std::ranges::empty;
using std::tuple_cat;
using std::max;
using std::min;
using std::swap;
using std::is_sorted;
} // namespace graph
