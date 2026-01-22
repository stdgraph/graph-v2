# Graph Library - Agent Quick Reference

> **Technical reference for AI agents working with the codebase**

## Quick Navigation

### Most Important Files

When modifying the library:

| Task              | Files to Examine                                            |
| ----------------- | ----------------------------------------------------------- |
| Add new algorithm | `include/graph/algorithm/*.hpp`, `tests/*_tests.cpp`        |
| Add new view      | `include/graph/views/*.hpp`, `include/graph/graph_info.hpp` |
| Modify CPOs       | `include/graph/detail/graph_cpo.hpp`                        |
| Add concepts      | `include/graph/graph.hpp`                                   |
| Add container     | `include/graph/container/*.hpp`                             |
| Fix build issues  | `CMakeLists.txt`, `cmake/*.cmake`                           |

---

## Code Patterns

### Adding a New Algorithm

1. Create header in `include/graph/algorithm/`
2. Use the pattern from existing algorithms:

```cpp
#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"

namespace graph {

template <index_adjacency_list G,
          /* other template params */>
requires /* constraints */
void my_algorithm(G&& g, /* params */) {
    // Implementation
}

} // namespace graph
```

3. Add tests in `tests/`
4. Include in `tests/CMakeLists.txt`

### Adding a New View

Views follow this structure:

```cpp
// 1. Iterator class
template <adjacency_list G, class VVF = void>
class my_view_iterator {
public:
    using iterator_category = forward_iterator_tag;
    using value_type = /* vertex_info or edge_info */;
    // ...
};

// 2. View class
template <adjacency_list G, class VVF = void>
class my_view : public ranges::view_base {
    // ...
};

// 3. CPO / factory function in namespace views
namespace views {
    template <adjacency_list G>
    auto my_view(G&& g) { ... }
}
```

### Creating Graph from Data

```cpp
// Edge type for loading
using copyable_edge_t<VId, EV> = edge_info<VId, true, void, EV>;
// {source_id, target_id, value}

// Vertex type for loading
using copyable_vertex_t<VId, VV> = vertex_info<VId, void, VV>;
// {id, value}
```

---

## Common Type Deductions

```cpp
// Given a graph type G:
using VertexRange = vertex_range_t<G>;           // decltype(vertices(g))
using VertexIter  = vertex_iterator_t<G>;        // iterator_t<vertex_range_t<G>>
using Vertex      = vertex_t<G>;                 // range_value_t<vertex_range_t<G>>
using VertexRef   = vertex_reference_t<G>;       // range_reference_t<vertex_range_t<G>>
using VertexId    = vertex_id_t<G>;              // typically integral

using EdgeRange   = vertex_edge_range_t<G>;      // decltype(edges(g, u))
using EdgeIter    = vertex_edge_iterator_t<G>;   // iterator_t<vertex_edge_range_t<G>>
using Edge        = edge_t<G>;                   // range_value_t<...>
using EdgeRef     = edge_reference_t<G>;         // range_reference_t<...>
```

---

## Test Patterns

### Basic Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include "graph/graph.hpp"
#include "graph/container/dynamic_graph.hpp"
// ... other includes

TEST_CASE("Feature description", "[tag1][tag2]") {
    // Setup graph
    using G = /* graph type */;
    auto g = /* create or load graph */;
    
    SECTION("sub-test 1") {
        // Test code
        REQUIRE(condition);
    }
    
    SECTION("sub-test 2") {
        // Test code
    }
}
```

### Loading Test Data

```cpp
// CSV loading (from csv_routes.hpp in tests)
auto g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");
auto g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "file.csv", order_policy);
```

---

## Concept Hierarchy

```
basic_adjacency_list
├── index_adjacency_list (+ random_access + integral id)
├── basic_sourced_adjacency_list (+ source_id on edge)
│   └── basic_sourced_index_adjacency_list
│
adjacency_list (+ vertex object)
├── index_adjacency_list
├── sourced_adjacency_list (+ source_id)
│   └── sourced_index_adjacency_list
```

---

## CPO Implementation Pattern

CPOs in `graph_cpo.hpp` follow this pattern:

```cpp
namespace _FunctionName {
    void function_name() = delete;  // Block ADL
    
    template <class G>
    concept _Has_member = requires(G&& g) { g.function_name(); };
    
    template <class G>
    concept _Has_ADL = requires(G&& g) { function_name(g); };
    
    template <class G>
    concept _Can_eval = /* default implementation requirements */;
    
    class _Cpo {
        // Choice enumeration and selection
        // operator() implementation
    };
}
inline constexpr _FunctionName::_Cpo function_name;
```

---

## Error Handling

```cpp
// Standard exception
throw graph_error("description");

// Range validation (common in algorithms)
if (source >= num_vertices(g)) {
    throw std::out_of_range(std::format("source {} out of range", source));
}
```

---

## Important Compiler Considerations

```cpp
// MSVC requires:
#pragma once  // Instead of include guards
// C++23 mode (set in CMakeLists.txt)

// GCC requires:
// --concepts flag (set in CMakeLists.txt)

// Clang: Generally most conformant
```

---

## Debugging Tips

1. **Concept errors**: Check that graph type satisfies required concepts
2. **CPO not found**: Ensure proper ADL namespace or member function exists
3. **Iterator issues**: Check `vertex_info`/`edge_info` template instantiation
4. **Range errors**: Verify sizes with `num_vertices(g)` and `size(edges(g,u))`

---

## File Size Reference

| File                          | Approx Lines | Complexity            |
| ----------------------------- | ------------ | --------------------- |
| `graph_cpo.hpp`               | 2600+        | Very High (core CPOs) |
| `dynamic_graph.hpp`           | 1900+        | High (full container) |
| `compressed_graph.hpp`        | 1100+        | High (CSR impl)       |
| `breadth_first_search.hpp`    | 1100+        | Medium-High           |
| `dijkstra_shortest_paths.hpp` | 300+         | Medium                |
| `graph.hpp`                   | 400+         | Medium (concepts)     |

---

## Common Modifications

### Add a Property CPO

1. Define in `graph_cpo.hpp` following existing pattern
2. Add concept in `graph.hpp` if needed
3. Add type alias if needed

### Extend a Container

1. Specialize CPOs for new behavior
2. Or add member functions that CPOs will find

### Add Algorithm Visitor Events

```cpp
// Check for event existence
template <class G, class Visitor>
concept has_on_my_event = requires(Visitor& vis, event_type& evt) {
    { vis.on_my_event(evt) };
};

// Use in algorithm
if constexpr (has_on_my_event<G, Visitor>) {
    visitor.on_my_event(event_data);
}
```
