// Copyright (C) 2025 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// This test file demonstrates how one can adapt one's own graph container for use with
// this Graph Library

#include <graph/views/depth_first_search.hpp>
#include <graph/graph.hpp>
#include <string>
#include <vector>
#include <iostream>


namespace MyLibrary { // custom graph container, conceptually an adjacency list

struct MyEdge {
  std::string content;
  int         indexOfTarget;
};

struct MyVertex {
  std::string         content;
  std::vector<MyEdge> outEdges;
};

class MyGraph {
  std::vector<MyVertex> _vertices;

public:
  MyVertex const* getVertexByIndex(int index) const { return &_vertices[static_cast<unsigned>(index)]; }

  std::vector<MyVertex> const& getAllVertices() const // !! one of customization points
  {
    return _vertices;
  } //    forced me to add this fun

  void setTopology(std::vector<MyVertex> t) { _vertices = std::move(t); }
};

} // namespace MyLibrary

namespace MyLibrary { // customization for graph, unintrusive
                      // although forcing me to provide `vertices()` is superfluous

auto vertices(MyGraph const& g) { return std::views::all(g.getAllVertices()); } // for vertex_range_t

auto edges(MyGraph const&, const MyLibrary::MyVertex& v) { return std::views::all(v.outEdges); }

auto edges(MyGraph const& g, int i) { return edges(g, *g.getVertexByIndex(i)); }

int vertex_id(MyGraph const& g, std::vector<MyVertex>::const_iterator it) {
  return static_cast<int>(std::distance(g.getAllVertices().begin(), it));
}

int target_id(MyGraph const&, MyEdge const& uv) { return uv.indexOfTarget; }

} // namespace MyLibrary


int main() {
  static_assert(graph::adjacency_list<MyLibrary::MyGraph>);

  const MyLibrary::MyGraph g = [] { // populate the graph
    MyLibrary::MyGraph               r;
    std::vector<MyLibrary::MyVertex> topo{
          //         A       |
          /*0*/ {"A", {{"", 1}, {"", 2}}}, //       /  \      |
          /*1*/ {"B", {{"", 3}}},          //      B    C     |
          /*2*/ {"C", {{"", 3}}},          //       \  /      |
          /*3*/ {"D", {}}                  //        D        |
    };
    r.setTopology(std::move(topo));
    return r;
  }();

  for (auto const& [vid, v] : graph::views::vertices_depth_first_search(g, 0))
    std::cout << v.content << " ";

  std::cout << std::endl;
}