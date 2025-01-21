/**
 * @file connected_components.hpp
 * 
 * @brief Single-Source Shortest paths and shortest sistances algorithms using Dijkstra & 
 * Bellman-Ford algorithms.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 *   Kevin Deweese
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/depth_first_search.hpp"
#include "graph/views/breadth_first_search.hpp"
#include <stack>

#ifndef GRAPH_CC_HPP
#  define GRAPH_CC_HPP

namespace graph {

template <adjacency_list      G,
          adjacency_list      GT,
          random_access_range Component>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void kosaraju(G&&        g,        // graph
              GT&&       g_t,      // graph transpose
              Component& component // out: strongly connected component assignment

) {
  size_t            N(size(vertices(g)));
  std::vector<bool> visited(N, false);
  using CT = typename std::decay<decltype(*component.begin())>::type;
  std::fill(component.begin(), component.end(), std::numeric_limits<CT>::max());
  std::vector<vertex_id_t<G>> order;

  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (visited[uid]) {
      continue;
    }
    visited[uid] = true;
    std::stack<vertex_id_t<G>> active;
    active.push(uid);
    auto dfs = graph::views::sourced_edges_depth_first_search(g, uid);
    for (auto&& [vid, wid, vw] : dfs) {
      while (vid != active.top()) {
        order.push_back(active.top());
        active.pop();
      }
      if (visited[wid]) {
        dfs.cancel(cancel_search::cancel_branch);
      } else {
        active.push(wid);
        visited[wid] = true;
      }
    }
    while (!active.empty()) {
      order.push_back(active.top());
      active.pop();
    }
  }

  size_t                    cid = 0;
  std::ranges::reverse_view reverse{order};
  for (auto& uid : reverse) {
    if (component[uid] == std::numeric_limits<CT>::max()) {
      component[uid] = cid;
      vertices_depth_first_search_view<GT> dfs(g_t, uid);
      for (auto&& [vid, v] : dfs) {
        if (component[vid] != std::numeric_limits<CT>::max()) {
          dfs.cancel(cancel_search::cancel_branch);
        } else {
          component[vid] = cid;
        }
      }
      ++cid;
    }
  }
}

template <adjacency_list      G,
          random_access_range Component>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
size_t connected_components(G&&        g,        // graph
                            Component& component // out: connected component assignment
) {
  size_t            N(size(vertices(g)));
  std::vector<bool> visited(N, false);
  using CT = typename std::decay<decltype(*component.begin())>::type;
  std::fill(component.begin(), component.end(), std::numeric_limits<CT>::max());

  CT cid = 0;
  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (visited[uid]) {
      continue;
    }
    visited[uid]   = true;
    component[uid] = cid;
    if (!size(edges(g, u))) {
      ++cid;
      continue;
    }
    vertices_breadth_first_search_view<G, void> bfs(g, uid);
    for (auto&& [vid, v] : bfs) {
      component[vid] = cid;
      visited[vid]   = true;
    }
    ++cid;
  }
  return cid;
}

} // namespace graph

#endif //GRAPH_CC_HPP
