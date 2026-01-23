// Test to validate Phase 6, Task 6.1: vertexlist() in graph::adj_list::views

#include <catch2/catch_test_macros.hpp>
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/vertexlist.hpp"
#include <vector>

using namespace graph;
using namespace graph::container;

TEST_CASE("vertexlist in adj_list::views namespace", "[vertexlist][views][namespace]") {
  // Create a simple graph
  std::vector<std::vector<int>> g(5);

  SECTION("graph::views::vertexlist (original namespace)") {
    auto vl = graph::views::vertexlist(g);
    REQUIRE(std::ranges::distance(vl) == 5);

    // Verify iteration works
    int count = 0;
    for (auto&& [uid, u] : vl) {
      REQUIRE(uid == count);
      ++count;
    }
    REQUIRE(count == 5);
  }

  SECTION("graph::adj_list::views::vertexlist (new namespace)") {
    auto vl = graph::adj_list::views::vertexlist(g);
    REQUIRE(std::ranges::distance(vl) == 5);

    // Verify iteration works
    int count = 0;
    for (auto&& [uid, u] : vl) {
      REQUIRE(uid == count);
      ++count;
    }
    REQUIRE(count == 5);
  }

  SECTION("Both namespaces produce equivalent results") {
    auto vl1 = graph::views::vertexlist(g);
    auto vl2 = graph::adj_list::views::vertexlist(g);

    // Both should have same distance
    REQUIRE(std::ranges::distance(vl1) == std::ranges::distance(vl2));

    // Both should iterate the same elements
    auto it1 = vl1.begin();
    auto it2 = vl2.begin();
    while (it1 != vl1.end() && it2 != vl2.end()) {
      auto [uid1, u1] = *it1;
      auto [uid2, u2] = *it2;
      REQUIRE(uid1 == uid2);
      REQUIRE(&u1 == &u2); // Same vertex references
      ++it1;
      ++it2;
    }
    REQUIRE(it1 == vl1.end());
    REQUIRE(it2 == vl2.end());
  }
}
