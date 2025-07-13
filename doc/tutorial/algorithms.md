Algorithms
==========

This library offers a number of generic algorithms. 
Due to the nature of graph algorithms, the way they communicate the results requires some additional code.

Let's use the following graph representation, natively recognized by this library, 
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


Index-based access
------------------

You will notice that a lot of function arguments passed to the algorithms takes
vectors as "output" arguments, such as `predecessors` or `distances` in the above examples.
They do not have to be vectors, but they need to be something that is *indexable* by an integral
number. This is how the algorithms are able to fill in some data associated with a given vertex.
Vertices are here represented by a *vertex index*. This requires that graph representations
that interact with the algorithms need to be able to provide an *index* uniquely identifying each
vertex. This requirement is encoded in the concept `index_adjacency_list`.


Visitors
--------

Sometimes, when working with the algorithms, there is a need to get more than just the result.
For troubleshooting or debugging reasons we may want to know what indices and edges are processed,
and in what order. We might also want to display the progress of the algorithm (like, "25% done")
before the algorithm finishes. 

To address this need some of the algorithms provide a notion of a *visitor*. A visitor is
like a set of callbacks that you can pass as an optional parameter to an algorithm. Whenever 
an *event* occurs during the execution of the algorithm – such as "new vertex discovered"
or "a step through an edge taken" – a corresponding callback is invoked.

for illustration, consider that in the above example, we want the algorithm to display the
progress to `STDOUT` upon every third vertex processed. We would have to create our custom
visitor, tell it which event we are interested in intercepting, and what we want to do then.

```c++
class ShowProgress
{
  using G = std::vector<std::vector<std::tuple<int, double>>>;
  using VD = graph::vertex_info<graph::vertex_id_t<G>, graph::vertex_reference_t<G>, void>;    
  int vertex_count = 0;

public:   
  void on_discover_vertex(VD const& v)
  {
    if (vertex_count % 3 == 0)
      std::cout << "processing vertex " << v.id << "\n";
    ++vertex_count;
  }
};
```

And then pass it to the algorithm:

```c++
graph::dijkstra_shortest_paths(g, 0, distances, predecessors, weight, ShowProgress{});
```

Name `on_discover_vertex` is one of the event handlers in visitors. For a comprehensive 
list, see section [Visitors](../reference/visitors.md). 