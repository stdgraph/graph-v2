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
 *   Kevin Deweese
 */

#include "graph/graph.hpp"
#include "graph/edgelist.hpp"
#include "graph/views/edgelist.hpp"
#include <queue>

#ifndef GRAPH_MST_HPP
#  define GRAPH_MST_HPP

namespace std::graph {

template <typename vertex_id_t>
vertex_id_t disjoint_find(vector<pair<vertex_id_t, size_t>>& subsets, vertex_id_t vtx) {
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

template <typename vertex_id_t>
void disjoint_union(vector<pair<vertex_id_t, size_t>>& subsets, vertex_id_t u, vertex_id_t v) {
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

template <typename vertex_id_t>
bool disjoint_union_find(vector<pair<vertex_id_t, size_t>>& subsets, vertex_id_t u, vertex_id_t v) {
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

template <typename ED>
concept has_integral_vertices = requires( ED ed ) {
  requires integral<decltype(ed.source_id)> && integral<decltype(ed.target_id)>;
};

template <typename ED>
concept has_same_vertex_type = requires( ED ed ) {

  requires same_as<decltype(ed.source_id),decltype(ed.target_id)>;
};

template <typename ED>
concept has_edge_value = requires( ED ed ) {
  requires !same_as<decltype(ed.value), void>;
};

/*template<typename T, typename = void>
struct has_edge : false_type { };
template<typename T>
struct has_edge<T, decltype(declval<T>().edge, void())> : true_type { };*/

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning using Kruskal's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam IELR       The input egelist type.
 * @tparam OELR       The output edgelist type.
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 */
template<class IELR, class OELR>
requires ranges::forward_range<IELR> && has_integral_vertices<ranges::range_value_t<IELR>> && has_edge_value<ranges::range_value_t<IELR>> &&
ranges::forward_range<OELR> && has_integral_vertices<ranges::range_value_t<OELR>> && has_edge_value<ranges::range_value_t<OELR>> &&
has_same_vertex_type<ranges::range_value_t<IELR>> && has_same_vertex_type<ranges::range_value_t<OELR>>
void kruskal(IELR&& e, OELR&& t)
{
  kruskal(e, t, [](auto&& i, auto&& j) { return i < j; });
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree using Kruskal's algorithm.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam IELR       The input egelist type.
 * @tparam OELR       The output edgelist type.
 * @tparam CompareOp  The comparison operation type.
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 * @param compare     The comparison operator.
 */
template <class IELR, class OELR, class CompareOp>
requires ranges::forward_range<IELR> && has_integral_vertices<ranges::range_value_t<IELR>> && has_edge_value<ranges::range_value_t<IELR>> &&
ranges::forward_range<OELR> && has_integral_vertices<ranges::range_value_t<OELR>> && has_edge_value<ranges::range_value_t<OELR>> &&
has_same_vertex_type<ranges::range_value_t<IELR>> && has_same_vertex_type<ranges::range_value_t<OELR>>
void kruskal(IELR&&    e,      // graph
             OELR&&    t,      // tree
             CompareOp compare // edge value comparitor
) {
  using edge_descriptor = ranges::range_value_t<IELR>;
  using VId = remove_const<typename edge_descriptor::source_id_type>::type;
  using EV = edge_descriptor::value_type;
  
  vector<tuple<VId, VId, EV>> e_copy;
  ranges::transform( e, back_inserter(e_copy), [](auto&& ed) { return make_tuple(ed.source_id, ed.target_id, ed.value);});
  VId N = 0;
  auto outer_compare = [&](auto&& i, auto&& j) {
    if (get<0>(i) > N) {
      N = get<0>(i);
    }
    if (get<1>(i) > N) {
      N = get<1>(i);
    } 
    return compare(get<2>(i), get<2>(j));
  };
  ranges::sort(e_copy, outer_compare);

  vector<pair<VId, size_t>> subsets(N+1);
  for (VId uid = 0; uid < N; ++uid) {
    subsets[uid].first  = uid;
    subsets[uid].second = 0;
  }

  t.reserve(N);  
  for (auto && [uid, vid, val] : e_copy) {
    if (disjoint_union_find(subsets, uid, vid)) {
      t.push_back(ranges::range_value_t<OELR>());
      t.back().source_id = uid;
      t.back().target_id = vid;
      t.back().value = val;
    }
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree using Kruskal's algorithm modifying input edgelist.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam IELR       The input egelist type.
 * @tparam OELR       The output edgelist type.
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 */
template<class IELR, class OELR>
requires ranges::forward_range<IELR> && has_integral_vertices<ranges::range_value_t<IELR>> && has_edge_value<ranges::range_value_t<IELR>> &&
ranges::forward_range<OELR> && has_integral_vertices<ranges::range_value_t<OELR>> && has_edge_value<ranges::range_value_t<OELR>> &&
has_same_vertex_type<ranges::range_value_t<IELR>> && has_same_vertex_type<ranges::range_value_t<OELR>> &&
permutable<ranges::iterator_t<IELR>>
void inplace_kruskal(IELR&& e, OELR&& t)
{
  inplace_kruskal(e, t, [](auto&& i, auto&& j) { return i < j; });
}

/**
 * @ingroup graph_algorithms
 * @brief Find the minimum weight spanning tree using Kruskal's algorithm modifying input edgelist.
 * 
 * Complexity: O(|E|log|V|)
 * 
 * @tparam IELR       The input egelist type.
 * @tparam OELR       The output edgelist type.
 * @tparam CompareOp  The comparison operation type.
 * 
 * @param e           The input edgelist.
 * @param t           The output edgelist containing the tree.
 * @param compare     The comparison operator.
 */
template <class IELR, class OELR, class CompareOp>
requires ranges::forward_range<IELR> && has_integral_vertices<ranges::range_value_t<IELR>> && has_edge_value<ranges::range_value_t<IELR>> &&
ranges::forward_range<OELR> && has_integral_vertices<ranges::range_value_t<OELR>> && has_edge_value<ranges::range_value_t<OELR>> &&
has_same_vertex_type<ranges::range_value_t<IELR>> && has_same_vertex_type<ranges::range_value_t<OELR>> &&
permutable<ranges::iterator_t<IELR>>
void inplace_kruskal(IELR&&    e,       // graph
                     OELR&&    t,       // tree
                     CompareOp compare  // edge value comparitor
) {
  using edge_descriptor = ranges::range_value_t<IELR>;
  using VId = remove_const<typename edge_descriptor::source_id_type>::type;
  
  VId N = 0;
  auto outer_compare = [&](auto&& i, auto&& j) {
    if (i.source_id > N) {
      N = i.source_id;
    }
    if (i.target_id > N) {
      N = i.target_id;
    } 
    return compare(i.value, j.value);
  };
  ranges::sort(e, outer_compare);

  vector<pair<VId, size_t>> subsets(N+1);
  for (VId uid = 0; uid < N; ++uid) {
    subsets[uid].first  = uid;
    subsets[uid].second = 0;
  }

  t.reserve(N);
  for (auto && [uid, vid, val] : e) {
    if (disjoint_union_find(subsets, uid, vid)) {
      t.push_back(ranges::range_value_t<OELR>());
      t.back().source_id = uid;
      t.back().target_id = vid;
      t.back().value = val;
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
template <adjacency_list              G,
          ranges::random_access_range Predecessor,
          ranges::random_access_range Weight>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(G&&            g,           // graph
          Predecessor&   predecessor, // out: predecessor[uid] of uid in tree
          Weight&        weight,      // out: edge value weight[uid] from tree edge uid to predecessor[uid]
          vertex_id_t<G> seed = 0     // seed vtx
) {
  prim(
        g, predecessor, weight, [](auto&& i, auto&& j) { return i < j; },
        numeric_limits<ranges::range_value_t<Weight>>::max(), seed);
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
template <adjacency_list              G,
          ranges::random_access_range Predecessor,
          ranges::random_access_range Weight,
          class CompareOp>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
void prim(G&&                           g,           // graph
          Predecessor&                  predecessor, // out: predecessor[uid] of uid in tree
          Weight&                       weight,    // out: edge value weight[uid] from tree edge uid to predecessor[uid]
          CompareOp                     compare,   // edge value comparitor
          ranges::range_value_t<Weight> init_dist, // initial distance
          vertex_id_t<G>                seed = 0   // seed vtx
) {
  typedef ranges::range_value_t<Weight> EV;
  size_t                                N(size(vertices(g)));
  vector<EV>                            distance(N, init_dist);
  distance[seed]    = 0;
  predecessor[seed] = seed;

  using weighted_vertex = tuple<vertex_id_t<G>, EV>;

  auto evf           = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
  auto outer_compare = [&](auto&& i, auto&& j) { return compare(get<1>(i), get<1>(j)); };

  priority_queue<weighted_vertex, vector<weighted_vertex>, decltype(outer_compare)> Q(outer_compare);
  Q.push({seed, distance[seed]});
  while (!Q.empty()) {
    auto uid = get<0>(Q.top());
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, evf)) {
      if (compare(w, distance[vid])) {
        distance[vid] = w;
        Q.push({vid, distance[vid]});
        predecessor[vid] = uid;
        weight[vid] = w;
      }
    }
  }
}
} // namespace std::graph

#endif //GRAPH_MST_HPP
