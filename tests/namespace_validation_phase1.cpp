#include <catch2/catch_test_macros.hpp>
#include "graph/graph.hpp"
#include "graph/edgelist.hpp"
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

    // Edge concepts exist in namespace (Task 1.2)
    // Note: Not all concepts apply to all graph types
    // std::vector<std::vector<int>> doesn't have source_id on edges
  }

  SECTION("edge_list namespace renamed (Task 1.4)") {
    using EL = std::vector<std::pair<int, int>>;
    
    // New namespace works
    static_assert(graph::edge_list::basic_sourced_edgelist<EL>);
    static_assert(graph::edge_list::basic_sourced_index_edgelist<EL>);
    
    // Old namespace still works (compatibility)
    static_assert(graph::edgelist::basic_sourced_edgelist<EL>);
    static_assert(graph::edgelist::basic_sourced_index_edgelist<EL>);
  }

  SECTION("Both namespaces have same behavior") {
    // Verify both evaluate the same
    constexpr bool new_ns = graph::adj_list::index_adjacency_list<G>;
    constexpr bool old_ns = graph::index_adjacency_list<G>;
    static_assert(new_ns == old_ns);
  }
}
