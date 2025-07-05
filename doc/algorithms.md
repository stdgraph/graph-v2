# Algorithms

## Definitions

A graph path _p_ is a possibly empty sequence of graph edges (_e_<sub>0</sub>, _e_<sub>1</sub>, ..., _e_<sub>_N_</sub>) where:
  * _e_<sub>_i_</sub> ≠ _e_<sub>_j_</sub> for _i_ ≠ _j_,
  * target(_e_<sub>_i_</sub>) = source(_e_<sub>_i_+1</sub>),
  * source(_e_<sub>_i_</sub>) != source(_e_<sub>_j_</sub>) for _i_ ≠ _j_.

<code><i>path-source</i>(<i>p</i>)</code> = source(_e_<sub>0</sub>). <code><i>path-target</i>(<i>p</i>)</code> = target(_e_<sub>_N_</sub>).

<code><i>distance(p)</i></code> is a sum over _i_ of <code>weight</code>(_e_<sub>_i_</sub>).

<code><i>shortest-path</i>(g, u, v)</code> is a path in the set of all paths `p` in graph `g` with <code><i>path-source</i>(<i>p</i>)</code> = `u` 
and <code><i>path-target</i>(<i>p</i>)</code> = v that has the smallest value of <code><i>distance(p)</i></code>.

<code><i>shortest-path-distance</i>(g, u, v)</code> is <code><i>distance</i>(<i>shortest-path</i>(g, u, v))</code> if it exists and _infinite-distance_ otherwise.

<code><i>shortest-path-predecessor</i>(g, u, v)</code>, in the set of all shortest paths <code><i>shortest-path</i>(g, u, v)</code> for any `v`:
 * if there exists an edge _e_ with target(_e_) = v, then it is source(_e_),
 * otherwise it is `v`.

## Visitors

A number of functions in this section take a _visitor_ as an optional argument. 
As different _events_, related to vertices and edges, occur during the execution of an algorithm,
a corresponding member function, if present, is called for the visitor.

Each algorithm defines the events that it supports. Visitor functions corresponding to not supported events, even if present in the visitor are ignored.

If an algorithm supports a given event but the specified visitor does not provide the corresponding valid member function, no runtime overhead related to processing this event is incurred.


### <code><em>GraphVisitor</em></code> requirements

The following lists the visitation events and the corresponding visitor member functions.
For each of the events the visitor may choose to support it via making the corresponding member
function valid.

The notation used:

| name  | type | definition  |
|-------|------|-------------|
| `vis` |      | the visitor |
| `G`   |      | the type of the graph that the algorithm is instantiated for           |
| `vd`  | `vertex_info<vertex_id_t<G>, vertex_reference_t<G>, void>`   | visited vertex |
| `ed`  | `edge_info<vertex_id_t<G>, true, edge_reference_t<G>, void>` | visited edge   |

```c++
vis.on_discover_vertex(vd)
```

If valid, it is called whenever a new vertex is identified for future examination in the 
course of executing an algorithm. 

(Note: the vertices provided as _sources_ to algorithms are initially discovered.)

```c++
vis.on_examine_vertex(vd)
```

If valid, it is called whenever a previously discovered vertex is started being examined.

(Note: examining a vertex usually triggers the discovery of other vertices and edges.)

```c++
vis.on_finish_vertex(vd)
```

If valid, it is called whenever an algorithm finishes examining the vertex.

(Note: If the graph is unbalanced and another path to this vertex has a lower accumulated
       weight, the algorithm will process `vd` again.
       A consequence is that `on_examine_vertex` could be called twice (or more) on the 
       same vertex.)

```c++
vis.on_examine_edge(ed)
```
 
If valid, it is called whenever a new edge is started being examined.



 
```c++
vis.on_edge_relaxed(ed)
```

If valid, it is called whenever an edge is _relaxed_. Relaxing an edge means reducing 
the stored minimum accumulated distance found so far from the given source to the target 
of the examined edge `ed`.


```c++
vis.on_edge_not_relaxed(ed)
```

If valid, it is called whenever a new edge `ed` is inspected but not relaxed (because
the stored accumulated distance to the target of `ed` found so far is smaller than the path via `ed`.)

```c++
vis.on_edge_minimized(ed)
```

If valid, it is called when no cycles have been detected while examining the edge `ed`.


```c++
vis.on_edge_not_minimized(ed)
```

If valid, it is called when a cycles have been detected while examining the edge `ed`.
This happens in shortest paths algorithms that accept negative weights, and means that 
no finite minimum exists.


### `empty_visitor`

This library comes with an empty class `empty_visitor`:

```c++
namespace graph {
  struct empty_visitor{};
}
```

It is used as the default visitor type for the algorithms. This visitor supports no events, and therefore triggers no runtime overhead on any event.


## `dijkstra_shortest_paths` 

The shortest paths algorithm builds on the idea that each edge in a graph has its associated _weight_.
A path _distance_ is determined by the composition of weights of edges that constitute the path.

By default the composition of the edge weights is summation and the default weight is 1, 
so the path distance is the number of edges that it comprises.

Dijkstra's shortest paths algorithm also makes an assumption that appending an edge to a path _increases_ 
the path's distance. In terms of the default composition and weight this assumption is expressed as `weight(uv) >= 0`.

The distances of each path are returned directly vie the output function argument. 
The paths themselves, if requested, are only returned indirectly by providing for each vertex
its predecessor in any shortest path. 


### The single source version

Header `<graph/algorithm/dijkstra_shortest_paths.hpp>`

```c++
template <index_adjacency_list             G,
          std::ranges::random_access_range Distances,
          std::ranges::random_access_range Predecessors,
          class WF      = function<std::ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Visitor = empty_visitor,
          class Compare = less<std::ranges::range_value_t<Distances>>,
          class Combine = plus<std::ranges::range_value_t<Distances>>>
requires std::is_arithmetic_v<std::ranges::range_value_t<Distances>> &&
         std::ranges::sized_range<Distances> && 
         std::ranges::sized_range<Predecessors> && 
         convertible_to<vertex_id_t<G>, std::ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, std::ranges::range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](edge_reference_t<G> uv) { return std::ranges::range_value_t<Distances>(1); },
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<std::ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<std::ranges::range_value_t<Distances>>());
```

*Preconditions:* 
  * <code>distances[<i>i</i>] == shortest_path_infinite_distance&lt;range_value_t&lt;Distances&gt;&gt;()</code> for each <code><i>i</i></code> in range [`0`; `num_vertices(g)`),
  * <code>predecessor[<i>i</i>] == <i>i</i></code> for each <code><i>i</i></code> in range [`0`; `num_vertices(g)`),
  * `weight` returns non-negative values.
  * `visitor` adheres to the _GraphVisitor_ requirements.
    
*Hardened preconditions:* 
  * `0 <= source && source < num_vertices(g)` is `true`,
  * `std::size(distances) >= num_vertices(g)` is `true`,
  * `std::size(predecessor) >= num_vertices(g)` is `true`.
    
*Effects:* Supports the following visitation events: `on_initialize_vertex`, `on_discover_vertex`,
    `on_examine_vertex`, `on_finish_vertex`, `on_examine_edge`, `on_edge_relaxed`, and `on_edge_not_relaxed`.

*Postconditions:* For each <code><i>i</i></code> in range [`0`; `num_vertices(g)`):
  * <code>distances[<i>i</i>]</code> is <code><i>shortest-path-distance</i>(g, source, <i>i</i>)</code>,
  * <code>predecessor[<i>i</i>]</code> is <code><i>shortest-path-predecessor</i>(g, source, <i>i</i>)</code>.

*Throws:* `std::bad_alloc` if memory for the internal data structures cannot be allocated.

*Complexity:* Either 𝒪((|_E_| + |_V_|)⋅log |_V_|) or 𝒪(|_E_| + |_V_|⋅log |_V_|), depending on the implementation.

*Remarks:* Duplicate sources do not affect the algorithm’s complexity or correctness.


## TODO

Document all other algorithms...
