#pragma once

#include "graph/algorithm/co_cmn.hpp"

/*
* coroutine notes
* 1. coroutine function must be the one to call co_yield - can't be called function from the coroutine using simple genrator
*   a. is there a way around this if we implement our own generator-like function?
*   b. does the one in the standard offer more flexibility?
* 2. to offer bgl-like bfs composition, would something like CRTP work, where dijkstra visitor is the base class with eveent handlers?
* 3. Can an abstraction be added to offer optional colors[] processing, where they aren't included for high performance?
*/


#ifndef GRAPH_CO_BFS_HPP
#  define GRAPH_CO_BFS_HPP

namespace std::graph {

// These events duplicate boost::graph's BFSVisitorConcept
enum class bfs_events {
  none              = 0,
  initialize_vertex = 0x0001,
  discover_vertex   = 0x0002, // e.g. white_target
  examine_vertex    = 0x0004,
  examine_edge      = 0x0008,
  tree_edge         = 0x0010,
  non_tree_edge     = 0x0020,
  gray_target       = 0x0040,
  black_target      = 0x0080,
  finish_vertex     = 0x0100,
};
constexpr bfs_events& operator&=(bfs_events& lhs, bfs_events rhs) noexcept {
  lhs = static_cast<bfs_events>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_events  operator&(bfs_events lhs, bfs_events rhs) noexcept { return (lhs &= rhs); }
constexpr bfs_events& operator|=(bfs_events& lhs, bfs_events rhs) noexcept {
  lhs = static_cast<bfs_events>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_events operator|(bfs_events lhs, bfs_events rhs) noexcept { return (lhs |= rhs); }

// co_bfs is a coroutine that yields bfs_value_t<G> values as it traverses the graph, based on the
// events that are passed in. The generator is templated on the graph type, and the events are passed
// in as a bitmask.
//
// The implementation is based on boost::graph::breadth_first_visit.
//
// @param g      The graph to use
// @param seed   The starting vertex_id
// @param events A bitmap of the different events to stop at
//
template <index_adjacency_list G>
Generator<bfs_value_t<bfs_events, G>> co_bfs(G&&            g,    // graph
                                             vertex_id_t<G> seed, // starting vertex_id
                                             bfs_events     events)   // events to stop at
{
  using id_type         = vertex_id_t<G>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<bfs_events, G>;

  size_t N(num_vertices(g));
  assert(seed < N && seed >= 0);

  vector<three_colors> color(N, three_colors::white);

  if ((events & bfs_events::initialize_vertex) != bfs_events::none) {
    for (id_type uid = 0; uid < num_vertices(g); ++uid) {
      yield_vertex(bfs_events::initialize_vertex, uid);
    }
  }
  color[seed] = three_colors::gray;
  yield_vertex(bfs_events::discover_vertex, seed);

  using q_compare = decltype([](const id_type& a, const id_type& b) { return a > b; });
  std::priority_queue<id_type, vector<id_type>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push(seed);

  while (!Q.empty()) {
    const id_type uid = Q.top();
    Q.pop();
    yield_vertex(bfs_events::examine_vertex, uid);

    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      yield_edge(bfs_events::examine_edge, uid, vid, uv);

      if (color[vid] == three_colors::white) {
        color[vid] = three_colors::gray;
        yield_vertex(bfs_events::discover_vertex, vid);
        yield_edge(bfs_events::tree_edge, uid, vid, uv);
        Q.push(vid);
      } else {
        yield_edge(bfs_events::non_tree_edge, uid, vid, uv);
        if (color[vid] == three_colors::gray) {
          yield_vertex(bfs_events::gray_target, vid);
        } else {
          yield_vertex(bfs_events::black_target, vid);
        }
      }
    }
    color[uid] = three_colors::black;
    yield_vertex(bfs_events::finish_vertex, uid);
  }
}


// BGL list of all possible events?
enum event_visitor_enum {
  on_no_event_num,
  on_initialize_vertex_num,
  on_start_vertex_num,
  on_discover_vertex_num,
  on_finish_vertex_num,
  on_examine_vertex_num,
  on_examine_edge_num,
  on_tree_edge_num,
  on_non_tree_edge_num,
  on_gray_target_num,
  on_black_target_num,
  on_forward_or_cross_edge_num,
  on_back_edge_num,
  on_finish_edge_num,
  on_edge_relaxed_num,
  on_edge_not_relaxed_num,
  on_edge_minimized_num,
  on_edge_not_minimized_num
};

} // namespace std::graph

#endif // GRAPH_CO_BFS_HPP
