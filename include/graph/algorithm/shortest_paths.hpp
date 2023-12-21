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
#include "graph/graph.hpp"

#ifndef GRAPH_SHORTEST_PATHS_HPP
#  define GRAPH_SHORTEST_PATHS_HPP

namespace std::graph {

#  if 1
template <class G, class WF, class DistanceValue, class Compare, class Combine>
concept basic_edge_weight_function = // e.g. weight(uv)
      is_arithmetic_v<DistanceValue> && strict_weak_order<Compare, DistanceValue, DistanceValue> &&
      assignable_from<add_lvalue_reference_t<DistanceValue>,
                      invoke_result_t<Combine, DistanceValue, invoke_result_t<WF, edge_reference_t<G>>>>;

template <class G, class WF, class DistanceValue>
concept edge_weight_function = // e.g. weight(uv)
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
*/
class null_range_type : public std::vector<size_t> {
  using T         = size_t;
  using Allocator = std::allocator<T>;
  using Base      = std::vector<T, Allocator>;

public:
  null_range_type() noexcept(noexcept(Allocator())) = default;
  explicit null_range_type(const Allocator& alloc) noexcept {}
  null_range_type(Base::size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  explicit null_range_type(Base::size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  null_range_type(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  null_range_type(const null_range_type& other) : Base() {}
  null_range_type(const null_range_type& other, const Allocator& alloc) {}
  null_range_type(null_range_type&& other) noexcept {}
  null_range_type(null_range_type&& other, const Allocator& alloc) {}
  null_range_type(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
};

inline static null_range_type null_predecessors;


/**
 * @brief Dijkstra Shortest Paths common algorithm.
 * 
 * @tparam G Graph type.
 * @tparam Distances Distances type.
 * @tparam Predecessors Vertex predecessors type.
 * @tparam WF Weight function type.
 * 
 * @param g The graph.
 * @param source       The seed vertex to find the shortest paths for.
 * @param distances    The distances for each vertex to source. If distances[i]==shortest_path_invalid_distance()
 *                     then there is no path to source.
 * @param predecessors The previous vertex in the shortest path to source. If vertex[i]==i then i==source.
 * @param weight       The weight function. It must return non-negative values.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF = function<ranges::range_value_t<Distances>(edge_reference_t<G>)>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> && //
         edge_weight_function<G, WF, ranges::range_value_t<Distances>>
void dijkstra_shortest_paths(
      G&&            g,            // graph
      vertex_id_t<G> source,       // starting vertex_id
      Distances&     distances,    // out: Distances[uid] of uid from source
      Predecessors&  predecessors, // out: predecessor[uid] of uid in path
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }) // default weight(uv) -> 1
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  // Remark(Andrew): Do we want to allow null Distances?  What about if both are null?  Still run algorithm at all?

  size_t N(size(vertices(g))); // Question(Andrew): Do we want a num_vertices(g) CPO?
  assert(source < N && source >= 0);

  std::ranges::fill(distances, numeric_limits<weight_type>::max());
  distances[source] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  // Remark(Andrew):  We may want to make this parameterizable as different types of heaps give different performance
  // But std::priority_queue is probably reasonable for now
  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  priority_queue<weighted_vertex, vector<weighted_vertex>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push({source, distances[source]});

  while (!Q.empty()) {

    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) { // Remark(Andrew): +1
      //weight_type w = weight(uv);
      if (distances[uid] + w < distances[vid]) {
        distances[vid] = distances[uid] + w;
        if constexpr (!is_same_v<Predecessors, null_range_type>)
          predecessors[vid] = uid;
        Q.push({vid, distances[vid]});
      }
    }
  }
}

/**
 * @brief Dijkstra Shortest Paths common algorithm.
 * 
 * @tparam G Graph type.
 * @tparam Distances Distances type.
 * @tparam WF Weight function type.
 * 
 * @param g            The graph.
 * @param source       The seed vertex to find the shortest paths for.
 * @param distances    The distances for each vertex to source. If distances[i]==shortest_path_invalid_distance()
 *                     then there is no path to source.
 * @param weight       The weight function. It must return non-negative values.
*/
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          class WF = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         edge_weight_function<G, WF, ranges::range_value_t<Distances>>
void dijkstra_shortest_paths(
      G&&            g,         // graph
      vertex_id_t<G> seed,      // starting vertex_id
      Distances&     distances, // out: Distances[uid] of uid from seed
      WF&&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }) // default weight(uv) -> 1
{
  dijkstra_shortest_paths(g, seed, distances, null_predecessors,
                          forward<WF>(weight)); // default weight(uv) -> 1
}

#  else
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
         //convertible_to<vertex_id_t<G>, ranges::range_value_t<PredecessorRange>> && //
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
#  endif //0

} // namespace std::graph

#endif //GRAPH_SHORTEST_PATHS_HPP
