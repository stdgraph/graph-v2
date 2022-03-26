#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <queue>

namespace std::graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && regular_invocable<F&, ranges::range_reference_t<vertex_edge_range_t<G>>>;


// The index into weight vector stored as the first property
template <incidence_graph G, class WF>
requires edge_weight_function<G, WF> &&
      is_arithmetic_v<invoke_result_t<WF, ranges::range_reference_t<vertex_edge_range_t<G>>>> &&
      ranges::random_access_range<vertex_range_t<G>>
auto dijkstra_book(
      G&&            g,      //
      vertex_id_t<G> source, //
      WF             weight = [&g](edge_reference_t<G> uv) { return 1; }) {
  using id_type     = vertex_id_t<G>;
  using weight_type = decltype(weight(std::declval<edge_reference_t<G>>()));

  size_t N(size(vertices(g)));
  assert(source < N);

  vector<weight_type> distance(N, numeric_limits<weight_type>::max());
  distance[source] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  priority_queue<weighted_vertex, vector<weighted_vertex>,
                 decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto uid = Q.top().vertex_id;
    Q.pop();

    //for (auto&& [vid, w] : g[u]) -- pretty but would only allow one property
    //
    //for (auto&& uv : std::graph::edges(g, g[u])) {
    //  auto        vid = target_id(g, uv);
    //
    //extension:
    //for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
    //
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      weight_type w = weight(uv);
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        Q.push({vid, distance[vid]});
      }
    }
  }

  return distance;
}

} // namespace std::graph
