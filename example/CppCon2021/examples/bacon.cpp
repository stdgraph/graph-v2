#include "graph/views/breadth_first_search.hpp"
#include "imdb-graph.hpp"

#include <iostream>
#include <string>
#include <vector>

std::vector<std::vector<size_t>> costars{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                                         {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                                         {1, 4}};

int main() {

  std::vector<size_t> bacon_number(size(actors));

  // for (auto&& [u, v] : bfs_edge_range(costars, 1)) {
  for (auto&& [u, v, uv] : graph::views::sourced_edges_breadth_first_search(costars, 1)) {
    bacon_number[v] = bacon_number[u] + 1;
  }

  for (size_t i = 0; i < size(actors); ++i) {
    std::cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
  }

  return 0;
}
