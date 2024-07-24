#pragma once
#include "mm_files.hpp"
#include "timer.hpp"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4242) // '=': conversion from 'int' to 'char', possible loss of data
#  pragma warning(disable : 4701) // potentially uninitialized local variable 'value' used
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


template <typename IT, typename VT>
void load_matrix_market(const bench_files&      bench_target,
                        triplet_matrix<IT, VT>& triplet,
                        array_matrix<IT>&       sources,
                        const bool              requires_ordered_rows = false) {
  namespace fmm = fast_matrix_market;
  using std::filesystem::path;
  using std::numeric_limits;

  fmt::println("Loading data from the '{}' dataset", bench_target.name);

  path target_mtx  = bench_target.mtx_path; // compressed_path requires a sorted mtx
  path sources_mtx = bench_target.sources_path;

  // Read triplet from Matrix Market. Use std::ifstream to read from a file.
  {
    {
      timer read_time("Reading matrix data", true);

      std::ifstream ifs(target_mtx);
      assert(ifs.is_open());

      // The `nrows` and `ncols` below are a brace initialization of the header.
      // If you are interested in the other aspects of the Matrix Market header then
      // construct an instance of fast_matrix_market::matrix_market_header.
      // This is how you would manipulate the comment field: header.comment = std::string("comment");
      // You may also set header.field = fast_matrix_market::pattern to write a pattern file (only indices, no values).
      // Non-pattern field types (integer, real, complex) are deduced from the template type and cannot be overriden.

      fmm::matrix_market_header header;
      fmm::read_options         options;
      options.generalize_symmetry = true;
      options.parallel_ok         = false;
      //options.num_threads = 1;
      fmm::read_matrix_market_triplet(ifs, header, triplet.rows, triplet.cols, triplet.vals, options);
      triplet.nrows = header.nrows;
      triplet.ncols = header.ncols;

      // compressed_graph requires edges are ordered by source_id.
      // Non-general symmetry means edges are generated in unordered ways and cause an assertion/exception.
      //assert(header.symmetry == fmm::symmetry_type::general);

      read_time.set_count(size(triplet.rows), "rows");
    }

    // Read the sources
    if (bench_target.sources_path.stem().empty()) {
      // If no sources file is provided, use the first vertex as the source
      if (triplet.nrows > 0) {
        sources.nrows = 1;
        sources.ncols = 1;
        sources.vals.push_back(0);
      }
    } else {
      timer         read_time("Reading source data", true);
      std::ifstream ifs(bench_target.sources_path);
      assert(ifs.is_open());
      fmm::read_matrix_market_array(ifs, sources.nrows, sources.ncols, sources.vals, fmm::row_major);
      read_time.set_count(sources.nrows, "sources");
    }
  }

  // Sort the triplet & sources, if needed
  if (!requires_ordered_rows) {
    fmt::println("The matrix doesn't require sorting.");
  } else if (std::ranges::is_sorted(std::views::zip(triplet.rows, triplet.cols))) {
    fmt::println("The matrix is already sorted.");
  } else {
    fmt::println("The matrix requires sorting.");

    // Find sort permutation
    std::vector<std::size_t> perm(triplet.rows.size());
    {
      timer sort_time("Sorting permutations", true);

      std::ranges::iota(perm, 0);
      std::ranges::sort(perm.begin(), perm.end(), [&](std::size_t i, std::size_t j) {
        if (triplet.rows[i] != triplet.rows[j])
          return triplet.rows[i] < triplet.rows[j];
        if (triplet.cols[i] != triplet.cols[j])
          return triplet.cols[i] < triplet.cols[j];

        return false;
      });
      sort_time.set_count(size(perm), "permutations");
    }

    // Reorder edges
    {
      timer permute_time("Reorder edges", true);

      std::vector<IT> sorted_rows;
      sorted_rows.reserve(triplet.rows.size());
      std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_rows),
                     [&](auto i) { return triplet.rows[i]; });
      swap(sorted_rows, triplet.rows);
      sorted_rows = std::vector<IT>();

      std::vector<IT> sorted_cols;
      sorted_cols.reserve(triplet.cols.size());
      std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_cols),
                     [&](auto i) { return triplet.cols[i]; });
      swap(sorted_cols, triplet.cols);
      sorted_cols = std::vector<IT>();

      std::vector<VT> sorted_vals;
      sorted_vals.reserve(triplet.vals.size());
      std::transform(perm.begin(), perm.end(), std::back_inserter(sorted_vals),
                     [&](auto i) { return triplet.vals[i]; });
      swap(sorted_vals, triplet.vals);
      sorted_vals = std::vector<VT>();

      permute_time.set_count(size(triplet.rows), "edges");
    }
  }

  // Check for duplicates & self-loops (informational only)
  // Duplicate entries will only be detected if the matrix is sorted.
  {
    timer check_time("Checking for duplicates and self-loops", true);
    auto  row_col_view = std::views::zip(triplet.rows, triplet.cols);
    using pair_t       = std::ranges::range_value_t<decltype(row_col_view)>;
    size_t data_row    = 1;
    pair_t last_entry  = {numeric_limits<IT>::max(), numeric_limits<IT>::max()}; // unlikely to be in the data
    size_t self_loops  = 0;
    size_t duplicates  = 0;
    for (auto&& row_col : std::views::zip(triplet.rows, triplet.cols)) {
      auto [row, col] = row_col;
      if (row == col) {
        //fmt::println("Warning: self-loop detected at row {} with row/col of {}/{}", data_row, row, col);
        ++self_loops;
      }
      if (row_col == last_entry) {
		//fmt::println("Warning: duplicate entry detected at row {} with row/col of {}/{}", data_row, row, col);
        ++duplicates;
	  }

      last_entry = row_col;
      ++data_row;
    }
    if (self_loops > 0)
	  fmt::println("Warning: {} self-loops detected", self_loops);
    if (duplicates > 0)
        fmt::println("Warning: {} duplicate entries detected", duplicates);
  }
}
