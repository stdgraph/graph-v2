/**
 * @file shortest_paths.hpp
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
 */

#include "graph/graph.hpp"
#include "graph/views/edgelist.hpp"

#ifndef GRAPH_KRUSKAL_HPP
#  define GRAPH_KRUSKAL_HPP

namespace std::graph {

template<typename vertex_id_t>
vertex_id_t disjoint_find(
    std::vector<std::pair<vertex_id_t, size_t>>& subsets,
    vertex_id_t                                  vtx
) {
  vertex_id_t parent = subsets[vtx].first;
  while (parent != subsets[parent].first) {
    parent = subsets[parent].first;
  }
  while (vtx != parent) {
    vtx                = subsets[vtx].first;
    subsets[vtx].first = parent;
  }

  return parent;
}

template<typename vertex_id_t>
void disjoint_union(
    std::vector<std::pair<vertex_id_t, size_t>>& subsets,
    vertex_id_t                                  u,
    vertex_id_t                                  v
) {
  vertex_id_t u_root = disjoint_find(subsets, u);
  vertex_id_t v_root = disjoint_find(subsets, v);

  if (subsets[u_root].second < subsets[v_root].second)
    subsets[u_root].first = v_root;

  else if (subsets[u_root].second > subsets[v_root].second)
    subsets[v_root].first = u_root;

  else {
    subsets[v_root].first = u_root;
    subsets[u_root].second++;
  }
}

template<typename vertex_id_t>
bool disjoint_union_find(
    std::vector<std::pair<vertex_id_t, size_t>>& subsets,
    vertex_id_t                                  u,
    vertex_id_t                                  v
) {
  vertex_id_t u_root = disjoint_find(subsets, u);
  vertex_id_t v_root = disjoint_find(subsets, v);

  if (u_root != v_root) {

    if (subsets[u_root].second < subsets[v_root].second)
      subsets[u_root].first = v_root;

    else if (subsets[u_root].second > subsets[v_root].second)
      subsets[v_root].first = u_root;

    else {
      subsets[v_root].first = u_root;
      subsets[u_root].second++;
    }

    return true;
  }

  return false;
}


template<adjacency_list G, class Iter>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
  std::output_iterator<Iter, std::tuple<vertex_id_t<G>,vertex_id_t<G>,edge_t<G>>>
void kruskal(
    G&&  g, // graph
    Iter t  // out: spanning tree edge list
) {
  kruskal(g, t, [](auto&& i, auto&& j){ return i < j; });
}
template<adjacency_list G, class Iter, typename Compare>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
  std::output_iterator<Iter, std::tuple<vertex_id_t<G>, vertex_id_t<G>, edge_t<G>>>
void kruskal(
    G&&     g,      // graph
    Iter    t,      // out: spanning tree edge list
    Compare compare // edge value comparitor
) {

  size_t N(size(vertices(g)));
  size_t M = 0;
  for (auto && [uid, u] : std::graph::views::vertexlist(g)) {
    M += std::graph::degree(g, u);
  }
  std::vector<std::tuple<vertex_id_t<G>,vertex_id_t<G>,edge_t<G>>> E, T;
  E.reserve(M);

  auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
  for (auto && [uid,vid,uv] : std::graph::views::edgelist(g)) {
    E.push_back(make_tuple(uid,vid,uv));
  }
  
  auto comp = [&](auto&& i, auto &&j)
    { return compare(evf(std::get<2>(i)), evf(std::get<2>(j))); };
  std::sort(E.begin(), E.end(), comp);

  std::vector<std::pair<vertex_id_t<G>, size_t>> subsets(N);
  for (auto && [uid, u] : std::graph::views::vertexlist(g)) {
    subsets[uid].first  = uid;
    subsets[uid].second = 0;
  }

  for (auto e : E) {
    vertex_id_t<G> u = std::get<0>(e);
    vertex_id_t<G> v = std::get<1>(e);

    if (disjoint_union_find(subsets, u, v)) {
      *t++ = e;
    }
  }
}
} // namespace std::graph

#endif //GRAPH_KRUSKAL_HPP
