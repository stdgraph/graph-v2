#include <iostream>
#include <tuple>
#include <string>
#include <vector>

#include "graph/views/breadth_first_search.hpp"

std::vector<std::string> actors{"Tom Cruise",      "Kevin Bacon",    "Hugo Weaving",      "Carrie-Anne Moss",
                                "Natalie Portman", "Jack Nicholson", "Kelly McGillis",    "Harrison Ford",
                                "Sebastian Stan",  "Mila Kunis",     "Michelle Pfeiffer", "Keanu Reeves",
                                "Julia Roberts"};

using G = std::vector<std::vector<size_t>>;
using G2 = std::vector<std::vector<std::tuple<size_t,double>>>; // to include weight
G costar_adjacency_list{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                        {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                        {1, 4}};

int main() {
  std::vector<size_t> bacon_number(size(actors));

  // 1 -> Kevin Bacon
  for (auto&& [uid, vid, uv] : graph::views::sourced_edges_breadth_first_search(costar_adjacency_list, 1)) {
    bacon_number[vid] = bacon_number[uid] + 1;
  }

  for (size_t i = 0; i < size(actors); ++i) {
    std::cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
  }
}
