#pragma once

#include "graph/graph.hpp"
#include <queue>
#include <algorithm>

namespace std::graph::experimental {
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
 * @brief A concept that describes a queueable container. It reflects the capabilities of
 * std::queue and std::priority_queue.
 * 
 * Use of this defines the required capabilities, including those of containers in std and
 * the caller's domain.
*/
template <class Q> // For exposition only
concept _queueable = requires(Q&& q, typename Q::value_type value) {
  typename Q::value_type;
  typename Q::size_type;
  typename Q::reference;

  { q.top() };
  { q.push(value) };
  { q.pop() };
  { q.empty() };
  { q.size() };
};
} // namespace std::graph::experimental
