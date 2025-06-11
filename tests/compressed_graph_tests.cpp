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

const std::vector<graph::copyable_edge_t<unsigned, double>> ve {
  {0, 1, 0.8}, {0, 2, 0.4}, {1, 3, 0.1}, {2, 3, 0.2}
};

const std::vector<graph::copyable_vertex_t<unsigned, std::string>> vv {
  {0, "A"}, {1, "B"}, {2, "C"}, {3, "D"}
};

TEST_CASE("compressed_graph constructor", "[compressed_graph]") {
  using graph_t = graph::container::compressed_graph<double, std::string>;
  const graph_t g(ve, vv);
    
  std::vector<std::string> vertex_names;
    
  for (graph_t::vertex_type const& u : vertices(g))
    vertex_names.push_back(vertex_value(g, u));
        
  REQUIRE(vertex_names == std::vector<std::string>{"A", "B", "C", "D"});
}

