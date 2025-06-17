# Customization Points

The algorithms and views in this library operate on graph representations via _Customization Point Objects_ (CPO). 
A user-defined graph representation `G` is adapted for use with this library by making sure that the necessary CPOs are _valid_ for `G`. 

A CPO is a function object, so it can be passed as an argument to functions.

Each customization point specifies individually what it takes to make it valid. 
A customization point can be made valid in a number of ways. 
For each customization point we provide an ordered list of ways in which it can be made valid.
The order in this list matters: the match for validity is performed in order,
and if a given customization is determined to be valid, the subsequent ways, even if they would be valid, are ignored.

Often, the last item from the list serves the purpose of a "fallback" or "default" customization.

If none of the customization ways is valid for a given type, or set of types, the customization point is considered _invalid_ for this set of types. 
The property or being valid or invalid can be statically tested in the program via SFINAE (like `enable_if`) tricks or `requires`-expressions.

All the customization points in this library are defined in namespace `::graph` and brought into the program code via including header  `<graph/graph.hpp>`.


## The list of customization points

We use the following notation to represent the customization points:


| Symbol | Type                           | Meaning                                  |
|--------|--------------------------------|------------------------------------------|
| `G`    |                                | the type of the graph representation     |
| `g`    | `G`                            | the graph representation                 | 
| `u`    | `graph::vertex_reference_t<G>` | vertex in `g`                            |
| `ui`   | `graph::vertex_iterator_t<G>`  | iterator to a vertex in `g`              |
| `uid`  | `graph::vertex_id_t<G>`        | _id_ of a vertex in `g` (often an index) |
| `uv`   | `graph::edge_reference_t<G>`   | an edge in `g`                           |


### `vertices`

The CPO `vertices(g)` is used to obtain the list of all vertices, in form of a `std::ranges::random_access_range`, from the graph-representing object `g`.
We also use its return type to determine the type of the vertex: `vertex_t<G>`.

#### Customization

 1. Returns `g.vertices()`, if such member function exists and returns a `std::move_constructible` type.
 2. Returns `vertices(g)`, if such function is ADL-discoverable and returns a `std::move_constructible` type.
 3. Returns `g`, if it is a `std::ranges::random_access_range`.


### `vertex_id`

The CPO `vertex_id(g, ui)` is used obtain the _id_ of the vertex, given the iterator.
We also use its return type to determine the type of the vertex id: `vertex_id_t<G>`.

#### Coustomization

 1. Returns `ui->vertex_id(g)`, if this expression is valid and its type is `std::move_constructible`.
 2. Returns `vertex_id(g, ui)`, if this expression is valid and its type is `std::move_constructible`.
 3. Returns <code>static_cast&lt;<em>vertex-id-t</em>&lt;G&gt;&gt;(ui - begin(vertices(g)))</code>,
    if `std::ranges::random_access_range<vertex_range_t<G>>` is `true`, where <code><em>vertex-id-t</em></code> is defined as:

    * `I`, when the type of `G` matches pattern `ranges::forward_list<ranges::forward_list<I>>` and `I` is `std::integral`,
    * `I0`, when the type of `G` matches pattern <code>ranges::forward_list&lt;ranges::forward_list&lt;<em>tuple-like</em>&lt;I0, ...&gt;&gt;&gt;</code> and `I0` is `std::integral`,
    * `std::size_t` otherwise.


### `find_vertex`

TODO `find_vertex(g, uid)`

### `edges(g, u)`

### `edges(g, uid)`

### `num_edges(g)`

### `target_id(g, uv)`

### `target_id(e)`

### `source_id(g, uv)`

### `source_id(e)`

### `target(g, uv)`

### `source(g, uv)`

### `find_vertex_edge(g, u, vid)`

### `find_vertex_edge(g, uid, vid)`

### `contains_edge(g, uid, vid)`

### `partition_id(g, u)`

### `partition_id(g, uid)`

### `num_vertices(g, pid)`

### `num_vertices(g)`

### `degree(g, u)`

### `degree(g, uid)`

### `vertex_value(g, u)`

### `edge_value(g, uv)`

### `edge_value(e)`

### `graph_value(g)`

### `num_partitions(g)`

### `has_edge(g)`

