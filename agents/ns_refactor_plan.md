# Namespace Refactoring Implementation Plan

> **Detailed step-by-step instructions for executing the namespace refactoring**
> 
> This document breaks down the strategy from `ns_refactor_strategy.md` into specific, actionable tasks that can be executed incrementally. Each task includes exact file locations, code changes, and validation steps.

## How to Use This Document

- Each task is independent and can be completed in sequence
- Tasks are organized by phase from the strategy document
- Each task has: **Goal**, **Files**, **Changes**, **Validation**, **Rollback**
- Check off tasks as you complete them
- Run validation tests before moving to next task
- Keep work on `namespace` branch; commit after each task or small group of tasks

## Prerequisites

- [ ] Read `ns_refactor_goal.md` to understand the rationale
- [ ] Read `ns_refactor_strategy.md` to understand the overall approach
- [ ] Ensure you're on the `namespace` branch
- [ ] Ensure build environment is set up and tests pass on current code
- [ ] Create backup/tag of current state: `git tag pre-namespace-refactor`

---

## Phase 0: Preparation and Analysis

### Task 0.1: Create Dependency Map

**Goal**: Document which headers depend on which other headers.

**Actions**:
1. Create `agents/dependency_map.txt`
2. Run analysis:
   ```bash
   cd include/graph
   for file in $(find . -name "*.hpp"); do
       echo "=== $file ==="
       grep -h "^#include" "$file" | sort | uniq
       echo
   done > ../../agents/dependency_map.txt
   ```

**Validation**:
- [ ] File created with include dependencies listed
- [ ] Review to understand header structure

**Rollback**: Delete file if needed

---

### Task 0.2: List All CPO Definitions

**Goal**: Inventory all Customization Point Objects to know what needs to move.

**Actions**:
1. Create `agents/cpo_inventory.txt`
2. Run search:
   ```bash
   cd include/graph
   grep -rn "class _Cpo" --include="*.hpp" > ../../agents/cpo_inventory.txt
   grep -rn "inline constexpr.*_Cpo" --include="*.hpp" >> ../../agents/cpo_inventory.txt
   ```

**Validation**:
- [ ] File lists all CPO definitions with file paths and line numbers
- [ ] Review to confirm: `vertices`, `edges`, `vertex_id`, `target_id`, `source_id`, etc.

**Rollback**: Delete file if needed

---

### Task 0.3: List All Concepts

**Goal**: Inventory all concept definitions.

**Actions**:
1. Create `agents/concept_inventory.txt`
2. Run search:
   ```bash
   cd include/graph
   grep -rn "^template.*concept " --include="*.hpp" > ../../agents/concept_inventory.txt
   ```

**Validation**:
- [ ] File lists all concepts with locations
- [ ] Identify which go to `adj_list` namespace vs `edge_list`

**Rollback**: Delete file if needed

---

### Task 0.4: Baseline Test Run

**Goal**: Establish that all tests pass before any changes.

**Actions**:
1. Build and run tests:
   ```bash
   cd build
   cmake --build .
   ctest --output-on-failure
   ```
2. Record results in `agents/baseline_test_results.txt`

**Validation**:
- [ ] All tests pass
- [ ] Record number of tests and duration
- [ ] If any tests fail, fix them first before proceeding

**Rollback**: N/A

---

## Phase 1: Create New Namespace Structure

### Task 1.1: Add `graph::adj_list` Namespace to graph.hpp

**Goal**: Create the new `graph::adj_list` namespace and copy concepts into it.

**File**: `include/graph/graph.hpp`

**Changes**:
1. After the existing concepts (around line 180-280), add:
   ```cpp
   //
   // New namespace structure for refactoring
   //
   namespace adj_list {
       
       // Copy existing concepts here, maintaining same definitions
       
       /**
        * @ingroup graph_concepts
        * @brief Concept for a basic range of vertices.
        */
       template <class G>
       concept _common_vertex_range = sized_range<vertex_range_t<G>> &&
                                      requires(G&& g, vertex_iterator_t<G> ui) { vertex_id(g, ui); };
       
       template <class G>
       concept vertex_range = _common_vertex_range<vertex_range_t<G>> &&
                              forward_range<vertex_range_t<G>>;
       
       template <class G>
       concept index_vertex_range = _common_vertex_range<vertex_range_t<G>> &&
                                    random_access_range<vertex_range_t<G>> &&
                                    integral<vertex_id_t<G>>;
       
       template <class G>
       concept basic_targeted_edge_range = requires(G&& g, vertex_id_t<G> uid) {
           { edges(g, uid) } -> forward_range;
       };
       
       template <class G>
       concept targeted_edge_range = basic_targeted_edge_range<G> &&
                                     requires(G&& g, vertex_reference_t<G> u) {
                                         { edges(g, u) } -> forward_range;
                                     };
       
       // Basic adjacency list concepts
       template <class G>
       concept basic_adjacency_list = vertex_range<G> &&
                                      basic_targeted_edge_range<G> &&
                                      targeted_edge<G>;
       
       template <class G>
       concept basic_index_adjacency_list = index_vertex_range<G> &&
                                            basic_targeted_edge_range<G> &&
                                            basic_targeted_edge<G>;
       
       template <class G>
       concept basic_sourced_adjacency_list = vertex_range<G> &&
                                              basic_targeted_edge_range<G> &&
                                              basic_sourced_targeted_edge<G>;
       
       template <class G>
       concept basic_sourced_index_adjacency_list = index_vertex_range<G> &&
                                                    basic_targeted_edge_range<G> &&
                                                    basic_sourced_targeted_edge<G>;
       
       // Full adjacency list concepts
       template <class G>
       concept adjacency_list = vertex_range<G> &&
                                targeted_edge_range<G> &&
                                targeted_edge<G>;
       
       template <class G>
       concept index_adjacency_list = index_vertex_range<G> &&
                                      targeted_edge_range<G> &&
                                      targeted_edge<G>;
       
       template <class G>
       concept sourced_adjacency_list = vertex_range<G> &&
                                        targeted_edge_range<G> &&
                                        sourced_targeted_edge<G>;
       
       template <class G>
       concept sourced_index_adjacency_list = index_vertex_range<G> &&
                                              targeted_edge_range<G> &&
                                              sourced_targeted_edge<G>;
       
   } // namespace adj_list
   ```

**Validation**:
- [ ] File compiles without errors
- [ ] Test that concepts are accessible:
   ```cpp
   static_assert(graph::adj_list::adjacency_list</* some graph type */>);
   ```
- [ ] Original concepts in `graph` namespace still work

**Test to add** (in a scratch test file or existing test):
```cpp
TEST_CASE("adj_list namespace concepts") {
    using namespace graph::container;
    using G = std::vector<std::vector<int>>;
    
    static_assert(graph::adj_list::adjacency_list<G>);
    static_assert(graph::adj_list::index_adjacency_list<G>);
}
```

**Rollback**: Remove the `namespace adj_list { ... }` block

---

### Task 1.2: Add Edge Concepts to graph::adj_list

**Goal**: Copy edge-related concepts to the new namespace.

**File**: `include/graph/graph.hpp`

**Changes**:
Add inside the `namespace adj_list` block (from Task 1.1):

```cpp
// Edge concepts
template <class G>
concept basic_targeted_edge = requires(G&& g, edge_reference_t<G> uv) { target_id(g, uv); };

template <class G>
concept basic_sourced_edge = requires(G&& g, edge_reference_t<G> uv) { source_id(g, uv); };

template <class G>
concept basic_sourced_targeted_edge = basic_targeted_edge<G> && basic_sourced_edge<G>;

template <class G>
concept targeted_edge = basic_targeted_edge<G> &&
                        requires(G&& g, edge_reference_t<G> uv) { target(g, uv); };

template <class G>
concept sourced_edge = basic_sourced_edge<G> &&
                       requires(G&& g, edge_reference_t<G> uv) { source(g, uv); };

template <class G>
concept sourced_targeted_edge = targeted_edge<G> && sourced_edge<G>;

template <class G>
concept unordered_edge = basic_sourced_edge<G> && define_unordered_edge<G>::value;

template <class G>
concept ordered_edge = !unordered_edge<G>;
```

**Validation**:
- [ ] File compiles
- [ ] Edge concepts accessible: `graph::adj_list::targeted_edge<G>`

**Rollback**: Remove added code

---

### Task 1.3: Add Property Concepts to graph::adj_list

**Goal**: Copy property concepts to the new namespace.

**File**: `include/graph/graph.hpp`

**Changes**:
Add inside the `namespace adj_list` block:

```cpp
// Property concepts
template <class G>
concept has_degree = requires(G&& g, vertex_reference_t<G> u) {
    { degree(g, u) };
};

template <class G>
concept has_find_vertex = requires(G&& g, vertex_id_t<G> uid) {
    { find_vertex(g, uid) } -> forward_iterator;
};

template <class G>
concept has_find_vertex_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
    { find_vertex_edge(g, u, vid) } -> forward_iterator;
    { find_vertex_edge(g, uid, vid) } -> forward_iterator;
};

template <class G>
concept has_contains_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
    { contains_edge(g, uid, vid) } -> convertible_to<bool>;
};
```

**Validation**:
- [ ] File compiles
- [ ] Property concepts accessible

**Rollback**: Remove added code

---

### Task 1.4: Rename graph::edgelist to graph::edge_list

**Goal**: Rename the namespace and add compatibility alias.

**File**: `include/graph/edgelist.hpp`

**Changes**:
1. Find the line `namespace edgelist {` (around line 82)
2. Change to:
   ```cpp
   // New namespace name (underscore instead of no separator)
   namespace edge_list {
   ```

3. At the end of the namespace (around line 146), change:
   ```cpp
   } // namespace edge_list
   
   // Temporary compatibility alias
   namespace edgelist = edge_list;
   
   } // namespace graph
   ```

**Validation**:
- [ ] File compiles
- [ ] New namespace works: `graph::edge_list::basic_sourced_edgelist<EL>`
- [ ] Old namespace still works: `graph::edgelist::basic_sourced_edgelist<EL>`

**Test to add**:
```cpp
TEST_CASE("edge_list namespace rename") {
    // Both should work
    using EL = std::vector<std::pair<int, int>>;
    static_assert(graph::edge_list::basic_sourced_edgelist<EL>);
    static_assert(graph::edgelist::basic_sourced_edgelist<EL>); // compatibility
}
```

**Rollback**: Revert namespace names

---

### Task 1.5: Create graph::adj_list::views Namespace Structure

**Goal**: Add the views sub-namespace to view headers.

**Files**: 
- `include/graph/views/vertexlist.hpp`
- `include/graph/views/incidence.hpp`
- `include/graph/views/neighbors.hpp`

**Changes for each file**:

At the end of the file (before final `}` closing `namespace graph`), add:

```cpp
namespace adj_list {
    namespace views {
        // Factory functions will be added in Phase 6
        // Placeholder for now to establish namespace structure
    } // namespace views
} // namespace adj_list
```

**Validation**:
- [ ] All files compile
- [ ] Namespace exists (even if empty)

**Rollback**: Remove added namespace blocks

---

### Task 1.6: Test Phase 1 Completion

**Goal**: Verify all Phase 1 changes work together.

**Actions**:
1. Build entire project: `cmake --build build`
2. Run all tests: `ctest --test-dir build --output-on-failure`
3. Create validation test file `tests/namespace_validation_phase1.cpp`:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "graph/graph.hpp"
#include "graph/edgelist.hpp"
#include <vector>

TEST_CASE("Phase 1: Namespace structure exists", "[namespace][phase1]") {
    using G = std::vector<std::vector<int>>;
    
    SECTION("adj_list namespace concepts work") {
        static_assert(graph::adj_list::adjacency_list<G>);
        static_assert(graph::adj_list::index_adjacency_list<G>);
        static_assert(graph::adj_list::basic_adjacency_list<G>);
    }
    
    SECTION("edge_list namespace renamed") {
        using EL = std::vector<std::pair<int, int>>;
        // New name works
        static_assert(graph::edge_list::basic_sourced_edgelist<EL>);
        // Old name works (compatibility)
        static_assert(graph::edgelist::basic_sourced_edgelist<EL>);
    }
    
    SECTION("Original graph namespace still works") {
        static_assert(graph::adjacency_list<G>);
        static_assert(graph::index_adjacency_list<G>);
    }
}
```

4. Add to `tests/CMakeLists.txt` if using separate test file
5. Run new test

**Validation**:
- [ ] All existing tests still pass
- [ ] New validation test passes
- [ ] No compiler warnings
- [ ] Both old and new namespaces work

**Commit point**: `git commit -m "Phase 1: Create new namespace structure"`

---

## Phase 2: Move CPOs to New Namespaces

### Task 2.1: Identify CPOs in graph_cpo.hpp

**Goal**: Understand the structure before moving.

**File**: `include/graph/detail/graph_cpo.hpp`

**Actions**:
1. Review the file structure (it's ~2600 lines)
2. Identify the CPO namespaces (they follow pattern `_FunctionName`)
3. List them:
   - `_Vertices` → `vertices(g)`
   - `_Edges` → `edges(g, u)`
   - `_Vertex_id` → `vertex_id(g, ui)`
   - `_Target_id` → `target_id(g, uv)`
   - `_Source_id` → `source_id(g, uv)`
   - `_Target` → `target(g, uv)`
   - `_Source` → `source(g, uv)`
   - `_Find_vertex` → `find_vertex(g, uid)`
   - etc.

**Validation**:
- [ ] Create list in `agents/cpo_move_plan.txt`
- [ ] Note line ranges for each CPO

**Rollback**: N/A (just analysis)

---

### Task 2.2: Move vertices() CPO to graph::adj_list

**Goal**: Move the first CPO as a template for others.

**File**: `include/graph/detail/graph_cpo.hpp`

**Changes**:

1. Find the `_Vertices` namespace (around line 145-260)
2. Wrap it in `namespace adj_list`:

```cpp
namespace graph {
    namespace adj_list {
        //
        // vertices(g) -> vertex_range_t<G>
        //
        namespace _Vertices {
            // ... keep all existing implementation ...
        } // namespace _Vertices
        
        inline namespace _Cpos {
            inline constexpr _Vertices::_Cpo vertices;
        }
    } // namespace adj_list
    
    // Temporary: keep in graph namespace too for compatibility
    using adj_list::vertices;
    
} // namespace graph
```

**Validation**:
- [ ] File compiles
- [ ] `graph::adj_list::vertices(g)` works
- [ ] `graph::vertices(g)` still works (via using declaration)
- [ ] Test: Create simple test calling both versions

**Test to add**:
```cpp
TEST_CASE("vertices CPO in adj_list namespace") {
    std::vector<std::vector<int>> g(5);
    
    // New namespace
    auto vr1 = graph::adj_list::vertices(g);
    REQUIRE(std::ranges::size(vr1) == 5);
    
    // Old namespace (compatibility)
    auto vr2 = graph::vertices(g);
    REQUIRE(std::ranges::size(vr2) == 5);
}
```

**Rollback**: Remove namespace wrapper and using declaration

---

### Task 2.3: Move edges() CPO to graph::adj_list

**Goal**: Move edges CPO following same pattern.

**File**: `include/graph/detail/graph_cpo.hpp`

**Changes**:
Follow same pattern as Task 2.2:
1. Find `_Edges` namespace
2. Wrap in `namespace adj_list { ... }`
3. Add to inline namespace `_Cpos`
4. Add `using adj_list::edges;` in `graph` namespace

**Validation**:
- [ ] Compiles
- [ ] Both `graph::adj_list::edges(g, u)` and `graph::edges(g, u)` work
- [ ] Test with actual graph

**Rollback**: Revert changes

---

### Task 2.4: Move Remaining CPOs to graph::adj_list

**Goal**: Move all remaining adjacency list CPOs.

**File**: `include/graph/detail/graph_cpo.hpp`

**CPOs to move** (follow pattern from Tasks 2.2-2.3):
- `vertex_id(g, ui)`
- `target_id(g, uv)`
- `source_id(g, uv)`
- `target(g, uv)`
- `source(g, uv)`
- `find_vertex(g, uid)`
- `num_vertices(g)`
- `degree(g, u)`
- `vertex_value(g, u)` (if exists)
- `edge_value(g, uv)` (if exists)
- `graph_value(g)` (if exists)

**For each CPO**:
1. Wrap namespace in `namespace adj_list`
2. Add to `inline namespace _Cpos`
3. Add `using` declaration in `graph` namespace

**Validation**:
- [ ] File compiles
- [ ] Each CPO works in both namespaces
- [ ] All existing tests pass

**Commit point**: `git commit -m "Phase 2: Move CPOs to adj_list namespace"`

---

## Phase 3: Move Type Aliases

### Task 3.1: Add Type Aliases to graph::adj_list

**Goal**: Add type aliases for adjacency list types.

**File**: `include/graph/detail/graph_using.hpp` or inline in `graph.hpp`

**Changes**:

Add to `graph.hpp` inside `namespace adj_list`:

```cpp
namespace adj_list {
    // Vertex type aliases
    template <class G>
    using vertex_range_t = decltype(vertices(declval<G&>()));
    
    template <class G>
    using vertex_iterator_t = iterator_t<vertex_range_t<G>>;
    
    template <class G>
    using vertex_t = range_value_t<vertex_range_t<G>>;
    
    template <class G>
    using vertex_reference_t = range_reference_t<vertex_range_t<G>>;
    
    template <class G>
    using vertex_id_t = decltype(vertex_id(declval<G&>(), declval<vertex_iterator_t<G>>()));
    
    // Edge type aliases
    template <class G>
    using edge_range_t = decltype(edges(declval<G&>(), declval<vertex_id_t<G>>()));
    
    template <class G>
    using edge_iterator_t = iterator_t<edge_range_t<G>>;
    
    template <class G>
    using edge_t = range_value_t<edge_range_t<G>>;
    
    template <class G>
    using edge_reference_t = range_reference_t<edge_range_t<G>>;
    
    template <class G>
    using vertex_edge_range_t = edge_range_t<G>;
    
    template <class G>
    using vertex_edge_iterator_t = edge_iterator_t<G>;
    
} // namespace adj_list

// Compatibility: keep aliases in graph namespace too
template <class G>
using vertex_range_t = adj_list::vertex_range_t<G>;

template <class G>
using vertex_iterator_t = adj_list::vertex_iterator_t<G>;

// ... etc for all aliases
```

**Validation**:
- [ ] Compiles
- [ ] Type aliases work: `graph::adj_list::vertex_id_t<G>`
- [ ] Old aliases still work: `graph::vertex_id_t<G>`

**Rollback**: Remove added aliases

---

### Task 3.2: Test Type Alias Resolution

**Goal**: Verify type aliases deduce correctly.

**Test to add**:
```cpp
TEST_CASE("Type aliases in adj_list namespace") {
    using G = std::vector<std::vector<int>>;
    
    // New namespace
    using vid_t = graph::adj_list::vertex_id_t<G>;
    static_assert(std::is_same_v<vid_t, size_t>);
    
    using vref_t = graph::adj_list::vertex_reference_t<G>;
    static_assert(std::is_same_v<vref_t, std::vector<int>&>);
    
    // Old namespace (compatibility)
    using vid_t_old = graph::vertex_id_t<G>;
    static_assert(std::is_same_v<vid_t_old, vid_t>);
}
```

**Validation**:
- [ ] Test compiles and passes
- [ ] Type deduction works correctly

**Commit point**: `git commit -m "Phase 3: Add type aliases to adj_list namespace"`

---

## Phase 4: Update Container Implementations (Non-CPO Interface)

### Task 4.1: Update compressed_graph Type References

**Goal**: Update internal type references to use new namespaces.

**File**: `include/graph/container/compressed_graph.hpp`

**Changes**:
Look for uses of type aliases and update them. For example:
- `vertex_id_t<G>` → consider if it should reference `graph::adj_list::vertex_id_t<G>`
- Update template parameter concepts to use `graph::adj_list::adjacency_list`

This might not require changes if types are deduced, but review for explicit references.

**Validation**:
- [ ] File compiles
- [ ] Can construct: `compressed_graph<double> g;`
- [ ] Can load data: `g.load_edges(...);`

**Rollback**: Revert changes

---

### Task 4.2: Test compressed_graph Non-CPO Interface

**Goal**: Verify basic container functionality without CPOs.

**Test to add**:
```cpp
TEST_CASE("compressed_graph non-CPO interface") {
    using namespace graph::container;
    using Edge = copyable_edge_t<uint32_t, double>;
    
    std::vector<Edge> edges = {
        {0, 1, 1.0}, {0, 2, 2.0}, {1, 2, 3.0}
    };
    
    compressed_graph<double> g;
    g.load_edges(edges);
    
    // Direct member function calls (not CPO)
    auto vr = g.vertices();
    auto er = g.edges(0);
    
    REQUIRE(std::ranges::size(vr) == 3);
    REQUIRE(std::ranges::size(er) == 2);
}
```

**Validation**:
- [ ] Test passes
- [ ] Member functions work

**Rollback**: Revert test

---

### Task 4.3: Update dynamic_graph Type References

**Goal**: Same as 4.1 but for dynamic_graph.

**File**: `include/graph/container/dynamic_graph.hpp`

**Changes**: Similar to Task 4.1

**Validation**:
- [ ] File compiles
- [ ] Basic construction and loading work

**Rollback**: Revert changes

---

### Task 4.4: Test dynamic_graph Non-CPO Interface

**Goal**: Verify basic container functionality.

**Test to add**: Similar to Task 4.2 but for `dynamic_graph`

**Validation**:
- [ ] Test passes
- [ ] Member functions work

**Commit point**: `git commit -m "Phase 4: Update container non-CPO interfaces"`

---

## Phase 5: Add CPO Support to Containers

### Task 5.1: Test compressed_graph CPO Resolution

**Goal**: Verify CPOs resolve correctly via ADL or member functions.

**Test to add**:
```cpp
TEST_CASE("compressed_graph CPO satisfaction") {
    using namespace graph::container;
    using namespace graph::adj_list;
    using Edge = copyable_edge_t<uint32_t, double>;
    
    std::vector<Edge> edges = {{0, 1, 1.0}, {1, 2, 2.0}};
    compressed_graph<double> g;
    g.load_edges(edges);
    
    // Test CPO calls
    auto vr = vertices(g);           // CPO from adj_list
    REQUIRE(std::ranges::size(vr) == 3);
    
    auto er = edges(g, 0);           // CPO from adj_list
    REQUIRE(std::ranges::size(er) == 1);
    
    auto n = num_vertices(g);        // CPO from adj_list
    REQUIRE(n == 3);
    
    // Test concept satisfaction
    static_assert(adjacency_list<compressed_graph<double>>);
    static_assert(index_adjacency_list<compressed_graph<double>>);
}
```

**Validation**:
- [ ] Test compiles
- [ ] Test passes
- [ ] All CPO calls resolve correctly
- [ ] Concepts are satisfied

**Rollback**: Remove test

---

### Task 5.2: Test dynamic_graph CPO Resolution

**Goal**: Same as 5.1 for dynamic_graph.

**Test to add**: Similar to Task 5.1 but for all dynamic_graph variants (vofl, vol, vov)

**Validation**:
- [ ] Tests pass
- [ ] CPOs work
- [ ] Concepts satisfied

**Commit point**: `git commit -m "Phase 5: Verify CPO support for containers"`

---

## Phase 6: Update View Implementations

### Task 6.1: Add vertexlist() to graph::adj_list::views

**Goal**: Make vertexlist accessible via the views namespace.

**File**: `include/graph/views/vertexlist.hpp`

**Changes**:
In the `namespace adj_list::views` block added in Phase 1, add factory function:

```cpp
namespace adj_list {
    namespace views {
        namespace _Vertexlist {
            template <class G>
            class _Cpo {
                // Implementation that returns the existing graph::vertexlist_view<G>
            };
        }
        
        inline namespace _Cpos {
            inline constexpr _Vertexlist::_Cpo vertexlist;
        }
    } // namespace views
} // namespace adj_list

// Compatibility: keep in graph::views too
namespace views {
    using adj_list::views::vertexlist;
}
```

**Validation**:
- [ ] Compiles
- [ ] `graph::adj_list::views::vertexlist(g)` works
- [ ] `graph::views::vertexlist(g)` still works

**Rollback**: Remove added code

---

### Task 6.2-6.6: Update Remaining Views

**Goal**: Update all view headers to be accessible via `graph::adj_list::views`.

**Files**:
- `include/graph/views/incidence.hpp` → `incidence`
- `include/graph/views/neighbors.hpp` → `neighbors`
- `include/graph/views/edgelist.hpp` → `edgelist`
- `include/graph/views/breadth_first_search.hpp` → BFS views
- `include/graph/views/depth_first_search.hpp` → DFS views

**Pattern**: Follow Task 6.1 for each view

**Validation for each**:
- [ ] Compiles
- [ ] New namespace works
- [ ] Old namespace works (compatibility)
- [ ] Existing tests pass

**Commit point**: `git commit -m "Phase 6: Update views to adj_list::views namespace"`

---

## Phase 7: Update Algorithm Implementations

### Task 7.1: Update dijkstra_shortest_paths.hpp

**Goal**: Update algorithm to use new namespaces.

**File**: `include/graph/algorithm/dijkstra_shortest_paths.hpp`

**Changes**:
1. Add using declarations at top of `namespace graph`:
   ```cpp
   namespace graph {
       // Using declarations for convenience
       using adj_list::index_adjacency_list;
       using adj_list::vertex_id_t;
       using adj_list::edge_reference_t;
       using adj_list::vertex_reference_t;
       // ... others as needed
   ```

2. Or use fully qualified names in template parameters:
   ```cpp
   template <graph::adj_list::index_adjacency_list G, ...>
   ```

**Validation**:
- [ ] File compiles
- [ ] Algorithm tests pass
- [ ] No performance regression

**Rollback**: Remove using declarations

---

### Task 7.2-7.10: Update Remaining Algorithms

**Goal**: Update all algorithm headers.

**Files**:
- `bellman_ford_shortest_paths.hpp`
- `breadth_first_search.hpp`
- `depth_first_search.hpp`
- `topological_sort.hpp`
- `connected_components.hpp`
- `transitive_closure.hpp`
- `mst.hpp`
- `mis.hpp`
- `tc.hpp`

**Pattern**: Follow Task 7.1 for each algorithm

**Validation for each**:
- [ ] Compiles
- [ ] Tests pass
- [ ] Results unchanged

**Commit point**: `git commit -m "Phase 7: Update algorithms to use new namespaces"`

---

## Phase 8-11: Tests, Examples, Documentation, Cleanup

### Task 8.1: Update Test Using Declarations

**Goal**: Add consistent using declarations to test files.

**Pattern for each test file**:
```cpp
// At top of test file
using namespace graph;
using namespace graph::adj_list;
using namespace graph::adj_list::views;
using namespace graph::container;
```

**Files**: All files in `tests/`

**Validation**:
- [ ] All tests compile
- [ ] All tests pass

---

### Task 9.1: Update Examples

**Goal**: Update example code and add namespace documentation.

**Files**: All files in `example/`

**Changes**: Add using declarations and comments explaining namespace structure

**Validation**:
- [ ] Examples compile
- [ ] Examples run correctly

---

### Task 10.1: Update Documentation

**Goal**: Update all documentation to reflect new namespace structure.

**Files**:
- `agents/overview.md` - Update namespace section
- `doc/reference/*.md` - Update code examples
- `README.md` - Update examples

**Validation**:
- [ ] Documentation is accurate
- [ ] Examples compile

---

### Task 11.1: Remove Compatibility Aliases

**Goal**: Clean up temporary `using` declarations and namespace aliases.

**Actions**:
1. Remove `namespace edgelist = edge_list;` from `edgelist.hpp`
2. Remove `using adj_list::vertices;` and other CPO compatibility aliases
3. Remove duplicate type aliases from `graph` namespace

**Validation**:
- [ ] Still compiles (if targeting new namespaces throughout)
- [ ] All tests pass

**Commit point**: `git commit -m "Phase 11: Remove compatibility aliases - breaking change complete"`

---

## Final Validation

### Task F.1: Full Build and Test

**Actions**:
1. Clean build: `rm -rf build && mkdir build && cd build && cmake ..`
2. Build: `cmake --build .`
3. Test: `ctest --output-on-failure`
4. Benchmarks (if available): Check for performance regression

**Validation**:
- [ ] Clean build succeeds
- [ ] All tests pass
- [ ] No warnings
- [ ] Performance unchanged

---

### Task F.2: Code Review Checklist

**Review**:
- [ ] All CPOs in correct namespaces
- [ ] All type aliases in correct namespaces
- [ ] All concepts in correct namespaces
- [ ] Containers satisfy new concepts
- [ ] Views use new namespaces
- [ ] Algorithms use new namespaces
- [ ] Tests updated
- [ ] Examples updated
- [ ] Documentation updated
- [ ] No compatibility shims remain (if fully migrated)

---

### Task F.3: Create Migration Guide

**Goal**: Document for future users how to update their code.

**Create**: `doc/migration_guide.md`

**Content**:
- Namespace changes table
- Search-and-replace patterns
- Common migration scenarios
- Before/after code examples

---

## Success Criteria

The refactoring is complete when:
- [ ] All code compiles without warnings
- [ ] All tests pass (same count as baseline)
- [ ] Namespace structure matches `ns_refactor_goal.md`
- [ ] CPOs resolve correctly in new namespaces
- [ ] Concepts work in new namespaces
- [ ] No performance regression
- [ ] Documentation is complete and accurate
- [ ] Code review approved

---

## Notes

- Commit frequently (after each task or small group of tasks)
- Run tests after each task
- If a task fails validation, fix before proceeding
- Keep `namespace` branch until review complete
- Tag final state: `git tag namespace-refactor-complete`
