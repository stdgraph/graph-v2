# Graph Library Proposal for the C++ Standard
[![codecov](https://codecov.io/github/stdgraph/graph-v2/branch/master/graph/badge.svg?token=49LGWDN0U1)](https://codecov.io/github/stdgraph/graph-v2) [![MacOS](https://github.com/stdgraph/graph-v2/actions/workflows/macos.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/macos.yml) [![Ubuntu](https://github.com/stdgraph/graph-v2/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/ubuntu.yml) [![Windows](https://github.com/stdgraph/graph-v2/actions/workflows/windows.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/windows.yml) [![Documentation](https://github.com/stdgraph/graph-v2/actions/workflows/pages.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/pages.yml)

> This library is in the **alpha stage** that may include significant changes to the interface. It is not 
> recommended for general use.

> A **breaking change** in the near future will introduce a new concept of identifier for vertices and edges, 
> similar to the descriptor concept in the Boost Graph Library. It will replace the existing id and references 
> in the API and will allow for more flexibility in the design as well as reducing the number of concepts and
> functions. Most functions and views will be affected.  *Phil - September 2024*.

## Overview
This library designed to provide a useful set of algorithms, views and container(s) for graphs. It also defines
a core Graph Container Interface that provide the basis of interacting with an abstract adjacency list graph, and 
to provide easy integration with external adjacency list graphs.

- Hyper-graphs are outside the scope of this project.
- Comments and questions are welcome and can be directed to GitHub [discussions](https://github.com/stdgraph/graph-v2/discussions) 
  or [issues](https://github.com/stdgraph/graph-v2/issues).

### Purpose
This prototype library is an implementation of the proposed Graph Library for ISO Standard C++. 
It has gone through major revisions since it was first introduced in 2019. While we are comfortable of the core design, there is
still plenty of activity being done and refinements made in its design and implementation. Experimenting with this library is 
encouraged, keeping in mind that breaking changes are expected.

### Goals
The goals of the library include:
1. Support creation of high-performance, state-of-the-art algorithms.
2. Syntax that is simple, expressive and easy to understand when writing algorithms.
3. Define useful concepts and traits that can be used by algorithms to describe their requirements.
4. Support views for graph traversal commonly used by algorithms.
5. Support optional, user-defined value_types for an edge, vertex and graph.
6. Allow the use of standard containers to define simple graphs.
7. Easy integration of existing graph data structures.
8. Have an open design to allow for extensions in the future:
   1. Support the incoming edges on a vertex (e.g. bidirectional graphs).
   2. Support sparse vertex_ids stored in bi-directional ranges (e.g. `map` and `unordered_map`).
   
## Getting Started
This is being actively developed with the latest releases of MSVC (VS2022) on Windows and gcc (13) on Linux/MacOS. 
Other releases or compilers may or may not work.

### Prerequesites
- C\+\+20 compliant compiler that fully supports concepts and ranges. (C\+\+23 is required for building the benchmarks.)
- CMake 3.20 or later (for CMake Presets)

### Quick Start Guide (Linux, WSL, MacOS)
```bash
git clone https://github.com/stdgraph/graph-v2.git
cd graph-v2
mkdir build && cd build
cmake ..
make
```

### Editor/IDE Configuration (Windows)
You'll need to assure CMake Presets are enabled in your IDE or other development tool. 
See https://docs.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170 for configuring Microsoft tools.

## Description

### Graph Algorithms ([P3128](https://wg21.link/P3128))
The following algorithms are planned for the Graph Library.

| Algorithm                           | Status                                                                          |
| :---------------------------------- | :-------------------------------------------------------------------------------|
| Breadth First Search                | revision scheduled                                                              |
| Depth First Search                  | revision scheduled                                                              |
| Topological Sort                    | implementation scheduled                                                        |
| Dijkstra Shortest Paths             | Completed                                                                       |
| Bellman-Ford Shortest Paths         | Completed                                                                       |
| Triangle Counting                   | needs implementation                                                            |
| Label propagation (Comm. Detection) | needs implementation                                                            |
| Articulation Points                 | needs implementation                                                            |
| Bi-Connected Components             | needs implementation                                                            |
| Connected Components                | needs implementation                                                            |
| Strongly Connected Components       | needs implementation                                                            |
| Maximal Independent Set             | needs implementation                                                            |
| Jaccard Coefficient                 | needs implementation                                                            |
| Kruskell Minimum Spanning Tree      | needs implementation                                                            |
| Prim Minimum Spanning Tre           | needs implementation                                                            |


### Graph Views ([P3129](https://wg21.link/P3129) )

| View                            | Description                                                                     | 
| :-------------------------------| :-------------------------------------------------------------------------------|
| vertexlist                      | Iterates over vertices                                                          |
| incidence                       | Iterates over outgoing edges of a vertex                                        |
| neighbors                       | Iterates over outgoing neighbor vertices of a vertex                            |
| edgelist                        | Iterates over edges of a graph                                                  |
| depth_first_search              | Iterates over vertices or edges of a seed vertex in depth-first order           |
| breadth_first_search            | Iterates over vertices or edges of a seed vertex in breadth-first order         |
| topological_sort                | Iterates over vertices or edges of a seed vertex in topological sort order      |

### Graph Containers ([P3131](https://wg21.link/P3130))

| Container                       | Description                                                                     | 
| :-------------------------------| :-------------------------------------------------------------------------------|
| compressed_graph                | Compresed Sparse Row graph. High performance, static structure.                 |
| dynamic_graph                   | Easy to use different containers for vertices and edges.                        |
| (std containers)                | Use standard containers like `vector` and `list` to define a simple graph.      |

## Papers
The following papers make up the current proposal for the Graph Library.

| Paper                                                     | Description                                                                                             | 
| :---------------------------------------------------------| :-------------------------------------------------------------------------------------------------------|
| [P3126 Overview](https://wg21.link/P3126)                 | Describes the big picture of what we are proposing.                                                     |
| [P3127 Background](https://wg21.link/P3127)               | Background and Terminology, provides the motivation and theoretical background underlying the proposal. |
| [P3128 Algoritms](https://wg21.link/P3128)                | Covers the initial algorithms as well as the ones we'd like to see in the future.                       |
| [P3129 Views](https://wg21.link/P3129)                    | Helpful views for traversing a graph.                                                                   |
| [P3130 Graph Container Interface](https://wg21.link/P3130)| The core interface for uniformly accessing graph data structure and adapting to external graphs.        |
| [P3131 Graph Containers](https://wg21.link/P3130)         | Includes the `compressed_graph` and how to use standard containers to define simple graphs.            |
| [P3337 Graph Comparison](https://wg21.link/P3337)         | *[future]* Syntax and performance comparison to the Boost Graph Library.                                |


## Acknowledgments
- The NWGraph team for all their collaborations and support, along with providing the algorithm implementations
[NWGraph Library](https://github.com/NWmath/NWgr)
- Numerous comments and support from the Machine Learning study group (SG19) in the ISO C++ Standards
Committee ([WG21](https://isocpp.org/std/the-committee)).
- Bob Steagal for his [gcc-builder & clang-builder scripts](https://github.com/BobSteagall)
- Jason Turner for his [cpp_starter_project](https://github.com/lefticus/cpp_starter_project)
- Vincent La for his [cvs-parser](https://github.com/vincentlaucsb/csv-parser) (copied into tests).
- The ISO C++ Standards Committee (WG21) for [C++](http://eel.is/c++draft/)
