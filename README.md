# Graph Library Proposal for the C++ Standard
[![MacOS](https://github.com/stdgraph/graph-v2/actions/workflows/macos.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/macos.yml) [![Ubuntu](https://github.com/stdgraph/graph-v2/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/ubuntu.yml) [![Windows](https://github.com/stdgraph/graph-v2/actions/workflows/windows.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/windows.yml) [![Documentation](https://github.com/stdgraph/graph-v2/actions/workflows/pages.yml/badge.svg)](https://github.com/stdgraph/graph-v2/actions/workflows/pages.yml)

> This library is in the alpha stage that may include significant changes to the interface. It is not recommended for general use.

## Overview
This library designed to provide a useful set of algorithms, views and container(s) for graphs. It also defines
a core Graph Container Interface that provide the basis of interacting with an abstract adjacency list graph, and 
to provide easy integration with external adjacency list graphs.

- bi-partite and n-partite graphs are under investigation.
- Hyper-graphs are outside the scope of this project.
- Comments and questions are welcome and can be directed to phil.ratzloff@sas.com.

### Purpose
This prototype library is an implementation of the proposed Graph Library for ISO Standard C++ as described in P1709. 
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
5. Easy integration of existing graph containers.
6. Have an open design to allow for extensions in the future: 
   1. Support for partite (partitioned) graphs. This requires extending (changing?) the Graph Container Interface.
      This is under investigation.
   2. Support the incoming edges on a vertex (e.g. bidirectional graphs).
   3. Investigate features that might make the Interface useful outside P1709, such as sparse vertex_ids.
      This can help validate the existing design and guide decisions for the future.
   
## Getting Started
This is being actively developed with the latest releases of MSVC (VS2022) on Windows and gcc (11) on Linux/MacOS. 
Other releases or compilers may or may not work.

### Prerequesites
- C++20 compliant compiler that fully supports concepts and ranges.
- CMake 20 or later (for CMake Presets)

### Quick Start Guide (Linux, WSL, MacOS)
```bash
git clone https://github.com/stdgraph/graph-v2.git
cd graph-v2
mkdir build && cd build
cmake ..
make -j8
```

### Editor/IDE Configuration (Windows)
You'll need to assure CMake Presets are enabled in your IDE or other development tool. 
See https://docs.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170 for configuring Microsoft tools.

## Description
In the following tables, P1709 identifies that the feature is in the P1709 proposal. A value of "TBD" indicates that it
is being considered, subject to the size of the proposal and other priorities.

### Graph Algorithms

| Algorithm                       | P1709 | Status                                                                          | 
| :-------------------------------| :---- | :-------------------------------------------------------------------------------|
| Dijkstra Shortest Paths         | Yes   | dijkstra_clrs: needs review                                                     |
| Bellman-Ford Shortest Paths     | Yes   | needs implementation                                                            |
| Connected Components            | Yes   | needs implementation                                                            |
| Strongly Connected Components   | Yes   | needs implementation                                                            |
| Bi-Connected Components         | Yes   | needs implementation                                                            |
| Articulation Points             | Yes   | needs implementation                                                            |
| Minimum Spanning Tree           | Yes   | needs implementation                                                            |
| Page Rank                       | TBD   | needs implementation                                                            |
| Betweenness Centrality          | TBD   | needs implementation                                                            |
| Triangle Count                  | TBD   | needs implementation                                                            |
| Subgraph Isomorphism            | TBD   | needs implementation                                                            |
| Kruskell Minimum Spanning Tree  | TBD   | needs implementation                                                            |
| Prim Minimum Spanning Tre       | TBD   | needs implementation                                                            |
| Louvain (Community Detection)   | TBD   | needs implementation                                                            |
| Label propagation (Comm. Detection) | TBD   | needs implementation                                                        |


### Graph Views

| View                            | Done? | Description                                                                     | 
| :-------------------------------| :---- | :-------------------------------------------------------------------------------|
| vertexlist                      | Yes   | Iterates over vertices                                                          |
| incidence                       | Yes   | Iterates over outgoing edges of a vertex                                        |
| neighbors                       | Yes   | Iterates over outgoing neighbor vertices of a vertex                            |
| edgelist                        | Yes   | Iterates over edges of a graph                                                  |
| depth_first_search              | Yes   | Iterates over vertices or edges of a seed vertex in depth-first order           |
| breadth_first_search            | Yes   | Iterates over vertices or edges of a seed vertex in breadth-first order         |
| topological_sort                | No    | Iterates over vertices or edges of a seed vertex in topological sort order      |

### Graph Containers

| Container                       | P1709 | Description                                                                     | 
| :-------------------------------| :---- | :-------------------------------------------------------------------------------|
| csr_graph                       | Yes   | Compresed Sparse Row graph. High performance, static structure.                 |
| csr_partite_graph               | No    | Partitioned graph. Needs investigation.                                         |
| dynamic_graph                   | No    | Easy to use different containers for vertices and edges.                        |


## Acknowledgments
- The NWGraph team for all their collaborations and support, along with providing the algorithm implementations
[NWGraph Library](https://github.com/NWmath/NWgr)
- Numerous comments and support from the Machine Learning study group (SG19) in the ISO C++ Standards
Committee ([WG21](https://isocpp.org/std/the-committee)).
- Bob Steagal for his [gcc-builder & clang-builder scripts](https://github.com/BobSteagall)
- Jason Turner for his [cpp_starter_project](https://github.com/lefticus/cpp_starter_project)
- René Ferdinand Rivera Morell for his [duck_invoke](https://github.com/bfgroup/duck_invoke), an implementation
of tag_invoke ([P1895](https://wg21.link/P1895)) that works for both gcc and msvc. Minor modifications have
been made so it it in the std namespace.
- Vincent La for his [cvs-parser](https://github.com/vincentlaucsb/csv-parser)
- The ISO C++ Standards Committee (WG21) for [C++](http://eel.is/c++draft/)
