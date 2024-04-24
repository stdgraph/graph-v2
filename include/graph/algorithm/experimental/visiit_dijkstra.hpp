#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/experimental/co_cmn.hpp"

#include <variant>
#include <queue>
#include <algorithm>
#include <ranges>

namespace std::graph::experimental {

// Design Considerations:
// 1. The visitor should be a template parameter to the algorithm.


template <adjacency_list G>
class dijkstra_visitor {
  // Types
public:
  using graph_type        = G;
  using vertex_type       = vertex_descriptor<vertex_id_t<G>, vertex_reference_t<G>, void>;
  using sourced_edge_type = edge_descriptor<vertex_id_t<G>, true, edge_reference_t<G>, void>;

  // Construction, Destruction, Assignment
public:
  dijkstra_visitor(graph_type& g) : g_(forward<graph_type>(g)) {}

  dijkstra_visitor()                        = delete;
  dijkstra_visitor(const dijkstra_visitor&) = default;
  dijkstra_visitor(dijkstra_visitor&&)      = default;
  ~dijkstra_visitor()                       = default;

  dijkstra_visitor& operator=(const dijkstra_visitor&) = default;
  dijkstra_visitor& operator=(dijkstra_visitor&&)      = default;

  // Property Functions
public:
  graph_type& graph() const noexcept { return g_; }  

  // Visitor Functions
public:
  // vertex visitor functions
  constexpr void on_initialize_vertex(vertex_type& vdesc) noexcept {}
  constexpr void on_discover_vertex(vertex_type& vdesc) noexcept {}
  constexpr void on_examine_vertex(vertex_type& vdesc) noexcept {}
  constexpr void on_finish_vertex(vertex_type& vdesc) noexcept {}

  // edge visitor functions
  constexpr void on_examine_edge(sourced_edge_type& edesc) noexcept {}
  constexpr void on_edge_relaxed(sourced_edge_type& edesc) noexcept {}
  constexpr void on_edge_not_relaxed(sourced_edge_type& edesc) noexcept {}

  // Data Members
private:
  reference_wrapper<graph_type> g_;
};

} // namespace std::graph::experimental
