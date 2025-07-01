# Utilities

Some components in this library, such as graph traversal views or visitors, 
need to represent a number of properties of a graph vertes at once, like vertex id,
vertex reference, and vertex value. For this purpose class template `vertex_info` is used.

```c++
// header <graph/graph_info.hpp>

namespace graph {

template <class VId, class V, class VV>
struct vertex_info {
  using id_type     = VId; // usually vertex_id_t<G>
  using vertex_type = V;   // usually vertex_reference_t<G>
  using value_type  = VV;  // result of coputing the value of a vertex

  id_type     id;          // absent when `VId` is `void`
  vertex_type vertex;      // absent when `V` is `void`
  value_type  value;       // absent when `VV` is `void`
};

}
```

This class template comes with a number of specializations which make certain data members disappear when the corresponding teplate parameter is `void`.

Thre is an analogous utility class for representing edge information.

```c++
// header <graph/graph_info.hpp>

namespace graph {

template <class VId, bool Sourced, class E, class EV>
struct edge_info {
  using source_id_type = VId; // this type is `void` when `Sourced` is `false`
  using target_id_type = VId; // usually vertex_id_t<G>
  using edge_type      = E;   // usually edge_reference_t<G>
  using value_type     = EV;  //

  source_id_type source_id;   // absent when `Sourced` is `false`
  target_id_type target_id;   // absent when `VId` is `void`
  edge_type      edge;        // absent when `E` is `void`
  value_type     value;       // absent when `EV` is `void`
};

}
```
