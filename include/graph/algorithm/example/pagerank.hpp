#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <queue>

namespace std::graph {

// The index into weight vector stored as the first property
template <adjacency_list              G,
          ranges::random_access_range Degrees,
          ranges::random_access_range PageRank,
          typename type_t,
          class WF = std::function<ranges::range_value_t<Distance>(edge_reference_t<G>)>>
// requires ranges::random_access_range<vertex_range_t<G>> &&                  //
//       integral<vertex_id_t<G>> &&                                           //
//       is_arithmetic_v<ranges::range_value_t<Distance>> &&                   //
//       convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessor>> && //
//       edge_weight_function<G, WF>
void pagerank(
      G&&       g,               // graph
      PageRank& page_rank,       // out: page rank scores
      type_t    damping_factor,  // in: Damping factor (generally set to 0.85)
      type_t    threshold,       // in: Error threshold to control converge rate
      std::size_t max_iterations // in: Maximum number of iterations to converge
            WF weight =
                  [](edge_reference_t<G> uv) { return ranges::range_value_t<Distance>(1); }) // default weight(uv) -> 1
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  std::size_t N          = graph.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  // Initialize the page rank.
  tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      page_rank[i] = init_score;
    }
  });

  std::unique_ptr<Real[]> outgoing_contrib(new Real[N]);

  tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      outgoing_contrib[i] = page_rank[i] / degrees[i];
    }
  });

  for (size_t iter = 0; iter < max_iters; ++iter) {
    auto&& [time, error] = pagerank::time_op([&] {
      return tbb::parallel_reduce(
            tbb::blocked_range(0ul, N), 0.0,
            [&](auto&& r, auto partial_sum) {
              for (size_t i = r.begin(), e = r.end(); i != e; ++i) {
                Real z = 0.0;
                for (auto&& j : graph[i]) {
                  z += outgoing_contrib[std::get<0>(j)];
                }
                auto old_rank = page_rank[i];
                page_rank[i]  = base_score + damping_factor * z;
                partial_sum += fabs(page_rank[i] - old_rank);
                outgoing_contrib[i] = page_rank[i] / (Real)degrees[i];
              }
              return partial_sum;
            },
            std::plus{});
    });

    if (error < threshold) {
      return;
    }
  }
}
} // namespace std::graph
