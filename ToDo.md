# ToDo & Issues

## Open

### ToDo 
- API
  - [ ] Common
  - [ ] concepts and type_traits
  - [x] graph types
  - [x] vertex types
  - [x] edge types
  - [ ] Accessor functions
    - [ ] as CPO
    - [ ] default implementations (e.g. degree==size)
  - [x] ranges
    - [x] vertex_range
    - [x] vertex_edge_range
    - [x] vertex_vertex_range
    - [x] edge_range
- Algorithms
  - [ ] Common
  - [ ] Ranges
    - [ ] BFS
    - [ ] DFS
    - [ ] Toplogical Sort
  - [ ] Range Adaptors
    - [ ] edge_range<graph, vertex_range, vertex_edge_range>
    - [ ] vertex_vertex_range<graph, vertex_range, vertex_edge_range>
    - [ ] indexed_vertex_range: returns pair<vertex_key,vertex&>
  - [ ] Algorithms (full & simplified/book)
    - [ ] Shortest Paths
      - [ ] Dijkstra (book)
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
    - [ ] vol (vector of forward_list)
      - [x] Implement graph, vector, edge
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
  - [x] Fork & modify? No response to my issue I submitted
- [ ] vertex_key(g,u) now takes a vertex reference (not iterator)
  - [ ] how to define vertex_key_t without calling vertex_key()?
  - [ ] make it obvious it's unavailable for non-contiguous vertices
  - [ ] create index_vertex_range that returns pair<index,vertex&>
- [ ] Are the CSR vertex & edge types different? (requirement of incidence and adjacency concepts)

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
