#include <graph/graph.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

using std::vector;
using namespace graph;

TEST_CASE("Type aliases in adj_list namespace", "[namespace][phase3][task3.2]") {
  using G = vector<vector<int>>;

  SECTION("Vertex type aliases") {
    // New namespace
    using vrange_t_new = graph::adj_list::vertex_range_t<G>;
    using viter_t_new  = graph::adj_list::vertex_iterator_t<G>;
    using v_t_new      = graph::adj_list::vertex_t<G>;
    using vref_t_new   = graph::adj_list::vertex_reference_t<G>;
    using vid_t_new    = graph::adj_list::vertex_id_t<G>;

    // Old namespace (compatibility)
    using vrange_t_old = graph::vertex_range_t<G>;
    using viter_t_old  = graph::vertex_iterator_t<G>;
    using v_t_old      = graph::vertex_t<G>;
    using vref_t_old   = graph::vertex_reference_t<G>;
    using vid_t_old    = graph::vertex_id_t<G>;

    // Verify they're the same types
    static_assert(std::is_same_v<vrange_t_new, vrange_t_old>);
    static_assert(std::is_same_v<viter_t_new, viter_t_old>);
    static_assert(std::is_same_v<v_t_new, v_t_old>);
    static_assert(std::is_same_v<vref_t_new, vref_t_old>);
    static_assert(std::is_same_v<vid_t_new, vid_t_old>);
  }

  SECTION("Edge type aliases") {
    // New namespace
    using erange_t_new = graph::adj_list::vertex_edge_range_t<G>;
    using eiter_t_new  = graph::adj_list::vertex_edge_iterator_t<G>;
    using e_t_new      = graph::adj_list::edge_t<G>;
    using eref_t_new   = graph::adj_list::edge_reference_t<G>;

    // Old namespace (compatibility)
    using erange_t_old = graph::vertex_edge_range_t<G>;
    using eiter_t_old  = graph::vertex_edge_iterator_t<G>;
    using e_t_old      = graph::edge_t<G>;
    using eref_t_old   = graph::edge_reference_t<G>;

    // Verify they're the same types
    static_assert(std::is_same_v<erange_t_new, erange_t_old>);
    static_assert(std::is_same_v<eiter_t_new, eiter_t_old>);
    static_assert(std::is_same_v<e_t_new, e_t_old>);
    static_assert(std::is_same_v<eref_t_new, eref_t_old>);
  }

  SECTION("Partition and value type aliases") {
    // Just verify the aliases exist and are the same in both namespaces
    // We can't test specific types without a concrete graph type with values
    using G = vector<vector<int>>;

    // Verify partition_id_t aliases match
    using pid_t_new = graph::adj_list::partition_id_t<G>;
    using pid_t_old = graph::partition_id_t<G>;
    static_assert(std::is_same_v<pid_t_new, pid_t_old>);
  }

  SECTION("Type aliases work with actual graph") {
    G g = {{1, 2, 3}, {4, 5}, {6, 7, 8, 9}};

    // Verify both namespaces produce same vertex range
    auto vrange_new = graph::adj_list::vertices(g);
    auto vrange_old = graph::vertices(g);
    REQUIRE(std::ranges::size(vrange_new) == 3);
    REQUIRE(std::ranges::size(vrange_old) == 3);

    // Get iterators from the new namespace
    using viter_t_new = graph::adj_list::vertex_iterator_t<G>;
    viter_t_new it    = std::ranges::begin(vrange_new);

    // Compute vertex_id using new namespace
    graph::adj_list::vertex_id_t<G> vid_new = graph::adj_list::vertex_id(g, it);

    // Compute vertex_id using old namespace (should work with same iterator)
    graph::vertex_id_t<G> vid_old = graph::vertex_id(g, it);

    // Both should give same result for same iterator
    REQUIRE(vid_new == vid_old);
  }
}
