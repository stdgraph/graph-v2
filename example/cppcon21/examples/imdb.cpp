//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

//  IMDB Example

#if defined(__GNUC__) || defined(__clang__)
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4267) // warning C4267: 'argument': conversion from 'size_t' to 'int', possible loss of data
#endif

#include <iostream>
#include <list>
#include <vector>

#include "graph/views/breadth_first_search.hpp"
//#include "bfs_edge_range.hpp"
#include "imdb-graph.hpp"
#include "utilities.hpp"

int main() {

  // Get actor-movie and movie-actor graphs
  auto&& [G, H] = make_plain_bipartite_graphs<>(movies, actors, movies_actors);

  // Will also work with this type of graph
  auto&& [J, K] = make_bipartite_graphs<>(movies, actors, movies_actors);

  // Create actor-actor graph
  auto L = join(G, H);
  auto M = join(H, G);

  // Can also join this way
  auto N = join<decltype(G), decltype(H), std::vector<std::vector<std::tuple<size_t, size_t>>>>(G, H);
  auto O = join<decltype(H), decltype(G), std::vector<std::list<std::tuple<int, int>>>>(H, G);

  size_t              kevin_bacon = 1;
  std::vector<size_t> distance(L.size());
  std::vector<size_t> parents(L.size());
  std::vector<size_t> together_in(L.size());

  //for (auto&& [u, v, k] : bfs_edge_range(L, kevin_bacon)) {
  auto kprop = [&L](auto&& e) { return std::get<1>(std::graph::edge_value(L, e)); };
  for (auto&& [u, v, uv, k] : std::graph::views::sourced_edges_breadth_first_search(L, kevin_bacon, kprop)) {
    distance[v]    = distance[u] + 1;
    parents[v]     = u;
    together_in[v] = k;
  }

  std::cout << actors[kevin_bacon] << " has a bacon number of " << distance[kevin_bacon] << std::endl;
  std::cout << std::endl;

  // Iterate through all actors (other than Kevin Bacon)
  for (size_t i = 0; i < actors.size(); ++i) {
    if (i != kevin_bacon) {
      auto bacon_number = distance[i];
      std::cout << actors[i] << " has a bacon number of " << distance[i] << std::endl;

      auto   k = i;
      size_t d = distance[k];
      while (k != kevin_bacon) {
        std::cout << "   " << actors[k] << " starred with " << actors[parents[k]] << " in " << movies[together_in[k]] << std::endl;
        k = parents[k];
        if (d-- == 0) {
          break;
        }
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
