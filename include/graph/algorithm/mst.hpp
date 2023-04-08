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
#include "graph/edgelist.hpp"
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
 * @tparam E                The input egelist type.
 * @tparam T                The output edgelist type.
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 */
template<edgelist::edgelist E, edgelist::edgelist T>
void kruskal(
    E&& e, // edge list
    T&& t  // out: spanning tree edge list
) {
  kruskal(e, t, [](auto&& i, auto&& j){ return i < j; });
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree from a single seed vertex using Kruskal's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam E             The input edgelist type.
 * @tparam T             The output edgelist type.
 * @tparam CompareOp
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 * @param compare     The comparison operator.
 */
template<edgelist::edgelist E, edgelist::edgelist T, class CompareOp>
void kruskal(
    E&&       e,      // graph
    T&&       t,      // out: spanning tree edge list
    CompareOp compare // edge value comparitor
) {
  using VId = edgelist::vertex_source_id_t<E>;

  auto outer_compare = [&](auto&& i, auto &&j)
    { return compare(std::get<2>(i), std::get<2>(j)); };
  std::sort(e.begin(), e.end(), outer_compare);

  VId N = e.max_vid();
  std::vector<std::pair<VId, size_t>> subsets(N);
  for ( VId uid = 0; uid < N; ++uid ) {
    subsets[uid].first  = uid;
    subsets[uid].second = 0;
  }

  for (auto iter = e.begin(); iter != e.end(); ++iter) {
    VId u = std::get<0>(*iter);
    VId v = std::get<1>(*iter);
    
    if (disjoint_union_find(subsets, u, v)) {
      t.push_back(*iter);
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
template<adjacency_list G, ranges::random_access_range Predecessor,
ranges::random_access_range Weight>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(
    G&&            g,           // graph
    Predecessor&   predecessor, // out: predecessor[uid] of uid in tree
    Weight&        weight,      // out: edge value weight[uid] from tree edge uid to predecessor[uid]
    vertex_id_t<G> seed = 0     // seed vtx
) {
  prim(g, predecessor, weight,
    [](auto&& i, auto&& j){ return i < j; },
    std::numeric_limits<ranges::range_value_t<Weight>>::max(), seed);
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
 * @param init_dist   The initial distance value.
 * @param seed        The single source vertex to start the search.
 */
template<adjacency_list G, ranges::random_access_range Predecessor,
ranges::random_access_range Weight, class CompareOp>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(
    G&&                           g,           // graph
    Predecessor&                  predecessor, // out: predecessor[uid] of uid in tree
    Weight&                       weight,      // out: edge value weight[uid] from tree edge uid to predecessor[uid]
    CompareOp                     compare,     // edge value comparitor
    ranges::range_value_t<Weight> init_dist,   // initial distance
    vertex_id_t<G>                seed = 0     // seed vtx
) {
  typedef ranges::range_value_t<Weight> EV;
  size_t N(size(vertices(g)));
  std::vector<EV> distance(N, init_dist);
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
    
    for (auto&& [vid, uv, w] : views::incidence(g, uid, evf)) {
      if (compare(w, distance[vid])) {
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
