// Copyright (C) 2025 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "graph/container/compressed_graph.hpp"
#include <string>
#include <vector>

//            A
//      0.8 /  \ 0.4
//         B    C
//      0.1 \  / 0.2
//           D

const std::vector<graph::copyable_edge_t<unsigned, double>> ve{{0, 1, 0.8}, {0, 2, 0.4}, {1, 3, 0.1}, {2, 3, 0.2}};

const std::vector<graph::copyable_vertex_t<unsigned, std::string>> vv{{0, "A"}, {1, "B"}, {2, "C"}, {3, "D"}};

TEST_CASE("compressed_graph constructor", "[compressed_graph]") {
  using graph_t = graph::container::compressed_graph<double, std::string>;
  const graph_t g(ve, vv);

  std::vector<std::string> vertex_names;

  for (graph_t::vertex_type const& u : vertices(g))
    vertex_names.push_back(vertex_value(g, u));

  REQUIRE(vertex_names == std::vector<std::string>{"A", "B", "C", "D"});
}

TEST_CASE("compressed_graph container operations", "[compressed_graph]") {
  using namespace graph::container;
  using Edge = graph::copyable_edge_t<uint32_t, double>;
  
  // Test basic construction and loading
  std::vector<Edge> edge_data = {
    {0, 1, 1.0}, {0, 2, 2.0}, {1, 2, 3.0}
  };
  
  compressed_graph<double> g;
  g.load_edges(edge_data);
  
  // Verify graph structure through CPOs
  auto vr = graph::vertices(g);
  REQUIRE(std::ranges::size(vr) == 3);
  
  // Verify edges from vertex 0
  auto er0 = graph::edges(g, 0);
  REQUIRE(std::ranges::size(er0) == 2);
  
  // Verify edges from vertex 1  
  auto er1 = graph::edges(g, 1);
  REQUIRE(std::ranges::size(er1) == 1);
  
  // Verify edges from vertex 2
  auto er2 = graph::edges(g, 2);
  REQUIRE(std::ranges::size(er2) == 0);
}
