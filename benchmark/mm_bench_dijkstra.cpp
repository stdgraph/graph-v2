#include <filesystem>
#include <fstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>
#include <fmt/format.h>

#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"
#include "graph/algorithm/experimental/visitor_dijkstra.hpp"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4242) // '=': conversion from 'int' to 'char', possible loss of data
#else
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include <fast_matrix_market/fast_matrix_market.hpp>
#ifdef _MSC_VER
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif

#include "timer.hpp"
#include "util.hpp"

namespace fmm = fast_matrix_market;
using std::string_view;
using std::filesystem::path;
using std::vector;
using std::tuple;
using path = std::filesystem::path;

using std::ranges::size;
using std::cout;
using std::wcout;
using std::endl;

using namespace std::graph;
using namespace std::graph::experimental;

struct bench_files {
  path        mtx_path;
  path        mtx_sorted_path;
  path        sources_path;
  std::string name;

  bench_files() = delete;
  bench_files(const path&        base_path,
              const string_view& subpath,
              const string_view& mtx_file,
              const string_view& mtx_sorted_file,
              const string_view& sources_file)
        : mtx_path(base_path / subpath / mtx_file)
        , mtx_sorted_path(base_path / subpath / mtx_sorted_file)
        , sources_path(base_path / subpath / sources_file)
        , name(mtx_path.stem().string()) {}
};

#ifdef _MSC_VER
path gap = "D:\\dev_graph\\data\\GAP";
#else
path gap = "/mnt/d/dev_graph/data/GAP";
#endif
bench_files gap_road(gap, "GAP-road", "GAP-road.mtx", "GAP-road.sorted.mtx", "GAP-road_sources.mtx");          // 599MB
bench_files
      gap_twitter(gap, "GAP-twitter", "GAP-twitter.mtx", "GAP-twitter.sorted.mtx", "GAP-twitter_sources.mtx"); // 29.2GB
bench_files gap_web(gap, "GAP-web", "GAP-web.mtx", "GAP-web.sorted.mtx", "GAP-web_sources.mtx"); // 38.1GB; sort= 490.4s
bench_files
      gap_kron(gap, "GAP-kron", "GAP-kron.mtx", "GAP-kron.sorted.mtx", "GAP-kron_sources.mtx");  // 43.1GB; sort=1261.9s
bench_files gap_urand(
      gap, "GAP-urand", "GAP-urand.mtx", "GAP-urand.sorted.mtx", "GAP-urand_sources.mtx");       // 43.8GB; sort=1377.7s


#ifdef _MSC_VER
path g2bench = "D:\\dev_graph\\graph-v2-bench\\datasets";
#else
path gap = "/mnt/d/dev_graph/graph-v2-bench/datasets";
#endif
bench_files g2bench_chesapeake(g2bench, "", "chesapeake.mtx", "", "");
bench_files g2bench_bips98_606(g2bench, "", "bips98_606.mtx", "", "");


// Visitor class for visitor_dijkstra
template <class G, class Distances>
struct discover_vertex_visitor : dijkstra_visitor_base<G> {
  using base_t = dijkstra_visitor_base<G>;

  discover_vertex_visitor(G& g, Distances& distances) : base_t(g), distances_(distances) {}
  ~discover_vertex_visitor() = default;

  void on_discover_vertex(const base_t::vertex_desc_type& vdesc) noexcept {
    //auto&& [uid, u] = vdesc;
    ++vertices_discovered_;
    //if ((vertices_discovered_ & 0xff) == 0)
    //  fmt::print("\n{:L} vertices discovered", vertices_discovered_);
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
  using G = std::vector<std::vector<tuple<int64_t, int64_t>>>;

  //const bench_files& bench_target = gap_road;
  const bench_files& bench_target = g2bench_bips98_606;

  // Read triplet from Matrix Market. Use std::ifstream to read from a file.
  {
    timer read_time("Reading matrix data", true);

    // Read the matrix
    {
      std::ifstream ifs(bench_target.mtx_path);
      assert(ifs.is_open());
      fmm::read_matrix_market_triplet(ifs, triplet.nrows, triplet.ncols, triplet.rows, triplet.cols, triplet.vals);
      read_time.set_count(size(triplet.rows), "rows");
    }

    // Read the sources
    if (bench_target.sources_path.stem().empty()) {
      // If no sources file is provided, use the first vertex as the source
      sources.nrows = 1;
      sources.ncols = 1;
      sources.vals.push_back(0);
    } else {
      std::ifstream ifs(bench_target.sources_path);
      assert(ifs.is_open());
      fmm::read_matrix_market_array(ifs, sources.nrows, sources.ncols, sources.vals, fmm::row_major);
    }
  }

  // Load the graph
  G g(triplet.nrows);
  {
    timer load_time("Loading graph", true);

    for (size_t i = 0; i < triplet.rows.size(); ++i) {
      g[triplet.rows[i]].emplace_back(triplet.cols[i], triplet.vals[i]);
    }

    load_time.set_count(size(triplet.rows), "edges");
  }

  fmt::println("Graph {} has {:L} vertices and {:L} edges", bench_target.name, size(vertices(g)), size(triplet.rows));

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
    std::string desc = fmt::format("Running {} with event(s) {} and using {} source(s)", "visitor_dijkstra",
                                   "discover_vertex", sources.nrows);
    timer       run_time(desc, true);

    using Distance     = int64_t;
    using Distances    = std::vector<Distance>;
    using Predecessors = std::vector<vertex_id_t<G>>;

    Distances    distances(size(vertices(g)));
    Predecessors predecessors(size(vertices(g)));
    init_shortest_paths(distances, predecessors);

    auto distance_fnc = [&g](edge_reference_t<G> uv) -> int64_t { return std::get<1>(edge_value(g, uv)); };
    discover_vertex_visitor visitor(g, distances);
    dijkstra_with_visitor(g, visitor, sources.vals, predecessors, distances, distance_fnc);

    run_time.set_count(visitor.vertices_discovered(), "vertice(s) discovered"); // includes sources
  };

  // Run the algorithm
  {
    run_visitor_dijkstra();
    //run_co_dijkstra();
  }
}
