#pragma once

#include <algorithm>
#include <numeric>
#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_COMMON_SHORTEST_PATHS_HPP
#  define GRAPH_COMMON_SHORTEST_PATHS_HPP

namespace graph {
template <class G, class WF, class DistanceValue, class Compare, class Combine> // For exposition only
concept basic_edge_weight_function =                                            // e.g. weight(uv)
      is_arithmetic_v<DistanceValue> && std::strict_weak_order<Compare, DistanceValue, DistanceValue> &&
      std::assignable_from<std::add_lvalue_reference_t<DistanceValue>,
                           invoke_result_t<Combine, DistanceValue, invoke_result_t<WF, edge_reference_t<G>>>>;

template <class G, class WF, class DistanceValue> // For exposition only
concept edge_weight_function =                    // e.g. weight(uv)
      is_arithmetic_v<invoke_result_t<WF, edge_reference_t<G>>> &&
      basic_edge_weight_function<G, WF, DistanceValue, less<DistanceValue>, plus<DistanceValue>>;

/**
 * @ingroup graph_algorithms
 * @brief Returns a value to define an infinite distance used to initialize distance values
 * in the distance range before one of the shorts paths functions.
 * 
 * @tparam DistanceValue The type of the distance.
 * 
 * @return A unique sentinal value for an infinite distance.
*/
template <class DistanceValue>
constexpr auto shortest_path_infinite_distance() {
  return std::numeric_limits<DistanceValue>::max();
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
 * @brief Intializes the distance values to shortest_path_infinite_distance().
 * 
 * @tparam Distances The range type of the distances.
 * 
 * @param distances The range of distance values to initialize.
*/
template <class Distances>
constexpr void init_shortest_paths(Distances& distances) {
  std::ranges::fill(distances, shortest_path_infinite_distance<range_value_t<Distances>>());
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
  iota(predecessors.begin(), predecessors.end(), 0);
}

//
// Visitor concepts and classes
//

// Vertex visitor concepts
template <class G, class Visitor>
concept has_on_initialize_vertex = // For exposition only
      requires(Visitor& v, vertex_info<vertex_id_t<G>, vertex_reference_t<G>, void> vdesc) {
        { v.on_initialize_vertex(vdesc) };
      };
template <class G, class Visitor>
concept has_on_discover_vertex = // For exposition only
      requires(Visitor& v, vertex_info<vertex_id_t<G>, vertex_reference_t<G>, void> vdesc) {
        { v.on_discover_vertex(vdesc) };
      };
template <class G, class Visitor>
concept has_on_examine_vertex = // For exposition only
      requires(Visitor& v, vertex_info<vertex_id_t<G>, vertex_reference_t<G>, void> vdesc) {
        { v.on_examine_vertex(vdesc) };
      };
template <class G, class Visitor>
concept has_on_finish_vertex = // For exposition only
      requires(Visitor& v, vertex_info<vertex_id_t<G>, vertex_reference_t<G>, void> vdesc) {
        { v.on_finish_vertex(vdesc) };
      };

// Edge visitor concepts
template <class G, class Visitor>
concept has_on_examine_edge = // For exposition only
      requires(Visitor& v, edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void> edesc) {
        { v.on_examine_edge(edesc) };
      };
template <class G, class Visitor>
concept has_on_edge_relaxed = // For exposition only
      requires(Visitor& v, edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void> edesc) {
        { v.on_edge_relaxed(edesc) };
      };
template <class G, class Visitor>
concept has_on_edge_not_relaxed = // For exposition only
      requires(Visitor& v, edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void> edesc) {
        { v.on_edge_not_relaxed(edesc) };
      };
template <class G, class Visitor>
concept has_on_edge_minimized = // For exposition only
      requires(Visitor& v, edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void> edesc) {
        { v.on_edge_minimized(edesc) };
      };
template <class G, class Visitor>
concept has_on_edge_not_minimized = // For exposition only
      requires(Visitor& v, edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void> edesc) {
        { v.on_edge_not_minimized(edesc) };
      };

// Visitor structs and classes
struct empty_visitor {};

/**
 * @brief An always-empty random_access_range.
 * 
 * A unique range type that can be used at compile time to determine if predecessors need to
 * be evaluated.
 * 
 * This is not in the P1709 proposal. It's an implementation detail that allows us to have
 * a single implementation for the Dijkstra and Bellman-Ford shortest paths algorithms that 
 * can be used by other overloads.
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

} // namespace graph

#endif // GRAPH_COMMON_SHORTEST_PATHS_HPP
