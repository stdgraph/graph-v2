# ToDo & Issues

## Open

### ToDo 
- API
  - [ ] Common
  - [ ] concepts and type_traits
  - [ ] graph types
  - [ ] vertex types
  - [ ] edge types
  - [ ] ranges
    - [ ] vertex_range
    - [ ] vertex_edge_range
    - [ ] vertex_vertex_range
    - [ ] edge_range
- Algorithms
  - [ ] Common
  - [ ] Ranges
    - [ ] BFS
    - [ ] DFS
    - [ ] Toplogical Sort
  - [ ] Range Adaptors
    - [ ] edge_range<graph, vertex_range, vertex_edge_range>
    - [ ] vertex_vertex_range<graph, vertex_range, vertex_edge_range>
  - [ ] Algorithms (full & simplified/book)
    - [ ] Shortest Paths
      - [ ] Dijkstra
      - [ ] Bellman-Ford
    - [ ] Connected Components
    - [ ] Strongly Connected Components
    - [ ] Biconnected Components
    - [ ] Articulation Points
    - [ ] Transitive Closure
- Graph Containers (data structures)
    - [ ] csr_adjacency graph
      - [x] Define
      - [ ] Create vertex type
      - [ ] Create edge type
      - [ ] Create vertex iterators
      - [ ] Create vertex_edge iterators
      - [ ] routes_csv_csr_graph
        - [ ] Load from CSV file
        - [ ] output to cout to validate
        - [ ] unit tests
          - [ ] Validate structure & weights
      - [ ] Validate constexpr using initializer_list
    - [ ] vofl (vector of forward_list)
    - [ ] directed_adjacency_vector
    - [ ] undirected_adjacency_list
- C\+\+20 and C\+\+23
  - [ ] connection points (e.g. tag_invoke)
  - [ ] modules
  - [ ] coroutines (simplify DFS, BFS & TopoSort?)
- Tools, Libraries & Infrastructure
  - [ ] Create mtx parser (from nwgraph)
  - [ ] Constexpr unit tests (initial tests to verify pattern)
  - [ ] Generate doxygen output
  - [ ] Validate address sanitizer build
  - [ ] Support Clang (waiting for full concepts support)
- Feature & performance comparison
  - [ ] boost::graph
  - [ ] NWGraph
- Documentation
  - [ ] REAME.md
    - [ ] Add general description
    - [ ] Add link to paper
    - [ ] Add instructions for building
- Feedback

### Issues
- [ ] Can't run tests in VS+WSL2 (they will run in VSCode)
  - [ ] Submit a defect?
- [ ] csv_parser
  - [ ] CSVReader doesn't conform to the C++20 input_range concept; can't define load functions properly
    - [ ] What's missing? const types; other?
  - [ ] leading & trailing spaces for quoted values aren't ignored
  - [ ] can it be reused for multi-pass?
  - [ ] Fork & modify?

## Resolved
### ToDo Completed
- API
- Algorithms
- Graph Containers (data structures)
- C\+\+20 and C\+\+23
- Tools, Libraries & Infrastructure
  - [x] Use CMake Presets; must work with both VS & VSCode
  - [x] Use conan for libraries when possible: catch2, fmt, spdlog, range-v3
  - [x] Include csv_parser library
- Documentation
- Feedback

### Issues Resolved
