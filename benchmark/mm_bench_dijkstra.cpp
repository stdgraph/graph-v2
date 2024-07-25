#include <fmt/format.h>

#include "mm_load.hpp"

#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"
#include "graph/algorithm/experimental/visitor_dijkstra.hpp"

namespace fmm = fast_matrix_market;
using std::string_view;
using std::vector;
using std::tuple;

using std::ranges::size;
using std::cout;
using std::wcout;
using std::endl;

using namespace std::graph;
using namespace std::graph::experimental;

// Visitor class for visitor_dijkstra
template <class G, class Distances>
struct discover_vertex_visitor : dijkstra_visitor_base<G> {
  using base_t = dijkstra_visitor_base<G>;

  discover_vertex_visitor(G& g, Distances& distances) : base_t(g), distances_(distances) {}
  ~discover_vertex_visitor() = default;

  void on_discover_vertex(const base_t::vertex_desc_type& vdesc) {
    //auto&& [uid, u] = vdesc;
    ++vertices_discovered_;
    //if ((vertices_discovered_ & 0xff) == 0)
    //  fmt::print("\n{:L} vertices discovered", vertices_discovered_);
    if (vertices_discovered_ >= size(distances_) * 2)
	  throw std::runtime_error("Too many vertices discovered");
  }
  constexpr void on_edge_relaxed(const base_t::sourced_edge_desc_type& edesc) noexcept {
    ++edges_relaxed_;
    //if ((edges_relaxed_ & 0xff) == 0)
    //  fmt::print("\n{:L} edges relaxed", edges_relaxed_);
  }

  constexpr size_t vertices_discovered() const noexcept { return vertices_discovered_; }
  constexpr size_t edges_relaxed() const noexcept { return edges_relaxed_; }

private:
  Distances& distances_;
  size_t     vertices_discovered_ = 0;
  size_t     edges_relaxed_       = 0;
};


void bench_dijkstra() {
  triplet_matrix<int64_t, int64_t> triplet;
  array_matrix<int64_t>            sources;

  load_matrix_market(gap_road, triplet, sources, true); // gap_road, g2bench_bips98_606, g2bench_chesapeake

  using G = vector<vector<tuple<int64_t, int64_t>>>;
  G g;
  load_graph(triplet, g);

  // Use first source only (temporary)
  sources.ncols = 1;
  sources.nrows = 1;
  sources.vals.erase(sources.vals.begin() + 1, sources.vals.end());

  fmt::println("{} source(s) will be used", sources.nrows);
  fmt::println("");

  auto run_co_dijkstra = [&g, &sources]() {
    std::string desc = fmt::format("Running {} with event(s) {} and using {} source(s)", "co_dijkstra",
                                   "discover_vertex", sources.nrows);
    timer       run_time(desc, true);

    using Distance     = int64_t;
    using Distances    = std::vector<Distance>;
    using Predecessors = std::vector<vertex_id_t<G>>;

    Distances    distances(size(vertices(g)));
    Predecessors predecessors(size(vertices(g)));
    init_shortest_paths(distances, predecessors);

    auto   distance_fnc        = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
    size_t vertices_discovered = 0;
    for (auto bfs =
               co_dijkstra(g, dijkstra_events::discover_vertex, sources.vals, predecessors, distances, distance_fnc);
         bfs;) {
      auto&& [event, payload] = bfs();
      switch (event) {
      case dijkstra_events::discover_vertex: {
        auto&& [uid, u] = get<bfs_vertex_value_t<G>>(payload); // or get<1>(payload);
        ++vertices_discovered;
      } break;
      default: break;
      }
    }
    run_time.set_count(vertices_discovered, "vertice(s) discovered"); // includes sources
  };

  auto run_visitor_dijkstra = [&g, &sources]() {
    std::string desc   = fmt::format("Running {} with event(s) {} and using {} source(s)", "visitor_dijkstra",
                                     "discover_vertex", sources.nrows);
    using Distance     = int64_t;
    using Distances    = std::vector<Distance>;
    using Predecessors = std::vector<vertex_id_t<G>>;

    Distances    distances(size(vertices(g)));
    Predecessors predecessors(size(vertices(g)));
    size_t       reported_vertices_discovered = 0;
    {
      timer run_time(desc, true);

      init_shortest_paths(distances, predecessors);

      auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
      discover_vertex_visitor visitor(g, distances);
      dijkstra_with_visitor(g, visitor, sources.vals, predecessors, distances, distance_fnc);

      reported_vertices_discovered = visitor.vertices_discovered();
      run_time.set_count(visitor.vertices_discovered(), "vertice(s) discovered"); // includes sources
    }

    // A vertex may actually visited more than once for ragged graphs, where the same vertex is visited from different
    // sources that have a shorter path to the vertex.
    {
      size_t actual_vertices_discovered = 0;
      for (auto d : distances) {
        if (d != shortest_path_invalid_distance<Distance>())
          ++actual_vertices_discovered;
      }

      if (actual_vertices_discovered != reported_vertices_discovered)
        fmt::println("Warning: actual vertices discovered ({:L}) does not match reported vertices discovered ({:L})",
                     actual_vertices_discovered, reported_vertices_discovered);
      fmt::println("{:1.1f}% of all vertices were visited", //
                   100.0 * static_cast<double>(actual_vertices_discovered) / static_cast<double>(num_vertices(g)));
    }
  };

  // Run the algorithm
  try {
    {
      run_visitor_dijkstra();
      //run_co_dijkstra();
    }
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }

  fmt::println("");
}
