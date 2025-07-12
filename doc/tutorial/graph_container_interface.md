Graph Container Interface
=========================

Generic algorithms and views in this library require that your graph container 
is represented as an [adjacency list](https://en.wikipedia.org/wiki/Adjacency_list).
Your container is never accessed directly. Instead, the access is performed via the 
*graph container interface*.
 

```c++
  std::vector<std::vector<int>> g {  //  
    /*0*/ {1},                       //
    /*1*/ {0}                        //      (0) ----- (1)
  };    

  auto && vtcs = graph::vertices(g);    // get the std::range representing vertices
  auto vit = std::ranges::begin(vtcs);
  int id0 = graph::vertex_id(g, vit);   // get the id of the vertex
  assert(id0 == 0);
  auto && edgs = graph::edges(g, *vit); // 
  auto eit = std::ranges::begin(edgs);
  int id1 = graph::target_id(g, *eit);
  assert(id1 == 1);
```