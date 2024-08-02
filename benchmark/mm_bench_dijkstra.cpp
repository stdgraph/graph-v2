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

//#define ENABLE_DISCOVER_VERTEX 1
//#define ENABLE_EXAMINE_VERTEX 1
//#define ENABLE_EDGE_RELAXED 1

constexpr const size_t test_trials = 3;

struct bench_results {
  size_t vertices_discovered = 0;
  size_t vertices_examined   = 0;
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
#if ENABLE_EXAMINE_VERTEX
  constexpr void on_examine_vertex(const base_t::vertex_desc_type& vdesc) { results_.vertices_examined += 1; }
#endif
#if ENABLE_EXAMINE_VERTEX
#endif
#if ENABLE_EDGE_RELAXED
  constexpr void on_edge_relaxed(const base_t::sourced_edge_desc_type& edesc) noexcept { results_.edges_relaxed += 1; }
#endif

private:
  bench_results& results_;
};


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
#if ENABLE_EXAMINE_VERTEX
  events += "examine_vertex ";
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

    double total_elapsed() const { return std::accumulate(begin(elapsed), end(elapsed), 0.0); }
    double average_elapsed() const { return total_elapsed() / size(elapsed); }
  };
  std::vector<dijkstra_algo> algos;

  // Add the algorithms
  algos.emplace_back(dijkstra_algo{"nwgraph_dijkstra",
                                   [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& sources) {
                                     auto returned_distances = nwgraph_dijkstra(g, sources, distance_fnc);
                                   }});

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
#if ENABLE_EXAMINE_VERTEX
          events |= dijkstra_events::examine_vertex;
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
#if ENABLE_EXAMINE_VERTEX
            case dijkstra_events::examine_vertex: {
              auto&& [uid, u] = get<bfs_vertex_value_t<G>>(payload); // or get<1>(payload);
              results.vertices_examined += 1;
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

  size_t name_width = 0;
  for (auto& algo : algos)
    name_width = std::max(name_width, size(algo.name));

  // Run the algorithm on all sources
  try {
    fmt::println("================================================================");
    fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
    fmt::println("Running tests on all {} sources using {} algorithms", size(sources.vals), size(algos));
    fmt::println("{} tests are run and the minimum is taken", test_trials);
    fmt::println("Events included: {}", events);
    fmt::println("Benchmark starting at {}\n", current_timestamp());

    fmt::print("{:3}  {:{}}  {:5}", "Obs", "Algorithm", name_width, "Elapsed (s)");
#if ENABLE_DISCOVER_VERTEX
    fmt::print("  Vertices Discovered");
#endif
#if ENABLE_EXAMINE_VERTEX
    fmt::print("  Vertices Examined");
#endif
#if ENABLE_EDGE_RELAXED
    fmt::print("  Edges Relaxed");
#endif
    cout << endl;

    size_t observation = 0;
    for (dijkstra_algo& algo : algos) {

      double min_elapsed = std::numeric_limits<double>::max(); // seconds

      for (size_t t = 0; t < test_trials; ++t) {
        results = {};
        init_shortest_paths(distances, predecessors);
        simple_timer run_time;
        algo.run(sources.vals);
        min_elapsed = std::min(min_elapsed, run_time.elapsed());
      }
      algo.elapsed.push_back(min_elapsed);
      print("{:3}  {:<{}}  {:>11.3f}", ++observation, algo.name, name_width, min_elapsed);
#if ENABLE_DISCOVER_VERTEX
      print("  {:>19L}", results.vertices_discovered);
#endif
#if ENABLE_EXAMINE_VERTEX
      print("  {:>17L}", results.vertices_examined);
#endif
#if ENABLE_EDGE_RELAXED
      print("  {:>13L}", results.edges_relaxed);
#endif
      cout << endl;
    }
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }
  cout << endl << endl;

  // Run the algorithm for each source
  try {
    fmt::println("================================================================");
    fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
    fmt::println("Running tests on {} sources individually using {} algorithms", size(sources.vals), size(algos));
    fmt::println("{} tests are run for each source and the minimum is taken", test_trials);
    fmt::println("Events included: {}", events);
    fmt::println("Benchmark starting at {}\n", current_timestamp());

    fmt::print("{:3}  {:{}}  {:^11}  {:5}", "Obs", "Algorithm", name_width, "Source", "Elapsed (s)");
#if ENABLE_DISCOVER_VERTEX
    fmt::print("  Vertices Discovered");
#endif
#if ENABLE_EXAMINE_VERTEX
    fmt::print("  Vertices Examined");
#endif
#if ENABLE_EDGE_RELAXED
    fmt::print("  Edges Relaxed");
#endif
    cout << endl;

    size_t observation = 0;
    for (dijkstra_algo& algo : algos) {

      for (size_t s = 0; s < sources.vals.size(); ++s) {
        const SourceIds one_source  = {sources.vals[s]};
        double          min_elapsed = std::numeric_limits<double>::max(); // seconds

        for (size_t t = 0; t < test_trials; ++t) {
          results = {};
          init_shortest_paths(distances, predecessors);
          simple_timer run_time;
          algo.run(one_source);
          min_elapsed = std::min(min_elapsed, run_time.elapsed());
        }
        algo.elapsed.push_back(min_elapsed);
        print("{:3}  {:<{}}  {:>11L}  {:>11.3f}", ++observation, algo.name, name_width, one_source[0], min_elapsed);
#if ENABLE_DISCOVER_VERTEX
        print("  {:>19L}", results.vertices_discovered);
#endif
#if ENABLE_EXAMINE_VERTEX
        print("  {:>17L}", results.vertices_examined);
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
