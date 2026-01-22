# Refactor namespaces

Adjacency lists and edge lists are two abstract data structures used by graphs.
While different, there are similarities because the edges in an adjacency list
can be presented as a flat edge list. The structs in include/graph/graph_info.hpp
contain the core data structures that help tie the design together so that
algorithms can use an edge list from a flat data structure (e.g. vector of structs)
or from a range of ranges (e.g. adjacency list).

The current namespace structure stores everything related to the abstract adjacency lists 
in the `graph` namespace. The `graph::edgelist` namespace holds everything related to
the abstract edge lists. But abstract edge lists are a peer of adjacency lists, not a
subset or refinement to them.

## Goal

Reorganize the namespaces into the following structure:

```
graph                                    # Root: algorithms, common types (vertex_info, edge_info, graph_error)
├── graph::adj_list                      # Adjacency list concepts, types, CPOs
│   └── graph::adj_list::views           # View factories for adjacency lists
├── graph::edge_list                     # Edge list concepts, types, CPOs
│   └── graph::edge_list::views          # View factories for edge lists
├── graph::container                     # Concrete implementations (compressed_graph, dynamic_graph)
└── graph::experimental                  # Experimental/unstable features
```

The existing `graph::edgelist` must be renamed `graph::edge_list` to help distinguish between a namespace,
concepts and functions.

Most of the existing functionality in the `graph` namespace will be put into the `graph::adj_list`
namespace.

## Rationale

### Why Algorithms Stay in `graph` Root Namespace

Algorithms remain in the `graph` namespace (not in `graph::adj_list`) because:

1. **Algorithms operate on abstract concepts via the Graph Container Interface (GCI)**
   - They work through CPOs like `vertices()`, `edges()`, `target_id()`
   - They don't care about the underlying representation (adjacency list, edge list, or matrix)
   - Placing them in `adj_list` would incorrectly imply they're tied to that data structure

2. **Algorithms are the primary user-facing API**
   - Users expect `graph::dijkstra_shortest_paths()`, not `graph::adj_list::dijkstra_shortest_paths()`
   - Keeping them at root level improves discoverability and ergonomics
   - Follows standard library precedent: `std::sort`, not `std::ranges::sort`

3. **Clear separation of concerns**
   - **Representation** (`adj_list`, `edge_list`) - how graph data is structured
   - **Behavior** (`graph` algorithms) - operations on abstract graphs
   - **Implementation** (`container`) - concrete data structures

### Namespace Contents

#### `graph` (Root Namespace)

**Contains:**
- **Algorithms**: `dijkstra_shortest_paths`, `bellman_ford_shortest_paths`, `breadth_first_search`, 
  `depth_first_search`, `topological_sort`, `connected_components`, `transitive_closure`, etc.
- **Common types**: `vertex_info<VId,V,VV>`, `edge_info<VId,Sourced,E,EV>`, 
  `copyable_vertex_t<VId,VV>`, `copyable_edge_t<VId,EV>`
- **Exception types**: `graph_error`
- **Utilities**: Functions that work across both adjacency lists and edge lists

**Rationale:** These are fundamental, representation-agnostic components used by all graph code.

#### `graph::adj_list`

**Contains:**
- **Concepts**: `adjacency_list`, `index_adjacency_list`, `sourced_adjacency_list`, 
  `basic_adjacency_list`, `vertex_range`, `targeted_edge_range`, etc.
- **CPOs**: `vertices(g)`, `edges(g, u)`, `edges(g, uid)`, `vertex_id(g, ui)`, 
  `target_id(g, uv)`, `source_id(g, uv)`, `target(g, uv)`, `source(g, uv)`, 
  `find_vertex(g, uid)`, `num_vertices(g)`, `degree(g, u)`, `vertex_value(g, u)`, 
  `edge_value(g, uv)`, `graph_value(g)`
- **Type aliases**: `vertex_range_t<G>`, `vertex_iterator_t<G>`, `vertex_t<G>`, 
  `vertex_reference_t<G>`, `vertex_id_t<G>`, `edge_range_t<G>`, `edge_iterator_t<G>`, 
  `edge_t<G>`, `edge_reference_t<G>`, etc.
- **Edge concepts for adjacency lists**: `basic_targeted_edge<G>`, `targeted_edge<G>`, 
  `basic_sourced_edge<G>`, `sourced_edge<G>`, `unordered_edge<G>`, `ordered_edge<G>`
- **Property concepts**: `has_degree<G>`, `has_find_vertex<G>`, `has_find_vertex_edge<G>`, 
  `has_contains_edge<G>`
- **Traits**: `define_unordered_edge<G>`, `define_adjacency_matrix<G>`

**Rationale:** These define the interface for graphs represented as adjacency lists.

#### `graph::adj_list::views`

**Contains:**
- **Basic views**: `vertexlist(g)`, `vertexlist(g, vvf)`, `incidence(g, u)`, `incidence(g, u, evf)`, 
  `neighbors(g, u)`, `neighbors(g, u, vvf)`, `edgelist(g)`, `edgelist(g, evf)`
- **Traversal views**: `vertices_breadth_first_search(g, seed)`, `edges_breadth_first_search(g, seed)`, 
  `sourced_edges_breadth_first_search(g, seed)`, `vertices_depth_first_search(g, seed)`, 
  `edges_depth_first_search(g, seed)`, `topological_sort_vertices(g, seed)`, etc.

**Rationale:** These views provide iteration over adjacency list graph structures. The `views` 
sub-namespace allows convenient `using namespace graph::adj_list::views;` without polluting the 
parent namespace.

#### `graph::edge_list`

**Contains:**
- **Concepts**: `basic_sourced_edgelist<EL>`, `basic_sourced_index_edgelist<EL>`, 
  `has_edge_value<EL>`
- **CPOs**: `source_id(e)`, `target_id(e)`, `edge_value(e)` (for standalone edge objects)
- **Type aliases**: `edge_range_t<EL>`, `edge_iterator_t<EL>`, `edge_t<EL>`, 
  `edge_reference_t<EL>`, `edge_value_t<EL>`, `vertex_id_t<EL>`
- **Traits**: `is_directed<EL>`

**Rationale:** These define the interface for graphs represented as flat edge lists. Note that CPOs 
like `source_id()` are overloaded - the adjacency list version takes `(graph, edge)` while the 
edge list version takes just `(edge)`.

#### `graph::edge_list::views`

**Contains:**
- Views that operate directly on edge list data structures
- Potentially: filtering views, transformation views, etc. for edge lists
- TBD: Specific views to be designed based on edge list use cases

**Rationale:** Provides views specifically designed for edge list iteration patterns, parallel to 
`adj_list::views`.

#### `graph::container`

**Contains:**
- **Containers**: `compressed_graph<EV,VV,GV,VId,EIndex,Alloc>`, 
  `dynamic_graph<EV,VV,GV,VId,Sourced,Traits>`
- **Dynamic graph components**: `dynamic_edge`, `dynamic_vertex`
- **Traits**: `vofl_graph_traits`, `vol_graph_traits`, `vov_graph_traits`
- **Type aliases**: `dynamic_adjacency_graph<Traits>`
- **Utility types**: `csr_row<EIndex>`, `csr_col<VId>`
- **Helpers**: `copyable_edge_t` (for loading), concepts like `reservable`, `resizable`, 
  `has_emplace_back`, etc.

**Rationale:** These are concrete implementations of graph containers that satisfy the adjacency 
list concepts defined in `graph::adj_list`.

#### `graph::experimental`

**Contains:**
- Coroutine-based algorithms: `co_bfs`, `co_dijkstra`
- Visitor-based algorithm variants
- Features being tested before moving to stable namespaces

**Rationale:** Isolated area for unstable features that may change or be removed.

## Key Design Decisions

### 1. Function Overloading for CPOs

Both `graph::adj_list` and `graph::edge_list` may have CPOs with the same names (e.g., `source_id`, 
`target_id`), but they're distinguished by signature:

```cpp
// Adjacency list version - takes graph + edge
graph::adj_list::source_id(g, edge_ref)  

// Edge list version - takes standalone edge
graph::edge_list::source_id(edge)        
```

This allows clear intent through the signature while avoiding name collisions.

### 2. Shared Types in Root Namespace

Types like `vertex_info` and `edge_info` stay in `graph` root namespace because:
- They're used by both adjacency list views and edge list structures
- They're fundamental building blocks
- Placing them in a sub-namespace would require qualification in too many places

### 3. Using Namespace Convenience

Users can selectively import namespaces:

```cpp
// For algorithm-focused code
#include "graph/algorithm/dijkstra_shortest_paths.hpp"
using namespace graph;  // Gets algorithms

// For view-heavy code
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
using namespace graph::adj_list::views;  // Gets view factories

// For everything
using namespace graph;
using namespace graph::adj_list;
using namespace graph::adj_list::views;
```

## Potential Issues & Resolutions

### 1. Name Ambiguity with Type Aliases

**Issue**: `vertex_id_t<G>` (adjacency list) vs `vertex_id_t<EL>` (edge list) have the same name.

**Resolution**: 
- Keep in separate namespaces
- Template parameter type (`G` vs `EL`) distinguishes them
- Users qualify explicitly when needed: `graph::adj_list::vertex_id_t<MyGraph>`

### 2. Migration from Current Code

**Issue**: This is a massive breaking change affecting all existing code.

**Resolution** (for when made public):
- Provide comprehensive migration guide
- Consider namespace aliases for transition period:
  ```cpp
  namespace graph {
      namespace views = adj_list::views;  // Temporary compatibility
  }
  ```
- Update all examples and documentation simultaneously
- Provide automated migration script if feasible

### 3. Import Verbosity

**Issue**: Users may need multiple `using` declarations.

**Resolution**:
- Provide a convenience header `graph/graph_all.hpp` that includes common components
- Document recommended import patterns for different use cases
- Most users will only need 2-3 namespaces maximum

## Testing Strategy

Since this is experimental:

1. **Create parallel namespace structure** without removing old namespaces
2. **Implement key components** in new structure to validate design
3. **Write example code** using new namespaces to test ergonomics
4. **Validate concept checking** works correctly with new organization
5. **Test overload resolution** for CPOs that exist in multiple namespaces
6. **Document usage patterns** that emerge from examples

## Open Questions

1. **Edge list views**: What specific views are needed for `graph::edge_list::views`?
2. **Visitor patterns**: Do visitors stay in `graph` or move to a sub-namespace?
3. **Utility functions**: Where do graph construction utilities (from `graph_utility.hpp`) belong?
4. **Internal namespaces**: How to handle implementation detail namespaces (currently prefixed with `_`)?

## Success Criteria

The refactoring is successful if:

1. ✅ Clear separation between representation (adj_list, edge_list) and behavior (algorithms)
2. ✅ Algorithms are easily discoverable at root level
3. ✅ Related functionality is grouped logically
4. ✅ Common usage patterns require minimal namespace qualification
5. ✅ Name collisions are avoided or resolved through overloading
6. ✅ Migration path is clear (when made public)
