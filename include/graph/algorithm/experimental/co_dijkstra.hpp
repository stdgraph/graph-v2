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

namespace std::graph::experimental {


// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#  define dijkstra_yield_vertex(event, uid)                                                                            \
    if ((event & events) == event)                                                                                     \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, *find_vertex(g_, uid) }                                                          \
      }

#  define dijkstra_yield_edge(event, uid, vid, uv)                                                                     \
    if ((event & events) != event)                                                                                     \
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
template <index_adjacency_list        G,
          ranges::input_range         Seeds,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class Compare    = less<ranges::range_value_t<Distances>>,
          class Combine    = plus<ranges::range_value_t<Distances>>,
          class WF         = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          _queueable Queue = priority_queue<vertex_id_t<G>, vector<vertex_id_t<G>>, greater<vertex_id_t<G>>>>
requires convertible_to<ranges::range_value_t<Seeds>, vertex_id_t<G>> &&        //
         is_arithmetic_v<ranges::range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
Generator<bfs_value_t<dijkstra_events, G>> co_dijkstra(
      G&                    g_,
      const dijkstra_events events,
      const Seeds&          seeds,
      Predecessors&         predecessor,
      Distances&            distances,
      WF&                   weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>(),
      Queue     queue   = Queue()) {
  using id_type         = vertex_id_t<G>;
  using DistanceValue   = ranges::range_value_t<Distances>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<dijkstra_events, G>;

  auto relax_target = [&g_, &predecessor, &distances, &weight, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid) -> bool {
    vertex_id_t<G>      vid = target_id(g_, e);
    const DistanceValue d_u = distances[uid];
    const DistanceValue d_v = distances[vid];
    const auto          w_e = weight(e);

    // From BGL; This may no longer apply since the x87 is long gone:
    //
    // The seemingly redundant comparisons after the distance assignments are to
    // ensure that extra floating-point precision in x87 registers does not
    // lead to relax() returning true when the distance did not actually
    // change.
    if (compare(combine(d_u, w_e), d_v)) {
      distances[vid] = combine(d_u, w_e);
      if (compare(distances[vid], d_v)) {
        predecessor[vid] = uid;
        return true;
      }
    }
    return false;
  };

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_invalid_distance<DistanceValue>();

  id_type N(static_cast<id_type>(num_vertices(g_)));

  if ((events & dijkstra_events::initialize_vertex) == dijkstra_events::initialize_vertex) {
    for (id_type uid = 0; uid < static_cast<id_type>(num_vertices(g_)); ++uid) {
      co_yield bfs_value_type{dijkstra_events::initialize_vertex, bfs_vertex_type{uid, *find_vertex(g_, uid)}};
    }
  }

  //using q_compare = decltype([](const id_type& a, const id_type& b) { return a > b; });
  //std::priority_queue<id_type, vector<id_type>, q_compare> Q;

  for (auto seed : seeds) {
    assert(seed < N && seed >= 0);
    queue.push(seed);
    distances[seed] = zero; // mark seed as discovered
    dijkstra_yield_vertex(dijkstra_events::discover_vertex, seed);
  }

  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
    dijkstra_yield_vertex(dijkstra_events::examine_vertex, uid);

    for (auto&& [vid, uv] : views::incidence(g_, uid)) {
      dijkstra_yield_edge(dijkstra_events::examine_edge, uid, vid, uv);

      if (distances[vid] == infinite) {
        // tree_edge
        bool decreased = relax_target(uv, uid);
        if (decreased) {
          dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
        } else {
          dijkstra_yield_edge(dijkstra_events::edge_not_relaxed, uid, vid, uv);
        }
        dijkstra_yield_vertex(dijkstra_events::discover_vertex, vid);
        queue.push(vid);
      } else {
        // non-tree edge
        bool decreased = relax_target(uv, uid);
        if (decreased) {
          dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
          queue.push(vid);
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
}


// refinements needed
// 1. queue is created 2x

template <index_adjacency_list        G,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class Compare    = less<ranges::range_value_t<Distances>>,
          class Combine    = plus<ranges::range_value_t<Distances>>,
          class WF         = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          _queueable Queue = priority_queue<vertex_id_t<G>,
                                            vector<vertex_id_t<G>>,
                                            greater<vertex_id_t<G>>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>, Compare, Combine>
Generator<bfs_value_t<dijkstra_events, G>> co_dijkstra(
      G&                    g_,
      const dijkstra_events events,
      vertex_id_t<G>        seed,
      Predecessors&         predecessor,
      Distances&            distances,
      WF&                   weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>(),
      Queue     queue   = Queue()) {
  //ranges::subrange seeds{&seed, (&seed + 1)}; // segfault in gcc-13 when iterating over seeds in called co_dijkstra
  array<vertex_id_t<G>, 1> seeds{seed};
  return co_dijkstra(g_, events, seeds, predecessor, distances, weight, forward<Compare>(compare),
                     forward<Combine>(combine), queue);
}

} // namespace std::graph::experimental

#endif // GRAPH_CO_DIJKSTRA_CLRS_HPP
