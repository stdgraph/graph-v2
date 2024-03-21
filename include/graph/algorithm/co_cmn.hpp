#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"

#include <variant>
#include <queue>
#include <algorithm>

namespace std::graph {

// These types comprise the bfs value type, made up of bfs_events and variant<vertex_descriptor, edge_descriptor>.
// monostate is used to indicate that the value is not set and to make it default-constructible.
template <class G>
using bfs_vertex_value_t = vertex_descriptor<vertex_id_t<G>, reference_wrapper<vertex_t<G>>, void>;
template <class G>
using bfs_edge_value_t = edge_descriptor<vertex_id_t<G>, true, reference_wrapper<edge_t<G>>, void>;
template <class G>
using bfs_variant_value_t = variant<monostate, bfs_vertex_value_t<G>, bfs_edge_value_t<G>>;

template <class Events, class G>
using bfs_value_t = pair<Events, bfs_variant_value_t<G>>;

// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#define yield_vertex(event, uid)                                                                                       \
  if ((event & events) != bfs_events::none)                                                                            \
    co_yield bfs_value_type {                                                                                          \
      event, bfs_vertex_type { uid, *find_vertex(g, uid) }                                                             \
    }

#define yield_edge(event, uid, vid, uv)                                                                                \
  if ((event & events) != bfs_events::none)                                                                            \
    co_yield bfs_value_type {                                                                                          \
      event, bfs_edge_type { uid, vid, uv }                                                                            \
    }

} // namespace std::graph
