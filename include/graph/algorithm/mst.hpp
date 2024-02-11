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

template <class VId>
struct disjoint_element {
  VId    id    = VId();
  size_t count = 0;
};

template <class VId>
using disjoint_vector = vector<disjoint_element<VId>>;

template <class VId>
VId disjoint_find(disjoint_vector<VId>& subsets, VId vtx) {
  VId parent = subsets[vtx].id;
  while (parent != subsets[parent].id) {
    parent = subsets[parent].id;
  }
  while (vtx != parent) {
    vtx             = subsets[vtx].id;
    subsets[vtx].id = parent;
  }

  return parent;
}

template <class VId>
void disjoint_union(disjoint_vector<VId>& subsets, VId u, VId v) {
  VId u_root = disjoint_find(subsets, u);
  VId v_root = disjoint_find(subsets, v);

  if (subsets[u_root].count < subsets[v_root].count)
    subsets[u_root].first = v_root;

  else if (subsets[u_root].count > subsets[v_root].count)
    subsets[v_root].first = u_root;

  else {
    subsets[v_root].first = u_root;
    subsets[u_root].count++;
  }
}

template <class VId>
bool disjoint_union_find(disjoint_vector<VId>& subsets, VId u, VId v) {
  VId u_root = disjoint_find(subsets, u);
  VId v_root = disjoint_find(subsets, v);

  if (u_root != v_root) {

    if (subsets[u_root].count < subsets[v_root].count)
      subsets[u_root].id = v_root;

    else if (subsets[u_root].count > subsets[v_root].count)
      subsets[v_root].id = u_root;

    else {
      subsets[v_root].id = u_root;
      subsets[u_root].count++;
    }

    return true;
  }

  return false;
}

//template <typename ED>
//concept has_integral_vertices =
//      requires(ED ed) { requires integral<decltype(ed.source_id)> && integral<decltype(ed.target_id)>; };
//
//template <typename ED>
//concept has_same_vertex_type = requires(ED ed) { requires same_as<decltype(ed.source_id), decltype(ed.target_id)>; };
//
//template <typename ED>
//concept has_edge_value = requires(ED ed) { requires !same_as<decltype(ed.value), void>; };



template<class ELVT>
concept _has_edgelist_value = !is_void_v<typename ELVT::value_type>;

template<class ELVT>
concept _basic_edgelist_type = is_same_v<typename ELVT::target_id_type, typename ELVT::source_id_type>;

template<class ELVT>
concept _basic_index_edgelist_type = _basic_edgelist_type<ELVT> && is_integral_v<typename ELVT::target_id_type>;

template<class ELVT>
concept _edgelist_type = _basic_edgelist_type<ELVT> && _has_edgelist_value<ELVT>;

template<class ELVT>
concept _index_edgelist_type = _basic_index_edgelist_type<ELVT> && _has_edgelist_value<ELVT>;


template<class EL>
concept basic_edgelist_range = ranges::forward_range<EL> && _basic_edgelist_type<ranges::range_value_t<EL>>;

template<class EL>
concept basic_index_edgelist_range = ranges::forward_range<EL> && _basic_index_edgelist_type<ranges::range_value_t<EL>>;

template<class EL>
concept edgelist_range = ranges::forward_range<EL> && _edgelist_type<ranges::range_value_t<EL>>;

template<class EL>
concept index_edgelist_range = ranges::forward_range<EL> && _index_edgelist_type<ranges::range_value_t<EL>>;


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
template <index_edgelist_range IELR, index_edgelist_range OELR>
void kruskal(IELR&& e, OELR&& t) {
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
template <index_edgelist_range IELR, index_edgelist_range OELR, class CompareOp>
void kruskal(IELR&&    e,      // graph
             OELR&&    t,      // tree
             CompareOp compare // edge value comparitor
) {
  using edge_descriptor = ranges::range_value_t<IELR>;
  using VId             = remove_const<typename edge_descriptor::source_id_type>::type;
  using EV              = edge_descriptor::value_type;

  vector<tuple<VId, VId, EV>> e_copy;
  ranges::transform(e, back_inserter(e_copy),
                    [](auto&& ed) { return make_tuple(ed.source_id, ed.target_id, ed.value); });
  VId  N             = 0;
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

  disjoint_vector<VId> subsets(N + 1);
  for (VId uid = 0; uid < N; ++uid) {
    subsets[uid].id    = uid;
    subsets[uid].count = 0;
  }

  t.reserve(N);
  for (auto&& [uid, vid, val] : e_copy) {
    if (disjoint_union_find(subsets, uid, vid)) {
      t.push_back(ranges::range_value_t<OELR>());
      t.back().source_id = uid;
      t.back().target_id = vid;
      t.back().value     = val;
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
template <index_edgelist_range IELR, index_edgelist_range OELR>
requires permutable<ranges::iterator_t<IELR>>
void inplace_kruskal(IELR&& e, OELR&& t) {
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
template <index_edgelist_range IELR, index_edgelist_range OELR, class CompareOp>
requires permutable<ranges::iterator_t<IELR>>
void inplace_kruskal(IELR&&    e,      // graph
                     OELR&&    t,      // tree
                     CompareOp compare // edge value comparitor
) {
  using edge_descriptor = ranges::range_value_t<IELR>;
  using VId             = remove_const<typename edge_descriptor::source_id_type>::type;

  VId  N             = 0;
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

  disjoint_vector<VId> subsets(N + 1);
  for (VId uid = 0; uid < N; ++uid) {
    subsets[uid].id    = uid;
    subsets[uid].count = 0;
  }

  t.reserve(N);
  for (auto&& [uid, vid, val] : e) {
    if (disjoint_union_find(subsets, uid, vid)) {
      t.push_back(ranges::range_value_t<OELR>());
      t.back().source_id = uid;
      t.back().target_id = vid;
      t.back().value     = val;
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
        weight[vid]      = w;
      }
    }
  }
}
} // namespace std::graph

#endif //GRAPH_MST_HPP
