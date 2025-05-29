/**
 * @file dijkstra_shortest_paths.hpp
 * 
 * @brief Single-Source & multi-source shortest paths & shortest distances algorithms using Dijkstra's algorithm.
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
#include "graph/algorithm/common_shortest_paths.hpp"

#include <queue>
#include <vector>
#include <ranges>
#include <format>

#ifndef GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
#  define GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP

namespace graph {

/**
 * @brief Dijkstra's single-source shortest paths algorithm with a visitor.
 * 
 * The implementation was taken from boost::graph dijkstra_shortes_paths_no_init.
 * 
 * Complexity: O(V * E)
 * 
 * Pre-conditions:
 *  - 0 <= source < num_vertices(g)
 *  - predecessors has been initialized with init_shortest_paths().
 *  - distances has been initialized with init_shortest_paths().
 *  - The weight function must return a value that can be compared (e.g. <) with the Distance 
 *    type and combined (e.g. +) with the Distance type.
 *  - The visitor must implement the dijkstra_visitor concept and is typically derived from
 *    dijkstra_visitor_base.
 * 
 * Throws:
 *  - out_of_range if the source vertex is out of range.
 *  - graph_error if a negative edge weight is encountered.
 *  - logic_error if an edge to a new vertex was not relaxed.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance random access range.
 * @tparam Predecessors The predecessor random access range.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Visitor      Visitor type with functions called for different events in the algorithm.
 *                      Function calls are removed by the optimizer if not uesd.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<distance_type>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         sized_range<Distances> &&                                 //
         sized_range<Predecessors> &&                              //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using distance_type = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;

  // relxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    const id_type       vid = target_id(g, e);
    const distance_type d_u = distances[static_cast<size_t>(uid)];
    const distance_type d_v = distances[static_cast<size_t>(vid)];

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
          std::format("dijkstra_shortest_paths: size of distances of {} is less than the number of vertices {}",
                      size(distances), size(vertices(g))));
  }
  if constexpr (!is_same_v<Predecessors, _null_range_type>) {
    if (size(predecessor) < size(vertices(g))) {
      throw std::out_of_range(
            std::format("dijkstra_shortest_paths: size of predecessor of {} is less than the number of vertices {}",
                        size(predecessor), size(vertices(g))));
    }
  }

  constexpr auto zero     = shortest_path_zero<distance_type>();
  constexpr auto infinite = shortest_path_infinite_distance<distance_type>();

  const id_type N = static_cast<id_type>(num_vertices(g));

  auto qcompare = [&distances](id_type a, id_type b) {
    return distances[static_cast<size_t>(a)] > distances[static_cast<size_t>(b)];
  };
  using Queue = std::priority_queue<id_type, std::vector<id_type>, decltype(qcompare)>;
  Queue queue(qcompare);

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  if constexpr (has_on_initialize_vertex<G, Visitor>) {
    for (id_type uid = 0; uid < N; ++uid) {
      visitor.on_initialize_vertex({uid, *find_vertex(g, uid)});
    }
  }

  // Seed the queue with the initial vertice(s)
  for (auto&& source : sources) {
    if (source >= N || source < 0) {
      throw std::out_of_range(std::format("dijkstra_shortest_paths: source vertex id '{}' is out of range", source));
    }
    queue.push(source);
    distances[static_cast<size_t>(source)] = zero; // mark source as discovered
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex({source, *find_vertex(g, source)});
    }
  }

  // Main loop to process the queue
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
    if constexpr (has_on_examine_vertex<G, Visitor>) {
      visitor.on_examine_vertex({uid, *find_vertex(g, uid)});
    }

    // Process all outgoing edges from the current vertex
    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge({uid, vid, uv});
      }

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (is_signed_v<weight_type>) {
        if (w < zero) {
          throw std::out_of_range(
                std::format("dijkstra_shortest_paths: invalid negative edge weight of '{}' encountered", w));
        }
      }

      const bool is_neighbor_undiscovered = (distances[static_cast<size_t>(vid)] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          if constexpr (has_on_edge_relaxed<G, Visitor>) {
            visitor.on_edge_relaxed({uid, vid, uv});
          }
          if constexpr (has_on_discover_vertex<G, Visitor>) {
            visitor.on_discover_vertex({vid, *find_vertex(g, vid)});
          }
          queue.push(vid);
        } else {
          // This is an indicator of a bug in the algorithm and should be investigated.
          throw std::logic_error(
                "dijkstra_shortest_paths: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          if constexpr (has_on_edge_relaxed<G, Visitor>) {
            visitor.on_edge_relaxed({uid, vid, uv});
          }
          queue.push(vid); // re-enqueue vid to re-evaluate its neighbors with a shorter path
        } else {
          if constexpr (has_on_edge_not_relaxed<G, Visitor>) {
            visitor.on_edge_not_relaxed({uid, vid, uv});
          }
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be called twice (or more) on the same vertex.
    if constexpr (has_on_finish_vertex<G, Visitor>) {
      visitor.on_finish_vertex({uid, *find_vertex(g, uid)});
    }
  } // while(!queue.empty())
}

template <index_adjacency_list G,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> && //
         sized_range<Distances> &&                    //
         sized_range<Predecessors> &&                 //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, predecessor, weight,
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

/**
 * @brief Shortest distnaces from a single source using Dijkstra's single-source shortest paths algorithm 
 *         with a visitor.
 * 
 * This is identical to dijkstra_shortest_paths() except that it does not require a predecessors range.
 * 
 * Pre-conditions:
 *  - distances has been initialized with init_shortest_paths().
 *  - The weight function must return a value that can be compared (e.g. <) with the Distance 
 *    type and combined (e.g. +) with the Distance type.
 *  - The visitor must implement the dijkstra_visitor concept and is typically derived from
 *    dijkstra_visitor_base.
 * 
 * Throws:
 *  - out_of_range if the source vertex is out of range.
 *  - graph_error if a negative edge weight is encountered.
 *  - logic_error if an edge to a new vertex was not relaxed.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance random access range.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Visitor      Visitor type with functions called for different events in the algorithm.
 *                      Function calls are removed by the optimizer if not uesd.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<distance_type>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         sized_range<Distances> &&                                 //
         is_arithmetic_v<range_value_t<Distances>> &&              // 
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, sources, distances, _null_predecessors, forward<WF>(weight), forward<Visitor>(visitor),
                          forward<Compare>(compare), forward<Combine>(combine));
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
constexpr void dijkstra_shortest_distances(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      WF&&      weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, _null_predecessors, forward<WF>(weight),
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
