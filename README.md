# Graph Library Proposal for the C++ Standard

**[This library is in the alpha stage that may include significant changes to the interface. It is not recommended for general use.]**

## Overview
This prototype library is an implementation of the proposed Graph Library for ISO Standard C++ as described in [P1709](http://wg21.link/P1709). 
It has gone through major revisions since it was first introduced in 2019. While the hope is that we have settled on a useful and acceptable
design, it is still being refined. Comments and questions are welcome and can be directed to phil.ratzloff@sas.com.

### Purpose
The purpose of the library is to provide a foundation for graph algorithms and data structures with the same goals of the original
STL, where graph algorithms written for the library can work against indepdendent graph data structures that support the core graph 
API functions. It includes some common algorithms, views and a graph data structure with the belief that it will grow with time.

### Goals
The goals of the library include:
1. Support creation of high-performance algorithms that are as good as the existing state-of-the art, or better.
2. Syntax that is simple, expressive and easy to understand when writing algorithms.
3. Present different views of incidence graphs that are commonly used by the algorithms (incidence, neighbors/adjacency, 
   edge-list).
4. Easy integration of existing graph data structures.
5. Support optional, user-defined value_types for an edge, vertex and graph.
6. Be able to extend the design for future standardization, or uses outside the standard. For instance:

   a. All algorithms in the standard will only support vertices in random-access containers. However, graph algorithms and 
      data structures outside of the standard may use a map, unordered_map or other data structure and the design should 
      support that use.

   b. While vertices only have "outgoing" edges in the standard because most algorithms only require that, bi-directional 
      graphs with both incoming and outgoing edges can also exist and the design should should be extensible to support
      that.

7. Define useful traits and concepts that can be used by algorithms.
8. Provide an initial set of useful algorithms and a graph data structure.

## Getting Started
### Build & Run Requirements
This is being actively developed with the latest releases of MSVC (VS2022) on Windows and gcc (11) on Linux. Other releases
or compilers may or may not work. (At the time of this writing Clang doesn't have a \<concepts\> header and so it hasn't
been used.)

#### Prerequesites
1. C++20 compliant compiler that fully supports concepts and ranges. 
   gcc 11 and the latest version of MSVC have been used for development. 
   Others may also work but haven't been tested.
2. CMake 20 or later (needed for CMake Presets)
3. Python3
4. Conan package manager (Python: pip install conan)

#### Cloning & Building

```C++
git clone https://github.com/pratzl/graph-v2.git
cd graph-v2
mkdir build
cd build
cmake ../???
make
```

You'll need to assure CMake Presets are enabled in your IDE or other development tool. 
See https://docs.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170 for configuring Microsoft tools.

The following library(s) are dependencies and are loaded as git sub-modules

1. csv-parser

The following libraries will automatically be installed by Conan

1. Catch2 unit test framework

Other Useful Tools

1. clang-format

## Description

### Naming Conventions

Template parameters:

| Abbr     | Description                            | 
| :--------| :--------------------------------------|
| G        | Graph                                  |
| GV       | Graph Value (user-defined or void)     |
| GVF      | Graph Value Function: gvf(g) -> value; declval(value) may be different than GV  |
| V        | Vertex type                            |
| VKey     | Vertex key type                        |
| VV       | Vertex Value (user-defined or void)    |
| VVF      | Vertex Value Function: vvf(u) -> value; declval(value) may be different than VV |
| VR       | Vertex Range                           |
| E        | Edge type                              |
| EV       | Edge Value (user-defined or void)      |
| ER       | Edge Range                             |
| EVF      | Edge Value Function: evf(uv) -> value; declval(value) may be different than EV  |

Parameters:

| Abbr      | Description                            | 
| :---------| :--------------------------------------|
| g         | graph reference                        |
| u,v,x,y   | vertex referenc                        |
| ukey,vkey | vertex keys                            |
| ui,vi     | vertex iterators                       |
| uv        | edge reference                         |
| uvi       | edge iterator                          |

### Graph Views

#### vertexlist View
#### incidence and sourced_incidence_Views
#### neighbors and sourced_neighbors View (adjacency)
#### edgelist View

### Graph Range Algorithms

#### Depth-First Search
dfs_vertex_range
dfs_edge_range

#### Breadth-First Search
bfs_vertex_range
bfs_edge_range

### Graph Algorithms

#### Shortest Paths
vertex_path_t
vertex_path_range
edge_path_t
edge_path_range
shortest_path

dijkstra_shortest_paths
bellman_ford_shortest_paths

#### Components

component
bicomponent

connected_components
strongly_connected_components
biconnected_components
articulation_points

#### Transitive Closure

reaches
dfs_transitive_closure
warshall_transitive_closure

### Graph API

#### Types
#### Traits & Concepts
#### Functions
#### Integrating with Existing Graphs

### Graph Data Structures

#### csr_graph

#### dynamic_graph


## Thanks to:
Andrew Lumsdaine and the NWGraph team for numerous insights and collaborations with their C++20 
[NWGraph Library](https://github.com/NWmath/NWgr)

Numerous comments and support from the Machine Learning study group (SG19) in the ISO C++ Standards
Committee ([WG21](https://isocpp.org/std/the-committee)).

Bob Steagal for his [gcc-builder & clang-builder scripts](https://github.com/BobSteagall)

Jason Turner for his [cpp_starter_project](https://github.com/lefticus/cpp_starter_project)

René FerdinandRivera Morell for his [duck_invoke](https://github.com/bfgroup/duck_invoke), an implementation
of tag_invoke ([P1895](https://wg21.link/P1895)) that works for both gcc and msvc. Minor modifications have
been made so it it in the std namespace.

Vincent La for his [cvs-parser](https://github.com/vincentlaucsb/csv-parser)

The ISO C++ Standards Committee (WG21) for [C++](http://eel.is/c++draft/)
