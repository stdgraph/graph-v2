#include <filesystem>
#include <fstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>

#include "graph/graph.hpp"
#include "graph/algorithm/experimental/co_dijkstra.hpp"

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

struct bench_files {
  path mtx_path;
  path mtx_sorted_path;
  path sources_path;

  bench_files() = delete;
  bench_files(const path&        base_path,
              const string_view& subpath,
              const string_view& mtx_file,
              const string_view& mtx_sorted_file,
              const string_view& sources_file)
        : mtx_path(base_path / subpath / mtx_file)
        , mtx_sorted_path(base_path / subpath / mtx_sorted_file)
        , sources_path(base_path / subpath / sources_file) {}
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


void bench_co_dijkstra() {
  triplet_matrix<int64_t, int64_t> triplet;
  array_matrix<int64_t>            array_mtx;
  using G = std::vector<vector<tuple<int64_t, int64_t>>>;

  const bench_files& bench_target = gap_road;

  // Read triplet from Matrix Market. Use std::ifstream to read from a file.
  {
    timer read_time("Reading matrix data", true);

    {
      std::ifstream ifs(bench_target.mtx_path);
      assert(ifs.is_open());

      fmm::read_matrix_market_triplet(ifs, triplet.nrows, triplet.ncols, triplet.rows, triplet.cols, triplet.vals);
      read_time.set_count(triplet.nrows, "rows");
    }

    {
      std::ifstream ifs(bench_target.sources_path);
      fmm::read_matrix_market_array(ifs, array_mtx.nrows, array_mtx.ncols, array_mtx.vals, fmm::row_major);
    }
  }

  // Load the graph
  G g(triplet.nrows);
  {
    timer load_time("Loading graph", true);

    for (size_t i = 0; i < triplet.rows.size(); ++i) {
      g[triplet.rows[i]].emplace_back(triplet.cols[i], triplet.vals[i]);
    }

    load_time.set_count(g.size(), "vertices");
  }


  // // Print the matrix
  // std::cout << "Matrix Market:\n";
  // for (int64_t i = 0; i < triplet.rows.size(); ++i) {
  //std::cout << triplet.rows[i] << " " << triplet.cols[i] << " " << triplet.vals[i] << std::endl;
  // }
  // std::cout << std::endl;
}
