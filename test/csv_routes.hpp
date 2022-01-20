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

void init_console(); // init cout for UTF-8

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
  routes_base(csv::string_view csv_file) { load_cities(csv_file); }

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

private: // Operations
  void load_cities(csv::string_view csv_file) {
    csv::CSVReader reader(csv_file);            // CSV file reader
    assert(reader.get_col_names().size() >= 2); // expecting from_city, to_city [, weight]

    // build set of unique city names
    // string_views remain valid while reader is open
    std::set<std::string_view> city_set;
    for (csv::CSVRow& row : reader) { // Input iterator
      std::string_view from = row[0].get<std::string_view>();
      std::string_view to   = row[1].get<std::string_view>();
      city_set.insert(from);
      city_set.insert(to);
      ++edges_read_;
    }

    // Preserve the city names in an ordered vector
    cities_.reserve(city_set.size());
    for (const std::string_view& city_name : city_set)
      cities_.emplace_back(city_name);
  }

private:                      // Member Variables
  cities_vec cities_;         ///< Ordered UTF-8 city names (case-sensitive)
  size_t     edges_read_ = 0; //
};
