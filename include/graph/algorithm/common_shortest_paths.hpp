#pragma once

#include <algorithm>

#ifndef GRAPH_COMMON_SHORTEST_PATHS_HPP
#  define GRAPH_COMMON_SHORTEST_PATHS_HPP

namespace std::graph {
template <class G, class WF, class DistanceValue, class Compare, class Combine> // For exposition only
concept basic_edge_weight_function =                                            // e.g. weight(uv)
      is_arithmetic_v<DistanceValue> && strict_weak_order<Compare, DistanceValue, DistanceValue> &&
      assignable_from<add_lvalue_reference_t<DistanceValue>,
                      invoke_result_t<Combine, DistanceValue, invoke_result_t<WF, edge_reference_t<G>>>>;

template <class G, class WF, class DistanceValue> // For exposition only
concept edge_weight_function =                    // e.g. weight(uv)
      is_arithmetic_v<invoke_result_t<WF, edge_reference_t<G>>> &&
      basic_edge_weight_function<G, WF, DistanceValue, less<DistanceValue>, plus<DistanceValue>>;

/**
 * @ingroup graph_algorithms
 * @brief Returns a value to define an invalid distance used to initialize distance values
 * in the distance range before one of the shorts paths functions.
 * 
 * @tparam DistanceValue The type of the distance.
 * 
 * @return A unique sentinal value to indicate that a value is invalid, or undefined.
*/
template <class DistanceValue>
constexpr auto shortest_path_invalid_distance() {
  return numeric_limits<DistanceValue>::max();
}

/**
 * @ingroup graph_algorithms
 * @brief Returns a distance value of zero.
 * 
 * @tparam DistanceValue The type of the distance.
 * 
 * @return A value of zero distance.
*/
template <class DistanceValue>
constexpr auto shortest_path_zero() {
  return DistanceValue();
}

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance values to shortest_path_invalid_distance().
 * 
 * @tparam Distances The range type of the distances.
 * 
 * @param distances The range of distance values to initialize.
*/
template <class Distances>
constexpr void init_shortest_paths(Distances& distances) {
  ranges::fill(distances, shortest_path_invalid_distance<ranges::range_value_t<Distances>>());
}

/**
 * @ingroup graph_algorithms
 * @brief Intializes the distance and predecessor values for shortest paths algorithms.
 * 
 * @tparam Distances The range type of the distances.
 * @tparam Predecessors The range type of the predecessors.
 * 
 * @param distances The range of distance values to initialize.
 * @param predecessors The range of predecessors to initialize.
*/
template <class Distances, class Predecessors>
constexpr void init_shortest_paths(Distances& distances, Predecessors& predecessors) {
  init_shortest_paths(distances);

  using pred_t = ranges::range_value_t<Predecessors>;
  pred_t i     = pred_t();
  for (auto& pred : predecessors)
    pred = i++;
}

/**
 * @brief An always-empty random_access_range.
 * 
 * A unique range type that can be used at compile time to determine if predecessors need to
 * be evaluated.
 * 
 * This is not in the P1709 proposal. It's a quick hack to allow us to implement quickly.
*/
class _null_range_type : public std::vector<size_t> {
  using T         = size_t;
  using Allocator = std::allocator<T>;
  using Base      = std::vector<T, Allocator>;

public:
  _null_range_type() noexcept(noexcept(Allocator())) = default;
  explicit _null_range_type(const Allocator& alloc) noexcept {}
  _null_range_type(Base::size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  explicit _null_range_type(Base::size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  _null_range_type(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  _null_range_type(const _null_range_type& other) : Base() {}
  _null_range_type(const _null_range_type& other, const Allocator& alloc) {}
  _null_range_type(_null_range_type&& other) noexcept {}
  _null_range_type(_null_range_type&& other, const Allocator& alloc) {}
  _null_range_type(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
};

inline static _null_range_type _null_predecessors;

} // namespace std::graph

#endif // GRAPH_COMMON_SHORTEST_PATHS_HPP
