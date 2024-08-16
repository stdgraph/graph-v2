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
#include <fmt/format.h>

#ifndef GRAPH_BELLMAN_SHORTEST_PATHS_HPP
#  define GRAPH_BELLMAN_SHORTEST_PATHS_HPP

//#  define ENABLE_EVAL_NEG_WEIGHT_CYCLE 1  // for debugging

namespace std::graph {

template <adjacency_list G>
class bellman_visitor_base {
  // Types
public:
  using graph_type             = G;
  using vertex_desc_type       = vertex_descriptor<vertex_id_t<G>, vertex_reference_t<G>, void>;
  using sourced_edge_desc_type = edge_descriptor<vertex_id_t<G>, true, edge_reference_t<G>, void>;

  // Visitor Functions
public:
  // vertex visitor functions
  //constexpr void on_initialize_vertex(vertex_desc_type&& vdesc) {}
  //constexpr void on_discover_vertex(vertex_desc_type&& vdesc) {}
  //constexpr void on_examine_vertex(vertex_desc_type&& vdesc) {}
  //constexpr void on_finish_vertex(vertex_desc_type&& vdesc) {}

  // edge visitor functions
  constexpr void on_examine_edge(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_relaxed(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_not_relaxed(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_minimized(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_not_minimized(sourced_edge_desc_type&& edesc) {}
};

template <class G, class Visitor>
concept bellman_visitor = //is_arithmetic<typename Visitor::distance_type> &&
      requires(Visitor& v, Visitor::vertex_desc_type& vdesc, Visitor::sourced_edge_desc_type& edesc) {
        //typename Visitor::distance_type;

        //{ v.on_initialize_vertex(vdesc) };
        //{ v.on_discover_vertex(vdesc) };
        //{ v.on_examine_vertex(vdesc) };
        //{ v.on_finish_vertex(vdesc) };

        { v.on_examine_edge(edesc) };
        { v.on_edge_relaxed(edesc) };
        { v.on_edge_not_relaxed(edesc) };
        { v.on_edge_minimized(edesc) };
        { v.on_edge_not_minimized(edesc) };
      };

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
 *  - out_of_range if the source vertex is out of range.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance random access range.
 * @tparam Predecessors The predecessor random access range.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Visitor      Visitor type with functions called for different events in the algorithm.
 *                      Function calls are removed by the optimizer if not uesd.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 * 
 * @return true if all edges were minimized, false if a negative weight cycle was found. If an edge
 *         was not minimized, the on_edge_not_minimized event is called.
 */
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = bellman_visitor_base<G>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
// && bellman_visitor<G, Visitor>
bool bellman_ford_shortest_paths(
      G&                   g,
      const vertex_id_t<G> source,
      Distances&           distances,
      Predecessors&        predecessor,
      WF&&                 weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = bellman_visitor_base<G>(),
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using DistanceValue = ranges::range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;

  // relxing the target is the function of reducing the distance from the source to the target
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

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_invalid_distance<DistanceValue>();

  const id_type N = static_cast<id_type>(num_vertices(g));

  if (source >= N || source < 0) {
    throw out_of_range(fmt::format("bellman_fored_shortest_paths: source vertex id of '{}' is out of range", source));
  }

  for (id_type k = 0; k < N; ++k) {
    bool at_least_one_edge_relaxed = false;
    for (auto&& [uid, vid, uv, w] : views::edgelist(g, weight)) {
      visitor.examine_edge({uid, vid, uv});
      if (relax(uv, uid, w)) {
        at_least_one_edge_relaxed = true;
        visitor.edge_relaxed({uid, vid, uv});
      } else
        visitor.edge_not_relaxed({uid, vid, uv});
    }
    if (!at_least_one_edge_relaxed)
      break;
  }

  // Check for negative weight cycles
  for (auto&& [uid, vid, uv, w] : views::edgelist(g, weight)) {
    if (compare(combine(distance[uid], w), distance[vid])) {
      visitor.edge_not_minimized({uid, vid, uv});

#  if ENABLE_EVAL_NEG_WEIGHT_CYCLE // for debugging
      // A negative cycle exists; find a vertex on the cycle
      // (Thanks to Wikipedia's Bellman-Ford article for this code)
      predecessor[vid] = uid;
      vector<bool> visited(num_vertices(g), false);
      visited[vid] = true;
      while (!visited[uid]) {
        visited[uid] = true;
        uid          = predecessor[uid];
      }
      // uid is a vertex in a negative cycle, fill ncycle with the vertices in the cycle
      vector<id_type> ncycle;
      ncycle.push_back(uid);
      vid = predecessor[uid];
      while (vid != uid) {
        ncycle.push_back(vid);
        vid = predecessor[vid]
      }
#  endif
      return false;
    } else {
      visitor.edge_minimized({uid, vid, uv});
    }
  }

  return true;
}

/**
 * @brief Shortest distnaces from a single source using Bellman-Ford's single-source shortest paths 
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
 *                      Function calls are removed by the optimizer if not uesd.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 * 
 * @return true if all edges were minimized, false if a negative weight cycle was found. If an edge
 *         was not minimized, the on_edge_not_minimized event is called.
 */
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = bellman_visitor_base<G>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
//&& bellman_visitor<G, Visitor>
bool bellman_ford_shortest_distances(
      G&                   g,
      const vertex_id_t<G> source,
      Distances&           distances,
      WF&&                 weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = bellman_visitor_base<G>(),
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, source, distances, _null_predecessors, forward<WF>(weight),
                                     std::forward<Visitor>(visitor), std::forward<Compare>(compare),
                                     std::forward<Combine>(combine));
}

} // namespace std::graph

#endif // GRAPH_BELLMAN_SHORTEST_PATHS_HPP
