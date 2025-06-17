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

TODO: explain the _GraphVisitor_ requirements

TODO: list and describe all possible visitation events


## `dijkstra_shortest_paths` (single source)

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
