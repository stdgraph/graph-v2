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

The CPO `vertices(g)` is used to obtain the range of all vertices, in form of a `std::ranges::random_access_range`, from the graph-representing object `g`.
We also use its return type to determine the type of the vertex: `vertex_t<G>`.

#### Customization

 1. Returns `g.vertices()`, if such member function exists and returns a `std::move_constructible` type.
 2. Returns `vertices(g)`, if such function is ADL-discoverable and returns a `std::move_constructible` type.
 3. Returns `g`, if it is a `std::ranges::random_access_range`.


### `vertex_id`

The CPO `vertex_id(g, ui)` is used obtain the _id_ of the vertex, given the iterator. <br>
We also use its return type to determine the type of the vertex id: `vertex_id_t<G>`.

#### Customization

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

The CPO `edges(g, u)` is used to obtain the sequence of outgoing edges for a vertex 
denoted by reference `u`. <br>
We also use its return type to determine the type of the edge type: `edge_t<G>`.

#### Customization

 1. Returns `u.edges(g)`, if this expression is valid and its type is `std::move_constructible`.
 2. Returns `edges(g, u)`, if this expression is valid and its type is `std::move_constructible`.
 3. `u`, if `G` is a user-defined type and type `vertex_t<G>` is `std::ranges::forward_range;


### `edges(g, uid)`

The CPO `edges(g, uid)` is used to obtain the sequence of outgoing edges for a vertex 
denoted by _id_ `uid`.

#### Customization

 1. Returns `edges(g, uid)`, if this expression is valid and its type is `std::move_constructible`.
 2. Returns `*find_vertex(g, uid)`, if
    * `vertex_t<G>` is `std::ranges::forward_range`, and
    * expression `find_vertex(g, uid)` is valid and its type is `std::move_constructible`.
 

### `num_edges(g)`

TODO


### `target_id(g, uv)`

The CPO `target_id(g, uv)` is used to obtain the _id_ of the target vertex of edge `uv`.

#### Customization

 1. Returns `uv.target_id(g)`, if this expression is valid and its type is `std::move_constructible`.
 2. Returns `target_id(g, uv)`, if this expression is valid and its type is `std::move_constructible`.
 3. Returns `uv`, if
    * `G` is `std::ranges::forward_range`, and
    * `std::ranges::range_value_t<G>` is `std::ranges::forward_range`, and
    * `std::ranges::range_value_t<std::ranges::range_value_t<G>>` is `std::integral`.
 4. Returns `get<0>(uv)`, if
    * `G` is `std::ranges::forward_range`, and
    * `std::ranges::range_value_t<G>` is `std::ranges::forward_range`, and
    * `std::ranges::range_value_t<std::ranges::range_value_t<G>>` is <code><em>tuple-like</em></code>,
    * `std::tuple_element_t<0, std::ranges::range_value_t<std::ranges::range_value_t<G>>>` is `std::integral`.


### `target_id(e)`

### `source_id(g, uv)`

### `source_id(e)`


### `target(g, uv)`

CPO `target(g, uv)` is used to access the target vertex of a given edge `uv`.

#### Customization

 1. Returns `target(g, uv)`, if this expression is valid and its type is `std::move_constructible`. 
 2. Returns `*find_vertex(g, target_id(g, uv))`, if
 
    * `vertex_range_t<G>` is a `std::ranges::random_access_range`, and
    * `find_vertex(g, uid)` is valid and its type is `std::move_constructible`, and
    * `target_id(g, uv)` is valid and its type is `std::integral`. 


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

