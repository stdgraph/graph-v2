//#define ENABLE_POP_COUNT 1
//#define ENABLE_EDGE_VISITED_COUNT 1

#include <fmt/format.h> // for outputting everything else
#include <format>       // for outputting current timestamp

#include "mm_load.hpp"

#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"
#include "graph/algorithm/experimental/visitor_dijkstra.hpp"
#include "nwgraph_dijkstra.hpp"
#include <algorithm>

namespace fmm = fast_matrix_market;
using std::string_view;
using std::vector;
using std::tuple;

using std::ranges::size;
using std::cout;
using std::wcout;
using std::endl;

using fmt::print;
using fmt::println;

using namespace std::graph;
using namespace std::graph::experimental;

//-------------------------------------------------------------------------------------------------
// bench_visitor_dijkstra
//

// Visitor class for visitor_dijkstra
template <class G, class Distances>
struct discover_vertex_visitor : dijkstra_visitor_base<G> {
  using base_t = dijkstra_visitor_base<G>;

  discover_vertex_visitor(G& g, Distances& distances) : base_t(g), distances_(distances) {}
  ~discover_vertex_visitor() = default;

#if 0
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
#endif

  constexpr size_t vertices_discovered() const noexcept { return vertices_discovered_; }
  constexpr size_t edges_relaxed() const noexcept { return edges_relaxed_; }

private:
  Distances& distances_;
  size_t     vertices_discovered_ = 0;
  size_t     edges_relaxed_       = 0;
};

std::string current_timestamp() {
  auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
  return std::format("{:%Y-%m-%d %X}", time); // std::format required for msvc
}

template <class G, class DistancesFnc>
auto bench_visitor_dijkstra(G& g, const array_matrix<int64_t>& sources, DistancesFnc& distance_fnc) {
  std::string desc =
        fmt::format("Running {} with event(s) {} and using {} source(s)", "visitor_dijkstra", "none", sources.nrows);
  using Distance     = int64_t;
  using Distances    = std::vector<Distance>;
  using Predecessors = std::vector<vertex_id_t<G>>;

  Distances    distances(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  size_t       reported_vertices_discovered = 0;
  {
    timer run_time(desc, true);

    init_shortest_paths(distances, predecessors);

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


//-------------------------------------------------------------------------------------------------
// bench_co_dijkstra
//
template <class G, class DistancesFnc>
auto bench_co_dijkstra(G& g, const array_matrix<int64_t>& sources, DistancesFnc& distance_fnc) {
  std::string desc = fmt::format("Running {} with event(s) {} and using {} source(s)", "co_dijkstra", "discover_vertex",
                                 sources.nrows);
  timer       run_time(desc, true);

  using Distance     = int64_t;
  using Distances    = std::vector<Distance>;
  using Predecessors = std::vector<vertex_id_t<G>>;

  Distances    distances(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  init_shortest_paths(distances, predecessors);

  // Don't worry about events for nowc
  size_t vertices_discovered = 0;
#if 1
  for (auto bfs = co_dijkstra(g, dijkstra_events::none, sources.vals, predecessors, distances, distance_fnc); bfs;
       bfs())
    ;
#else
  for (auto bfs = co_dijkstra(g, dijkstra_events::discover_vertex, sources.vals, predecessors, distances, distance_fnc);
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
#endif
  run_time.set_count(vertices_discovered, "vertice(s) discovered"); // includes sources
};

//-------------------------------------------------------------------------------------------------
// bench_nwgraph_dijkstra
//
template <class G, class DistancesFnc>
auto bench_nwgraph_dijkstra(G& g, const array_matrix<int64_t>& sources, DistancesFnc& distance_fnc) {
  std::string desc   = fmt::format("Running {} using {} source(s)", "nwgraph_dijkstra", sources.nrows);
  using Distance     = int64_t;
  using Distances    = std::vector<Distance>;
  //using Predecessors = std::vector<vertex_id_t<G>>;

  //auto distance_fnc = [&g](std::tuple<int64_t, int64_t>& uv) { return std::get<1>(uv); };


  Distances distances;
  //Predecessors predecessors(size(vertices(g)));
  //size_t       reported_vertices_discovered = 0;
  {
    timer run_time(desc, true);

    //init_shortest_paths(distances, predecessors);

    //discover_vertex_visitor visitor(g, distances);
    //dijkstra_with_visitor(g, visitor, sources.vals, predecessors, distances, distance_fnc);
    distances = nwgraph_dijkstra(g, static_cast<vertex_id_t<G>>(sources.vals[0]), distance_fnc);

    //reported_vertices_discovered = visitor.vertices_discovered();
    //run_time.set_count(visitor.vertices_discovered(), "vertice(s) discovered"); // includes sources
  }

  // A vertex may actually visited more than once for ragged graphs, where the same vertex is visited from different
  // sources that have a shorter path to the vertex.
  {
    size_t actual_vertices_discovered = 0;
    for (auto d : distances) {
      if (d != shortest_path_invalid_distance<Distance>())
        ++actual_vertices_discovered;
    }

    fmt::println("Vertices discovered was ({:L})", actual_vertices_discovered);
    fmt::println("{:1.1f}% of all vertices were visited", //
                 100.0 * static_cast<double>(actual_vertices_discovered) / static_cast<double>(num_vertices(g)));
  }
};


//-------------------------------------------------------------------------------------------------
// bench_dijkstra_main
//
void bench_dijkstra_main() {
  using G = vector<vector<tuple<int64_t, int64_t>>>;

  fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
  fmt::println("================================================================");
  fmt::println("Benchmark starting at {}", current_timestamp());
  cout << endl; // endl forces buffer flush; needed for Linux

  bench_files                      bench_source = gap_road; // gap_road, g2bench_bips98_606, g2bench_chesapeake
  triplet_matrix<int64_t, int64_t> triplet;
  array_matrix<int64_t>            sources;

  bool const requires_sort = !std_adjacency_graph<G>;
  load_matrix_market(bench_source, triplet, sources, requires_sort); // gap_road, g2bench_bips98_606, g2bench_chesapeake
  cout << endl;

  G           g;
  graph_stats stats = load_graph(triplet, g);
  fmt::println("Graph stats: {}", stats);
  cout << endl;

  // Use first source only (temporary)
  sources.ncols = 1;
  sources.nrows = 1;
  sources.vals.erase(sources.vals.begin() + 1, sources.vals.end());

  fmt::println("{:L} source(s) will be used", sources.nrows);
  cout << endl;

#if 1
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
  println("Edge weight function = edge_value(g, uv)");
#else
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return 1; };
  println("Edge weight function = 1");
#endif

  // Run the algorithm
  try {
    {
      bench_nwgraph_dijkstra(g, sources, distance_fnc);
      cout << endl;
      bench_visitor_dijkstra(g, sources, distance_fnc);
      cout << endl;
      bench_co_dijkstra(g, sources, distance_fnc);
    }
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }

  cout << endl;
}
