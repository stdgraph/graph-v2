Graph Container Interface
=========================

Generic algorithms and views in this library require that your graph container 
is represented as an [adjacency list](https://en.wikipedia.org/wiki/Adjacency_list).
Your container is never accessed directly. Instead, the access is performed via the 
*graph container interface* (GCI).
 

```c++
  std::vector<std::vector<int>> g {      //  
    /*0*/ {1},                           //
    /*1*/ {0}                            //      (0) ----- (1)
  };                                     //

  auto && vtcs = graph::vertices(g);     // get the std::range representing graph vertices
  
  auto vit = std::ranges::begin(vtcs);
  
  int id0 = graph::vertex_id(g, vit);    // get the id of the vertex
  assert(id0 == 0);
  
  auto && edgs = graph::edges(g, *vit);  // get the std::range representing the neighbors of the vertex
  
  auto eit = std::ranges::begin(edgs);
  
  int id1 = graph::target_id(g, *eit);   // get the id of the vertex on the other side of the edge
  assert(id1 == 1);
  
  auto wit = graph::find_vertex(g, id1); // get iterator to vertex based on the vertex id
  
  int id1_ = graph::vertex_id(g, wit);
  assert(id1_ == 1);
```

This set of operations may seem too rich: you need fewer operations to achieve the same 
for type `vector<vector<int>>`. But the Graph Container Interface needs to be prepared for
very different representations of an adjacency list.

The list of all CPOs in the Graph Container interface is bigger. 
All the customization points are described in detail in section [Customization Points](../reference/customization_points.md). 


Testing the customization
-------------------------

In order to test if your type has the necessary customization for all the CPOs, you can use concepts defined in this library.

The one that you will need most often is `graph::index_adjacency_list`:

```c++
#include <graph/graph.hpp>

static_assert(graph::index_adjacency_list<MyType>); 
```

In practice, you need to customize only a handful of CPOs. Most of the others have their default customizaiton.