#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence_view.hpp"
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
      G&&             g,      //
      vertex_key_t<G> source, //
      WF              weight = [&g](ranges::range_reference_t<vertex_edge_range_t<G>> uv) { return 1; }) {
  using key_type    = vertex_key_t<G>;
  using weight_type = decltype(weight(std::declval<ranges::range_reference_t<vertex_edge_range_t<G>>>()));

  size_t N(size(vertices(g)));
  assert(source < N);

  vector<weight_type> distance(N, numeric_limits<weight_type>::max());
  distance[source] = 0;

  struct weighted_vertex {
    key_type    vertex_key = key_type();
    weight_type weight     = weight_type();
  };

  priority_queue<weighted_vertex, vector<weighted_vertex>,
                 decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; })>
        Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto ukey = Q.top().vertex_key;
    Q.pop();

    //for (auto&& [vkey, w] : g[u]) -- pretty but would only allow one property
    //
    //for (auto&& uv : std::graph::edges(g, g[u])) {
    //  auto        vkey = target_key(g, uv);
    //
    //extension:
    //for (auto&& [vkey, uv, w] : std::graph::incidence(g, u, weight)) {
    //
    for (auto&& [vkey, uv] : views::incidence(g, ukey)) { // see zip
      weight_type w = weight(uv);
      if (distance[ukey] + w < distance[vkey]) {
        distance[vkey] = distance[ukey] + w;
        Q.push({vkey, distance[vkey]});
      }
    }
  }

  return distance;
}

template <incidence_graph G>
void vertex_key_example(G&& g) {
  auto ui   = begin(vertices(g));
  auto key = vertex_key(g,ui);

  for (auto&& [ukey, u] : views::vertexlist(g)) { //
  }
}

} // namespace std::graph
