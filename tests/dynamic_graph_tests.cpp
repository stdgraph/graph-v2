// Copyright (C) 2025 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "graph/container/dynamic_graph.hpp"
#include <string>
#include <vector>

//            A
//      0.8 /  \ 0.4
//         B    C
//      0.1 \  / 0.2
//           D

using routes_vofl_graph_traits = graph::container::vofl_graph_traits<double>;
using routes_vofl_graph_type   = graph::container::dynamic_adjacency_graph<routes_vofl_graph_traits>;

TEST_CASE("dynamic_graph container operations", "[dynamic_graph]") {
  using namespace graph::container;
  using G = routes_vofl_graph_type;
  using Edge = graph::copyable_edge_t<uint32_t, double>;
  
  // Test basic construction and loading
  std::vector<Edge> edge_data = {
    {0, 1, 1.0}, {0, 2, 2.0}, {1, 2, 3.0}
  };
  
  G g;
  // dynamic_graph requires vertices to exist before adding edges
  g.resize_vertices(3);
  g.load_edges(edge_data, std::identity{});
  
  // Verify graph structure through CPOs
  auto vr = graph::vertices(g);
  REQUIRE(std::ranges::size(vr) == 3);
  
  // Verify edges from vertex 0 (forward_list doesn't have size(), use distance)
  auto er0 = graph::edges(g, 0);
  REQUIRE(std::ranges::distance(er0) == 2);
  
  // Verify edges from vertex 1  
  auto er1 = graph::edges(g, 1);
  REQUIRE(std::ranges::distance(er1) == 1);
  
  // Verify edges from vertex 2
  auto er2 = graph::edges(g, 2);
  REQUIRE(std::ranges::distance(er2) == 0);
}
