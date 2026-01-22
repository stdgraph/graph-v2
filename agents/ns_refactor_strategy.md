# Namespace Refactoring Implementation Strategy

This document outlines the step-by-step strategy for implementing the namespace refactoring described in `ns_refactor_goal.md`.

## Overview

The refactoring moves from the current structure:
```
graph                    # Everything adjacency list related
└── graph::edgelist      # Edge list abstractions
```

To the new structure:
```
graph                           # Algorithms, common types
├── graph::adj_list             # Adjacency list abstractions
│   └── graph::adj_list::views  # Adjacency list views
├── graph::edge_list            # Edge list abstractions
│   └── graph::edge_list::views # Edge list views
├── graph::container            # Concrete containers
└── graph::experimental         # Unstable features
```

## Safety and Validation Principles

- **Parallel, non-breaking steps**: New namespaces and container interfaces are added alongside the old ones first; removals are deferred until the end.
- **Incremental validation**: Each phase lists explicit validation; run targeted tests at the end of every phase before proceeding.
- **Separation of risky work**: Containers are updated in two passes (non-CPO, then CPO satisfaction) before views, and views before algorithms.
- **Fast rollback**: Keep work on a feature branch; phases are isolated so you can revert to the prior phase if validation fails.
- **Scope gates**: Do not start the next phase until the current phase’s validation checklist passes (compile + targeted tests).

## Implementation Phases

### Phase 0: Preparation and Analysis

**Goal**: Understand dependencies and create tooling support.

**Tasks**:
1. ✅ Document current namespace structure (done in `overview.md`)
2. ✅ Document refactoring goals (done in `ns_refactor_goal.md`)
3. Create dependency map of headers
4. Identify all CPO definitions and their locations
5. List all concepts and their dependencies
6. Set up test branch for experimental work

**Tooling**:
```bash
# Find all namespace declarations
grep -r "namespace graph" include/graph --include="*.hpp"

# Find all CPO definitions (look for _Cpo classes)
grep -r "class _Cpo" include/graph --include="*.hpp"

# Find all concept definitions
grep -r "^template.*concept " include/graph --include="*.hpp"
```

**Validation**: Complete inventory of what needs to move and where.

---

### Phase 1: Create New Namespace Structure (Parallel Implementation)

**Goal**: Establish new namespaces alongside existing ones without breaking anything.

**Strategy**: Add new namespaces and gradually populate them while keeping old ones intact.

#### Step 1.1: Create `graph::adj_list` namespace

**Files to modify**:
- `include/graph/graph.hpp` - Add new namespace, move concepts
- `include/graph/detail/graph_cpo.hpp` - Add new namespace sections for CPOs

**Approach**:
```cpp
// In graph.hpp
namespace graph {
    // Keep existing for now
    
    namespace adj_list {
        // NEW: Copy concepts here
        template <class G>
        concept adjacency_list = /* existing definition */;
        
        template <class G>
        concept index_adjacency_list = /* existing definition */;
        
        // ... other concepts
    }
}
```

**Validation**:
- Existing code still compiles
- New namespace can be accessed: `graph::adj_list::adjacency_list<G>`

#### Step 1.2: Rename `graph::edgelist` to `graph::edge_list`

**Files to modify**:
- `include/graph/edgelist.hpp`

**Approach**:
```cpp
namespace graph {
    // Add new namespace
    namespace edge_list {
        // Move everything from old edgelist namespace
    }
    
    // Deprecated: temporary alias for compatibility
    namespace edgelist = edge_list;
}
```

**Validation**:
- `graph::edge_list::basic_sourced_edgelist<EL>` works
- Old `graph::edgelist::basic_sourced_edgelist<EL>` still works (via alias)

#### Step 1.3: Create views sub-namespaces

**Files to create/modify**:
- `include/graph/views/vertexlist.hpp` - Add `graph::adj_list::views` namespace
- `include/graph/views/incidence.hpp` - Add `graph::adj_list::views` namespace
- Other view headers

**Approach**:
```cpp
// In each view header
namespace graph {
    // Keep existing implementation
    
    namespace adj_list {
        namespace views {
            // NEW: Add factory functions here
            inline namespace _Cpos {
                inline constexpr auto vertexlist = /* ... */;
            }
        }
    }
}
```

**Validation**:
- `graph::adj_list::views::vertexlist(g)` works
- Old `graph::views::vertexlist(g)` still works

---

### Phase 2: Move CPOs to New Namespaces

**Goal**: Relocate CPO implementations to appropriate namespaces.

**Critical File**: `include/graph/detail/graph_cpo.hpp` (~2600 lines)

#### Step 2.1: Move adjacency list CPOs

**CPOs to move to `graph::adj_list`**:
- `vertices(g)` - `_Vertices::_Cpo`
- `edges(g, u)` / `edges(g, uid)` - `_Edges::_Cpo`
- `vertex_id(g, ui)` - `_Vertex_id::_Cpo`
- `target_id(g, uv)` - `_Target_id::_Cpo`
- `source_id(g, uv)` - `_Source_id::_Cpo`
- `target(g, uv)` - `_Target::_Cpo`
- `source(g, uv)` - `_Source::_Cpo`
- `find_vertex(g, uid)` - `_Find_vertex::_Cpo`
- `num_vertices(g)` - `_Num_vertices::_Cpo`
- `degree(g, u)` - `_Degree::_Cpo`
- `vertex_value(g, u)` - CPO for vertex values
- `edge_value(g, uv)` - CPO for edge values
- `graph_value(g)` - CPO for graph value

**Approach**:
```cpp
// In graph_cpo.hpp
namespace graph {
    namespace adj_list {
        // Move each CPO namespace here
        namespace _Vertices {
            /* existing implementation */
            class _Cpo { /* ... */ };
        }
        inline namespace _Cpos {
            inline constexpr _Vertices::_Cpo vertices;
        }
        
        namespace _Edges {
            /* existing implementation */
        }
        inline namespace _Cpos {
            inline constexpr _Edges::_Cpo edges;
        }
        
        // ... repeat for all CPOs
    }
}
```

**Validation**:
- Each CPO accessible as `graph::adj_list::vertices`, etc.
- Create test file that uses new qualified names

#### Step 2.2: Move edge list CPOs

**CPOs to move to `graph::edge_list`**:
- `source_id(e)` - For standalone edges
- `target_id(e)` - For standalone edges  
- `edge_value(e)` - For standalone edges

**Note**: These are likely implemented via ADL or member functions, not full CPO machinery.

**Validation**:
- Overload resolution correctly distinguishes `adj_list::source_id(g, uv)` from `edge_list::source_id(e)`

---

### Phase 3: Move Type Aliases

**Goal**: Relocate type aliases to appropriate namespaces.

**Files to modify**:
- `include/graph/detail/graph_using.hpp` (or inline in headers)

#### Step 3.1: Move adjacency list type aliases

**Aliases to move to `graph::adj_list`**:
```cpp
namespace graph::adj_list {
    // Vertex types
    template <class G>
    using vertex_range_t = /* ... */;
    
    template <class G>
    using vertex_iterator_t = /* ... */;
    
    template <class G>
    using vertex_t = /* ... */;
    
    template <class G>
    using vertex_reference_t = /* ... */;
    
    template <class G>
    using vertex_id_t = /* ... */;
    
    // Edge types
    template <class G>
    using edge_range_t = /* ... */;
    
    template <class G>
    using edge_iterator_t = /* ... */;
    
    template <class G>
    using edge_t = /* ... */;
    
    template <class G>
    using edge_reference_t = /* ... */;
    
    // ... etc
}
```

#### Step 3.2: Move edge list type aliases

**Aliases to move to `graph::edge_list`**:
```cpp
namespace graph::edge_list {
    template <class EL>
    using edge_range_t = /* ... */;
    
    template <class EL>
    using edge_iterator_t = /* ... */;
    
    template <class EL>
    using edge_t = /* ... */;
    
    template <class EL>
    using vertex_id_t = /* ... */;
    
    // ... etc
}
```

**Validation**:
- Type aliases compile in new locations
- Check that template argument deduction works correctly

---

### Phase 4: Update Container Implementations (Non-CPO Interface)

**Goal**: Update container implementations to work with new namespace structure, focusing on non-CPO interfaces first.

**Files to modify**:
- `include/graph/container/compressed_graph.hpp`
- `include/graph/container/dynamic_graph.hpp`
- `include/graph/container/container_utility.hpp`

**Approach**:
1. Container implementations stay in `graph::container`
2. Update type aliases used internally to reference new namespaces
3. Update concepts used in requires clauses
4. Keep member functions that will satisfy CPOs (but don't test CPO satisfaction yet)

**Example**:
```cpp
namespace graph::container {
    template <class EV = void, class VV = void, class GV = void,
              integral VId = uint32_t, integral EIndex = uint32_t,
              class Alloc = std::allocator<VId>>
    class compressed_graph {
    public:
        // Non-CPO interface (constructors, load functions)
        compressed_graph() = default;
        
        template <forward_range ERng>
        void load_edges(const ERng& erng) { /* ... */ }
        
        // Member functions that will satisfy CPOs later
        auto vertices() { /* ... */ }
        auto edges(vertex_id_type uid) { /* ... */ }
        
        // Direct accessors (non-CPO)
        size_t vertex_count() const { return row_index_.size() - 1; }
        size_t edge_count() const { return col_index_.size(); }
    };
}
```

**Testing at this phase**:
- Test container construction: `compressed_graph<double> g;`
- Test loading data: `g.load_edges(edge_data);`
- Test non-CPO accessors: `g.vertex_count()`, `g.edge_count()`
- Test member function calls directly: `g.vertices()`, `g.edges(0)`
- Do NOT test CPO calls yet: `graph::adj_list::vertices(g)` - that comes in Phase 5

**Validation**:
- Containers compile with new namespace references
- Non-CPO interface works (constructors, load functions, direct accessors)
- Member functions that will support CPOs work when called directly
- Create simple test to validate basic container functionality

---

### Phase 5: Add CPO Support to Containers

**Goal**: Verify that containers satisfy the new CPO interface from `graph::adj_list`.

**Files to modify**:
- Test files for containers
- Potentially container headers if ADL or concept satisfaction needs adjustment

**Approach**:
1. Containers already have member functions from Phase 4
2. Now test that CPOs resolve correctly via ADL or member functions
3. Verify concept satisfaction

**Example testing**:
```cpp
namespace graph::container {
    // Container already has member functions from Phase 4
}

// Now test CPO resolution
void test_cpo_satisfaction() {
    using namespace graph::container;
    using namespace graph::adj_list;
    
    compressed_graph<double> g;
    // ... load data ...
    
    // Test CPO calls
    auto vr = vertices(g);           // Should resolve to g.vertices()
    auto er = edges(g, 0);           // Should resolve to g.edges(0)
    auto n = num_vertices(g);        // Should work
    
    // Test concept satisfaction
    static_assert(adjacency_list<compressed_graph<double>>);
    static_assert(index_adjacency_list<compressed_graph<double>>);
}
```

**Testing at this phase**:
- Verify CPO calls resolve correctly: `graph::adj_list::vertices(g)`
- Verify concept satisfaction: `static_assert(graph::adj_list::adjacency_list<G>)`
- Test with both `compressed_graph` and `dynamic_graph`
- Ensure ADL finds the right functions

**Validation**:
- All CPOs resolve correctly for containers
- Containers satisfy `graph::adj_list::adjacency_list` and related concepts
- No ambiguous function calls
- Run container-specific tests to ensure behavior unchanged

---

### Phase 6: Update View Implementations

**Goal**: Update view implementations to use new namespaces and be accessible via `graph::adj_list::views`.

**Files to modify**: All files in `include/graph/views/`

#### Step 5.1: Update view iterator implementations

**For each view file**:
1. Keep main implementation in `graph` namespace (for now)
2. Add factory functions to `graph::adj_list::views`

**Example pattern** (in `vertexlist.hpp`):
```cpp
namespace graph {
    // Keep existing iterator and view classes here
    template <adjacency_list G, class VVF = void>
    class vertexlist_iterator { /* ... */ };
    
    template <adjacency_list G, class VVF = void>
    class vertexlist_view { /* ... */ };
}

namespace graph::adj_list::views {
    namespace _Vertexlist {
        // Factory implementation
        template <class G>
        class _Cpo {
            // Implementation that returns graph::vertexlist_view<G>
        };
    }
    
    inline namespace _Cpos {
        inline constexpr _Vertexlist::_Cpo vertexlist;
    }
}
```

#### Step 5.2: Views to update

- `vertexlist.hpp` → `graph::adj_list::views::vertexlist`
- `incidence.hpp` → `graph::adj_list::views::incidence`
- `neighbors.hpp` → `graph::adj_list::views::neighbors`
- `edgelist.hpp` → `graph::adj_list::views::edgelist`
- `breadth_first_search.hpp` → `graph::adj_list::views::{vertices,edges,sourced_edges}_breadth_first_search`
- `depth_first_search.hpp` → `graph::adj_list::views::{vertices,edges}_depth_first_search`

**Validation**:
- Views accessible via `graph::adj_list::views::vertexlist(g)`
- Using declaration works: `using namespace graph::adj_list::views;`

---

### Phase 7: Update Algorithm Implementations

**Goal**: Update algorithm headers to use new namespaces.

**Dependencies**: Phases 4, 5, 6 complete (containers and views working)

**Files to modify**: All files in `include/graph/algorithm/`

**Rationale for ordering**: Algorithms depend on views (like `incidence`, `edgelist`) which depend on containers. Therefore algorithms come last.

#### Approach for each algorithm file:

1. **Update includes** (if namespaced headers created)
2. **Add using declarations** at top of implementation:
   ```cpp
   namespace graph {
       // Algorithm stays in graph namespace
       
       // Use declarations for convenience
       using graph::adj_list::vertex_id_t;
       using graph::adj_list::edge_reference_t;
       using graph::adj_list::index_adjacency_list;
       // ... etc
       
       template <index_adjacency_list G, /* ... */>
       void my_algorithm(G&& g, /* ... */) {
           // Implementation can now use unqualified names
           vertex_id_t<G> uid = /* ... */;
       }
   }
   ```

3. **Or use fully qualified names**:
   ```cpp
   namespace graph {
       template <graph::adj_list::index_adjacency_list G,
                 /* ... */>
       void my_algorithm(G&& g, /* ... */) {
           graph::adj_list::vertex_id_t<G> uid = /* ... */;
       }
   }
   ```

**Files to update**:
- `dijkstra_shortest_paths.hpp`
- `bellman_ford_shortest_paths.hpp`
- `breadth_first_search.hpp`
- `depth_first_search.hpp`
- `topological_sort.hpp`
- `connected_components.hpp`
- `transitive_closure.hpp`
- `mst.hpp`
- `mis.hpp`
- `tc.hpp`
- All files in `algorithm/experimental/`

**Testing at this phase**:
- Test each algorithm after updating
- Run algorithm-specific unit tests
- Verify results match expected output

**Validation**:
- Each algorithm compiles
- Run existing unit tests to ensure behavior unchanged
- No performance regression (run quick benchmarks if available)

---

### Phase 8: Update Tests

**Goal**: Update all test files to use new namespaces.

**Strategy**:
1. Add using declarations at top of each test file
2. Update qualified names where needed

**Pattern for test files**:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "graph/graph.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/vertexlist.hpp"

// Using declarations for convenience
using namespace graph;
using namespace graph::adj_list;
using namespace graph::adj_list::views;
using namespace graph::container;

TEST_CASE("My test", "[tag]") {
    // Can now use unqualified names
    dynamic_graph<double, string> g;
    
    for (auto&& [uid, u] : vertexlist(g)) {
        // ...
    }
}
```

**Files to update**: All files in `tests/`

**Validation**:
- All tests compile
- All tests pass (behavior unchanged)
- No warnings about deprecated features

---

### Phase 9: Update Examples

**Goal**: Update example code to demonstrate new namespace usage.

**Files to update**: All files in `example/`

**Approach**:
- Update includes
- Add using declarations
- Add comments explaining namespace structure
- Show different usage patterns (fully qualified vs using declarations)

**Validation**:
- All examples compile
- Examples demonstrate best practices for namespace usage

---

### Phase 10: Update Documentation

**Goal**: Document new namespace structure and usage patterns.

**Files to create/update**:
- Update `agents/overview.md` with new namespace structure
- Create migration guide (when ready for public release)
- Update `doc/reference/*.md` files
- Update README.md with new examples

**Documentation to include**:
1. Namespace hierarchy diagram
2. What goes in each namespace
3. Common using declaration patterns
4. Examples of qualified vs unqualified usage
5. Rationale for design decisions

---

### Phase 11: Cleanup and Deprecation

**Goal**: Remove old namespace aliases and deprecated code.

**Tasks**:
1. Remove temporary `namespace edgelist = edge_list;` alias
2. Remove any duplicate definitions in old namespaces
3. Remove compatibility shims
4. Mark any remaining old code as deprecated

**This phase only executes when ready for public release.**

---

## Testing Strategy

### Continuous Validation

After each phase:
1. ✅ All existing tests must pass
2. ✅ No new compiler warnings
3. ✅ Concept satisfaction checking works
4. ✅ CPO overload resolution is correct

### Specific Test Cases

Create new test files to validate:

#### Test 1: Namespace qualification
```cpp
TEST_CASE("Namespace qualified access") {
    vector<vector<int>> g(5);
    
    // Fully qualified
    static_assert(graph::adj_list::adjacency_list<decltype(g)>);
    auto vr = graph::adj_list::vertices(g);
    
    // Via using
    using namespace graph::adj_list;
    auto er = edges(g, 0);
}
```

#### Test 2: CPO overload resolution
```cpp
TEST_CASE("CPO overload resolution") {
    // Adjacency list version
    vector<vector<int>> g(5);
    auto e = *begin(graph::adj_list::edges(g, 0));
    auto tid = graph::adj_list::target_id(g, e);  // Takes (graph, edge)
    
    // Edge list version
    graph::copyable_edge_t<int, void> edge{0, 1};
    auto src = graph::edge_list::source_id(edge);  // Takes (edge)
    auto tgt = graph::edge_list::target_id(edge);  // Takes (edge)
}
```

#### Test 3: View factories in sub-namespace
```cpp
TEST_CASE("View factories") {
    vector<vector<int>> g(5);
    
    using namespace graph::adj_list::views;
    
    for (auto&& [uid, u] : vertexlist(g)) {
        // Should work
    }
    
    for (auto&& [vid, e] : incidence(g, 0)) {
        // Should work  
    }
}
```

#### Test 4: Algorithm usage with new namespaces
```cpp
TEST_CASE("Algorithms with new namespaces") {
    using namespace graph;
    using namespace graph::container;
    
    compressed_graph<double> g(/* ... */);
    
    vector<double> distances(num_vertices(g));
    vector<uint32_t> predecessors(num_vertices(g));
    
    // Algorithm in root graph namespace
    dijkstra_shortest_paths(g, {0}, distances, predecessors);
}
```

---

## Rollback Strategy

If issues arise:
1. **Each phase is independent** - can rollback to previous phase
2. **Use git branches** - keep main/master clean
3. **Parallel implementation** - old code still works during transition
4. **Feature flags** - could use conditional compilation if needed:
   ```cpp
   #ifdef GRAPH_NEW_NAMESPACES
       namespace graph::adj_list {
   #else
       namespace graph {
   #endif
   ```

---

## Tooling Support

### Scripts to Create

#### 1. Dependency analyzer
```python
# analyze_dependencies.py
# Parse headers and build dependency graph
# Output: which headers depend on which namespaces
```

#### 2. Namespace migrator (semi-automated)
```python
# migrate_namespace.py
# Help move code from one namespace to another
# Update includes, using declarations, etc.
```

#### 3. Test validator
```bash
#!/bin/bash
# validate_phase.sh
# Run tests and check for issues after each phase
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Timeline Estimation

**For experimental/validation phase:**

| Phase                            | Estimated Time | Dependencies            |
| -------------------------------- | -------------- | ----------------------- |
| 0: Preparation                   | 1-2 days       | None                    |
| 1: Create namespaces             | 2-3 days       | Phase 0                 |
| 2: Move CPOs                     | 3-5 days       | Phase 1                 |
| 3: Move type aliases             | 1-2 days       | Phase 2                 |
| 4: Update containers (non-CPO)   | 2-3 days       | Phase 2, 3              |
| 5: Add CPO support to containers | 1-2 days       | Phase 4                 |
| 6: Update views                  | 3-4 days       | Phase 5                 |
| 7: Update algorithms             | 2-3 days       | Phase 6                 |
| 8: Update tests                  | 2-3 days       | Phase 4-7 (incremental) |
| 9: Update examples               | 1-2 days       | Phase 4-7               |
| 10: Update docs                  | 2-3 days       | All phases              |
| 11: Cleanup                      | 1-2 days       | All phases              |

**Total: ~3-4 weeks** of focused work

---

## Success Criteria

The refactoring is complete when:

1. ✅ All code compiles without warnings
2. ✅ All existing tests pass
3. ✅ New namespace structure is fully populated
4. ✅ Concept satisfaction works correctly
5. ✅ CPO overload resolution is correct
6. ✅ Examples demonstrate usage patterns
7. ✅ Documentation is complete
8. ✅ Code review completed
9. ✅ Performance is unchanged (run benchmarks)
10. ✅ Team consensus that structure makes sense

---

## Notes

- **Experimental phase**: Keep work on feature branch
- **Incremental commits**: Commit after each sub-step
- **Test frequently**: Run tests after each change
- **Document decisions**: Update this doc with any changes to strategy
- **Ask for review**: Get feedback on structure before full implementation
