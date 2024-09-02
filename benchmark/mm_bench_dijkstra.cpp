#include <cstddef>
// Troublshooting values
//#define ENABLE_POP_COUNT
//#define ENABLE_EDGE_VISITED_COUNT

#define ENABLE_PREDECESSORS
#define ENABLE_INLINE_RELAX_TARGET 0 // values 0 or 1 (do not disable)
//#define ENABLE_EDGE_WEIGHT_ONE

// Optional events
//#define ENABLE_DISCOVER_VERTEX 1
//#define ENABLE_EXAMINE_VERTEX 1
//#define ENABLE_EDGE_RELAXED 1

// Number of trials to run to get the minimum time
constexpr const size_t test_trials = 6;


#include <fmt/format.h> // for outputting everything else
#include <format>       // for outputting current timestamp

#include "mm_load.hpp"

#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"
#include "graph/algorithm/experimental/visitor_dijkstra.hpp"
#include "nwgraph_dijkstra.hpp"
#include <algorithm>

#ifdef _MSC_VER
#  define NOMINMAX
#  include <Windows.h>
#endif

namespace fmm = fast_matrix_market;
using std::string_view;
using std::string;
using std::vector;
using std::tuple;

using std::ranges::size;
using std::cout;
using std::wcout;
using std::endl;

using fmt::print;
using fmt::println;

using namespace graph;
using namespace graph::experimental;

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
size_t vertices_visited(const std::vector<Distance>& distances) {
  size_t visited = std::accumulate(distances.begin(), distances.end(), 0ULL, [](size_t count, Distance dist) {
    return (dist != shortest_path_infinite_distance<Distance>()) ? count + 1 : count;
  });
  //fmt::println("{:L} vertices were actually visited", visited);
  return visited;
}


//-------------------------------------------------------------------------------------------------
// bench_visitor_dijkstra
//

// Visitor class for visitor_dijkstra
template <class G, class Distances>
struct discover_vertex_visitor : dijkstra_visitor_base<G> {
  using base_t = dijkstra_visitor_base<G>;

  discover_vertex_visitor(G&, bench_results& results) : base_t(), results_(results) {}
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

using vertex_id_type = int64_t;
using SourceIds      = vector<vertex_id_type>;

struct dijkstra_algo {
  std::string                           name;
  std::function<void(const SourceIds&)> run;
  std::vector<double>                   elapsed     = {};  // seconds for each source
  double                                all_elapsed = 0.0; // seconds for all sources

  double total_elapsed() const { return std::accumulate(begin(elapsed), end(elapsed), 0.0); }
  double average_elapsed() const { return total_elapsed() / static_cast<double>(size(elapsed)); }
};
std::vector<dijkstra_algo> algos;


class output_algo_results {

public:
  output_algo_results(std::vector<dijkstra_algo>& algorithms) : algos_(algorithms) {}

  void output_all_summary() {
    auto totalSecFnc = [](const dijkstra_algo& algo) { return algo.all_elapsed; };
    auto avgSecFnc   = [](const dijkstra_algo& algo) { return algo.all_elapsed; };
    do_output(totalSecFnc, avgSecFnc);
  }
  void output_detail_summary() {
    auto totalSecFnc = [](const dijkstra_algo& algo) { return algo.total_elapsed(); };
    auto avgSecFnc   = [](const dijkstra_algo& algo) { return algo.average_elapsed(); };
    do_output(totalSecFnc, avgSecFnc);
  }

private:
  template <typename TotalSecFnc, typename AvgSecFnc>
  void do_output(const TotalSecFnc totalSecFnc, const AvgSecFnc& avgSecFnc) {
    const size_t         row_cnt    = 5;
    vector<string>       row_labels = {"Dijkstra:", "Total (sec)", "Average (sec)", "% of Previous", "% of Fastest"};
    const size_t         col_cnt    = size(algos_) + 1;
    const vector<string> col_header_formats = {"{:<{}}", "{:>11}", "{:>11}", "{:>11}", "{:>11}"};
    const string         sec_fmt            = "{:>11.3f} "; // space for '%' gutter
    const string         pct_fmt            = "{:>11.2f}%";
    vector<size_t>       col_widths(col_cnt);
    vector<string>       col_labels;

    vector<vector<string>> outtbl(row_cnt);
    for (auto& row : outtbl)
      row.resize(size(algos_) + 1);

    // Evaluate length for column 0 width (row labels)
    for (auto& label : row_labels)
      col_widths[0] = std::max(col_widths[0], size(label));

    // Evalulate for remaining columns (uniform width)
    const size_t digits     = 8;
    const size_t dec_digits = 3;
    size_t       col_width  = digits + ((digits - 1) / 3) + 1 + dec_digits; // digits + digits per group + 1 for '.'
    for (size_t c = 1; c < col_cnt; ++c)
      col_width = std::max(col_width, size(algos_[c - 1].name));
    for (size_t c = 1; c < col_cnt; ++c)
      col_widths[c] = col_width;

    double fastest_sec = std::numeric_limits<double>::max();
    for (auto& algo : algos)
      fastest_sec = std::min(fastest_sec, totalSecFnc(algo));

    auto fmt_col_hdr = [&col_width](const string& lbl) { return fmt::format("{:^{}}", lbl, col_width); };

    auto fmt_col_num = [&col_width, &dec_digits](double num) {
      string nstr = fmt::format("{:.{}Lf}", num, dec_digits);
      return fmt::format("{:>{}} ", nstr, col_width);
    };
    auto fmt_col_pct = [&col_width, &dec_digits](double num) {
      string nstr = fmt::format("{:.{}Lf}", num, dec_digits);
      return fmt::format("{:>{}}%", nstr, col_width);
    };

    // Fill in the row labels
    for (size_t r = 0; r < row_cnt; ++r)
      outtbl[r][0] = fmt::format("{:<{}}", row_labels[r], col_widths[0]);

    // Fill in the column values
    string nstr;
    for (size_t a = 0, c = 1; a < size(algos); ++a, ++c) {
      outtbl[0][c] = fmt_col_hdr(algos[a].name);

      size_t prev  = (a == 0) ? a : a - 1;
      outtbl[1][c] = fmt_col_num(totalSecFnc(algos[a]));                                          // total
      outtbl[2][c] = fmt_col_num(avgSecFnc(algos[a]));                                            // avg
      outtbl[3][c] = fmt_col_pct((totalSecFnc(algos[a]) / totalSecFnc(algos[prev]) - 1) * 100.0); // % of previous
      outtbl[4][c] = fmt_col_pct((totalSecFnc(algos[a]) / fastest_sec - 1) * 100.0);              // % of fastest
    }

    for (auto& row : outtbl) {
      for (auto& col : row) {
        fmt::print("{:<{}}  ", col, col_width);
      }
      print("\n");
    }
    cout << endl;
  }

  vector<dijkstra_algo>& algos_;
  bool                   include_average_ = false;
};

//-------------------------------------------------------------------------------------------------
// bench_dijkstra_runner
//
void bench_dijkstra_runner() {
  using Distance     = int64_t;
  using G            = vector<vector<tuple<vertex_id_type, Distance>>>;
  using Distances    = std::vector<Distance>;
  using Predecessors = std::vector<vertex_id_type>;

  timer session_timer("Total session");

  bench_files bench_source = g2bench_chesapeake; // gap_road, g2bench_bips98_606, g2bench_chesapeake
  triplet_matrix<vertex_id_type, vertex_id_type> triplet;
  array_matrix<vertex_id_type>                   sources;

  // Read the Matrix Market file
  bool const requires_sort = !std_adjacency_graph<G>;
  load_matrix_market(bench_source, triplet, sources,
                     requires_sort); // gap_road, g2bench_bips98_606, g2bench_chesapeake
  cout << endl;

  // Load the graph
  G           g;
  graph_stats stats = load_graph(triplet, g);
  fmt::println("Graph stats: {}", stats);
  cout << endl;

  Distances     distances(size(vertices(g)));
  Predecessors  predecessors(size(vertices(g)));
  bench_results results;

#if defined(ENABLE_EDGE_WEIGHT_ONE)
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return 1; };
  println("Edge weight function = 1");
#else
  auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
  println("Edge weight function = edge_value(g, uv)");
#endif
  cout << endl;

  std::string events_str;
#if ENABLE_DISCOVER_VERTEX
  events_str += "discover_vertex ";
#endif
#if ENABLE_EXAMINE_VERTEX
  events_str += "examine_vertex ";
#endif
#if ENABLE_EDGE_RELAXED
  events += "edge_relaxed ";
#endif
  if (events_str.empty())
    events_str = "(none)";

  // Add the algorithms
  algos.emplace_back(
        dijkstra_algo{"nwgraph", [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& srcs) {
                        auto returned_distances = nwgraph_dijkstra(g, srcs, distance_fnc);
                        assert(size(returned_distances) == size(distances));
                        std::copy(begin(returned_distances), end(returned_distances), begin(distances));
                        for (size_t i = 0; i < size(distances); ++i)
                          distances[i] = static_cast<Distance>(returned_distances[i]);
                      }});

  algos.emplace_back(
        dijkstra_algo{"visitor", [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& srcs) {
                        discover_vertex_visitor<G, Distances> visitor(g, results);
                        dijkstra_with_visitor(g, srcs, predecessors, distances, distance_fnc, visitor);
                      }});

  algos.emplace_back(
        dijkstra_algo{"coroutine", [&g, &distance_fnc, &distances, &predecessors, &results](const SourceIds& srcs) {
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
                        for (auto bfs = co_dijkstra(g, events, srcs, predecessors, distances, distance_fnc); bfs;) {
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

  size_t algo_name_width = 0;
  for (auto& algo : algos)
    algo_name_width = std::max(algo_name_width, size(algo.name));

#ifdef _MSC_VER
  // Minimize impact from other activity while we're gathering data
  HANDLE hThread = GetCurrentThread();
  SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

  // Run the algorithm on all sources
  try {
    fmt::println("================================================================");
    fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
    fmt::println("Running tests on all {} sources using {} algorithms", size(sources.vals), size(algos));
    fmt::println("{} tests are run and the minimum is taken", test_trials);
    fmt::println("Events included: {}", events_str);
    fmt::println("Inline relax_targerts is {}", (ENABLE_INLINE_RELAX_TARGET ? "enabled" : "disabled"));
#ifndef ENABLE_PREDECESSORS
    fmt::println("Predecessors are disabled");
#endif
    fmt::println("Benchmark starting at {}\n", current_timestamp());

    fmt::print("{:5}  {:{}}  {:^11}  {:5}", "A.Obs", "Algorithm", algo_name_width, "V.Visited", "Elapsed (s)");
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

    size_t algo_num = 0;
    for (dijkstra_algo& algo : algos) {

      ++algo_num;
      size_t observation = 0;
      double min_elapsed = std::numeric_limits<double>::max(); // seconds

      for (size_t t = 0; t < test_trials; ++t) {
        results = {};
        init_shortest_paths(distances, predecessors); // we want to highlight algorithm time, not setup
        simple_timer run_time;
        algo.run(sources.vals);
        min_elapsed = std::min(min_elapsed, run_time.elapsed());
      }
      algo.all_elapsed = min_elapsed;
      size_t visited   = vertices_visited(distances);
      print("{}.{:03}  {:<{}}  {:11L}  {:>11.3f}", algo_num, ++observation, algo.name, algo_name_width, visited,
            min_elapsed);
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

    cout << endl;
    output_algo_results output(algos);
    output.output_all_summary();
    cout << endl;
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }
  cout << endl;

  // Run the algorithm for each source
  try {
    fmt::println("================================================================");
    fmt::println("Benchmarking Dijkstra's Algorithm Using Visitors and Co-routines");
    fmt::println("Running tests on {} sources individually using {} algorithms", size(sources.vals), size(algos));
    fmt::println("{} tests are run for each source and the minimum is taken", test_trials);
    fmt::println("Events included: {}", events_str);
    fmt::println("Inline relax_targerts is {}", (ENABLE_INLINE_RELAX_TARGET ? "enabled" : "disabled"));
#ifndef ENABLE_PREDECESSORS
    fmt::println("Predecessors are disabled");
#endif
    fmt::println("Benchmark starting at {}\n", current_timestamp());

    fmt::print("{:5}  {:{}}  {:^9}  {:^11}  {:5}", "A.Obs", "Algorithm", algo_name_width, "Source", "V.Visited",
               "Elapsed (s)");
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

    size_t algo_num = 0;
    for (dijkstra_algo& algo : algos) {

      ++algo_num;
      size_t observation = 0;
      for (size_t s = 0; s < sources.vals.size(); ++s) {
        const SourceIds one_source  = {sources.vals[s]};
        double          min_elapsed = std::numeric_limits<double>::max(); // seconds

        for (size_t t = 0; t < test_trials; ++t) {
          results = {};
          init_shortest_paths(distances, predecessors); // we want to highlight algorithm time, not setup
          simple_timer run_time;
          algo.run(one_source);
          min_elapsed = std::min(min_elapsed, run_time.elapsed());
        }
        algo.elapsed.push_back(min_elapsed);
        size_t visited = vertices_visited(distances);
        print("{}.{:03}  {:<{}}  {:>9}  {:11L}  {:>11.3f}", algo_num, ++observation, algo.name, algo_name_width,
              one_source[0], visited, min_elapsed);
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

    cout << endl;
    output_algo_results output(algos);
    output.output_detail_summary();
    cout << endl;
  } catch (const std::exception& e) {
    fmt::print("Exception caught: {}\n", e.what());
  }
}
