#pragma once

#include "graph/graph.hpp"
#include "graph/view/incidence_edge_view.hpp"
#include <queue>
#include <tuple>

namespace std::graph {

template <typename G, typename F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && regular_invocable<F&, ranges::range_reference_t<vertex_edge_range_t<G>>>;


// The index into weight vector stored as the first property
template <incidence_graph G, typename WF>
requires edge_weight_function<G, WF> &&
      is_arithmetic_v<invoke_result_t<WF, ranges::range_reference_t<vertex_edge_range_t<G>>>> &&
      ranges::random_access_range<vertex_range_t<G>>
auto dijkstra_book(
      G&&             g,      //
      vertex_key_t<G> source, //
      WF              weight = [&g](ranges::range_reference_t<vertex_edge_range_t<G>> uv) { return 1; }) {
  using key_type    = vertex_key_t<G>;
  using weight_type = decltype(weight(std::declval<ranges::range_reference_t<vertex_edge_range_t<G>>>()));

  size_t N(size(g));
  assert(source < N);

  vector<weight_type> distance(N, numeric_limits<weight_type>::max());
  distance[source] = 0;

  struct weighted_vertex {
    key_type    vertex_key = key_type();
    weight_type weight     = weight_type();
  };

  std::priority_queue<weighted_vertex, vector<weighted_vertex>,
                      decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto u = Q.top().vertex_key;
    Q.pop();

    //for (auto&& [v, w] : g[u]) -- pretty but would only allow one property
    //
    //auto vw = incidence_edges_view(g, u, target_key); // (can't put it in for stmt below right now)
    //for (auto&& [v, uv] : vw) {
    //
    //for (auto&& uv : std::graph::edges(g, g[u])) {
    //  auto        v = target_key(g, uv);
    //
    //extension:
    //for (auto&& [v, uv, w] : std::graph::edges_view(g, u, weight)) {
    //
    for (auto&& [v, uv] : edges_view(g, u)) {
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
