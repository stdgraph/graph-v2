/**
 * @file bellman_ford_shortest_paths.hpp
 * 
 * @brief Single-Source Shortest paths and shortest distances algorithms using Bellman-Ford's algorithm.
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/edgelist.hpp"
#include "graph/algorithm/common_shortest_paths.hpp"

#include <ranges>
#include <optional>
#include <stdexcept>
#include <format>

#ifndef GRAPH_BELLMAN_SHORTEST_PATHS_HPP
#  define GRAPH_BELLMAN_SHORTEST_PATHS_HPP

namespace graph {

/**
 * @brief Get the vertex ids in a negative weight cycle.
 * 
 * If a negative weight cycle exists, the vertex ids in the cycle are output to the output iterator.
 * If no negative weight cycle exists, the output iterator is not modified.
 * 
 * @tparam G            The graph type.
 * @tparam Predecessors The predecessor range type.
 * @tparam OutputIterator The output iterator type.
 * 
 * @param g              The graph.
 * @param predecessor    The predecessor range.
 * @param cycle_vertex_id A vertex id in the negative weight cycle. If no negative weight cycle exists 
 *                       then there will be no vertex id defined.
 * @param out_cycle      The output iterator that the vertex ids in the cycle are output to.
 */
template <index_adjacency_list G, forward_range Predecessors, class OutputIterator>
requires output_iterator<OutputIterator, vertex_id_t<G>>
void find_negative_cycle(G&                              g,
                         const Predecessors&             predecessor,
                         const optional<vertex_id_t<G>>& cycle_vertex_id,
                         OutputIterator                  out_cycle) {
  // Does a negative weight cycle exist?
  if (cycle_vertex_id.has_value()) {
    vertex_id_t<G> uid = cycle_vertex_id.value();
    do {
      *out_cycle++ = uid;
      uid          = predecessor[uid];
    } while (uid != cycle_vertex_id.value());
  }
}


/**
 * @brief Bellman-Ford's single-source shortest paths algorithm with a visitor.
 * 
 * The implementation was taken from boost::graph bellman_ford_shortest_paths.
 * 
 * Complexity: O(V * E)
 * 
 * Pre-conditions:
 *  - 0 <= source < num_vertices(g)
 *  - predecessors has been initialized with init_shortest_paths().
 *  - distances has been initialized with init_shortest_paths().
 *  - The weight function must return a value that can be compared (e.g. <) with the Distance 
 *    type and combined (e.g. +) with the Distance type.
 *  - The visitor must implement the bellman_visitor concept and is typically derived from
 *    bellman_visitor_base.
 * 
 * Throws:
 *  - out_of_range if a source vertex id is out of range.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance random access range.
 * @tparam Predecessors The predecessor random access range.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Visitor      Visitor type with functions called for different events in the algorithm.
 *                      Function calls are removed by the optimizer if not used.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanceValue>.
 * 
 * @return optional<vertex_id_t<G>>, where no vertex id is returned if all edges were minimized.
 *         If an edge was not minimized, the on_edge_not_minimized event is called and a vertex id
 *         in the negative weight cycle is returned.
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&      //
         is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         sized_range<Distances> &&                                      //
         sized_range<Predecessors> &&                                   //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using DistanceValue = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;
  using return_type   = optional<vertex_id_t<G>>;

  // relaxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    id_type             vid = target_id(g, e);
    const DistanceValue d_u = distances[static_cast<size_t>(uid)];
    const DistanceValue d_v = distances[static_cast<size_t>(vid)];

    if (compare(combine(d_u, w_e), d_v)) {
      distances[static_cast<size_t>(vid)] = combine(d_u, w_e);
      if constexpr (!is_same_v<Predecessors, _null_range_type>) {
        predecessor[static_cast<size_t>(vid)] = uid;
      }
      return true;
    }
    return false;
  };

  if (size(distances) < size(vertices(g))) {
    throw std::out_of_range(
          std::format("bellman_ford_shortest_paths: size of distances of {} is less than the number of vertices {}",
                      size(distances), size(vertices(g))));
  }

  if constexpr (!is_same_v<Predecessors, _null_range_type>) {
    if (size(predecessor) < size(vertices(g))) {
      throw std::out_of_range(
            std::format("bellman_ford_shortest_paths: size of predecessor of {} is less than the number of vertices {}",
                        size(predecessor), size(vertices(g))));
    }
  }

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_infinite_distance<DistanceValue>();

  const id_type N = static_cast<id_type>(num_vertices(g));

  // Seed the queue with the initial vertice(s)
  for (auto&& source : sources) {
    if (source >= N || source < 0) {
      throw std::out_of_range(
            std::format("bellman_ford_shortest_paths: source vertex id '{}' is out of range", source));
    }
    distances[static_cast<size_t>(source)] = zero; // mark source as discovered
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex({source, *find_vertex(g, source)});
    }
  }

  // Evaluate the shortest paths
  bool at_least_one_edge_relaxed = false;
  for (id_type k = 0; k < N; ++k) {
    at_least_one_edge_relaxed = false;
    for (auto&& [uid, vid, uv, w] : views::edgelist(g, weight)) {
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge({uid, vid, uv});
      }
      if (relax_target(uv, uid, w)) {
        at_least_one_edge_relaxed = true;
        if constexpr (has_on_edge_relaxed<G, Visitor>) {
          visitor.on_edge_relaxed({uid, vid, uv});
        }
      } else if constexpr (has_on_edge_not_relaxed<G, Visitor>) {
        visitor.on_edge_not_relaxed({uid, vid, uv});
      }
    }
    if (!at_least_one_edge_relaxed)
      break;
  }

  // Check for negative weight cycles
  if (at_least_one_edge_relaxed) {
    for (auto&& [uid, vid, uv, w] : views::edgelist(g, weight)) {
      if (compare(combine(distances[uid], w), distances[vid])) {
        if constexpr (!is_same_v<Predecessors, _null_range_type>) {
          predecessor[vid] = uid; // close the cycle
        }
        if constexpr (has_on_edge_not_minimized<G, Visitor>) {
          visitor.on_edge_not_minimized({uid, vid, uv});
        }
        return return_type(uid);
      } else {
        if constexpr (has_on_edge_minimized<G, Visitor>) {
          visitor.on_edge_minimized({uid, vid, uv});
        }
      }
    }
  }

  return return_type();
}

template <index_adjacency_list G,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         sized_range<Distances> &&                                      //
         sized_range<Predecessors> &&                                   //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distances, predecessor, weight,
                                     forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}


/**
 * @brief Shortest distances from a single source using Bellman-Ford's single-source shortest paths 
 *        algorithm with a visitor.
 * 
 * This is identical to bellman_ford_shortest_paths() except that it does not require a predecessors range.
 * 
 * Complexity: O(V * E)
 * 
 * Pre-conditions:
 *  - distances has been initialized with init_shortest_paths().
 *  - The weight function must return a value that can be compared (e.g. <) with the Distance 
 *    type and combined (e.g. +) with the Distance type.
 *  - The visitor must implement the bellman_visitor concept and is typically derived from
 *    bellman_visitor_base.
 * 
 * Throws:
 *  - out_of_range if the source vertex is out of range.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance random access range.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Visitor      Visitor type with functions called for different events in the algorithm.
 *                      Function calls are removed by the optimizer if not used.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanceValue>.
 * 
 * @return optional<vertex_id_t<G>>, where no vertex id is returned if all edges were minimized.
 *         If an edge was not minimized, the on_edge_not_minimized event is called and a vertex id
 *         in the negative weight cycle is returned.
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         sized_range<Distances> &&                                 //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, sources, distances, _null_predecessors, forward<WF>(weight),
                                     forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

template <index_adjacency_list G,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> && //
         sized_range<Distances> &&                    //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distances, _null_predecessors,
                                     forward<WF>(weight), forward<Visitor>(visitor), forward<Compare>(compare),
                                     forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_BELLMAN_SHORTEST_PATHS_HPP
