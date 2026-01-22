#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "graph/graph.hpp"

using std::vector;
using namespace graph;

TEST_CASE("vertices() CPO available in both namespaces", "[namespace][phase2][task2.2]") {
  // Create simple adjacency list
  vector<vector<int>> g{{1, 2}, {0, 2}, {0, 1}};

  SECTION("Original namespace still works (compatibility)") {
    auto vtx_rng = vertices(g);
    REQUIRE(std::ranges::size(vtx_rng) == 3);
  }

  SECTION("New adj_list namespace works") {
    auto vtx_rng = graph::adj_list::vertices(g);
    REQUIRE(std::ranges::size(vtx_rng) == 3);
  }

  SECTION("Both produce identical results") {
    auto rng1 = vertices(g);
    auto rng2 = graph::adj_list::vertices(g);
    
    auto it1 = std::ranges::begin(rng1);
    auto it2 = std::ranges::begin(rng2);
    
    REQUIRE(*it1 == *it2);
    REQUIRE(std::ranges::size(rng1) == std::ranges::size(rng2));
  }
}

TEST_CASE("vertices() CPO type requirements", "[namespace][phase2][task2.2]") {
  vector<vector<int>> g{{1, 2}, {0, 2}, {0, 1}};

  SECTION("Return types match between namespaces") {
    using T1 = decltype(vertices(g));
    using T2 = decltype(graph::adj_list::vertices(g));
    REQUIRE(std::is_same_v<T1, T2>);
  }
}
