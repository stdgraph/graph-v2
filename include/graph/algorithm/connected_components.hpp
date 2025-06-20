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
#include <random>

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
  size_t N(size(vertices(g)));
  using CT = typename std::decay<decltype(*component.begin())>::type;
  std::fill(component.begin(), component.end(), std::numeric_limits<CT>::max());

  std::stack<vertex_id_t<G>> S;
  CT                         cid = 0;
  for (vertex_id_t<G> uid = 0; uid < N; ++uid) {
    if (component[uid] < std::numeric_limits<CT>::max()) {
      continue;
    }

    if (!size(edges(g, uid))) {
      component[uid] = cid++;
      continue;
    }

    component[uid] = cid;
    S.push(uid);
    while (!S.empty()) {
      auto vid = S.top();
      S.pop();
      for (auto&& [wid, vw] : views::incidence(g, vid)) {
        if (component[wid] == std::numeric_limits<CT>::max()) {
          component[wid] = cid;
          S.push(wid);
        }
      }
    }
    ++cid;
  }
  return cid;
}

template <typename vertex_id_t, random_access_range Component>
static void link(vertex_id_t u, vertex_id_t v, Component& component) {
  vertex_id_t p1 = component[u];
  vertex_id_t p2 = component[v];

  while (p1 != p2) {
    vertex_id_t high   = std::max(p1, p2);
    vertex_id_t low    = p1 + (p2 - high);
    vertex_id_t p_high = component[high];
    if (p_high == low)
      break;
    if (p_high == high) {
      if (component[high] == high) {
        component[high] = low;
        break;
      } else {
        high = low;
      }
    }
    p1 = component[p_high];
    p2 = component[low];
  }
}

template <random_access_range Component>
static void compress(Component& component) {
  for (size_t i = 0; i < component.size(); ++i) {
    if (component[i] != component[component[i]]) {
      component[i] = component[component[i]];
    }
  }
}

template <typename vertex_id_t, random_access_range Component>
static vertex_id_t sample_frequent_element(Component& component, size_t num_samples = 1024) {
  std::unordered_map<vertex_id_t, int>       counts(32);
  std::mt19937                               gen;
  std::uniform_int_distribution<vertex_id_t> distribution(0, component.size() - 1);

  for (size_t i = 0; i < num_samples; ++i) {
    vertex_id_t sample = distribution(gen);
    counts[component[sample]]++;
  }

  auto&& [num, count] = *std::max_element(counts.begin(), counts.end(),
                                          [](auto&& a, auto&& b) { return std::get<1>(a) < std::get<1>(b); });
  return num;
}

template <adjacency_list G, random_access_range Component>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         std::convertible_to<range_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, range_value_t<Component>>
void afforest(G&&          g,         // graph
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  size_t N(size(vertices(g)));
  std::iota(component.begin(), component.end(), 0);

  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r);
        link(uid, target_id(g, *it), component);
      }
    }
    compress(component);
  }

  vertex_id_t<G> c = sample_frequent_element<vertex_id_t<G>>(component);

  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (component[uid] == c) {
      continue;
    }
    if (neighbor_rounds < edges(g, uid).size()) {
      auto it = edges(g, u).begin();
      std::advance(it, neighbor_rounds);
      for (; it != edges(g, u).end(); ++it) {
        link(uid, target_id(g, *it), component);
      }
    }
  }

  compress(component);
}

template <adjacency_list G, adjacency_list GT, random_access_range Component>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         std::convertible_to<range_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, range_value_t<Component>>
void afforest(G&&          g,         // graph
              GT&&         g_t,       // graph transpose
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  size_t N(size(vertices(g)));
  std::iota(component.begin(), component.end(), 0);

  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r);
        link(uid, target_id(g, *it), component);
      }
    }
    compress(component);
  }

  vertex_id_t<G> c = sample_frequent_element<vertex_id_t<G>>(component);

  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (component[uid] == c) {
      continue;
    }
    if (neighbor_rounds < edges(g, uid).size()) {
      auto it = edges(g, u).begin();
      std::advance(it, neighbor_rounds);
      for (; it != edges(g, u).end(); ++it) {
        link(uid, target_id(g, *it), component);
      }
    }
    for (auto it2 = edges(g_t, u).begin(); it2 != edges(g_t, u).end(); ++it2) {
      link(uid, target_id(g_t, *it2), component);
    }
  }

  compress(component);
}

} // namespace graph

#endif //GRAPH_CC_HPP
