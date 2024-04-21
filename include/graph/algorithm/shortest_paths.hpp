/**
 * @file shortest_paths.hpp
 * 
 * @brief Single-Source Shortest paths and shortest sistances algorithms using Dijkstra & 
 * Bellman-Ford algorithms.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 */

#include <queue>
#include <vector>
#include <ranges>
#include <functional>
#include <fmt/format.h>
#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

#ifndef GRAPH_SHORTEST_PATHS_HPP
#  define GRAPH_SHORTEST_PATHS_HPP

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


/**
 * @brief Dijkstra Shortest Paths common algorithm.
 * 
 * @tparam G            The graph type.
 * @tparam Distances    The distance range type.
 * @tparam Predecessors The predecessors range type.
 * @tparam WF           The edge value function that returns the weight of an edge.
 * 
 * @param g            The graph.
 * @param source       The single source vertex to start the search.
 * @param distances    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                     must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                     dijkstra_invalid_distance().
 * @param predecessors [inout] The predecessor[uid] of vertex_id uid in path. predecessor[seed] == seed. The
 *                     caller must assure size(predecessor) >= size(vertices(g)). It is only valid when
 *                     distance[uid] != dijkstra_invalid_distance().
 * @param weight       The weight function object used to determine the distance between
 *                     vertices on an edge. Return values must be non-negative. The default return value is 1.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF        = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>, //
          class Allocator = allocator<vertex_id_t<G>>                                             //
          >
requires is_arithmetic_v<ranges::range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> && //
         edge_weight_function<G, WF, ranges::range_value_t<Distances>>
void dijkstra_shortest_paths(
      G&&            g,            // graph
      vertex_id_t<G> source,       // starting vertex_id
      Distances&     distances,    // out: Distances[uid] of uid from source
      Predecessors&  predecessors, // out: predecessor[uid] of uid in path
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Allocator alloc = Allocator())                                                    //
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  size_t N(num_vertices(g));
  if (source < 0 || source >= N) {
    throw out_of_range(fmt::format("source {} is outside the vertices range [{},{})", source, 0, N));
  }

  //std::ranges::fill(distances, numeric_limits<weight_type>::max());
  distances[source] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  priority_queue<weighted_vertex, vector<weighted_vertex>, q_compare> Q(alloc);

  // (CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source)
  Q.push({source, distances[source]});

  while (!Q.empty()) {
    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
      if (distances[uid] + w < distances[vid]) {
        distances[vid] = distances[uid] + w;
        if constexpr (!is_same_v<Predecessors, _null_range_type>)
          predecessors[vid] = uid;
        Q.push({vid, distances[vid]});
      }
    }
  }
}

/**
 * @brief Dijkstra Shortest Paths common algorithm.
 * 
 * @tparam G           The graph type.
 * @tparam Distances   The distance range type.
 * @tparam WF          The edge value function that returns the weight of an edge.
 * 
 * @param g            The graph.
 * @param source       The single source vertex to start the search.
 * @param distances    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                     must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                     dijkstra_invalid_distance().
 * @param weight       The weight function object used to determine the distance between
 *                     vertices on an edge. Return values must be non-negative. The default return value is 1.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          class WF        = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>, //
          class Allocator = allocator<vertex_id_t<G>>                                             //
          >
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         edge_weight_function<G, WF, ranges::range_value_t<Distances>>
void dijkstra_shortest_distances(
      G&&            g,         // graph
      vertex_id_t<G> seed,      // starting vertex_id
      Distances&     distances, // out: Distances[uid] of uid from seed
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Allocator alloc = Allocator())                                                    //
{
  dijkstra_shortest_paths(g, seed, distances, _null_predecessors, forward<WF>(weight),
                          alloc); // default weight(uv) -> 1
}

/**
 * @brief Dijkstra Shortest Paths general algorithm.
 * 
 * @tparam G            The graph type.
 * @tparam Distances    The distance range type.
 * @tparam Predecessors The predecessors range type.
 * @tparam WF           The edge value function that returns the weight of an edge.
 * 
 * @param g            The graph.
 * @param source       The single source vertex to start the search.
 * @param distances    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                     must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                     dijkstra_invalid_distance().
 * @param predecessors [inout] The predecessor[uid] of vertex_id uid in path. predecessor[seed] == seed. The
 *                     caller must assure size(predecessor) >= size(vertices(g)). It is only valid when
 *                     distance[uid] != dijkstra_invalid_distance().
 * @param weight       The weight function object used to determine the distance between
 *                     vertices on an edge. Return values must be non-negative. The default return value is 1.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class Compare,
          class Combine,
          class WF        = function<ranges::range_value_t<Distances>(edge_reference_t<G>)>, //
          class Allocator = allocator<vertex_id_t<G>>                                        //
          >
requires is_arithmetic_v<ranges::range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
void dijkstra_shortest_paths(
      G&&            g,            // graph
      vertex_id_t<G> source,       // starting vertex_id
      Distances&     distances,    // out: Distances[uid] of uid from source
      Predecessors&  predecessors, // out: predecessor[uid] of uid in path
      Compare&&      compare,
      Combine&&      combine,
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Allocator alloc = Allocator())                                                    //
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  const size_t N(num_vertices(g));
  if (source < 0 || source >= N) {
    throw out_of_range(fmt::format("source {} is outside the vertices range [{},{})", source, 0, N));
  }

  //std::ranges::fill(distances, numeric_limits<weight_type>::max());
  distances[source] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  priority_queue<weighted_vertex, vector<weighted_vertex>, q_compare> Q(alloc);

  // (CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source)
  Q.push({source, distances[source]});

  while (!Q.empty()) {
    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
      if (compare(combine(distances[uid], w), distances[vid])) {
        distances[vid] = combine(distances[uid], w);
        if constexpr (!is_same_v<Predecessors, _null_range_type>)
          predecessors[vid] = uid;
        Q.push({vid, distances[vid]});
      }
    }
  }
}

/**
 * @brief Dijkstra Shortest Paths common algorithm.
 * 
 * @tparam G           The graph type.
 * @tparam Distances   The distance range type.
 * @tparam WF          The edge value function that returns the weight of an edge.
 * 
 * @param g            The graph.
 * @param source       The single source vertex to start the search.
 * @param distances    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                     must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                     dijkstra_invalid_distance().
 * @param weight       The weight function object used to determine the distance between
 *                     vertices on an edge. Return values must be non-negative. The default return value is 1.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          class Compare,
          class Combine,
          class WF        = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>, //
          class Allocator = allocator<vertex_id_t<G>>                                             //
          >
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
void dijkstra_shortest_distances(
      G&&            g,         // graph
      vertex_id_t<G> seed,      // starting vertex_id
      Distances&     distances, // out: Distances[uid] of uid from seed
      Compare&&      compare,
      Combine&&      combine,
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Allocator alloc = Allocator())                                                    //
{
  dijkstra_shortest_paths(g, seed, distances, _null_predecessors, compare, combine, forward<WF>(weight),
                          alloc); // default weight(uv) -> 1
}


} // namespace std::graph

#endif //GRAPH_SHORTEST_PATHS_HPP
