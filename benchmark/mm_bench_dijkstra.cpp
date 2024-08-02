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

#define ENABLE_DISCOVER_VERTEX 1
#define ENABLE_EDGE_RELAXED 1

constexpr const size_t test_trials = 3;

struct bench_results {
  size_t vertices_discovered = 0;
  size_t edges_relaxed       = 0;
};

std::string current_timestamp() {
  auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
  return std::format("{:%Y-%m-%d %X}", time); // std::format required for msvc
}

template <typename Distance>
void output_vertices_visited(std::vector<Distance>& distances) {
  size_t visited = std::accumulate(distances.begin(), distances.end(), 0, [](size_t count, auto& dist) {
    return dist != shortest_path_invalid_distance<Distance>() ? count + 1 : count;
  });
  fmt::println("{:L} vertices were actually visited", visited);
}


//-------------------------------------------------------------------------------------------------
// bench_visitor_dijkstra
//

// Visitor class for visitor_dijkstra
template <class G, class Distances>
struct discover_vertex_visitor : dijkstra_visitor_base<G> {
  using base_t = dijkstra_visitor_base<G>;

  discover_vertex_visitor(G& g, bench_results& results) : base_t(g), results_(results) {}
  ~discover_vertex_visitor() = default;

#if ENABLE_DISCOVER_VERTEX
  void on_discover_vertex(const base_t::vertex_desc_type& vdesc) { results_.vertices_discovered += 1; }
#endif
#if ENABLE_EDGE_RELAXED
  constexpr void on_edge_relaxed(const base_t::sourced_edge_desc_type& edesc) noexcept { results_.edges_relaxed += 1; }
#endif

private:
  bench_results& results_;
};

#if 0
template <class G, class DistancesFnc>
auto bench_visitor_dijkstra(G&                           g,
                            const array_matrix<int64_t>& sources,
                            DistancesFnc&                distance_fnc,
                            bench_results&               results) {
  std::string desc = fmt::format("Running {}", "visitor_dijkstra");

  double min_elapsed         = std::numeric_limits<double>::max();
  size_t vertices_discovered = 0;
  size_t edges_relaxed       = 0;

  using Distance     = int64_t;
  using Distances    = std::vector<Distance>;
  using Predecessors = std::vector<vertex_id_t<G>>;

  Distances    distances(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));
  size_t       reported_vertices_discovered = 0;

  for (size_t t = 0; t < test_trials; ++t) {
    init_shortest_paths(distances, predecessors);
    discover_vertex_visitor visitor(g, results);
    {
      dijkstra_with_visitor(g, visitor, sources.vals, predecessors, distances, distance_fnc);

      reported_vertices_discovered = visitor.vertices_discovered();
      //run_time.set_count(visitor.vertices_discovered(), "vertice(s) discovered"); // includes sources
    }
  }

  output_vertices_visited(distances);
#  if ENABLE_DISCOVER_VERTEX
  fmt::println("{:L} vertice(s) were discovered", visitor.vertices_discovered()); // includes sources
#  endif
#  if ENABLE_EDGE_RELAXED
  fmt::println("{:L} edge(s) were relaxed", visitor.edges_relaxed());
#  endif
};


//-------------------------------------------------------------------------------------------------
// bench_co_dijkstra
//
template <class G, class DistancesFnc>
auto bench_co_dijkstra(G& g, const array_matrix<int64_t>& sources, DistancesFnc& distance_fnc) {
  std::string desc = fmt::format("Running {}", "co_dijkstra");

  size_t          vertices_discovered = 0;
  size_t          edges_relaxed       = 0;
  dijkstra_events events              = dijkstra_events::none;
#  if ENABLE_DISCOVER_VERTEX
  events |= dijkstra_events::discover_vertex;
#  endif
#  if ENABLE_EDGE_RELAXED
  events |= dijkstra_events::edge_relaxed;
#  endif

  using Distance     = int64_t;
  using Distances    = std::vector<Distance>;
  using Predecessors = std::vector<vertex_id_t<G>>;

  Distances    distances(size(vertices(g)));
  Predecessors predecessors(size(vertices(g)));

  init_shortest_paths(distances, predecessors);

  {
    timer run_time(desc, true);
    for (auto bfs = co_dijkstra(g, events, sources.vals, predecessors, distances, distance_fnc); bfs;) {
      auto&& [event, payload] = bfs();
      switch (event) {
#  if ENABLE_DISCOVER_VERTEX
      case dijkstra_events::discover_vertex: {
        auto&& [uid, u] = get<bfs_vertex_value_t<G>>(payload); // or get<1>(payload);
        ++vertices_discovered;
      } break;
#  endif
#  if ENABLE_EDGE_RELAXED
      case dijkstra_events::edge_relaxed: {
        auto&& [uid, vid, uv] = get<bfs_edge_value_t<G>>(payload); // or get<1>(payload);
        ++edges_relaxed;
      } break;
#  endif
      default: break;
      }
    }
  }

  output_vertices_visited(distances);
#  if ENABLE_DISCOVER_VERTEX
  fmt::println("{:L} vertice(s) were discovered", vertices_discovered); // includes sources
#  endif
#  if ENABLE_EDGE_RELAXED
  fmt::println("{:L} edge(s) were relaxed", edges_relaxed);
#  endif
};

//-------------------------------------------------------------------------------------------------
// bench_nwgraph_dijkstra
//
template <class G, class DistancesFnc>
auto bench_nwgraph_dijkstra(G& g, const array_matrix<int64_t>& sources, DistancesFnc& distance_fnc) {
  std::string desc = fmt::format("Running {}", "nwgraph_dijkstra");
  using Distance   = int64_t;
  using Distances  = std::vector<Distance>;
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
  cout << endl;                                             // endl forces buffer flush; needed for Linux

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

#  if 1
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
  println("Edge weight function = edge_value(g, uv)");
#  else
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return 1; };
  println("Edge weight function = 1");
#  endif
  cout << endl;


  // Run the algorithm
  try {
    {
      //bench_nwgraph_dijkstra(g, sources, distance_fnc);
      //cout << endl;
      bench_visitor_dijkstra(g, sources, distance_fnc);
      cout << endl;
      bench_co_dijkstra(g, sources, distance_fnc);
    }
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }

  cout << endl;
}
#endif // 0

//-------------------------------------------------------------------------------------------------
// bench_dijkstra_runner
//
void bench_dijkstra_runner() {
  using vertex_id_type = int64_t;
  using Distance       = int64_t;
  using G              = vector<vector<tuple<vertex_id_type, Distance>>>;
  using Distances      = std::vector<Distance>;
  using Predecessors   = std::vector<vertex_id_type>;
  using SourceIds      = vector<vertex_id_type>;

  bench_files bench_source = gap_road; // gap_road, g2bench_bips98_606, g2bench_chesapeake
  triplet_matrix<vertex_id_type, vertex_id_type> triplet;
  array_matrix<vertex_id_type>                   sources;

  // Read the Matrix Market file
  bool const requires_sort = !std_adjacency_graph<G>;
  load_matrix_market(bench_source, triplet, sources, requires_sort); // gap_road, g2bench_bips98_606, g2bench_chesapeake
  cout << endl;

  // Load the graph
  G           g;
  graph_stats stats = load_graph(triplet, g);
  fmt::println("Graph stats: {}", stats);
  cout << endl;

  Distances     distances(size(vertices(g)));
  Predecessors  predecessors(size(vertices(g)));
  bench_results results;

#if 1
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
  println("Edge weight function = edge_value(g, uv)");
#else
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return 1; };
  println("Edge weight function = 1");
#endif
  cout << endl;

  std::string events;
#if ENABLE_DISCOVER_VERTEX
  events += "discover_vertex ";
#endif
#if ENABLE_EDGE_RELAXED
  events += "edge_relaxed ";
#endif
  if (events.empty())
    events = "(none)";

  struct dijkstra_algo {
    std::string                           name;
    std::function<void(const SourceIds&)> run;
    std::vector<double>                   elapsed; // seconds for each source
  };
  std::vector<dijkstra_algo> algos;

  // Add the algorithms
  algos.emplace_back(dijkstra_algo{"visitor_dijkstra",
                                   [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& sources) {
                                     discover_vertex_visitor<G, Distances> visitor(g, results);
                                     dijkstra_with_visitor(g, visitor, sources, predecessors, distances, distance_fnc);
                                   }});

  algos.emplace_back(dijkstra_algo{
        "co_dijkstra", [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& sources) {
          dijkstra_events events = dijkstra_events::none;
#if ENABLE_DISCOVER_VERTEX
          events |= dijkstra_events::discover_vertex;
#endif
#if ENABLE_EDGE_RELAXED
          events |= dijkstra_events::edge_relaxed;
#endif
          for (auto bfs = co_dijkstra(g, events, sources, predecessors, distances, distance_fnc); bfs;) {
            auto&& [event, payload] = bfs();
            switch (event) {
#if ENABLE_DISCOVER_VERTEX
            case dijkstra_events::discover_vertex: {
              auto&& [uid, u] = get<bfs_vertex_value_t<G>>(payload); // or get<1>(payload);
              results.vertices_discovered += 1;
            } break;
#endif
#if ENABLE_EDGE_RELAXED
            case dijkstra_events::edge_relaxed: {
              auto&& [uid, vid, uv] = get<bfs_edge_value_t<G>>(payload); // or get<1>(payload);
              results.edges_relaxed += 1;
            } break;
#endif
            default: break;
            }
          }
        }});

  //algos.emplace_back(dijkstra_algo{"nwgraph_dijkstra", [&g, &sources, &distance_fnc, &distances, &predecessors,
  //                                                      &results](const SourceIds& sources) {
  //                                   auto returned_distances =
  //                                         nwgraph_dijkstra(g, static_cast<vertex_id_t<G>>(sources[0]), distance_fnc);
  //                                 }});

  size_t name_max = 0;
  for (auto& algo : algos)
    name_max = std::max(name_max, size(algo.name));

  // Run the algorithm
  try {
    fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
    fmt::println("Running tests on {} sources individually", size(sources.vals));
    fmt::println("{} tests are run for each source and the minimum is taken", test_trials);
    fmt::println("Events included: {}", events);
    fmt::println("================================================================");
    fmt::println("Benchmark starting at {}\n", current_timestamp());

    fmt::print("{:{}}  {:^11}  {:5}", "Algorithm", name_max, "Source", "Elapsed (s)");
#if ENABLE_DISCOVER_VERTEX
    fmt::print("  Vertices Discovered");
#endif
#if ENABLE_EDGE_RELAXED
    fmt::print("  Edges Relaxed");
#endif
    cout << endl;

    for (dijkstra_algo& algo : algos) {

      for (size_t s = 0; s < sources.vals.size(); ++s) {
        const SourceIds one_source  = {sources.vals[s]};
        double          min_elapsed = std::numeric_limits<double>::max(); // seconds
        //fmt::println("Source vertex: {}", source);

        for (size_t t = 0; t < test_trials; ++t) {
          results = {};
          init_shortest_paths(distances, predecessors);
          simple_timer run_time;
          algo.run(one_source);
          min_elapsed = std::min(min_elapsed, run_time.elapsed());
        }
        algo.elapsed.push_back(min_elapsed);
        print("{:<{}}  {:>11L}  {:>11.3f}", algo.name, name_max, one_source[0], min_elapsed);
#if ENABLE_DISCOVER_VERTEX
        print("  {:>19L}", results.vertices_discovered);
#endif
#if ENABLE_EDGE_RELAXED
        print("  {:>13L}", results.edges_relaxed);
#endif
        cout << endl;
      }
    }
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }
}
