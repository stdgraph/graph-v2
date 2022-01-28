#pragma once

#include "graph/graph.hpp"
#include <queue>
#include <tuple>

namespace std::graph {

template <typename G, typename F>
concept edge_weight_function = // e.g. weight(uv)
      std::copy_constructible<F> && std::regular_invocable<F&, std::ranges::range_reference_t<vertex_edge_range_t<G>>>;

// The index into weight vector stored as the first property
template <std::graph::incidence_graph G, typename WF>
requires edge_weight_function<G, WF> &&
      std::is_arithmetic_v<std::invoke_result_t<WF, std::ranges::range_reference_t<vertex_edge_range_t<G>>>> &&
      std::ranges::random_access_range<std::graph::vertex_range_t<G>>
auto dijkstra_book(
      G&& g, vertex_key_t<G> source, WF weight = [](std::ranges::range_reference_t<vertex_edge_range_t<G>> uv) {
        return 1;
      }) {
  using vertex_key_type = vertex_key_t<G>;
  using weight_type     = decltype(weight(std::declval<std::ranges::range_reference_t<vertex_edge_range_t<G>>>()));

  size_t N(size(g));
  assert(source < N);

  std::vector<weight_type> distance(N, std::numeric_limits<weight_type>::max());
  distance[source] = 0;

  using weighted_vertex = std::tuple<vertex_key_type, weight_type>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>,
                      decltype([](auto&& a, auto&& b) { return (std::get<1>(a) > std::get<1>(b)); })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto u = std::get<0>(Q.top());
    Q.pop();

    // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

    //auto vw = incidence_edges_view(g, u, target_key); // (can't put it in for stmt below right now)
    //for (auto&& [v, uv] : vw) {
    for (auto&& uv : std::graph::edges(g, g[u])) {
      auto        v = target_key(g, uv);
      weight_type w = weight(uv);
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        Q.push({v, distance[v]});
      }
    }
  }

  return distance;
}

} // namespace std::graph
