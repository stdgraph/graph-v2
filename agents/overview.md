# Graph Library (graph-v2) - Project Overview

> **For Humans and AI Agents**  
> Last updated: January 2026

## Executive Summary

**graph-v2** is a C++20 graph library designed as a proposal for the ISO C++ Standard. It provides algorithms, views, and containers for working with adjacency list graphs. The library emphasizes:

- High-performance, state-of-the-art algorithms
- Simple, expressive syntax for writing graph algorithms
- Easy integration with existing graph data structures
- Use of modern C++ features (concepts, ranges, views)

**Repository:** [github.com/stdgraph/graph-v2](https://github.com/stdgraph/graph-v2)

**Status:** Alpha stage - significant interface changes expected

---

## Table of Contents

1. [Project Structure](#project-structure)
2. [Namespaces](#namespaces)
3. [Core Architecture](#core-architecture)
4. [Graph Container Interface (GCI)](#graph-container-interface-gci)
5. [Concepts and Type Traits](#concepts-and-type-traits)
6. [Graph Views](#graph-views)
7. [Graph Containers](#graph-containers)
8. [Algorithms](#algorithms)
9. [Building and Testing](#building-and-testing)
10. [Key Files Reference](#key-files-reference)
11. [Naming Conventions](#naming-conventions)
12. [Usage Examples](#usage-examples)
13. [Known Limitations](#known-limitations)

---

## Project Structure

```
graph-v2/
├── include/graph/                    # Main library headers
│   ├── graph.hpp                     # Core concepts and types
│   ├── graph_info.hpp                # vertex_info and edge_info structs
│   ├── graph_utility.hpp             # Utility functions for graph construction
│   ├── edgelist.hpp                  # Edgelist concepts and functions
│   ├── algorithm/                    # Graph algorithms
│   │   ├── dijkstra_shortest_paths.hpp
│   │   ├── bellman_ford_shortest_paths.hpp
│   │   ├── breadth_first_search.hpp
│   │   ├── depth_first_search.hpp
│   │   ├── topological_sort.hpp
│   │   └── experimental/             # Experimental algorithms
│   ├── container/                    # Graph containers
│   │   ├── compressed_graph.hpp      # CSR format (high-performance, static)
│   │   ├── dynamic_graph.hpp         # Flexible, dynamic structure
│   │   └── container_utility.hpp     # Container helper concepts
│   ├── detail/                       # Implementation details
│   │   ├── graph_cpo.hpp             # Customization Point Objects
│   │   ├── descriptor.hpp            # Descriptor types
│   │   └── graph_using.hpp           # Using declarations
│   └── views/                        # Graph traversal views
│       ├── vertexlist.hpp
│       ├── incidence.hpp
│       ├── neighbors.hpp
│       ├── edgelist.hpp
│       ├── breadth_first_search.hpp
│       └── depth_first_search.hpp
├── tests/                            # Unit tests (Catch2)
├── example/                          # Example code
│   ├── CppCon2021/
│   ├── CppCon2022/
│   └── AdaptingThirdPartyGraph/
├── benchmark/                        # Performance benchmarks
├── doc/                              # Documentation
├── cmake/                            # CMake modules
└── data/                             # Test data files
```

---

## Namespaces

The library organizes functionality into a hierarchy of namespaces:

### Namespace Hierarchy

```
graph                           # Root namespace - core library
├── graph::views                # View factory functions (CPOs)
├── graph::container            # Graph container implementations
├── graph::edgelist             # Edgelist-specific concepts and types
└── graph::experimental         # Experimental/unstable features
```

### Namespace Details

| Namespace             | Purpose                | Key Contents                                                                                                                                                                                           |
| --------------------- | ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `graph`               | Core library namespace | Concepts (`adjacency_list`, `index_adjacency_list`), type aliases (`vertex_id_t<G>`, `edge_reference_t<G>`), CPOs (`vertices`, `edges`, `vertex_id`, `target_id`), algorithms, `graph_error` exception |
| `graph::views`        | View factory functions | `vertexlist(g)`, `incidence(g,u)`, `neighbors(g,u)`, `edgelist(g)`, `vertices_breadth_first_search(g,seed)`, `edges_depth_first_search(g,seed)`, etc.                                                  |
| `graph::container`    | Graph containers       | `compressed_graph`, `dynamic_graph`, `dynamic_edge`, `dynamic_vertex`, traits (`vofl_graph_traits`, `vol_graph_traits`, `vov_graph_traits`), `copyable_edge_t`, `copyable_vertex_t`                    |
| `graph::edgelist`     | Edgelist abstractions  | Concepts (`basic_sourced_edgelist`, `basic_sourced_index_edgelist`), type aliases (`edge_t<EL>`, `vertex_id_t<EL>`), `source_id(e)`, `target_id(e)` for edgelists                                      |
| `graph::experimental` | Unstable features      | Coroutine-based algorithms (`co_bfs`, `co_dijkstra`), visitor-based Dijkstra variants                                                                                                                  |

### Usage Patterns

```cpp
// Common using declarations
using namespace graph;                    // Core types, concepts, CPOs
using namespace graph::views;             // View factory functions
using namespace graph::container;         // Container types

// Or use qualified names
graph::vertex_id_t<G> uid = 0;
graph::views::vertexlist(g);
graph::container::compressed_graph<double> csr;

// Typical algorithm file includes
#include "graph/graph.hpp"                          // Core (namespace graph)
#include "graph/views/incidence.hpp"                // Views (namespace graph + graph::views)
#include "graph/container/compressed_graph.hpp"    // Containers (namespace graph::container)
```

### Internal Namespaces (Implementation Details)

| Namespace                                                     | Purpose                                       |
| ------------------------------------------------------------- | --------------------------------------------- |
| `graph::_Vertices`, `graph::_Vertex_id`, etc.                 | CPO implementation details in `graph_cpo.hpp` |
| `graph::views::_Vertexlist`, `graph::views::_Incidence`, etc. | View CPO implementations                      |
| `graph::views::_Cpos`                                         | Inline namespace exposing view CPO objects    |
| `graph::container::detail`                                    | Container implementation helpers              |
| `graph::_detail`                                              | Shared implementation utilities               |

> **Note:** Namespaces prefixed with `_` are implementation details and should not be used directly.

---

## Core Architecture

### Design Philosophy

The library is built on a **Graph Container Interface (GCI)** that provides uniform access to graph data structures. This enables:

1. Writing algorithms that work with any compliant graph type
2. Easy adaptation of third-party graph implementations
3. Use of standard containers (`vector<vector<int>>`) as simple graphs

### Key Components

```
┌─────────────────────────────────────────────────────────────┐
│                      Algorithms                              │
│  (dijkstra, bellman_ford, bfs, dfs, topological_sort)       │
├─────────────────────────────────────────────────────────────┤
│                        Views                                 │
│  (vertexlist, incidence, neighbors, edgelist, bfs, dfs)     │
├─────────────────────────────────────────────────────────────┤
│              Graph Container Interface (GCI)                 │
│  (CPOs: vertices, edges, vertex_id, target_id, ...)         │
├─────────────────────────────────────────────────────────────┤
│                      Containers                              │
│  (compressed_graph, dynamic_graph, std containers)          │
└─────────────────────────────────────────────────────────────┘
```

---

## Graph Container Interface (GCI)

The GCI defines how to interact with graph data structures through **Customization Point Objects (CPOs)**.

### Primary CPOs (Functions)

| CPO                   | Signature                          | Description                 |
| --------------------- | ---------------------------------- | --------------------------- |
| `vertices(g)`         | `G& → vertex_range_t<G>`           | Range of all vertices       |
| `edges(g, u)`         | `G&, vertex_ref → edge_range`      | Edges incident to vertex    |
| `edges(g, uid)`       | `G&, vertex_id → edge_range`       | Edges by vertex id          |
| `vertex_id(g, ui)`    | `G&, vertex_iter → vertex_id_t<G>` | Get vertex's id             |
| `target_id(g, uv)`    | `G&, edge_ref → vertex_id_t<G>`    | Target vertex id of edge    |
| `source_id(g, uv)`    | `G&, edge_ref → vertex_id_t<G>`    | Source vertex id of edge    |
| `target(g, uv)`       | `G&, edge_ref → vertex_ref`        | Target vertex reference     |
| `source(g, uv)`       | `G&, edge_ref → vertex_ref`        | Source vertex reference     |
| `find_vertex(g, uid)` | `G&, vertex_id → vertex_iter`      | Find vertex by id           |
| `num_vertices(g)`     | `G& → size_t`                      | Number of vertices          |
| `degree(g, u)`        | `G&, vertex_ref → size_t`          | Number of edges from vertex |

### Optional Value CPOs

| CPO                  | Description                  |
| -------------------- | ---------------------------- |
| `vertex_value(g, u)` | User-defined value on vertex |
| `edge_value(g, uv)`  | User-defined value on edge   |
| `graph_value(g)`     | User-defined value on graph  |

### Type Aliases

```cpp
vertex_range_t<G>      // Type returned by vertices(g)
vertex_iterator_t<G>   // Iterator type for vertex range
vertex_t<G>            // Vertex value type
vertex_reference_t<G>  // Vertex reference type
vertex_id_t<G>         // Vertex identifier type (typically integral)

edge_range_t<G>        // Type returned by edges(g, u)
edge_iterator_t<G>     // Iterator type for edge range
edge_t<G>              // Edge value type
edge_reference_t<G>    // Edge reference type
```

---

## Concepts and Type Traits

### Core Graph Concepts

```cpp
// Basic adjacency list with vertex_id, without vertex object
template <class G>
concept basic_adjacency_list;

// Adjacency list with both vertex_id and vertex object
template <class G>
concept adjacency_list;

// Index adjacency list: random-access vertices, integral vertex_id
template <class G>
concept index_adjacency_list;

// Adjacency list with source_id on edges
template <class G>
concept sourced_adjacency_list;
```

### Edge Concepts

```cpp
template <class G>
concept basic_targeted_edge;    // Has target_id(g, uv)

template <class G>
concept targeted_edge;          // Has target_id + target(g, uv)

template <class G>
concept basic_sourced_edge;     // Has source_id(g, uv)

template <class G>
concept sourced_edge;           // Has source_id + source(g, uv)

template <class G>
concept unordered_edge;         // source_id/target_id are unordered
```

### Property Concepts

```cpp
template <class G>
concept has_degree;             // degree(g, u) exists

template <class G>
concept has_find_vertex;        // find_vertex(g, uid) exists

template <class G>
concept has_find_vertex_edge;   // find_vertex_edge(g, uid, vid) exists
```

---

## Graph Views

Views provide lazy, composable iteration over graph elements. Located in `include/graph/views/`.

### Basic Views

| View                   | Description         | Value Type                      |
| ---------------------- | ------------------- | ------------------------------- |
| `vertexlist(g)`        | Iterate vertices    | `{id, vertex&}`                 |
| `vertexlist(g, vvf)`   | With value function | `{id, vertex&, value}`          |
| `incidence(g, u)`      | Edges from vertex   | `{target_id, edge&}`            |
| `incidence(g, u, evf)` | With value function | `{target_id, edge&, value}`     |
| `neighbors(g, u)`      | Adjacent vertices   | `{id, vertex&}`                 |
| `edgelist(g)`          | All edges           | `{source_id, target_id, edge&}` |

### Traversal Views

| View                                          | Description                |
| --------------------------------------------- | -------------------------- |
| `vertices_breadth_first_search(g, seed)`      | BFS vertex traversal       |
| `edges_breadth_first_search(g, seed)`         | BFS edge traversal         |
| `sourced_edges_breadth_first_search(g, seed)` | BFS edges with source      |
| `vertices_depth_first_search(g, seed)`        | DFS vertex traversal       |
| `edges_depth_first_search(g, seed)`           | DFS edge traversal         |
| `topological_sort_vertices(g, seed)`          | Topological order vertices |

### Example Usage

```cpp
// Iterate over vertices with id
for (auto&& [uid, u] : vertexlist(g)) {
    // uid is vertex id, u is vertex reference
}

// BFS from a seed vertex
for (auto&& [vid, v] : vertices_breadth_first_search(g, start_id)) {
    // Process vertices in BFS order
}

// Iterate edges with values
for (auto&& [target_id, edge, weight] : incidence(g, u, weight_fn)) {
    // Process edges with custom weight function
}
```

---

## Graph Containers

### compressed_graph (CSR Format)

**Location:** `include/graph/container/compressed_graph.hpp`

High-performance, static structure using Compressed Sparse Row format.

```cpp
template <class EV        = void,         // Edge value type
          class VV        = void,         // Vertex value type
          class GV        = void,         // Graph value type
          integral VId    = uint32_t,     // Vertex id type
          integral EIndex = uint32_t,     // Edge index type
          class Alloc     = std::allocator<VId>>
class compressed_graph;
```

**Characteristics:**
- Memory efficient
- Cache-friendly iteration
- Static structure (no dynamic modification)
- Best for read-heavy workloads

**Construction:**
```cpp
// From edge data
vector<copyable_edge_t<VId, double>> edges = {...};
compressed_graph<double> g(edges);

// With projection
compressed_graph<double, string> g(edge_data, edge_proj, vertex_data, vertex_proj);
```

### dynamic_graph

**Location:** `include/graph/container/dynamic_graph.hpp`

Flexible graph with configurable vertex and edge containers.

```cpp
template <class EV     = void,
          class VV     = void,
          class GV     = void,
          class VId    = uint32_t,
          bool Sourced = false,
          class Traits = vofl_graph_traits<EV, VV, GV, VId, Sourced>>
class dynamic_graph;
```

**Traits Options:**

| Traits              | Vertices | Edges          | Use Case                     |
| ------------------- | -------- | -------------- | ---------------------------- |
| `vofl_graph_traits` | `vector` | `forward_list` | General purpose              |
| `vol_graph_traits`  | `vector` | `list`         | Need bidirectional iteration |
| `vov_graph_traits`  | `vector` | `vector`       | Dense graphs                 |

**Example:**
```cpp
using traits = vofl_graph_traits<double, string>;  // EV=double, VV=string
using Graph = dynamic_adjacency_graph<traits>;

Graph g;
g.load_edges(edge_range, edge_projection);
g.load_vertices(vertex_range, vertex_projection);
```

### Standard Container Graphs

Simple graphs can be created from standard containers:

```cpp
// Simple adjacency list: vector<vector<int>>
vector<vector<int>> adj_list(n_vertices);
adj_list[0].push_back(1);  // Edge 0 → 1

// With edge properties: vector<vector<tuple<int, double>>>
vector<vector<tuple<int, double>>> weighted_adj(n);
weighted_adj[0].push_back({1, 2.5});  // Edge 0 → 1, weight 2.5
```

---

## Algorithms

**Location:** `include/graph/algorithm/`

### Shortest Paths

#### Dijkstra's Algorithm
```cpp
#include "graph/algorithm/dijkstra_shortest_paths.hpp"

// Full version with visitor
dijkstra_shortest_paths(g, sources, distances, predecessors, weight_fn, visitor);

// Distances only
dijkstra_shortest_distances(g, source, distances, weight_fn);
```

**Complexity:** O((V + E) log V)  
**Requirements:** Non-negative edge weights

#### Bellman-Ford Algorithm
```cpp
#include "graph/algorithm/bellman_ford_shortest_paths.hpp"

auto cycle = bellman_ford_shortest_paths(g, sources, distances, predecessors, weight_fn);
// Returns optional<vertex_id> if negative cycle found
```

**Complexity:** O(V × E)  
**Features:** Handles negative weights, detects negative cycles

### Search Algorithms

#### Breadth-First Search
```cpp
#include "graph/algorithm/breadth_first_search.hpp"

// Algorithm form
breadth_first_search(g, source, visitor);

// View form
for (auto&& [vid, v] : vertices_breadth_first_search(g, source)) { }
```

#### Depth-First Search
```cpp
#include "graph/algorithm/depth_first_search.hpp"

// Algorithm form
depth_first_search(g, source, visitor);

// View form
for (auto&& [vid, v] : vertices_depth_first_search(g, source)) { }
```

### Other Algorithms

| Algorithm               | Header                     | Status               |
| ----------------------- | -------------------------- | -------------------- |
| Topological Sort        | `topological_sort.hpp`     | Implemented          |
| Connected Components    | `connected_components.hpp` | Implemented          |
| Transitive Closure      | `transitive_closure.hpp`   | Implemented          |
| Triangle Counting       | `tc.hpp`                   | Basic implementation |
| Maximal Independent Set | `mis.hpp`                  | Basic implementation |
| Minimum Spanning Tree   | `mst.hpp`                  | Basic implementation |

### Visitors

Algorithms support visitor patterns for event callbacks:

```cpp
struct my_visitor {
    void on_initialize_vertex(vertex_desc_type& vdesc) { }
    void on_discover_vertex(vertex_desc_type& vdesc) { }
    void on_examine_vertex(vertex_desc_type& vdesc) { }
    void on_finish_vertex(vertex_desc_type& vdesc) { }
    void on_examine_edge(edge_desc_type& edesc) { }
    void on_edge_relaxed(edge_desc_type& edesc) { }
    void on_edge_not_relaxed(edge_desc_type& edesc) { }
};
```

---

## Building and Testing

### Requirements

- C++20 compliant compiler (MSVC 2022, GCC 13+, Clang)
- CMake 3.20+

### Quick Build

```bash
git clone https://github.com/stdgraph/graph-v2.git
cd graph-v2
mkdir build && cd build
cmake ..
make
```

### CMake Options

| Option                | Default | Description                       |
| --------------------- | ------- | --------------------------------- |
| `ENABLE_TESTING`      | ON      | Build unit tests                  |
| `ENABLE_EXAMPLES`     | ON      | Build examples                    |
| `ENABLE_BENCHMARKING` | OFF     | Build benchmarks (requires C++23) |
| `ENABLE_PCH`          | OFF     | Use precompiled headers           |

### Running Tests

```bash
cd build
ctest
```

### Test Data

Test data files are in `data/`:
- `germany_routes.csv` - Road network example
- `karate.mtx` - Social network (Matrix Market format)
- Various `.mtx` and `.csv` files for algorithm testing

---

## Key Files Reference

### Must-Read for Understanding the Library

| File                                                                                            | Description                           |
| ----------------------------------------------------------------------------------------------- | ------------------------------------- |
| [include/graph/graph.hpp](../include/graph/graph.hpp)                                           | Core concepts and type aliases        |
| [include/graph/detail/graph_cpo.hpp](../include/graph/detail/graph_cpo.hpp)                     | CPO implementations (2600+ lines)     |
| [include/graph/graph_info.hpp](../include/graph/graph_info.hpp)                                 | `vertex_info` and `edge_info` structs |
| [include/graph/container/compressed_graph.hpp](../include/graph/container/compressed_graph.hpp) | CSR container implementation          |
| [include/graph/container/dynamic_graph.hpp](../include/graph/container/dynamic_graph.hpp)       | Dynamic container implementation      |

### Algorithm Examples

| File                                                                                              | Description             |
| ------------------------------------------------------------------------------------------------- | ----------------------- |
| [tests/dijkstra_shortest_paths_tests.cpp](../tests/dijkstra_shortest_paths_tests.cpp)             | Dijkstra usage examples |
| [tests/bfs_tests.cpp](../tests/bfs_tests.cpp)                                                     | BFS view usage          |
| [example/CppCon2022/germany_routes_example.cpp](../example/CppCon2022/germany_routes_example.cpp) | Complete example        |

---

## Naming Conventions

### Template Parameters

| Parameter | Variable         | Description                        |
| --------- | ---------------- | ---------------------------------- |
| `G`       | `g`              | Graph                              |
| `GV`      | -                | Graph Value (user-defined or void) |
| `V`       | -                | Vertex type                        |
| `VId`     | `uid, vid, seed` | Vertex Id                          |
| `VV`      | -                | Vertex Value                       |
| `VR`      | -                | Vertex Range                       |
| `VI`      | `ui, vi`         | Vertex Iterator                    |
| `VVF`     | -                | Vertex Value Function              |
| `E`       | -                | Edge type                          |
| `EV`      | -                | Edge Value                         |
| `ER`      | -                | Edge Range                         |
| `EI`      | `uvi, vwi`       | Edge Iterator                      |
| `EVF`     | `evf`            | Edge Value Function                |
| -         | `u, v, x, y`     | Vertex reference                   |
| -         | `uv, vw`         | Edge reference                     |

### Concept Naming

- `basic_*` - Minimal requirements (id-based)
- `index_*` - Requires integral ids and random-access ranges
- `sourced_*` - Includes source vertex information

---

## Usage Examples

### Simple Graph Construction

```cpp
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"

// Using standard containers
vector<vector<int>> g(5);  // 5 vertices
g[0].push_back(1);         // Edge: 0 → 1
g[0].push_back(2);         // Edge: 0 → 2
g[1].push_back(3);         // Edge: 1 → 3

// Iterate vertices
for (auto&& [uid, u] : graph::views::vertexlist(g)) {
    cout << "Vertex " << uid << endl;
}

// Iterate edges from vertex 0
for (auto&& [vid, edge] : graph::views::incidence(g, 0)) {
    cout << "Edge to " << vid << endl;
}
```

### Shortest Path Example

```cpp
#include "graph/container/compressed_graph.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"

using namespace graph;
using namespace graph::container;

// Create graph from edge data
using Edge = copyable_edge_t<uint32_t, double>;
vector<Edge> edges = {
    {0, 1, 1.0}, {0, 2, 4.0}, {1, 2, 2.0}, {2, 3, 1.0}
};
compressed_graph<double> g(edges);

// Run Dijkstra from vertex 0
vector<double> distances(num_vertices(g), numeric_limits<double>::max());
vector<uint32_t> predecessors(num_vertices(g));
iota(predecessors.begin(), predecessors.end(), 0);

distances[0] = 0;
dijkstra_shortest_paths(g, {0}, distances, predecessors,
    [](auto& edge) { return edge_value(g, edge); });
```

---

## Known Limitations

1. **Alpha Stage** - API may change significantly
2. **Breaking Change Planned** - New descriptor concept will replace current id/reference approach
3. **Hyper-graphs** - Not supported (out of scope)
4. **Concurrent Access** - Not thread-safe by default
5. **Dynamic Modifications** - `compressed_graph` is static; use `dynamic_graph` for modifications

---

## Related ISO Papers

| Paper                            | Description                |
| -------------------------------- | -------------------------- |
| [P3126](https://wg21.link/P3126) | Overview                   |
| [P3127](https://wg21.link/P3127) | Background and Terminology |
| [P3128](https://wg21.link/P3128) | Algorithms                 |
| [P3129](https://wg21.link/P3129) | Views                      |
| [P3130](https://wg21.link/P3130) | Graph Container Interface  |
| [P3131](https://wg21.link/P3131) | Graph Containers           |

---

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## License

Boost Software License 1.0 - See [LICENSE](../LICENSE)
