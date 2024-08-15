/**
 * @file dijkstra_shortest_paths.hpp
 * 
 * @brief Single-Source Shortest paths and shortest distances algorithms using Dijkstra's algorithm.
 * 
 * @copyright Copyright (c) 2024
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
#include <algorithm>
#include <fmt/format.h>
#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

#define NEW_DIJKSTRA

#ifndef GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
#  define GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP

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

template <adjacency_list G>
class dijkstra_visitor_base {
  // Types
public:
  using graph_type             = G;
  using vertex_desc_type       = vertex_descriptor<vertex_id_t<G>, vertex_reference_t<G>, void>;
  using sourced_edge_desc_type = edge_descriptor<vertex_id_t<G>, true, edge_reference_t<G>, void>;

  // Visitor Functions
public:
  // vertex visitor functions
  constexpr void on_initialize_vertex(vertex_desc_type&& vdesc) {}
  constexpr void on_discover_vertex(vertex_desc_type&& vdesc) {}
  constexpr void on_examine_vertex(vertex_desc_type&& vdesc) {}
  constexpr void on_finish_vertex(vertex_desc_type&& vdesc) {}

  // edge visitor functions
  constexpr void on_examine_edge(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_relaxed(sourced_edge_desc_type&& edesc) {}
  constexpr void on_edge_not_relaxed(sourced_edge_desc_type&& edesc) {}
};

template <class G, class Visitor>
concept dijkstra_visitor = //is_arithmetic<typename Visitor::distance_type> &&
      requires(Visitor& v, Visitor::vertex_desc_type& vdesc, Visitor::sourced_edge_desc_type& edesc) {
        //typename Visitor::distance_type;

        { v.on_initialize_vertex(vdesc) };
        { v.on_discover_vertex(vdesc) };
        { v.on_examine_vertex(vdesc) };
        { v.on_finish_vertex(vdesc) };

        { v.on_examine_edge(edesc) };
        { v.on_edge_relaxed(edesc) };
        { v.on_edge_not_relaxed(edesc) };
      };

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
 * @brief Dijkstra's single-source shortest paths algorithm with a visitor.
 * 
 * The implementation was taken from boost::graph dijkstra_shortes_paths_no_init.
 * 
 * Pre-conditions:
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
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 */
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = dijkstra_visitor_base<G>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
// && dijkstra_visitor<G, Visitor>
void dijkstra_shortest_paths(
      G&                   g,
      const vertex_id_t<G> source,
      Distances&           distances,
      Predecessors&        predecessor,
      WF&&                 weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = dijkstra_visitor_base<G>(),
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

  auto qcompare = [&distances](id_type a, id_type b) {
    return distances[static_cast<size_t>(a)] > distances[static_cast<size_t>(b)];
  };
  using Queue = std::priority_queue<id_type, vector<id_type>, decltype(qcompare)>;
  Queue queue(qcompare);

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  for (id_type uid = 0; uid < N; ++uid) {
    visitor.on_initialize_vertex({uid, *find_vertex(g, uid)});
  }

  // Seed the queue with the initial vertex
  if (source >= N || source < 0) {
    throw out_of_range(fmt::format("dijkstra_shortest_paths: source vertex id of '{}' is out of range", source));
  }
  queue.push(source);
  distances[static_cast<size_t>(source)] = zero; // mark source as discovered
  visitor.on_discover_vertex({source, *find_vertex(g, source)});

  // Main loop to process the queue
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
    visitor.on_examine_vertex({uid, *find_vertex(g, uid)});

    // Process all outgoing edges from the current vertex
    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
      visitor.on_examine_edge({uid, vid, uv});

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (is_signed_v<weight_type>) {
        if (w < zero) {
          throw graph_error(
                fmt::format("dijkstra_shortest_paths: invalid negative edge weight of '{}' encountered", w));
        }
      }

      const bool is_neighbor_undiscovered = (distances[static_cast<size_t>(vid)] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          visitor.on_edge_relaxed({uid, vid, uv});
          visitor.on_discover_vertex({vid, *find_vertex(g, vid)});
          queue.push(vid);
        } else {
          // This is an indicator of a bug in the algorithm and should be investigated.
          throw logic_error("dijkstra_shortest_paths: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          visitor.on_edge_relaxed({uid, vid, uv});
          queue.push(vid); // re-enqueue vid to re-evaluate its neighbors with a shorter path
        } else {
          visitor.on_edge_not_relaxed({uid, vid, uv});
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be called twice (or more) on the same vertex.
    visitor.on_finish_vertex({uid, *find_vertex(g, uid)});
  } // while(!queue.empty())
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
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 */
template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = dijkstra_visitor_base<G>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
//&& dijkstra_visitor<G, Visitor>
void dijkstra_shortest_distances(
      G&                   g,
      const vertex_id_t<G> source,
      Distances&           distances,
      WF&&                 weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = dijkstra_visitor_base<G>(),
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, source, distances, _null_predecessors, forward<WF>(weight), std::forward<Visitor>(visitor),
                          std::forward<Compare>(compare), std::forward<Combine>(combine));
}

} // namespace std::graph

#endif // GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
