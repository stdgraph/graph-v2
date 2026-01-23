// Test to validate Phase 6, Tasks 6.2-6.6: All view CPOs in graph::adj_list::views

#include <catch2/catch_test_macros.hpp>
#include "graph/container/dynamic_graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views/edgelist.hpp"
#include "graph/views/breadth_first_search.hpp"
#include "graph/views/depth_first_search.hpp"
#include <vector>

using namespace graph;
using namespace graph::container;

TEST_CASE("incidence in adj_list::views namespace", "[incidence][views][namespace]") {
  // Create a simple graph with edges
  std::vector<std::vector<int>> g = {{1, 2}, {2}, {}};

  SECTION("graph::views::incidence (original namespace)") {
    auto inc = graph::views::incidence(g, 0);
    REQUIRE(std::ranges::distance(inc) == 2);
  }

  SECTION("graph::adj_list::views::incidence (new namespace)") {
    auto inc = graph::adj_list::views::incidence(g, 0);
    REQUIRE(std::ranges::distance(inc) == 2);
  }

  SECTION("Both namespaces produce equivalent results") {
    auto inc1 = graph::views::incidence(g, 0);
    auto inc2 = graph::adj_list::views::incidence(g, 0);
    REQUIRE(std::ranges::distance(inc1) == std::ranges::distance(inc2));
  }
}

TEST_CASE("neighbors in adj_list::views namespace", "[neighbors][views][namespace]") {
  // Create a simple graph with edges
  std::vector<std::vector<int>> g = {{1, 2}, {2}, {}};

  SECTION("graph::views::neighbors (original namespace)") {
    auto nbr = graph::views::neighbors(g, 0);
    REQUIRE(std::ranges::distance(nbr) == 2);
  }

  SECTION("graph::adj_list::views::neighbors (new namespace)") {
    auto nbr = graph::adj_list::views::neighbors(g, 0);
    REQUIRE(std::ranges::distance(nbr) == 2);
  }

  SECTION("Both namespaces produce equivalent results") {
    auto nbr1 = graph::views::neighbors(g, 0);
    auto nbr2 = graph::adj_list::views::neighbors(g, 0);
    REQUIRE(std::ranges::distance(nbr1) == std::ranges::distance(nbr2));
  }
}

TEST_CASE("edgelist in adj_list::views namespace", "[edgelist][views][namespace]") {
  // Create a simple graph with edges
  std::vector<std::vector<int>> g = {{1, 2}, {2}, {}};

  SECTION("graph::views::edgelist (original namespace)") {
    auto el = graph::views::edgelist(g);
    REQUIRE(std::ranges::distance(el) == 3);
  }

  SECTION("graph::adj_list::views::edgelist (new namespace)") {
    auto el = graph::adj_list::views::edgelist(g);
    REQUIRE(std::ranges::distance(el) == 3);
  }

  SECTION("Both namespaces produce equivalent results") {
    auto el1 = graph::views::edgelist(g);
    auto el2 = graph::adj_list::views::edgelist(g);
    REQUIRE(std::ranges::distance(el1) == std::ranges::distance(el2));
  }
}

TEST_CASE("BFS views in adj_list::views namespace", "[bfs][views][namespace]") {
  // Create a simple graph with edges
  std::vector<std::vector<int>> g    = {{1, 2}, {2}, {}};
  vertex_id_t<decltype(g)>      seed = 0;

  SECTION("vertices_breadth_first_search") {
    auto vbfs1 = graph::views::vertices_breadth_first_search(g, seed);
    auto vbfs2 = graph::adj_list::views::vertices_breadth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(vbfs1) == std::ranges::distance(vbfs2));
  }

  SECTION("edges_breadth_first_search") {
    auto ebfs1 = graph::views::edges_breadth_first_search(g, seed);
    auto ebfs2 = graph::adj_list::views::edges_breadth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(ebfs1) == std::ranges::distance(ebfs2));
  }

  SECTION("sourced_edges_breadth_first_search") {
    auto sebfs1 = graph::views::sourced_edges_breadth_first_search(g, seed);
    auto sebfs2 = graph::adj_list::views::sourced_edges_breadth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(sebfs1) == std::ranges::distance(sebfs2));
  }
}

TEST_CASE("DFS views in adj_list::views namespace", "[dfs][views][namespace]") {
  // Create a simple graph with edges
  std::vector<std::vector<int>> g    = {{1, 2}, {2}, {}};
  vertex_id_t<decltype(g)>      seed = 0;

  SECTION("vertices_depth_first_search") {
    auto vdfs1 = graph::views::vertices_depth_first_search(g, seed);
    auto vdfs2 = graph::adj_list::views::vertices_depth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(vdfs1) == std::ranges::distance(vdfs2));
  }

  SECTION("edges_depth_first_search") {
    auto edfs1 = graph::views::edges_depth_first_search(g, seed);
    auto edfs2 = graph::adj_list::views::edges_depth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(edfs1) == std::ranges::distance(edfs2));
  }

  SECTION("sourced_edges_depth_first_search") {
    auto sedfs1 = graph::views::sourced_edges_depth_first_search(g, seed);
    auto sedfs2 = graph::adj_list::views::sourced_edges_depth_first_search(g, seed);

    // Both namespaces should produce same results
    REQUIRE(std::ranges::distance(sedfs1) == std::ranges::distance(sedfs2));
  }
}
