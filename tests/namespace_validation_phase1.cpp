#include <catch2/catch_test_macros.hpp>
#include "graph/graph.hpp"
#include <vector>

TEST_CASE("Phase 1: Namespace structure exists", "[namespace][phase1]") {
  using G = std::vector<std::vector<int>>;

  SECTION("adj_list namespace concepts work") {
    static_assert(graph::adj_list::adjacency_list<G>);
    static_assert(graph::adj_list::index_adjacency_list<G>);
    static_assert(graph::adj_list::basic_adjacency_list<G>);
    static_assert(graph::adj_list::basic_index_adjacency_list<G>);
    static_assert(graph::adj_list::vertex_range<G>);
    static_assert(graph::adj_list::index_vertex_range<G>);
    static_assert(graph::adj_list::targeted_edge_range<G>);
    static_assert(graph::adj_list::basic_targeted_edge_range<G>);
    static_assert(graph::adj_list::targeted_edge<G>);
    static_assert(graph::adj_list::basic_targeted_edge<G>);

    // Edge concepts (Task 1.2)
    static_assert(graph::adj_list::basic_sourced_edge<G>);
    static_assert(graph::adj_list::basic_sourced_targeted_edge<G>);
    static_assert(graph::adj_list::sourced_edge<G>);
    static_assert(graph::adj_list::sourced_targeted_edge<G>);
    static_assert(graph::targeted_edge<G>);
    static_assert(graph::basic_targeted_edge<G>);
  }

  SECTION("Both namespaces have same behavior") {
    // Verify both evaluate the same
    constexpr bool new_ns = graph::adj_list::index_adjacency_list<G>;
    constexpr bool old_ns = graph::index_adjacency_list<G>;
    static_assert(new_ns == old_ns);
  }
}
