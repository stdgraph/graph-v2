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

void mm_simple1() {
  // Create a matrix
  triplet_matrix<int64_t, double> triplet;

  triplet.nrows = 4;
  triplet.ncols = 4;

  triplet.rows = {1, 2, 3, 3};
  triplet.cols = {0, 1, 2, 3};
  triplet.vals = {1.0, 5, 2E5, 19};

  std::string mm;

  // Write triplet to Matrix Market. Use std::ostringstream to write to a string.
  {
    std::ostringstream oss;

    // The `nrows` and `ncols` below are a brace initialization of the header.
    // If you are interested in the other aspects of the Matrix Market header then
    // construct an instance of fast_matrix_market::matrix_market_header.
    // This is how you would manipulate the comment field: header.comment = std::string("comment");
    // You may also set header.field = fast_matrix_market::pattern to write a pattern file (only indices, no values).
    // Non-pattern field types (integer, real, complex) are deduced from the template type and cannot be overriden.

    fast_matrix_market::write_matrix_market_triplet(oss, {triplet.nrows, triplet.ncols}, triplet.rows, triplet.cols,
                                                    triplet.vals);

    mm = oss.str();
    std::cout << mm << std::endl << std::endl;
  }

  // Read Matrix Market into another triplet
  {
    triplet_matrix<int64_t, double> triplet2;
    std::istringstream              iss(mm);
    fast_matrix_market::read_matrix_market_triplet(iss, triplet2.nrows, triplet2.ncols, triplet2.rows, triplet2.cols,
                                                   triplet2.vals);

    assert(triplet.nrows == triplet2.nrows && triplet.ncols == triplet2.ncols && triplet.rows == triplet2.rows &&
           triplet.cols == triplet2.cols && triplet.vals == triplet2.vals);
  }

  // Read Matrix Market into complex dense array
  array_matrix<std::complex<double>> array;
  {
    // Dense arrays can be row- or column-major. FMM defaults to row-major but can be used with either.
    // Use the ordering that your code expects.
    // Using the wrong one can lead to transposed values (for square matrices) or scrambled values.
    //
    // read_matrix_market_array() accepts any resizable vector type, such as std::vector, and will resize
    // it to match the contents of the Matrix Market file.
    //
    // Note that the file being read happens to be a coordinate (sparse) file. That is ok, FMM performs the
    // conversion automatically and efficiently. In this case the resulting dense array is allocated
    // (and default initialized to zero by std::vector::resize()) then FMM writes only the values specified in the file.
    std::istringstream iss(mm);
    fast_matrix_market::read_matrix_market_array(iss, array.nrows, array.ncols, array.vals,
                                                 fast_matrix_market::row_major);
  }

  // Write dense array to Matrix Market
  {
    std::ostringstream oss;
    fast_matrix_market::write_matrix_market_array(oss, {array.nrows, array.ncols}, array.vals,
                                                  fast_matrix_market::row_major);

    mm = oss.str();
    std::cout << mm << std::endl << std::endl;
  }
}
