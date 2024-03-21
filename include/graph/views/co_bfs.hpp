#pragma once

#include <variant>

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"

#include <queue>
#include <algorithm>


#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Woverloaded-virtual"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

//#include <boost/cobalt/generator.hpp>

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif


#ifndef GRAPH_CO_BFS_HPP
#  define GRAPH_CO_BFS_HPP

namespace std::graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;


// These events duplicate boost::graph's BFSVisitorConcept
enum class bfs_events : int {
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

// These types comprise the bfs value type, made up of bfs_events and variant<vertex_descriptor, edge_descriptor>.
// monostate is used to indicate that the value is not set and to make it default-constructible.
template <class G>
using bfs_vertex_value_t = vertex_descriptor<vertex_id_t<G>, reference_wrapper<vertex_t<G>>, void>;
template <class G>
using bfs_edge_value_t = edge_descriptor<vertex_id_t<G>, true, reference_wrapper<edge_t<G>>, void>;
template <class G>
using bfs_variant_value_t = variant<monostate, bfs_vertex_value_t<G>, bfs_edge_value_t<G>>;

template <class G>
using bfs_value_t = pair<bfs_events, bfs_variant_value_t<G>>;

// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#  define yield_vertex(event, uid)                                                                                     \
    if ((event & events) != bfs_events::none)                                                                          \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, *find_vertex(g, uid) }                                                           \
      }

#  define yield_edge(event, uid, vid, uv)                                                                              \
    if ((event & events) != bfs_events::none)                                                                          \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_edge_type { uid, vid, uv }                                                                          \
      }

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
Generator<bfs_value_t<G>> co_bfs(G&&            g,    // graph
                                 vertex_id_t<G> seed, // starting vertex_id
                                 bfs_events     events)   // events to stop at
{
  using id_type         = vertex_id_t<G>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<G>;

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


enum class dijkstra_event : int {
  //tree_edge,
  //gray_target,

  initialize_vertex,
  discover_vertex,
  examine_vertex,
  examine_edge,
  edge_relaxed,
  edge_not_relaxed,
  finish_vertex
};

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
