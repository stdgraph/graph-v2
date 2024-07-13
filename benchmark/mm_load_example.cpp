#include <iostream>
#include <vector>

#include <fast_matrix_market/fast_matrix_market.hpp>

// If you prefer more concise code then you can use this to abbreviate the
// rather long namespace name. Then just use `fmm::` instead of `fast_matrix_market::`.
// This is not used below only to make the code simpler to understand at a glance and for easy copy/paste.
namespace fmm = fast_matrix_market;

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

void mm_load_example() {

}
