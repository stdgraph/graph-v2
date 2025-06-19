# Views

This library comes with a number of factory functions that allow you to represent 
a graph or parts thereof as _views_ over vertices or edges. Using these views you can 
define your own algorithms more conveniently.


## The "info" classes

TODO: describe `vertex_info` and `edge_info`.

## Breadth-first search views

Header `<graph/views/breadth_first_search.hpp>` defines view factories in namespace `graph`:

```c++
for (auto [vid, v]            : vertices_breadth_first_search(g, seed))         {}
for (auto [vid, v, val]       : vertices_breadth_first_search(g, seed, vvf))    {}

for (auto [vid, uv]           : edges_breadth_first_search(g, seed))            {}
for (auto [vid, uv, val]      : edges_breadth_first_search(g, seed, evf))       {}

for (auto [uid, vid, uv]      : sourced_edges_depth_first_search(g, seed))      {}
for (auto [uid, vid, uv, val] : sourced_edges_depth_first_search(g, seed, evf)) {}
```

They all give you a view of vertices in the order of the breadth-first traversal. 
They differ by the element type they produce.

Breadth-first traversal offers the following properties in terms of rendered vertices:

 * Vertex `seed` is never rendered.
 * Every vertex other than `seed` reachable from `seed` is rendered.
 * For three vertices `u`, `v`, `w`, if 
   * `u` is rendered before `v` and `w` and 
   * there is an edge from `u` to `v` and
   * there is no edge from `u` to `w`,
   
   then `v` will be rendered before `w`. 


```c++
template <index_adjacency_list G, 
          vertex_id_t<G> VI, 
          typename Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto&
vertices_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```

 
*Parameters:*

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `alloc` – allocator to be used for the internal storage.

*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.
 
*Returns:* A view with the value type 
         `vertex_info<const vertex_id_t<G>, vertex_t<G>&, void>`.
         
*Remarks:* The vertex corresponding to vertex id `seed` is not an element in the returned range.
 
 
```c++
template <index_adjacency_list         G, 
          vertex_id_t<G>               VI, 
          std::invocable<vertex_t<G>&> VVF, 
          typename                     Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto&
vertices_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```

Parameters:

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `vvf` – vertex value function,
 * `alloc` – allocator to be used for the internal storage.

*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.
 
*Returns:* A view  with the value type 
         `vertex_info<const vertex_id_t<G>, vertex_t<G>&, std::invoke_result_t<VVF, vertex_t<G>&>>`.
         The third argument is the result of invoking `vvf` with the vertex reference.

*Remarks:* The vertex corresponding to vertex id `seed` is not an element in the returned range.

```c++
template <index_adjacency_list G, 
          vertex_id_t<G>       VI, 
          typename             Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto&
edges_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```
 
Parameters:

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `alloc` – allocator to be used for the internal storage.

*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.
 
*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, false, edge_reference_t<G>, void>`.
 
 
```c++
template <index_adjacency_list                G, 
          vertex_id_t<G>                      VI, 
          std::invocable<edge_reference_t<G>> EVF,
          typename                            Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto&
edges_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```

*Parameters:*

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `evf` – edge value function,
 * `alloc` – allocator to be used for the internal storage.

*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.
 
*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, false, edge_reference_t<G>, std::invoke_result_t<EVF, edge_reference_t<G>>>`.
         The fourth argument is the result of invoking `evf` with the edge reference.



```c++
template <index_adjacency_list G, 
          vertex_id_t<G>       VI, 
          typename             Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto& 
sourced_edges_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```
 
*Parameters:*

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `alloc` – allocator to be used for the internal storage.
 
*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.

*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, true, edge_reference_t<G>, void>`.
 
 
```c++
template <index_adjacency_list                G, 
          vertex_id_t<G>                      VI, 
          std::invocable<edge_reference_t<G>> EVF,
          typename                            Alloc = std::allocator<vertex_id_t<G>>>
std::ranges::input_range auto&
sourced_edges_breadth_first_search(G&& g, const VI& seed, Alloc alloc = Alloc());
```

*Parameters:*

 * `g` – the graph representation,
 * `seed` – the initial vertex,
 * `evf` – edge value function,
 * `alloc` – allocator to be used for the internal storage.
 
*Hardened preconditions:* `find_vertex(g, seed) != nullptr`.

*Mandates:* `alloc` satisfies [<code><em>Cpp17Allocator</em></code>](https://eel.is/c++draft/allocator.requirements) requirements.

*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, true, edge_reference_t<G>, std::invoke_result_t<EVF, edge_reference_t<G>>>`.
         The fourth argument is the result of invoking `evf` with the edge reference.


### Cancellation

Given `bfs` as any of the above breadth-first views, the following two operations 
alter the view, so that some of all of the following elements are skipped:

 * `bfs.cancel(cancel_search::cancel_branch)` – skips the visitation of the current vertex.
 * `bfs.cancel(cancel_search::cancel_all)` – ends the entire visitation.



## Incidence view

Header `<graph/views/incidence.hpp>` defines view factories in namespace `graph`:

```c++
for (auto [vid, uv]      : incidence(g, uid))       {}
for (auto [vid, uv, val] : incidence(g, uid, evf))  {}
for (auto [vid]          : basic_incidence(g, uid)) {}
```

These offer a forward iteration over vertices incident with a given input vertex.

```c++
template <index_adjacency_list G, vertex_id_t<G> VI>
std::ranges::forward_range auto incidence(G&& g, const VI& uid);
```

*Parameters:*

 * `g` – the graph representation,
 * `uid` – the initial vertex.

*Hardened preconditions:* `find_vertex(g, uid) != nullptr`.
 
*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, false, edge_reference_t<G>, void>`.
         

```c++
template <index_adjacency_list G, vertex_id_t<G> VI, std::invocable<edge_reference_t<G>> EVF>
std::ranges::forward_range auto incidence(G&& g, const VI& uid, EFV evf);
```

*Parameters:*

 * `g` – the graph representation,
 * `uid` – the initial vertex.

*Hardened preconditions:* `find_vertex(g, uid) != nullptr`.
 
*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, false, edge_reference_t<G>, std::invoke_result_t<EVF, edge_reference_t<G>>>`.
 

```c++
template <index_adjacency_list G, vertex_id_t<G> VI>
std::ranges::forward_range auto basic_incidence(G&& g, const VI& uid);
```

*Parameters:*

 * `g` – the graph representation,
 * `uid` – the initial vertex.

*Hardened preconditions:* `find_vertex(g, uid) != nullptr`.
 
*Returns:* A view with the value type 
         `edge_info<const vertex_id_t<G>, false, void, void>`. 



## TODO: other viwes