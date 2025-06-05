# Overview

What this library has to offer:

 1. A number of ready-to-use algoritms, like Dijkstra's shortest paths computation.
 2. Make it easier for you to write your own graph algorithms: we provide a _view_ for traversing your graph in the preferred order (depth-first, breadth-first, topological), and you specify what is processed in each traversal step.
 3. Customization of your graph representation, so that it can be used with our algorithms and views.
 4. Our own container for representing a graph.  

## Concepts

This is a _generic_ library. Graph algorithms operate on various graph representations through a well defined interface: a concept. The primary concept in this library is `index_adjacency_list`.

```c++
#include <graph/graph.hpp>  // (1)

static_assert(graph::index_adjacency_list<MyType>);  // (2) (3)
```
Notes:
 1. Graph concepts are defined in header `<graph/graph.hpp>`.
 2. All declarations reside in namespace `::graph`.
 3. Use concept `graph::index_adjacency_list` to test if your type satisfies the syntactic requirements of an index adjacency list.

The graph representation that is most commonly used in this library is [_adjacency list_](https://en.wikipedia.org/wiki/Adjacency_list). 
Very conceptually, we call it a random access range (corresponding to vertices) of forward ranges (corresponding to outgoing edges of a vertex).

This representation allows the algorithms to:

 1. Perform an iteration over vertices, and to count them.
 2. For each vertex, perform an iteration over its outgoing edges.
 3. For each edge to to look up its target vertex in constant time.

Algorithms in this library express the requirements on the adjacency list representations via concept `graph::index_adjacency_list`.
This concept expresses its syntactic requirments mostly via [_customization points_](./customization_points.md).
We use the following notation to represent the constraints:

| Symbol | Meaning                                                 |
|--------|---------------------------------------------------------|
| `G`    | the type of the graph representation                    |
| `g`    | lvalue or lvalue reference of type `G`                  |
| `u`    | lvalue reference of type `graph::vertex_reference_t<G>` |
| `ui`   | value of type `graph::vertex_iterator_t<G>`             |
| `uid`  | value of type `graph::vertex_id_t<G>`                   |
| `uv`   | lvalue reference of type `graph::edge_reference_t<G>`   |

### Random access to vertices

Customization point `graph::vertices(g)` must be valid and its return type must satisfy type-requirements `std::ranges::sized_range` and `std::ranges::random_access_range`. 

The algorithms will use it to access the vertices of graph represented by `g` in form of a random-access range. 

Customization point `graph::vertex_id(g, ui)` must be valid and its return type must satisfy type-requirements `std::integral`.
The algorithms will use this function to convert the iterator pointing to a vertex to the _id_ of the vertex.


### Forward access to target edges

The following customization points must be valid and their return type shall satisfy type requirements `std::ranges::forward_range`:

 * `graph::edges(g, uid)`,
 * `graph::edges(g, u)`.

The algorithms will use this function to iterate over out edges of the vertex represended by either `uid` or `u`.


### Linking from target edges back to vertices

Customization point `graph::target_id(g, uv)` must be valid and its return type must satisfy type-requirements `std::integral`.

The algorithms will use this value to access a vertex in `graph::vertices(g)`. 
Therefore we have a _semantic_ constraint: that the look up of the value returned from `graph::target_id(g, uv)` returns value `uid` that satisfies the condition
`0 <= uid && uid < graph::num_vertices(g)`.


### Associated types

Based on the customization points the library provides a number of associated types in namespace `graph`:

| Associated type             | Definition                                               |
|-----------------------------|----------------------------------------------------------|
| `vertex_range_t<G>`         | `decltype(graph::vertices(g))`                           |
| `vertex_iterator_t<G>`      | `std::ranges::iterator_t<vertex_range_t<G>>`             |
| `vertex_t<G>`               | `std::ranges::range_value_t<vertex_range_t<G>>`          |
| `vertex_reference_t<G>`     | `std::ranges::range_reference_t<vertex_range_t<G>>`      |
| `vertex_id_t<G>`            | `decltype(graph::vertex_id(g, ui))`                      |
| `vertex_edge_range_t<G>`    | `decltype(graph::edges(g, u))`                           |
| `vertex_edge_iterator_t<G>` | `std::ranges::iterator_t<vertex_edge_range_t<G>>`        |
| `edge_t<G>`                 | `std::ranges::range_value_t<vertex_edge_range_t<G>>`     |
| `edge_reference_t<G>`       | `std::ranges::range_reference_t<vertex_edge_range_t<G>>` |


## Views

This library can help you write your own algorithms via _views_ that represent graph traversal patterns as C++ ranges.

Suppose, your task is to compute the distance, in edges, from a given vertex _u_ in your graph _g_, to all vertices in _g_. 
We will represent the adjacency list as a vector of vectors:


```c++
std::vector<std::vector<int>> g {   //  
  /*0*/ {1, 3},                     //
  /*1*/ {0, 2, 4},                  //      (0) ----- (1) ----- (2)
  /*2*/ {1, 5},                     //       |         |         |
  /*3*/ {0, 4},                     //       |         |         |
  /*4*/ {1, 3},                     //       |         |         |
  /*5*/ {2, 6},                     //      (3) ----- (4)       (5) ----- (6)
  /*6*/ {5}                         //
};                                  //
```

The algorithm is simple: you start by assigning value zero to the start vertex, 
then go through all the graph edges in the breadth-first order and for each edge (_u_, _v_) 
you will assign the value for _v_ as the value for _u_ plus 1. 

In order to do this, you can employ a depth-first search view from this library:

```c++
#include <graph/views/breadth_first_search.hpp>

int main()
{
  std::vector<int> distances(g.size(), 0); // fill with zeros

  for (auto const& [uid, vid, _] : graph::views::sourced_edges_breadth_first_search(g, 0))
    distances[vid] = distances[uid] + 1;

  assert((distances == std::vector{0, 1, 2, 1, 2, 3, 4}));
}
```

## Algorithms

This library offers also full fledged algorithms. 
However, due to the nature of graph algorithms, the way they communicate the results requires some additional code.
Let's, reuse the same graph topology, but use a different adjacency-list representation that is also able to store a weight for each edge:

```c++
std::vector<std::vector<std::tuple<int, double>>> g {
  /*0*/ {{(1), 9.1}, {(3), 1.1}            }, //           9.1       2.2
  /*1*/ {{(0), 9.1}, {(2), 2.2}, {(4), 3.5}}, //      (0)-------(1)-------(2)
  /*2*/ {{(1), 2.2}, {(5), 1.0}            }, //       |         |         |
  /*3*/ {{(0), 1.1}, {(4), 2.0}            }, //       |1.1      |3.5      |1.0
  /*4*/ {{(1), 3.5}, {(3), 2.0}            }, //       |   2.0   |         |   0.5
  /*5*/ {{(2), 1.0}, {(6), 0.5}            }, //      (3)-------(4)       (5)-------(6)
  /*6*/ {{(5), 0.5}}                          //
};
```

Now, let's use Dijkstra's Shortest Paths algorithm to determine the distance from vertex `0` to each vertex in `g`, and also to determine these paths. The pathst are not obtained directly, but instead the list of predecessors is returned for each vertex:

```c++
auto weight = [](std::tuple<int, double> const& uv) {
  return std::get<1>(uv);
};  

std::vector<int> predecessors(g.size()); // we will store the predecessor of each vertex here

std::vector<double> distances(g.size()); // we will store the distance to each vertex here
graph::init_shortest_paths(distances);   // fill with `infinity`

graph::dijkstra_shortest_paths(g, 0, distances, predecessors, weight); // from vertex 0

assert((distances == std::vector{0.0, 6.6, 8.8, 1.1, 3.1, 9.8, 10.3}));
assert((predecessors == std::vector{0, 4, 1, 0, 3, 2, 5}));
```

If you need to know the sequence of vertices in the path from, say, `0` to `5`, you have to compute it yourself:

```c++
auto path = [&predecessors](int from, int to)
{
  std::vector<int> result;
  for (; to != from; to = predecessors[to]) {
    assert(to < predecessors.size()); 
    result.push_back(to);
  }
  std::reverse(result.begin(), result.end());
  return result;
};

assert((path(0, 5) == std::vector{3, 4, 1, 2, 5}));
```
The algorithms in this library are described in section [Algorithms](./algorithms.md).


------

TODO:

- Adapting
- use our container
