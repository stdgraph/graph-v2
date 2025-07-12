# Tutorial

Note: this part is under development...

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
This concept expresses its syntactic requirements mostly via [_customization points_](./customization_points.md).
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

The algorithms will use this function to iterate over out edges of the vertex represented by either `uid` or `u`.


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



## Containers

This library comes with an efficient graph container that uses 
[Compressed Sparse Row](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_%28CSR%2C_CRS_or_Yale_format%29) 
(CSR) format: `compressed_graph`.

```c++
#include <graph/container/compressed_graph.hpp>

int main()
{
  std::vector<graph::copyable_edge_t<unsigned, double>> ve {
    {0, 1, 0.8}, {0, 2, 0.4}, {1, 3, 0.1}, {2, 3, 0.2}   // edges with weights
  };
  
  std::vector<graph::copyable_vertex_t<unsigned, std::string>> vv {
    {0, "A"}, {1, "B"}, {2, "C"}, {3, "D"}               // vertices with names
  };
  
  using graph_t = graph::container::compressed_graph<
    double,      // type of edge value
    std::string  // type of vertex value
  >;
  
  const graph_t g(ve, vv);
    
  std::vector<std::string> vertex_names;
  
  for (graph_t::vertex_type const& u : vertices(g))       // use graph interface
    vertex_names.push_back(vertex_value(g, u));           //
        
  assert((vertex_names == std::vector<std::string>{"A", "B", "C", "D"}));
}
```


------

TODO:

- Adapting
