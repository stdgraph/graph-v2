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
#include "../graph.hpp"

#ifndef GRAPH_SHORTEST_PATHS_HPP
#  define GRAPH_SHORTEST_PATHS_HPP

namespace std::graph {

/**
 * @ingroup graph_algorithms
 * @brief Requirements for an edge value function: evf(uv) -> value.
 * 
 * @tparam G Graph type.
 * @tparam F Function type.
*/
template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;

/**
 * @ingroup graph_algorithms
 * @brief A concept that describes a queueable container. It reflects the capabilities of
 * std::queue and std::priority_queue.
 * 
 * Use of this defines the required capabilities, including those of containers in std and
 * the caller's domain.
*/
template <class Q>
concept queueable = requires(Q&& q, typename Q::value_type value) {
                      typename Q::value_type;
                      typename Q::size_type;
                      typename Q::reference;

                      { q.top() };
                      { q.push(value) };
                      { q.pop() };
                      { q.empty() };
                      { q.size() };
                    };

/**
 * @brief Internal sentinal type to indicate that predecessor evaluation isn't needed when calling 
 * dijkstra_shortest_paths() by dijkstra_shortest_distances().
 */
struct _null_predecessor {};
using _null_predecessor_range_type = ranges::subrange<ranges::iterator_t<vector<_null_predecessor>>>;

/**
 * @brief The internal struct used by the priorty queue in dijkstra_shortest_paths() and
 * dijkstra_shortest_distances().
 * 
 * @tparam G Graph type.
 * @tparam W Weight type.
*/
template <class G, class W>
requires is_default_constructible_v<vertex_id_t<G>> && is_default_constructible_v<W>
struct weighted_vertex {
  vertex_id_t<G> vertex_id = vertex_id_t<G>();
  W              weight    = W();

  constexpr auto operator<=>(const weighted_vertex& rhs) const noexcept {
    if (vertex_id < rhs.vertex_id)
      return -1;
    else if (vertex_id > rhs.vertex_id)
      return +1;
    else
      return 0;
  }
};

/**
 * @ingroup graph_algorithms
 * @brief Returns a value to define an invalid distance type used to initialize distance values
 * in the distance range before calling dijkstra_shortest_paths() and dijkstra_shortest_distances().
 * 
 * @tparam G             The graph type.
 * @tparam DistanceValue The type of the distance for graph type G.
 * 
 * @return A unique invalid value to indicate that a value is invalid, or undefined.
*/
template <adjacency_list G, class DistanceValue>
auto dijkstra_invalid_distance() {
  return numeric_limits<DistanceValue>::max();
}

/**
 * @ingroup graph_algorithms
 * @brief Find the shortest paths and distances to vertices reachable from a single seed vertex for
 * non-negative weights.
 * 
 * Complexity: O(|E| + |V|log|V|)
 * 
 * @tparam G                The graph type.
 * @tparam DistanceRange    The distance range type.
 * @tparam PredecessorRange The predecessor range type.
 * @tparam EVF              The edge value function that returns the weight of an edge.
 * @tparam Q                The priority queue type.
 * 
 * @param g           The graph.
 * @param seed        The single source vertex to start the search.
 * @param distance    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                    must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                    dijkstra_invalid_distance().
 * @param predecessor [inout] The predecessor[uid] of vertex_id uid in path. predecessor[seed] == seed. The
 *                    caller must assure size(predecessor) >= size(vertices(g)). It is only valid when
 *                    distance[uid] != dijkstra_invalid_distance().
 * @param weight_fn   The weight function object used to determine the distance between
 *                    vertices on an edge. Return values must be non-negative. The default return value is 1.
 * @param q           The priority queue used internally by dijkstra_shortest_paths.
 */
template <adjacency_list              G,
          ranges::random_access_range DistanceRange,
          ranges::random_access_range PredecessorRange,
          class EVF   = std::function<ranges::range_value_t<DistanceRange>(edge_reference_t<G>)>,
          queueable Q = priority_queue<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>,
                                       vector<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>>,
                                       greater<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>>>>
requires ranges::random_access_range<vertex_range_t<G>> &&        //
         integral<vertex_id_t<G>> &&                              //
         is_arithmetic_v<ranges::range_value_t<DistanceRange>> && //
         //convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessor>> && //
         edge_weight_function<G, EVF>
constexpr void dijkstra_shortest_paths(
      G&&               g,
      vertex_id_t<G>    seed,
      DistanceRange&    distance,
      PredecessorRange& predecessor,
      EVF               weight_fn = [](edge_reference_t<G> uv) { return ranges::range_value_t<DistanceRange>(1); },
      Q                 q         = Q()) {
  // init distances
  using distance_type = ranges::range_value_t<DistanceRange>;
  assert(size(distance) >= size(vertices(g)));
  assert(seed >= 0 && static_cast<size_t>(seed) < size(vertices(g)));
  distance[seed] = distance_type();

  // init predecessor
  if constexpr (!is_same_v<PredecessorRange, _null_predecessor_range_type>)
    predecessor[seed] = seed;

  // Remark(Andrew): CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  q.push({seed, distance[seed]});
  while (!q.empty()) {
    auto uid = q.top().vertex_id;
    q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight_fn)) {
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        if constexpr (!is_same_v<PredecessorRange, _null_predecessor_range_type>)
          predecessor[vid] = uid;
        q.push({vid, distance[vid]});
      }
    }
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Find the shortest paths and distances to vertices reachable from a single seed vertex for
 * non-negative weights.
 * 
 * Complexity: O(|E| + |V|log|V|)
 * 
 * @tparam G          The graph type.
 * @tparam Distance   The distance range type.
 * @tparam EVF        The edge value function that returns the weight of an edge.
 * @tparam Q          The priority queue type.
 * 
 * @param g           The graph.
 * @param seed        The single source vertex to start the search.
 * @param distance    [inout] The distance[uid] of vertex_id uid from seed. distance[seed] == 0. The caller
 *                    must assure size(distance) >= size(vertices(g)) and set the values to be 
 *                    dijkstra_invalid_distance().
 * @param weight_fn   The weight function object used to determine the distance between
 *                    vertices on an edge. Return values must be non-negative. The default return value is 1.
 * @param q           The priority queue used internally by dijkstra_shortest_paths.
 */
template <adjacency_list              G,
          ranges::random_access_range DistanceRange,
          class EVF   = std::function<ranges::range_value_t<DistanceRange>(edge_reference_t<G>)>,
          queueable Q = priority_queue<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>,
                                       vector<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>>,
                                       greater<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>>>>
requires ranges::random_access_range<vertex_range_t<G>> &&        //
         integral<vertex_id_t<G>> &&                              //
         is_arithmetic_v<ranges::range_value_t<DistanceRange>> && //
         edge_weight_function<G, EVF>
constexpr void dijkstra_shortest_distances(
      G&&            g,        // graph
      vertex_id_t<G> seed,     // starting vertex_id
      DistanceRange& distance, // out: distance[uid] of vertex_id uid from seed
      EVF            weight_fn =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<DistanceRange>(1); }, // default weight(uv) -> 1
      Q q = Q()                                                                             //
) {
  _null_predecessor_range_type predecessor; // don't evaluate predecessor
  dijkstra_shortest_paths(g, seed, distance, predecessor, weight_fn, q);
}


} // namespace std::graph

#endif //GRAPH_SHORTEST_PATHS_HPP
