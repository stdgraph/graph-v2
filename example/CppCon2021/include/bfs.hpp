//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

// Basic index adjacency list graph bfs

#ifndef NWGRAPH_BFS_HPP
#define NWGRAPH_BFS_HPP

#include "graph_concepts.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>
#include <queue>
#include <ranges>
#include <tuple>
#include <vector>

enum COLOR { WHITE, BLACK, GREY };

template <adjacency_list Graph>
auto bfs(const Graph& graph, vertex_id_t<Graph> source) {
  using vertex_id_type = vertex_id_t<Graph>;

  std::vector<COLOR> color(size(graph));
  for (vertex_id_type u = 0; u < size(graph); ++u) {
    color[u] = WHITE;
  }
  color[source] = GREY;

  std::queue<vertex_id_type> Q;
  Q.push(source);

  while (!Q.empty()) {
    auto u = Q.front();
    Q.pop();
    for (auto&& e : graph[u]) {
      auto v = target(graph, e); // neighbor vertex
      if (color[v] == WHITE) {
        color[v] == GREY;
        Q.push(v);
      }
    }
    color[u] = BLACK;
  }
}

#endif // NWGRAPH_BFS_HPP
