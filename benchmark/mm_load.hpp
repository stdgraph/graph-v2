#pragma once
#include "mm_files.hpp"
#include "timer.hpp"
#include <graph/container/compressed_graph.hpp>
#include <numeric>
#include <fstream>
#include <fmt/format.h>

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

using std::integral;
using std::convertible_to;
using std::ranges::forward_range;
using std::ranges::random_access_range;
using std::ranges::range_value_t;
using graph::adjacency_list;
using graph::container::compressed_graph;
namespace fmm = fast_matrix_market;

template <typename T>
class is_edge_like : public std::false_type {};
template <typename T>
inline constexpr bool is_edge_like_v = is_edge_like<T>::value;

template <typename IT, typename VT, typename... Other>
class is_edge_like<std::tuple<IT, VT, Other...>> : public std::true_type {};

template <typename IT, typename VT>
class is_edge_like<std::pair<IT, VT>> : public std::true_type {};

template <class G>
concept std_adjacency_graph = adjacency_list<G> &&               //
                              random_access_range<G> &&          //
                              forward_range<range_value_t<G>> && //
                              is_edge_like_v<range_value_t<range_value_t<G>>>;

// fmt::formatter specialization for fast_matrix_market::object_type
template <>
struct fmt::formatter<fmm::object_type> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(fmm::object_type t, FormatContext& ctx) const {
    return fmt::formatter<std::string_view>::format(fmm::object_map.at(t), ctx);
  }
};

// fmt::formatter specialization for fast_matrix_market::symmetry_type
template <>
struct fmt::formatter<fmm::symmetry_type> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(fmm::symmetry_type t, FormatContext& ctx) const {
    return fmt::formatter<std::string_view>::format(fmm::symmetry_map.at(t), ctx);
  }
};

// fmt::formatter specialization for fast_matrix_market::field_type
template <>
struct fmt::formatter<fmm::field_type> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(fmm::field_type t, FormatContext& ctx) const {
    return fmt::formatter<std::string_view>::format(fmm::field_map.at(t), ctx);
  }
};

// fmt::formatter specialization for fast_matrix_market::format_type
template <>
struct fmt::formatter<fmm::format_type> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(fmm::format_type t, FormatContext& ctx) const {
    return fmt::formatter<std::string_view>::format(fmm::format_map.at(t), ctx);
  }
};

// fmt::formatter specialization for fast_matrix_market::matrix_market_header
template <>
struct fmt::formatter<fmm::matrix_market_header> : fmt::formatter<std::string> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const fmm::matrix_market_header& header, FormatContext& ctx) const {
    return format_to(ctx.out(), "{} {} {} {} nrows={:L} ncols={:L} vector_length={:L} nnz={:L} header_lines={}",
                     header.object, header.format, header.field, header.symmetry, header.nrows, header.ncols,
                     header.vector_length, header.nnz, header.header_line_count);
  }
};

template <integral IT, typename VT>
void load_matrix_market(const bench_files&      bench_target,
                        triplet_matrix<IT, VT>& triplet,
                        array_matrix<IT>&       sources,
                        const bool              requires_ordered_rows = false) {
  namespace fmm = fast_matrix_market;
  using std::filesystem::path;
  using std::numeric_limits;

  fmt::println("Dataset: {}\n", bench_target.name);

  path target_mtx  = bench_target.mtx_path; // compressed_path requires a sorted mtx
  path sources_mtx = bench_target.sources_path;

  // Read triplet from Matrix Market. Use std::ifstream to read from a file.
  {
    fmm::matrix_market_header header;
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

      fmm::read_options options;
      options.generalize_symmetry = true;
      options.parallel_ok         = false;
      //options.num_threads = 1;
      fmm::read_matrix_market_triplet(ifs, header, triplet.rows, triplet.cols, triplet.vals, options);
      triplet.nrows = header.nrows;
      triplet.ncols = header.ncols;

      // compressed_graph requires edges are ordered by source_id.
      // Non-general symmetry means edges are generated in unordered ways and cause an assertion/exception.

      read_time.set_count(ssize(triplet.rows), "rows");
    }
    fmt::println("Matrix header: {}", header);

    // Read the sources
    header = {};
    if (bench_target.sources_path.stem().empty()) {
      // If no sources file is provided, use the first vertex as the source
      sources.nrows = static_cast<int64_t>(sources.vals.size());
      sources.ncols = 1;
      sources.vals.push_back(0);
      header.object        = fmm::object_type::matrix;
      header.format        = fmm::format_type::array;
      header.field         = fmm::field_type::integer;
      header.symmetry      = fmm::symmetry_type::general;
      header.ncols         = sources.nrows;
      header.ncols         = sources.ncols;
      header.vector_length = sources.nrows;
      header.nnz           = static_cast<int64_t>(sources.vals.size());
    } else {
      timer         read_time("Reading source data", true);
      std::ifstream ifs(bench_target.sources_path);
      assert(ifs.is_open());
      fmm::read_matrix_market_array(ifs, header, sources.vals, fmm::row_major);
      assert(std::integral<IT>);
      if (header.field == fmm::field_type::integer) {
        // OK. No vertex id conversion needed.
      } else if (header.field == fmm::field_type::real) {
        // GAP_road has real vertex ids. Convert to integer.
        //fmt::print("Converting sources to integer\n");
        ifs.clear();
        ifs.seekg(0, std::ios::beg);
        array_matrix<double> dsources;
        fmm::read_matrix_market_array(ifs, header, dsources.vals, fmm::row_major);
        sources.vals.clear();
        std::transform(dsources.vals.begin(), dsources.vals.end(), std::back_inserter(sources.vals),
                       [](auto val) { return static_cast<IT>(val); });
      } else {
        fmt::print("Warning: sources field type is not integer or real: {}\n", header.field);
      }
      read_time.set_count(sources.nrows, "sources");
    }
    fmt::println("Sources header: {}", header);
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
      sort_time.set_count(ssize(perm), "permutations");
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

      permute_time.set_count(ssize(triplet.rows), "edges");
    }
  }

  // Check for duplicates & self-loops (informational only)
  // Duplicate entries will only be detected if the matrix is sorted.
  size_t self_loops = 0;
  size_t duplicates = 0;
  size_t negative   = 0;
  {
    timer check_time("Checking for duplicates and self-loops", true);
    auto  row_col_view = std::views::zip(triplet.rows, triplet.cols);
    using pair_t       = std::pair<IT&, IT&>; //std::ranges::range_value_t<decltype(row_col_view)>;
    IT     bignum      = numeric_limits<IT>::max();
    size_t data_row    = 1;
    pair_t last_entry  = {bignum, bignum}; // unlikely to be in the data
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
    for (auto&& val : triplet.vals) {
      if (val < 0) {
        ++negative;
      }
    }
  }
  if (self_loops > 0)
    fmt::println("Warning: {} self-loops detected", self_loops);
  if (duplicates > 0)
    fmt::println("Warning: {} duplicate entries detected", duplicates);
  if (negative > 0)
    fmt::println("Warning: {} negative entries detected", negative);
}


struct graph_stats {
  size_t vertex_count       = 0;
  size_t edge_count         = 0;
  size_t min_degree         = std::numeric_limits<size_t>::max();
  size_t max_degree         = 0;
  size_t self_loops_removed = 0;

  template <graph::adjacency_list G>
  graph_stats(const G& g, size_t self_loops = 0)
        : vertex_count(graph::num_vertices(g))
        , edge_count(static_cast<size_t>(graph::num_edges(g)))
        , self_loops_removed(self_loops) {
    for (auto&& u : graph::vertices(g)) {
      min_degree = std::min(min_degree, size(graph::edges(g, u)));
      max_degree = std::max(max_degree, size(graph::edges(g, u)));
    }
  }
};

template <>
struct fmt::formatter<graph_stats> {
  // Parse format specifications
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  // Format the graph_stats object
  template <typename FormatContext>
  auto format(const graph_stats& stats, FormatContext& ctx) const {
    return fmt::format_to(
          ctx.out(), "num_vertices={:L}, num_edges={:L}, min_degree={:L}, max_degree={:L}, self_loops_removed={:L}",
          stats.vertex_count, stats.edge_count, stats.min_degree, stats.max_degree, stats.self_loops_removed);
  }
};


// This should work for any graph defined with std-based containers that matches the following patterns:
//  1. std::vector<std::vector<std::tuple<IT, VT, ...>>>
//  2. std::vector<std::vector<std::pair<IT, VT>>>
// The inner range must support emplace_back (it can be extended to support insert or push_front also, if needed).
//
template <std_adjacency_graph G, integral IT, typename VT>
[[nodiscard]] graph_stats load_graph(const triplet_matrix<IT, VT>& triplet, G& g) {
  size_t self_loops = 0;

  {
    timer load_time("Loading the std graph", true);
    using edge_type = range_value_t<range_value_t<G>>;

    g.clear();
    g.resize(static_cast<size_t>(triplet.nrows));
    for (size_t i = 0; i < triplet.rows.size(); ++i) {
      if (triplet.rows[i] == triplet.cols[i])
        ++self_loops;
      else
        g[static_cast<size_t>(triplet.rows[i])].emplace_back(
              edge_type{static_cast<int64_t>(triplet.cols[i]), static_cast<int64_t>(triplet.vals[i])});
    }

    load_time.set_count(ssize(triplet.rows), "edges");
  }

  return graph_stats(g, self_loops);
}

template <typename EV, typename VV, typename GV, integral VId, integral EIndex = VId>
[[nodiscard]] graph_stats load_graph(const triplet_matrix<VId, EV>&             triplet,
                                     compressed_graph<EV, VV, GV, VId, EIndex>& g) {
  //using G = compressed_graph<EV, VV, GV, VId, EIndex>;
  {
    timer load_time("Loading the compressed_graph", true);

    auto zip_view   = std::views::zip(triplet.rows, triplet.cols, triplet.vals);
    using zip_value = std::ranges::range_value_t<decltype(zip_view)>;

    using edge_desc = graph::copyable_edge_t<VId, EV>;
    auto edge_proj  = [](const zip_value& val) -> edge_desc {
      return edge_desc{std::get<0>(val), std::get<1>(val), std::get<2>(val)};
    };

    g.load_edges(zip_view, edge_proj);
    load_time.set_count(ssize(triplet.rows), "edges");
  }

  return graph_stats(g, 0);
}
