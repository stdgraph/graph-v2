/**
 * @file pagerank.hpp
 * 
 * @brief PageRank (PR) ranking algorithm.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Muhammad Osama
 */

#ifndef GRAPH_PAGERANK_HPP
#define GRAPH_PAGERANK_HPP

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/edgelist.hpp"

#include <execution>
#include <algorithm>
#include <vector>

namespace std::graph {

/**
 * @brief Requirements for an edge value function: evf(uv) -> value.
 * 
 * @tparam G Graph type.
 * @tparam F Function type.
*/
template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;


/**
 * @brief PageRank (PR) algorithm.
 * 
 * @tparam G The graph type.
 * @tparam PageRank The ranks range type.
 * @tparam EVF The edge value function that returns the weight of an edge.
 * 
 * @param g The adjacency graph. 
 * @param page_rank [out] The page rank of each vertex in the graph, accessible through page_rank[uid], where uid is the vertex_id. The caller must assure size(page_rank) >= size(vertices(g)).
 * @param damping_factor The alpha/damping factor (default = 0.85.)
 * @param threshold The error threshold for convergence (default = 1e-4.)
 * @param max_iterations Maximum number of iterations for convergence (default = std::numeric_limits<unsigned int>::max().) 
 * @param weight_fn The edge value function (default returns 1 for each edge value.)
 */
template <adjacency_list              G,
          ranges::random_access_range PageRank,
          class EVF = std::function<ranges::range_value_t<PageRank>(edge_reference_t<G>)>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         is_arithmetic_v<ranges::range_value_t<PageRank>> && edge_weight_function<G, EVF>
void pagerank(
      G&&               g,         // graph
      PageRank&         page_rank, // out: page rank scores
      const double      damping_factor = 0.85,
      const double      threshold      = 1e-4,
      const std::size_t max_iterations = std::numeric_limits<unsigned int>::max(),
      EVF               weight_fn      = [](edge_reference_t<G> uv) { return ranges::range_value_t<PageRank>(1); }) {
  using id_type     = vertex_id_t<G>;
  using weight_type = ranges::range_value_t<PageRank>;

  std::vector<weight_type> plast(size(vertices(g)));
  std::vector<id_type>     degrees(size(vertices(g)));
  // alpha * 1 / (sum of outgoing weights) -- used to determine
  // out of mass spread from src to dst
  std::vector<weight_type> iweights(size(vertices(g)));

  // Initialize the data, pagerank as 1/n_vertices.
  std::ranges::fill(page_rank, 1.0 / size(vertices(g)));

  for (auto&& [uid, u] : views::vertexlist(g)) {
    // Calculate the degree of each vertex.
    size_t edge_cnt = 0;
    for (auto&& uv : edges(g, u)) {
      ++edge_cnt;
    }
    degrees[uid] = edge_cnt;

    // Find the sum of outgoing weights.
    weight_type val = 0;
    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight_fn)) {
      val += w;
    }
    iweights[uid] = (val != 0) ? damping_factor / val : 0;
  }

  size_t iter;
  for (iter = 0; iter < max_iterations; ++iter) {
    // Make a copy of pagerank from previous iteration.
    std::ranges::copy(page_rank.begin(), page_rank.end(), plast.begin());

    // Handle "dangling nodes" (nodes w/ zero outdegree)
    // could skip this if no nodes have sero outdegree
    weight_type dsum = 0.0f;
    for (auto&& [uid, u] : views::vertexlist(g)) {
      dsum += (iweights[uid] == 0) ? damping_factor * page_rank[uid] : 0;
    }
    std::ranges::fill(page_rank, (1 - damping_factor + dsum) / size(vertices(g)));

    double error = 0;
    for (auto&& [uid, vid, uv, val] : views::edgelist(g, weight_fn)) {
      weight_type update = plast[uid] * iweights[uid] * val;
      page_rank[vid] += update;
      error += fabs(page_rank[uid] - plast[uid]);
    }

    // Check for convergence
    if (error < threshold)
      break;
  }
}

} // namespace std::graph

#endif // GRAPH_PAGERANK_HPP
