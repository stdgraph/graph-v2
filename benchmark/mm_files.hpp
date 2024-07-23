#pragma once
#include <filesystem>
#include <string_view>
#include <vector>

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


/**
 * A simple triplet sparse matrix.
 */
template <typename IT, typename VT>
struct triplet_matrix {
  int64_t         nrows = 0, ncols = 0;
  std::vector<IT> rows;
  std::vector<IT> cols;
  std::vector<VT> vals;
};

/**
 * A simple dense matrix.
 */
template <typename VT>
struct array_matrix {
  int64_t         nrows = 0, ncols = 0;
  std::vector<VT> vals;
};

/**
 * Benchmark matrix market files.
 */
struct bench_files {
  std::filesystem::path mtx_path;
  std::filesystem::path mtx_sorted_path;
  std::filesystem::path sources_path;
  std::string           name;

  bench_files() = delete;
  bench_files(const std::filesystem::path& base_path,
              const std::string_view&      subpath,
              const std::string_view&      mtx_file,
              const std::string_view&      mtx_sorted_file,
              const std::string_view&      sources_file)
        : mtx_path(base_path / subpath / mtx_file)
        , mtx_sorted_path(base_path / subpath / mtx_sorted_file)
        , sources_path(base_path / subpath / sources_file)
        , name(mtx_path.stem().string()) {}
};

extern bench_files gap_road;    // 599MB
extern bench_files gap_twitter; // 29.2GB
extern bench_files gap_web;     // 38.1GB; sort= 490.4s
extern bench_files gap_kron;    // 43.1GB; sort=1261.9s
extern bench_files gap_urand;   // 43.8GB; sort=1377.7s


extern bench_files g2bench_chesapeake;
extern bench_files g2bench_bips98_606;
