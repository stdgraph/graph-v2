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
- [x] File created with include dependencies listed
- [x] Review shows header structure and dependencies

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
- [x] File lists all CPO definitions with file paths and line numbers
- [x] Review confirms 19 core CPOs in detail/graph_cpo.hpp: `vertices`, `edges`, `vertex_id`, `target_id`, `source_id`, `target`, `source`, `find_vertex`, `find_vertex_edge`, `contains_edge`, `partition_id`, `num_vertices`, `degree`, `vertex_value`, `edge_value`, `graph_value`, `num_partitions`, `num_edges`, `has_edge`
- [x] Plus 10 view CPOs in views/*.hpp: `neighbors`, `vertices_depth_first_search`, `edges_depth_first_search`, `sourced_edges_depth_first_search`, `edgelist`, `vertexlist`, `vertices_breadth_first_search`, `edges_breadth_first_search`, `sourced_edges_breadth_first_search`, `incidence`

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
- [x] File lists all concepts with locations
- [x] Found 86 concepts total across the codebase
- [x] Key concepts in graph.hpp for `adj_list` namespace: `adjacency_list`, `index_adjacency_list`, `targeted_edge`, `sourced_edge`, `vertex_range`, `edge_range`, etc.
- [x] Concepts in edgelist.hpp for `edge_list` namespace: `basic_edgelist_range`, `edgelist_range`, etc.

**Rollback**: Delete file if needed

---

### Task 0.4: Baseline Test Run

**Goal**: Establish that all tests pass before any changes.

**Actions**:
1. Configure and build:
   ```bash
   cmake --preset=linux-gcc-debug
   cd out/build/linux-gcc-debug
   cmake --build .
   ```
2. Run tests (note: -C Debug flag required):
   ```bash
   ctest -C Debug --output-on-failure 2>&1 | tee ../../../agents/baseline_test_results.txt
   ```

**Validation**:
- [x] All tests pass: 29/29 (100%)
- [x] Total test time: 0.13 sec
- [x] Configuration: linux-gcc-debug, build dir: out/build/linux-gcc-debug
- [x] Build succeeds with warnings (sign-conversion in DFS - pre-existing)

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
- [x] File compiles without errors
- [x] Test that concepts are accessible:
   ```cpp
   static_assert(graph::adj_list::adjacency_list</* some graph type */>);
   ```
- [x] Original concepts in `graph` namespace still work

**Test added**: `tests/namespace_validation_phase1.cpp`
```cpp
TEST_CASE("adj_list namespace concepts") {
    using G = std::vector<std::vector<int>>;
    
    static_assert(graph::adj_list::adjacency_list<G>);
    static_assert(graph::adj_list::index_adjacency_list<G>);
    // ... and all other concepts verified
}
```
- [x] Test created and added to CMakeLists.txt
- [x] All tests pass (30/30)

**Rollback**: Remove the `namespace adj_list { ... }` block

**Completed**: Commit 50cbdcb

---

### Task 1.2: Add Edge Concepts to graph::adj_list

**Goal**: Copy edge-related concepts to the new namespace.

**File**: `include/graph/graph.hpp`

**Status**: ✅ COMPLETED as part of Task 1.1 (commit 50cbdcb)

**Note**: All edge concepts were included when the `graph::adj_list` namespace was created in Task 1.1:
- `basic_targeted_edge`
- `basic_sourced_edge`
- `basic_sourced_targeted_edge`
- `targeted_edge`
- `sourced_edge`
- `sourced_targeted_edge`

**Validation**:
- [x] File compiles
- [x] Edge concepts accessible: `graph::adj_list::targeted_edge<G>`
- [x] Verified in namespace_validation_phase1.cpp test

**Rollback**: N/A (already included in Task 1.1)

**Completed**: Part of commit 50cbdcb

---

### Task 1.3: Add Property Concepts to graph::adj_list

**Goal**: Copy property concepts to the new namespace.

**File**: `include/graph/graph.hpp`

**Status**: ✅ COMPLETED as part of Task 1.1 (commit 50cbdcb)

**Note**: All property concepts were included when the `graph::adj_list` namespace was created in Task 1.1:
- `has_degree`
- `has_find_vertex`
- `has_find_vertex_edge`
- `has_contains_edge`

**Validation**:
- [x] File compiles
- [x] Property concepts accessible
- [x] Verified in namespace_validation_phase1.cpp test

**Rollback**: N/A (already included in Task 1.1)

**Completed**: Part of commit 50cbdcb

---

### Task 1.4: Rename graph::edgelist to graph::edge_list

**Goal**: Rename the namespace and add compatibility alias.

**File**: `include/graph/edgelist.hpp`

**Changes**:
1. Changed line ~82: `namespace edgelist {` → `namespace edge_list {`
2. Changed line ~146: Added compatibility alias before closing `} // namespace graph`

**Validation**:
- [x] File compiles
- [x] New namespace works: `graph::edge_list::basic_sourced_edgelist<EL>`
- [x] Old namespace still works: `graph::edgelist::basic_sourced_edgelist<EL>`
- [x] Test added to namespace_validation_phase1.cpp
- [x] All 30 tests pass

**Rollback**: Revert namespace names

**Completed**: Commit ad72cbc

---

### Task 1.5: Create graph::adj_list::views Namespace Structure

**Goal**: Add the views sub-namespace to view headers.

**Files**: 
- `include/graph/views/vertexlist.hpp`
- `include/graph/views/incidence.hpp`
- `include/graph/views/neighbors.hpp`

**Changes**: Added namespace structure at end of each file before closing `} // namespace graph`

**Validation**:
- [x] All files compile
- [x] Namespace exists (even if empty)
- [x] All 30 tests pass

**Rollback**: Remove added namespace blocks

**Completed**: Commit ce1fb04

---

### Task 1.6: Test Phase 1 Completion

**Goal**: Verify all Phase 1 changes work together.

**Actions**:
1. Build entire project: `cmake --build out/build/linux-gcc-debug`
2. Run all tests: `ctest -C Debug --test-dir out/build/linux-gcc-debug --output-on-failure`
3. Validation test file `tests/namespace_validation_phase1.cpp` already created
4. Test added to `tests/CMakeLists.txt` in Task 1.1

**Validation**:
- [x] All existing tests still pass (29/29)
- [x] New validation test passes (1/1)
- [x] No new compiler warnings introduced
- [x] Both old and new namespaces work
- [x] Total: 30/30 tests passing

**Completed**: Phase 1 complete - see agents/phase1_summary.txt

**Commit point**: Phase 1 complete across commits 50cbdcb, 8f1a89e, ad72cbc, ce1fb04

---

## Phase 2: Move CPOs to New Namespaces

### Task 2.1: Identify CPOs in graph_cpo.hpp

**Goal**: Understand the structure before moving.

**File**: `include/graph/detail/graph_cpo.hpp`

**Status**: ✅ COMPLETED in Phase 0 (Task 0.2)

**CPO namespaces identified** (from agents/cpo_inventory.txt):
   - `_Vertices` (line 181) → `vertices(g)` - declared line 236
   - `_Vertex_id` (line 291) → `vertex_id(g, ui)` - declared line 392
   - `_Find_vertex` (line 466) → `find_vertex(g, uid)` - declared line 523
   - `_Edges` (line 566) → `edges(g, u)` - declared line 679
   - `_NumEdges` (line 753) → `num_edges(g)` - declared line 818
   - `_Target_id` (line 874) → `target_id(g, uv)` - declared line 1000
   - `_Source_id` (line 1063) → `source_id(g, uv)` - declared line 1173
   - `_Target` (line 1209) → `target(g, uv)` - declared line 1263
   - `_Source` (line 1292) → `source(g, uv)` - declared line 1351
   - `_Find_vertex_edge` (line 1396) → `find_vertex_edge(g, u, vid)` - declared line 1506
   - `_Contains_edge` (line 1539) → `contains_edge(g, uid, vid)` - declared line 1603
   - `_Partition_id` (line 1643) → `partition_id(g, uid)` - declared line 1745
   - `_NumVertices` (line 1789) → `num_vertices(g)` - declared line 1891
   - `_Degree` (line 1931) → `degree(g, u)` - declared line 2034
   - `_Vertex_value` (line 2060) → `vertex_value(g, u)` - declared line 2112
   - `_Edge_value` (line 2163) → `edge_value(g, uv)` - declared line 2279
   - `_Graph_value` (line 2320) → `graph_value(g)` - declared line 2369
   - `_Num_partitions` (line 2397) → `num_partitions(g)` - declared line 2451
   - `_HasEdge` (line 2495) → `has_edge(g, uid, vid)` - declared line 2605

**Validation**:
- [x] All CPOs identified with exact line numbers in `agents/cpo_inventory.txt`
- [x] Line ranges documented above for reference
- [x] Completed in Phase 0 commit 16dbca0

**Rollback**: N/A (analysis only)

**Completed**: Phase 0, Task 0.2

---

### Task 2.2: Move vertices() CPO to graph::adj_list

**Status**: ✅ COMPLETED (commit 453ba25)

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
- [x] File compiles
- [x] `graph::adj_list::vertices(g)` works
- [x] `graph::vertices(g)` still works (via using declaration)
- [x] Test: Create simple test calling both versions (namespace_validation_phase2_task2_2.cpp)
- [x] All 31/31 tests passing

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

**Status**: ✅ COMPLETED (commit cb5f084)

**Goal**: Move edges CPO following same pattern.

**File**: `include/graph/detail/graph_cpo.hpp`

**Changes**:
Follow same pattern as Task 2.2:
1. Find `_Edges` namespace
2. Wrap in `namespace adj_list { ... }`
3. Add to inline namespace `_Cpos`
4. Add `using adj_list::edges;` in `graph` namespace

**Validation**:
- [x] Compiles
- [x] Both `graph::adj_list::edges(g, u)` and `graph::edges(g, u)` work
- [x] Test with actual graph (added to namespace_validation_phase2_task2_2.cpp)
- [x] All 31/31 tests passing

**Rollback**: Revert changes

---

### Task 2.4: Move Remaining CPOs to graph::adj_list

**Status**: ✅ COMPLETED (commit c0e80b6)

**Goal**: Move all remaining adjacency list CPOs.

**File**: `include/graph/detail/graph_cpo.hpp`

**CPOs moved** (following pattern from Tasks 2.2-2.3):
- `vertex_id(g, ui)`
- `find_vertex(g, uid)`
- `target_id(g, uv)`
- `source_id(g, uv)`
- `target(g, uv)`
- `source(g, uv)`
- `num_edges(g)`
- `find_vertex_edge(g, u, vid)`
- `contains_edge(g, uid, vid)`
- `partition_id(g, u)`
- `num_vertices(g)`
- `degree(g, u)`
- `vertex_value(g, u)`
- `edge_value(g, uv)`
- `graph_value(g)`
- `num_partitions(g)`
- `has_edge(g, uid, vid)`

**Implementation**:
1. Wrapped each CPO namespace in `namespace adj_list { ... }`
2. Added to `inline namespace _Cpos` inside adj_list
3. Added `using adj_list::<cpo_name>;` declarations for compatibility
4. Fixed _Source namespace opening that was missing adj_list wrapper
5. Updated test file references to graph::adj_list::_Target_id, _Source_id, _Edge_value

**Validation**:
- [x] File compiles
- [x] Each CPO works in both namespaces (graph:: and graph::adj_list::)
- [x] All 31/31 tests passing
- [x] Updated edgelist_tests.cpp to use new namespace paths

**Commit point**: `git commit -m "Task 2.4: Move all remaining CPOs to graph::adj_list namespace"`

**Completed**: All 19 core CPOs now in graph::adj_list namespace with backward compatibility

---

## Phase 3: Move Type Aliases

### Task 3.1: Add Type Aliases to graph::adj_list ✅ COMPLETED

**Goal**: Add type aliases for adjacency list types.

**File**: `include/graph/detail/graph_cpo.hpp`

**Status**: COMPLETED (Commit: d1cceaf)

**Implementation Notes**:
- All 11 type aliases moved to graph::adj_list namespace
- Critical insight: Each type alias placed **immediately after** its corresponding CPO to avoid circular dependencies
- Type alias locations:
  * Lines 249-262: 4 vertex aliases after vertices CPO
  * Lines 389-391: vertex_id_t after vertex_id CPO
  * Lines 639-658: 5 edge aliases after edges CPO
  * Lines 1704-1706: partition_id_t after partition_id CPO
  * Lines 2079-2081: vertex_value_t after vertex_value CPO
  * Lines 2249-2251: edge_value_t after edge_value CPO
- Compatibility layer updated (lines 2575-2593): using declarations in graph namespace reference adj_list versions
- Fixed compatibility layer: added find_vertex_edge, contains_edge, num_partitions; removed non-existent edge_id

**Validation**:
- ✅ Compiles successfully
- ✅ Type aliases work: `graph::adj_list::vertex_id_t<G>`
- ✅ Old aliases still work: `graph::vertex_id_t<G>`
- ✅ All 31 tests passing

---

### Task 3.2: Test Type Alias Resolution ✅ COMPLETED

**Goal**: Verify type aliases deduce correctly.

**Status**: COMPLETED (Commit: 9d739a0)

**Test file**: `tests/namespace_validation_phase3_task3_2.cpp` (88 lines)

**Implemented Tests**:
1. **Vertex type aliases** - Validates 5 vertex-related aliases match between namespaces:
   - vertex_range_t, vertex_iterator_t, vertex_t, vertex_reference_t, vertex_id_t
2. **Edge type aliases** - Validates 5 edge-related aliases match:
   - vertex_edge_range_t, vertex_edge_iterator_t, edge_t, edge_reference_t, edge_descriptor_t
3. **Partition type alias** - Validates partition_id_t matches
4. **Functional test** - Proves CPOs work correctly with types from both namespaces using vector<vector<int>> graph

**Implementation Notes**:
- Test went through 5 iterations to fix compilation errors and runtime issues
- Final version uses vector<vector<int>> as test graph type
- Validates both static type equivalence (via static_assert) and functional correctness (runtime CPO calls)
- Added to tests/CMakeLists.txt line 71

**Validation**:
- ✅ Test compiles and passes
- ✅ Type deduction works correctly
- ✅ All 32 tests passing (30 original + 2 validation tests)
- ✅ Both namespace access patterns work correctly

**Commit point**: Phase 3 complete with commits d1cceaf (Task 3.1) and 9d739a0 (Task 3.2)

---

## Phase 4: Update Container Implementations (Non-CPO Interface)

### Task 4.1: Update compressed_graph Type References ✅ COMPLETED

**Goal**: Update internal type references to use new namespaces.

**File**: `include/graph/container/compressed_graph.hpp`

**Status**: COMPLETED - No changes required

**Analysis Results**:
- Reviewed entire file for type alias usage
- Found type alias usage on line 517: `vertex_id_t<graph_type>` and `partition_id_t<graph_type>`
- These resolve correctly through the compatibility layer in graph namespace
- No explicit `graph::` namespace qualifications found (only namespace declaration)
- No template parameter concepts need updating
- All unqualified names resolve correctly through ADL and compatibility layer

**Validation**:
- ✅ File compiles successfully
- ✅ Can construct and load: compressed_graph_tests passes
- ✅ Existing tests already exercise CPO interface (vertices(), vertex_value())
- ✅ All 26 test executables passing

**Conclusion**: The compatibility layer (using declarations in graph namespace) allows compressed_graph to work seamlessly with the new namespace structure without any code changes.

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
