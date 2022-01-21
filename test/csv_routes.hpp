#pragma once
// Includes csv_parser/single_include/csv.hpp and wraps it pragma to disable distracting warnings

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4458) // declaration of 'value' hides class member
#  pragma warning(disable : 4244) // conversion from 'double' to 'unsigned __int64', possible loss of data
#  pragma warning(                                                                                                     \
        disable : 4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#else
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include "single_include/csv.hpp"

#ifdef _MSC_VER
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif

#include <set>
#include <algorithm>
#include <string_view>

void init_console(); // init cout for UTF-8

/// <summary>
/// Scans 2 columns in a CSV file and returns all the unique values in an ordered vector.
/// This is used to get all the unique labels for vertices.
/// </summary>
/// <typeparam name="ColNumOrName">A name or integer for a column in the CSV file</typeparam>
/// <param name="csv_file">The CSV file name (path)</param>
/// <param name="col1">The first column to get labels from. If the column doesn't exist the function is undefined.</param>
/// <param name="col2">The second column to get labels from. If the column doesn't exist the function is undefined.</param>
/// <returns></returns>
template <typename ColNumOrName>
auto unique_vertex_labels(csv::string_view csv_file, ColNumOrName col1, ColNumOrName col2) {
  csv::CSVReader reader(csv_file); // CSV file reader

  // gather unique labels (case sensitive)
  std::set<std::string_view> lbls; // string_view valid until file is closed
  for (csv::CSVRow& row : reader) {
    lbls.insert(row[col1].get_sv());
    lbls.insert(row[col2].get_sv());
  }

  // copy labels to vector (ordered)
  std::vector<std::string> lbl_vec;
  lbl_vec.reserve(lbls.size());
  for (auto&& lbl : lbls)
    lbl_vec.push_back(std::string(lbl));

  return std::pair(std::move(lbl_vec), reader.n_rows()); // return (unique lbls, num rows read)
}

#ifdef FUTURE
/// <summary>
/// Gets the maximum value of two columns in a CSV file with integral values.
/// </summary>
/// <typeparam name="T">The integer type that is used to hold the maximum value found</typeparam>
/// <typeparam name="ColNumOrName">A name or integer for a column in the CSV file</typeparam>
/// <param name="csv_file">The CSV file name (path)</param>
/// <param name="col1">The first column to get labels from. If the column doesn't exist the function is undefined</param>
/// <param name="col2">The second column to get labels from. If the column doesn't exist the function is undefined</param>
/// <returns></returns>
template <std::integral T, typename ColNumOrName>
auto max_vertex_key(csv::string_view csv_file, ColNumOrName col1, ColNumOrName col2) {
  csv::CSVReader reader(csv_file); // CSV file reader
  T              max_key = std::numeric_limits<T>::min();
  for (csv::CSVRow& row : reader)
    max_key = std::max(max_key, std::max(row[col1].get<T>(), row[col2].get<T>()));

  return std::pair(max_key, reader.n_rows()); // return (max_key, num rows read)
}
#endif // FUTURE

/// <summary>
/// Base class used to read CSV files in the form "<name1>,<name2>,distance".
/// The names are read into a set of unique values (case sensitive) of cities
/// A derived class can be used to re-read the same file to create a graph.
///
/// If addtional columns are included they are ignored.
/// </summary>
/// <typeparam name="VKey">Vertex key type</typeparam>
template <typename VKey>
class routes_base {
public: // types
  using key_type   = VKey;
  using name_type  = std::string;
  using cities_vec = std::vector<name_type>;

public: // Construction/Destruction/Assignment
  /// <summary>
  /// Reads the CSV file and constructs the vector of cities.
  /// </summary>
  /// <param name="csv_file">Path for the input CSV file of cities</param>
  routes_base(csv::string_view csv_file) {
    auto&& [labels, row_cnt] = unique_vertex_labels(csv_file, 0ull, 1ull);
    cities_                  = std::move(labels);
    edges_read_              = row_cnt;
  }

  routes_base()                   = default;
  routes_base(const routes_base&) = default;
  routes_base(routes_base&&)      = default;
  ~routes_base()                  = default;

  routes_base&           operator=(const routes_base&) = default;
  constexpr routes_base& operator=(routes_base&&) = default;

public: // Properties
  constexpr cities_vec&       cities() { return cities_; }
  constexpr const cities_vec& cities() const { return cities_; }

  constexpr cities_vec::iterator find_city(std::string_view city_name) {
    auto it = std::ranges::lower_bound(cities_, city_name);
    if (it != end(cities_) && *it == city_name)
      return it;
    return end(cities_);
  }
  constexpr cities_vec::const_iterator find_city(std::string_view city_name) const {
    auto it = std::ranges::lower_bound(cities_, city_name);
    if (it != end(cities_) && *it == city_name)
      return it;
    return end(cities_);
  }

  constexpr key_type find_city_key(std::string_view city_name) const {
    return static_cast<key_type>(find_city(city_name) - begin(cities_));
  }

  size_t num_cities() const { return cities_.size(); }
  size_t num_routes() const { return edges_read_; }

private:                      // Member Variables
  cities_vec cities_;         ///< Ordered UTF-8 city names (case-sensitive)
  size_t     edges_read_ = 0; //
};
