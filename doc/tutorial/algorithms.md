Algorithms
==========

This library offers a number of generic algorithms. 
Due to the nature of graph algorithms, the way they communicate the results requires some additional code.

Let's use the following graph representation, naively recognized by this library, 
that is able to store a weight for each edge:

```c++
std::vector<std::vector<std::tuple<int, double>>> g {
  /*0*/ { {(1), 9.1}, {(3), 1.1}             }, //           9.1       2.2
  /*1*/ { {(0), 9.1}, {(2), 2.2}, {(4), 3.5} }, //      (0)-------(1)-------(2)
  /*2*/ { {(1), 2.2}, {(5), 1.0}             }, //       |         |         |
  /*3*/ { {(0), 1.1}, {(4), 2.0}             }, //       |1.1      |3.5      |1.0
  /*4*/ { {(1), 3.5}, {(3), 2.0}             }, //       |   2.0   |         |   0.5
  /*5*/ { {(2), 1.0}, {(6), 0.5}             }, //      (3)-------(4)       (5)-------(6)
  /*6*/ { {(5), 0.5}                         }  //
};
```

Now, let's use Dijkstra's Shortest Paths algorithm to determine the distance from vertex `0` to each vertex in `g`, and also to determine these paths. The paths are not obtained directly, but instead the list of predecessors is returned for each vertex:

```c++
auto weight = [](std::tuple<int, double> const& uv) {
  return std::get<1>(uv);
};  

std::vector<double> distances(g.size());  // we will store the distance to each vertex here
std::vector<int> predecessors(g.size());  // we will store the predecessor of each vertex here

// fill `distances` with `infinity`
// fill `predecessors` at index `i` with value `i`
graph::init_shortest_paths(distances, predecessors);   

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

The algorithms from this library are described in section [Algorithms](../reference/algorithms.md).