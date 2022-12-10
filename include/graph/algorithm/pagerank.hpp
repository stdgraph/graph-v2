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

template <adjacency_list              G,
          ranges::random_access_range PageRank,
          typename type_t,
          class EVF = std::function<ranges::range_value_t<PageRank>(edge_reference_t<G>)>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         is_arithmetic_v<ranges::range_value_t<PageRank>> && edge_weight_function<G, EVF>
void pagerank(
      G&&               g,              // graph
      PageRank&         page_rank,      // out: page rank scores
      const type_t      damping_factor, // in: Damping factor (generally set to 0.85)
      const type_t      threshold,      // in: Error threshold to control converge rate
      const std::size_t max_iterations, // in: Maximum number of iterations to converge
      EVF weight = [](edge_reference_t<G> uv) { return ranges::range_value_t<PageRank>(1); }) // default weight(uv) -> 1
{
  using id_type     = vertex_id_t<G>;
  using weight_type = ranges::range_value_t<PageRank>;

  const type_t init_score = 1.0 / size(vertices(g));
  const type_t base_score = (1.0 - damping_factor) / size(vertices(g));

  std::vector<id_type> degrees(size(vertices(g)));
  std::vector<type_t>  outgoing_contrib(size(vertices(g)));

  std::ranges::fill(page_rank, init_score);

  for (auto&& [uid, u] : views::vertexlist(g)) {
    degrees[uid] = degree(g, u);
  }

  for (size_t iter = 0; iter < max_iterations; ++iter) {
    double error = 0;
    std::transform(page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(),
                   [&](auto&& x, auto&& y) { return x / (y + 0); });


    for (auto&& [uid, vid, uv] : views::edgelist(g)) {
      type_t z = 0;
      z += outgoing_contrib[vid];
      auto old_rank  = page_rank[uid];
      page_rank[uid] = base_score + damping_factor * z;
      error += fabs(page_rank[uid] - old_rank);
    }

    if (error < threshold)
      break;
  }
} // namespace std::graph

#endif // GRAPH_PAGERANK_HPP
