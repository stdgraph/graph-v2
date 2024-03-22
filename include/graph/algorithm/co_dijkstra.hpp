#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/co_cmn.hpp"

#include <variant>
#include <queue>
#include <algorithm>
#include <ranges>

#ifndef GRAPH_CO_DIJKSTRA_CLRS_HPP
#  define GRAPH_CO_DIJKSTRA_CLRS_HPP

namespace std::graph {


// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#  define dijkstra_yield_vertex(event, uid, distance)                                                                  \
    if ((event & events) == event)                                                                                     \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, *find_vertex(g_, uid), distance }                                                \
      }

#  define dijkstra_yield_edge(event, uid, vid, uv)                                                                     \
    if ((event & events) != event)                                                                                     \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_edge_type { uid, vid, uv }                                                                          \
      }

// distance[x] = 0, distance[x] + w < distance[v] : white, gray
enum class dijkstra_events {
  none = 0,
  initialize_vertex,
  discover_vertex,
  examine_vertex,
  examine_edge,
  edge_relaxed,
  edge_not_relaxed,
  finish_vertex
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


template <index_adjacency_list        G,
          ranges::random_access_range Distance,
          ranges::random_access_range Predecessor,
          class Compare = less<ranges::range_value_t<Distance>>,
          class Combine = plus<ranges::range_value_t<Distance>>,
          class WF      = std::function<ranges::range_value_t<Distance>(edge_reference_t<G>)>>
requires is_arithmetic_v<ranges::range_value_t<Distance>> &&                   //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessor>> && //
         basic_edge_weight_function<G, WF, ranges::range_value_t<Distance>, Compare, Combine>
class co_dijkstra {
public:
  using DistanceValue = ranges::range_value_t<Distance>;

  // Construction/Desstruction/Assignment
public:
  co_dijkstra() = delete;
  co_dijkstra(
        G&           g,
        Distance&    distance,
        Predecessor& predecessor,
        WF&&         weight =
              [](edge_reference_t<G> uv) { return ranges::range_value_t<Distance>(1); }, // default weight(uv) -> 1
        Compare&& compare = less<ranges::range_value_t<Distance>>(),
        Combine&& combine = plus<ranges::range_value_t<Distance>>())
        : g_(g)
        , distance_(distance)
        , predecessor_(predecessor)
        , weight_(forward<WF>(weight))
        , compare_(forward<Compare>(compare))
        , combine_(forward<Combine>(combine)) {}

  // Properties
public:
  // Operattions
public:
private:
  bool relax_target(edge_reference_t<G> e, vertex_id_t<G> uid) {
    vertex_id_t<G>      vid = target_id(g_, e);
    const DistanceValue d_u = distance_[uid];
    const DistanceValue d_v = distance_[vid];
    const auto          w_e = weight_(e);

    // From BGL; This may no longer apply since the x87 is long gone
    // The seemingly redundant comparisons after the distance assignments are to
    // ensure that extra floating-point precision in x87 registers does not
    // lead to relax() returning true when the distance did not actually
    // change.
    if (compare_(combine_(d_u, w_e), d_v)) {
      distance_[vid] = combine_(d_u, w_e);
      if (compare_(distance_[vid], d_v)) {
        predecessor_[vid] = uid;
        return true;
      }
    }
    return false;
  }

  // Operators
public:
  Generator<bfs_value_t<dijkstra_events, G, DistanceValue>> operator()(vertex_id_t<G>        seed,
                                                                       const dijkstra_events events) {
    using id_type         = vertex_id_t<G>;
    using bfs_vertex_type = bfs_vertex_value_t<G, DistanceValue>;
    using bfs_edge_type   = bfs_edge_value_t<G>;
    using bfs_value_type  = bfs_value_t<dijkstra_events, G, DistanceValue>;

    constexpr auto zero = DistanceValue{};

    size_t N(num_vertices(g_));
    assert(seed < N && seed >= 0);

    //vector<three_colors> color(N, three_colors::white);
    vector<bool> discovered(N);

    if ((events & dijkstra_events::initialize_vertex) == dijkstra_events::initialize_vertex) {
      for (id_type uid = 0; uid < num_vertices(g_); ++uid) {
        co_yield bfs_value_type{dijkstra_events::initialize_vertex,
                                bfs_vertex_type{uid, *find_vertex(g_, uid), distance_[uid]}};
      }
    }

    using q_compare = decltype([](const id_type& a, const id_type& b) { return a > b; });
    std::priority_queue<id_type, vector<id_type>, q_compare> Q;

    // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard prctice seems to be to enqueue source
    Q.push(seed);
    //color[seed] = three_colors::gray;
    discovered[seed] = true;
    distance_[seed]  = zero;
    dijkstra_yield_vertex(dijkstra_events::discover_vertex, seed, distance_[seed]);

    while (!Q.empty()) {
      const id_type uid = Q.top();
      Q.pop();
      dijkstra_yield_vertex(dijkstra_events::examine_vertex, uid, distance_[uid]);

      for (auto&& [vid, uv] : views::incidence(g_, uid)) {
        dijkstra_yield_edge(dijkstra_events::examine_edge, uid, vid, uv);

        // if weight(uv)==0, vid would be discovered more than once if another path to vid exists
        // could be mitigated by using vector<bool> to flag discovered vertices
        if (!discovered[vid] && distance_[vid] == zero) { //if (color[vid] == three_colors::white) {
          // tree_edge
          bool decreased = relax_target(uv, uid);
          if (decreased)
            dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
          else
            dijkstra_yield_edge(dijkstra_events::edge_not_relaxed, uid, vid, uv);
          //color[vid] = three_colors::gray; // distance_[vid] > 0
          discovered[vid] = true;
          dijkstra_yield_vertex(dijkstra_events::discover_vertex, vid, distance_[vid]);
          Q.push(vid);
        } else {
          // non-tree edge
          //if (color[vid] == three_colors::gray)
          {
            //  DistanceValue old_distance = distance_[vid];
            bool decreased = relax_target(uv, uid);
            if (decreased) {
              Q.push(vid);
              dijkstra_yield_edge(dijkstra_events::edge_relaxed, uid, vid, uv);
            } else {
              dijkstra_yield_edge(dijkstra_events::edge_not_relaxed, uid, vid, uv);
            }
            // Note: black node treated same as gray node. Is that OK? What use case am I missing?
            //    This will cause the same vertex to be processed multiple times in unbalanced
            // graph where a longer number of hops results in a lower accumulated weight.
            // It seems like this might be desired?
            //    ABC required a similar technique to accumulate all contributing costs.
          }
          //else {
          //  //black_target: ignore
          //}
        }
      }
      //color[uid] = three_colors::black;
      dijkstra_yield_vertex(dijkstra_events::finish_vertex, uid, distance_[uid]);
    }
  }

  // Member Variables
private:
  G&           g_;
  Distance&    distance_;
  Predecessor& predecessor_;
  WF           weight_;
  Compare      compare_;
  Combine      combine_;
};

} // namespace std::graph

#endif // GRAPH_CO_DIJKSTRA_CLRS_HPP
