# Introduction

This is a C++20 graph library with the following capabilities:

 1. Generic algorithms operating on graph representations.
 2. Generic views that allow the traversal of your graphs, or parts thereof, in a sequential manner.
 3. Customization of your graph representation, so that it can be used with our algorithms and views.
 4. A number of graph containers.


## Example

The following example shows how one can compute for each vertex of a given graph `g` its distance, 
measured in the number of edges, from the indicated vertex with index 0. 
 1. We need an [adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) representation of a graph.
 2. Vertices are identified by indices of a vector.

```c++
#include <cassert>
#include <vector>
#include <graph/views/breadth_first_search.hpp>

// `vector<vector<int>>` is recognized as an adjacency list by this library
std::vector<std::vector<int>> g {   //  
  /*0*/ {1, 3},                     //
  /*1*/ {0, 2, 4},                  //      (0) ----- (1) ----- (2)
  /*2*/ {1, 5},                     //       |         |         |
  /*3*/ {0, 4},                     //       |         |         |
  /*4*/ {1, 3},                     //       |         |         |
  /*5*/ {2, 6},                     //      (3) ----- (4)       (5) ----- (6)
  /*6*/ {5}                         //
};                                  //

int main()
{
  std::vector<int> distances(g.size(), 0);   // fill with zeros

  // a view of edges as they appear in the breadth-first order, from vertex 0.
  auto bfs_view = graph::views::sourced_edges_breadth_first_search(g, 0);

  for (auto const& [uid, vid, _] : bfs_view) // a directed edge (u, v)
    distances[vid] = distances[uid] + 1;

  assert((distances == std::vector{0, 1, 2, 1, 2, 3, 4}));
}
```

## Design

Algorithms and views in this library operate on graph representations via the [*Graph Container Interface*](./tutorial/graph_container_interface.md),
which is a set of *customization point objects* (CPO). In order to plug your graph container into this library you need to make sure that all the 
necessary CPOs have been customized for your container.

The generic algorithms and views in this library are constrained with concepts, which are expressed in terms of the above CPOs.

This library comes with two graph containers, encoding different engineering trade-offs. Also, some sufficiently simple nested ranges are automatically considered
compliant with the Graph Container Interface, such as:

 * `vector<vector<int>>`,
 * `vector<vector<tuple<int, ...>>>`.

The algorithms in this library do not mutate the graphs. There is not support for graph rewriting.

# Next steps

For a more detailed overview of the library, see section [tutorial](./tutorial). <br>
For a reference documentaiton, see section [reference](./reference).
