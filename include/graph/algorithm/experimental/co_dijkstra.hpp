#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/experimental/co_cmn.hpp"

#include <variant>
#include <queue>
#include <algorithm>
#include <ranges>
#include <array>
#include <cassert>

#ifndef GRAPH_CO_DIJKSTRA_CLRS_HPP
#  define GRAPH_CO_DIJKSTRA_CLRS_HPP

namespace graph::experimental {


// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#  define dijkstra_yield_vertex(event, uid)                                                                            \
    if ((event & events) == event)                                                                                     \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, *find_vertex(g_, uid) }                                                          \
      }

#  define dijkstra_yield_edge(event, uid, vid, uv)                                                                     \
    if ((event & events) == event)                                                                                     \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_edge_type { uid, vid, uv }                                                                          \
      }

// distance[x] = 0, distance[x] + w < distance[v] : white, gray
enum class dijkstra_events {
  none              = 0,
  initialize_vertex = 0x0001,
  discover_vertex   = 0x0002,
  examine_vertex    = 0x0004,
  examine_edge      = 0x0008,
  edge_relaxed      = 0x0010,
  edge_not_relaxed  = 0x0020,
  finish_vertex     = 0x0040,
  all               = 0x007F,
};

constexpr dijkstra_events& operator&=(dijkstra_events& lhs, dijkstra_events rhs) noexcept {
  lhs = static_cast<dijkstra_events>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
constexpr dijkstra_events operator&(dijkstra_events lhs, dijkstra_events rhs) noexcept { return (lhs &= rhs); }

constexpr dijkstra_events& operator|=(dijkstra_events& lhs, dijkstra_events rhs) noexcept {
  lhs = static_cast<dijkstra_events>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
constexpr dijkstra_events operator|(dijkstra_events lhs, dijkstra_events rhs) noexcept { return (lhs |= rhs); }


/**
 * @brief dijkstra shortest paths
 * 
 * This is an experimental implementation of Dijkstra's shortest paths.
 * 
 * The implementation was taken from boost::graph (BGL) dijkstra_shortes_paths_no_init.
 * 
 * Exposing the Queue type exposes the internals of the algorithm and requires that the
 * caller support the same semantics if they want to provide their own queue.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance vector.
 * @tparam Predecessors The predecessor vector.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Queue        The queue type. Defaults to a priority_queue.
 */
template <index_adjacency_list G,
          input_range          Seeds,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>,
          class WF      = std::function<range_value_t<Distances>(edge_reference_t<G>)>>
requires convertible_to<range_value_t<Seeds>, vertex_id_t<G>> &&        //
         is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
Generator<bfs_value_t<dijkstra_events, G>> co_dijkstra(
      G&                    g_,
      const dijkstra_events events,
      const Seeds&          seeds,
      Predecessors&         predecessor,
      Distances&            distances,
      WF&       weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  using id_type         = vertex_id_t<G>;
  using DistanceValue   = range_value_t<Distances>;
  using weight_type     = invoke_result_t<WF, edge_reference_t<G>>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<dijkstra_events, G>;

#  if ENABLE_INLINE_RELAX_TARGET == 0
  auto relax_target = [&g_, &predecessor, &distances, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    vertex_id_t<G>      vid = target_id(g_, e);
    const DistanceValue d_u = distances[static_cast<size_t>(uid)];
    const DistanceValue d_v = distances[static_cast<size_t>(vid)];
    //const auto          w_e = weight(e);

    if (compare(combine(d_u, w_e), d_v)) {
      distances[static_cast<size_t>(vid)] = combine(d_u, w_e);
#    ifdef ENABLE_PREDECESSORS
      predecessor[static_cast<size_t>(vid)] = uid;
#    endif
      return true;
    }
    return false;
  };
#  endif

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_infinite_distance<DistanceValue>();

  const id_type N(static_cast<id_type>(num_vertices(g_)));

  auto qcompare = [&distances](id_type a, id_type b) {
    return distances[static_cast<size_t>(a)] > distances[static_cast<size_t>(b)];
  };
  using Queue = std::priority_queue<vertex_id_t<G>, std::vector<vertex_id_t<G>>, decltype(qcompare)>;
  Queue queue(qcompare);

  if ((events & dijkstra_events::initialize_vertex) == dijkstra_events::initialize_vertex) {
    for (id_type uid = 0; uid < N; ++uid) {
      co_yield bfs_value_type{dijkstra_events::initialize_vertex, bfs_vertex_type{uid, *find_vertex(g_, uid)}};
    }
  }

  // Seed the queue with the initial vertice(s)
  for (auto seed : seeds) {
    if (seed >= N || seed < 0) {
      throw graph_error("co_dijkstra: seed vertex out of range");
    }
    queue.push(seed);
    distances[static_cast<size_t>(seed)] = zero; // mark seed as discovered
    dijkstra_yield_vertex(dijkstra_events::discover_vertex, seed);
  }

  // Main loop to process the queue
#  if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  size_t pop_cnt = 0, edge_cnt = 0;
#  endif
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
#  if defined(ENABLE_POP_COUNT)
    ++pop_cnt;
#  endif
#  if defined(ENABLE_EDGE_VISITED_COUNT)
    edge_cnt += size(edges(g_, uid));
#  endif
    dijkstra_yield_vertex(dijkstra_events::examine_vertex, uid);

    for (auto&& [vid, uv, w] : views::incidence(g_, uid, weight)) {
      dijkstra_yield_edge(dijkstra_events::examine_edge, uid, vid, uv);

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (std::is_signed_v<weight_type>) {
        if (w < zero) {
          throw graph_error("co_dijkstra: negative edge weight");
        }
      }

#  if ENABLE_INLINE_RELAX_TARGET
      const DistanceValue d_u                      = distances[uid];
      DistanceValue&      d_v                      = distances[vid];
      const bool          is_neighbor_undiscovered = (d_v == infinite);
      bool                was_edge_relaxed         = false;

      const DistanceValue d_v_new = combine(d_u, w);
      if (compare(d_v_new, d_v)) {
        d_v = d_v_new;
#    ifdef ENABLE_PREDECESSORS
        predecessor[vid] = uid;
#    endif
        was_edge_relaxed = true;
      }
#  else
      const bool is_neighbor_undiscovered = (distances[static_cast<size_t>(vid)] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);
#  endif

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
          dijkstra_yield_vertex(dijkstra_events::discover_vertex, vid);
          queue.push(vid);
        } else {
          throw graph_error("co_dijkstra: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
          queue.push(vid); // re-enqueue vid to re-evaluate its neighbors with a shorter path
        } else {
          dijkstra_yield_edge(dijkstra_events::edge_not_relaxed, uid, vid, uv);
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be call subsequently on the same vertex.
    dijkstra_yield_vertex(dijkstra_events::finish_vertex, uid);
  }

#  if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  fmt::print("dijkstra_with_visitor: pop_cnt = {:L}, edge_cnt = {:L}\n", pop_cnt, edge_cnt);
#  endif
}


// refinements needed
// 1. queue is created 2x

template <index_adjacency_list G,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class Compare    = less<range_value_t<Distances>>,
          class Combine    = plus<range_value_t<Distances>>,
          class WF         = std::function<range_value_t<Distances>(edge_reference_t<G>)>,
          _queueable Queue = std::priority_queue<vertex_id_t<G>,
                                                 std::vector<vertex_id_t<G>>,
                                                 std::greater<vertex_id_t<G>>>>
requires is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
Generator<bfs_value_t<dijkstra_events, G>> co_dijkstra(
      G&                    g_,
      const dijkstra_events events,
      vertex_id_t<G>        seed,
      Predecessors&         predecessor,
      Distances&            distances,
      WF&       weight  = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>(),
      Queue     queue   = Queue()) {
  //subrange seeds{&seed, (&seed + 1)}; // segfault in gcc-13 when iterating over seeds in called co_dijkstra
  std::array<vertex_id_t<G>, 1> seeds{seed};
  return co_dijkstra(g_, events, seeds, predecessor, distances, weight, forward<Compare>(compare),
                     forward<Combine>(combine), queue);
}

} // namespace graph::experimental

#endif // GRAPH_CO_DIJKSTRA_CLRS_HPP
