#include <filesystem>
#include <fstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>
#include <chrono>
#include <tuple>
#include <ranges>
#include <graph/container/compressed_graph.hpp>

#include "mm_load.hpp"

// If you prefer more concise code then you can use this to abbreviate the
// rather long namespace name. Then just use `fmm::` instead of `fast_matrix_market::`.
// This is used below only to make the code simpler to understand at a glance and for easy copy/paste.
namespace fmm = fast_matrix_market;

using std::filesystem::path;
using std::tuple;
using std::vector;

using namespace std::graph;
using namespace std::graph::container;

#if 0
void mm_load_example() {
  // Load a matrix
  triplet_matrix<int64_t, double> triplet;

  std::string mm = "%%MatrixMarket matrix coordinate real general\n"
                   "%\n"
                   "4 4 4\n"
                   "1 1 1.0\n"
                   "2 2 5.0\n"
                   "3 3 2.0e5\n"
                   "3 4 19.0\n";

  // Read triplet from Matrix Market. Use std::istringstream to read from a string.
  {
    std::istringstream iss(mm);

    // The `nrows` and `ncols` below are a brace initialization of the header.
    // If you are interested in the other aspects of the Matrix Market header then
    // construct an instance of fast_matrix_market::matrix_market_header.
    // This is how you would manipulate the comment field: header.comment = std::string("comment");
    // You may also set header.field = fast_matrix_market::pattern to write a pattern file (only indices, no values).
    // Non-pattern field types (integer, real, complex) are deduced from the template type and cannot be overriden.

    fmm::read_matrix_market_triplet(iss, triplet.nrows, triplet.ncols, triplet.rows, triplet.cols, triplet.vals);
  }

  // Print the matrix
  std::cout << "Matrix Market:\n";
  for (uint64_t i = 0; i < triplet.rows.size(); ++i) {
    std::cout << triplet.rows[i] << " " << triplet.cols[i] << " " << triplet.vals[i] << std::endl;
  }
  std::cout << std::endl;
}

template <typename IT, typename VT>
void sort_triplet(triplet_matrix<IT, VT>& triplet) {
  // Already sorted?
  if (std::ranges::is_sorted(std::views::zip(triplet.rows, triplet.cols))) {
    fmt::println("The matrix is already sorted.");
    return;
  }
  fmt::println("The matrix is not sorted yet.");

  // Find sort permutation
  std::vector<std::size_t> perm(triplet.rows.size());
  {
    timer sort_time("Sorting");

    std::ranges::iota(perm, 0);
    std::ranges::sort(perm.begin(), perm.end(), [&](std::size_t i, std::size_t j) {
      if (triplet.rows[i] != triplet.rows[j])
        return triplet.rows[i] < triplet.rows[j];
      if (triplet.cols[i] != triplet.cols[j])
        return triplet.cols[i] < triplet.cols[j];

      return false;
    });
  }

  // Apply permutation
  {
    timer permute_time("Apply permutation");

    std::vector<IT> sorted_rows;
    sorted_rows.reserve(triplet.rows.size());
    std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_rows), [&](auto i) { return triplet.rows[i]; });
    swap(sorted_rows, triplet.rows);
    sorted_rows = std::vector<IT>();

    std::vector<IT> sorted_cols;
    sorted_cols.reserve(triplet.cols.size());
    std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_cols), [&](auto i) { return triplet.cols[i]; });
    swap(sorted_cols, triplet.cols);
    sorted_cols = std::vector<IT>();

    std::vector<VT> sorted_vals;
    sorted_vals.reserve(triplet.vals.size());
    std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_vals), [&](auto i) { return triplet.vals[i]; });
    swap(sorted_vals, triplet.vals);
    sorted_vals = std::vector<VT>();
  }
}
#endif //0

// Dataset: gap_twitter, symmetry_type::general, 1,468,364,884 rows
//  Deb/Rel parallel_ok num_threads Read        Rows/Sec     LoadSimple  Edges/Sec    LoadCompressed  Edges/Sec
//  ------- ----------- ----------- ----------- ----------   ---------- -----------   --------------  -----------
//  Debug   false       1           6m0s(360)    4,077,499   5m32s(332)   4,077,499   5m49s(348)      4,213,302
//  Debug   true        2           12m42s(761)  1,927,867   5m13s(313)   4,688,601   5m36s(335)      4,373,910
//  Release false       1           2m18s(138)  10,619,350   1m24s(83)   17,557,093   1m2s(62)        23,531,613
//  Release true        2           1m19s(78)   18,625,828
//  Release true        4           1m20s(79)   18,460,595   1m18s(78)   18,752,507   0m45s(44)       32,708,977

void mm_load_file_example() {

  triplet_matrix<int64_t, int64_t> triplet;
  array_matrix<int64_t>            sources;

  load_matrix_market(gap_road, triplet, sources, true);

  // Load a simple graph: vector<vector<tuple<int64_t, int64_t>>>
  {
    timer load_time("Loading simple graph", true);

    using G = std::vector<std::vector<tuple<int64_t, int64_t>>>;
    G g(triplet.nrows);
    for (size_t i = 0; i < triplet.rows.size(); ++i) {
      g[triplet.rows[i]].emplace_back(triplet.cols[i], triplet.vals[i]);
    }

    load_time.set_count(size(triplet.rows), "edges");
  }

  // Load a compressed_graph
  {
    timer load_time("Loading compressed_graph", true);

    using G = compressed_graph<int64_t, void, void, int64_t, int64_t>;

    auto zip_view    = std::views::zip(triplet.rows, triplet.cols, triplet.vals);
    using zip_view_t = decltype(zip_view);
    using zip_value  = std::ranges::range_value_t<decltype(zip_view)>;

    using edge_desc = edge_descriptor<int64_t, true, void, int64_t>;
    auto edge_proj  = [](const zip_value& val) -> edge_desc {
      return edge_desc{std::get<0>(val), std::get<1>(val), std::get<2>(val)};
    };

    G g(zip_view, edge_proj);

    load_time.set_count(size(triplet.rows), "edges");
  }
}
