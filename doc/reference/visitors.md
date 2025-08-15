# Visitors

A number of functions in this section take a _visitor_ as an optional argument. 
As different _events_, related to vertices and edges, occur during the execution of an algorithm,
a corresponding member function, if present, is called for the visitor.

Each algorithm defines the events that it supports. Visitor functions corresponding to not 
supported events, even if present in the visitor are ignored.

If an algorithm supports a given event but the specified visitor does not provide the corresponding valid member function, no runtime overhead related to processing this event is incurred.


## <code><em>GraphVisitor</em></code> requirements

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


## `empty_visitor`

This library comes with an empty class `empty_visitor`:

```c++
namespace graph {
  struct empty_visitor{};
}
```

It is used as the default visitor type for the algorithms. This visitor supports no events, and therefore triggers no runtime overhead on any event.
