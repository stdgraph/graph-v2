/**
 * @file mst.hpp
 * 
 * @brief Minimum spanning tree using Kruskal's and Prim's algorithms.
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
#include <queue>

#ifndef GRAPH_MST_HPP
#  define GRAPH_MST_HPP

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

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree from a single seed vertex using Prim's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam E                The edge list type.
 * @tparam PredecessorRange The predecessor range type.
 * 
 * @param e           The edge list.
 * @param t           The output edge list iterator containing the tree.
 */
template<class VId, class EV, class E, class Iter>
requires std::output_iterator<Iter, std::tuple<VId,VId,EV>>
void kruskal(
    E&&  e, // edge list
    Iter t  // out: spanning tree edge list
) {
  kruskal<VId, EV>(e, t, [](auto&& i, auto&& j){ return i < j; });
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree from a single seed vertex using Kruskal's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam E                The edge list type.
 * @tparam Iter             The output iteratortype.
 * @tparam CompareOp
 * 
 * @param e           The edge list.
 * @param t           The output edge list iterator containing the tree.
 * @param compare     The comparison operator.
 */
template<class VId, class EV, class E, class Iter, class CompareOp>
requires std::output_iterator<Iter, std::tuple<VId, VId, EV>>
void kruskal(
    E&&     e,      // graph
    Iter    t,      // out: spanning tree edge list
    CompareOp compare // edge value comparitor
) {
  
  auto outer_compare = [&](auto&& i, auto &&j)
    { return compare(std::get<2>(i), std::get<2>(j)); };
  std::sort(e.begin(), e.end(), outer_compare);

  VId N = e.size();
  std::vector<std::pair<VId, size_t>> subsets(N);
  for ( VId uid = 0; uid < N; ++uid ) {
    subsets[uid].first  = uid;
    subsets[uid].second = 0;
  }

  for (auto iter = e.begin(); iter != e.end(); ++iter) {
    VId u = std::get<0>(*iter);
    VId v = std::get<1>(*iter);
    
    if (disjoint_union_find(subsets, u, v)) {
      *t++ = *iter;
    }
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree from a single seed vertex using Prim's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam G                The graph type.
 * @tparam PredecessorRange The predecessor range type.
 * 
 * @param g           The graph.
 * @param predecessor [inout] The predecessor[uid] of vertex_id uid in tree. predecessor[seed] == seed. The
 *                    caller must assure size(predecessor) >= size(vertices(g)).
 * @param seed        The single source vertex to start the search.
 */
template<class EV, adjacency_list G, ranges::random_access_range Predecessor,
ranges::random_access_range Weight>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(
    G&&            g,           // graph
    Predecessor&   predecessor, // out: predecessor[uid] of uid in tree
    Weight&        weight,      // out: edge value weight[uid] from tree edge uid to predecessor[uid]
    vertex_id_t<G> seed = 0     // seed vtx
) {
  prim<EV>(g, predecessor, weight,
    [](auto&& i, auto&& j){ return i < j; }, std::numeric_limits<EV>::max(), seed);
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree from a single seed vertex using Prim's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam G                The graph type.
 * @tparam PredecessorRange The predecessor range type.
 * @tparam CompareOp        The comparison operator. 
 *
 * @param g           The graph.
 * @param predecessor [inout] The predecessor[uid] of vertex_id uid in tree. predecessor[seed] == seed. The
 *                    caller must assure size(predecessor) >= size(vertices(g)).
 * @param compare     The comparison operator.
 * @param init_init   The initial distance value.
 * @param seed        The single source vertex to start the search.
 */
template<class EV, adjacency_list G, ranges::random_access_range Predecessor,
ranges::random_access_range Weight, class CompareOp>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(
    G&&            g,           // graph
    Predecessor&   predecessor, // out: predecessor[uid] of uid in tree
    Weight&        weight,      // out: edge value weight[uid] from tree edge uid to predecessor[uid]
    CompareOp      compare,     // edge value comparitor
    EV             init_dist,   // initial distance
    vertex_id_t<G> seed = 0     // seed vtx
) {
  size_t N(size(vertices(g)));
  std::vector<EV> distance(N, init_dist);
  std::vector<uint8_t> finished(N, false);
  distance[seed] = 0;
  predecessor[seed] = seed;

  using weighted_vertex = std::tuple<vertex_id_t<G>, EV>;

  auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
  auto outer_compare = [&](auto&& i, auto &&j)
    { return compare(std::get<1>(i), std::get<1>(j)); };
  
  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(outer_compare)> Q(outer_compare);
  Q.push({seed, distance[seed]});
  while (!Q.empty()) {
    auto uid = std::get<0>(Q.top());
    Q.pop();

    if (finished[uid]) {
      continue;
    }
    
    for (auto&& [vid, uv, w] : views::incidence(g, uid, evf)) {
      if (!finished[vid] && compare(w, distance[vid])) {
	      distance[vid] = w;
	      Q.push({ vid, distance[vid] });
	      predecessor[vid] = uid;
        weight[vid] = w;
      }
    }
  }
}
} // namespace std::graph

#endif //GRAPH_MST_HPP