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

#include "graph/graph.hpp"
#include <set>
#include <algorithm>
#include <string_view>
#include <iomanip>

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
    auto&& [labels, row_cnt] = unique_vertex_labels(csv_file, 0UL, 1UL);
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

  constexpr std::string_view city(key_type city_key) const { return cities_[city_key]; }

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

  constexpr cities_vec::iterator       frankfurt() { return find_city("Frankf\xC3\xBCrt"); }
  constexpr cities_vec::const_iterator frankfurt() const { return find_city("Frankf\xC3\xBCrt"); }
  constexpr key_type                   frankfurt_key() const {
    return static_cast<key_type>(find_city("Frankf\xC3\xBCrt") - begin(cities_));
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

template <typename G>
class routes_graph : public routes_base<std::graph::vertex_key_t<G>> {
public:
  using graph_type  = G;
  using key_type    = std::graph::vertex_key_t<G>;
  using name_view   = std::graph::vertex_value_t<G>;
  using weight_type = std::graph::edge_value_t<G, std::graph::vertex_edge_range_t<G>>;
  using base_type   = routes_base<key_type>;
  //constexpr inline static bool sourced = sourced_incidence_graph<G>;

public: // Construction/Destruction/Assignment
  routes_graph(csv::string_view csv_file) : base_type(csv_file), g_(load_routes(csv_file)) {}

  routes_graph()                    = default;
  routes_graph(const routes_graph&) = default;
  routes_graph(routes_graph&&)      = default;
  ~routes_graph()                   = default;

  routes_graph& operator=(const routes_graph&) = default;
  routes_graph& operator=(routes_graph&&) = default;

public: // Properties
  constexpr graph_type&       graph() { return g_; }
  constexpr const graph_type& graph() const { return g_; }

  using base_type::cities;
  using base_type::find_city_key;

public:  // Operations
private: // construction helpers
  graph_type load_routes(csv::string_view csv_file) {
    csv::CSVReader reader(csv_file); // CSV file reader

    auto ekey_fnc = [this](const csv::CSVRow& row) {
      auto from_key = find_city_key(row[0].get<std::string_view>());
      auto to_key   = find_city_key(row[1].get<std::string_view>());
      assert(from_key < cities().size() && to_key < cities().size());
      return std::pair{from_key, to_key};
    };
    auto evalue_fnc = [this](const csv::CSVRow& row) {
      auto to_key = find_city_key(row[1].get<std::string_view>());
      auto dist   = row[2].get<double>();
      assert(to_key < cities().size());
      return dist;
    };

    auto vvalue_fnc = [](const std::string& name) { return std::string_view(name); };

    const key_type max_city_key = static_cast<key_type>(size(cities())) - 1;
    //graph_type     g(reader, ekey_fnc, evalue_fnc, typename graph_type::allocator_type());
    //graph_type     g(max_city_key, reader, ekey_fnc, evalue_fnc, typename graph_type::allocator_type());
    graph_type g(reader, cities(), ekey_fnc, evalue_fnc, vvalue_fnc);

#if 0
    for (CSVRow& row : reader()) { // Input iterator-ish
      string_view from = row[0].get<string_view>();
      string_view to   = row[1].get<string_view>();
      weight_type dist = row[2].get<weight_type>();
    }
#endif
    return g;
  }

private:         // Member Variables
  graph_type g_; // CSR graph of routes
};

template <typename OStream, typename G>
OStream& operator<<(OStream& os, const routes_graph<G>& graph) {
  using namespace std::graph;
  auto&& g = graph.graph();

#if 0
  // experiment with transform
  using namespace std::ranges;
  auto rng = vertices(g) | views::transform([&g](auto&& u) { return std::tuple(u, vertex_value(g, u)); });

  //auto transform_incidence_edge = [&g](auto&& uv) { return std::tuple(target(g, uv), uv); };
  for (routes_vol_graph::key_type ukey = 0; auto&& u : vertices(g)) {
    os << '[' << ukey << ' ' << vertex_value(g, u) << ']' << std::endl;
    //auto vw = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    //auto x = [&g](auto&& uv) { return std::tuple(uv, target(g, uv); };

    //auto rng = edges(g, u) | views::transform([&g](auto&& uv) { return std::tuple(target(g, uv), uv); });

    for (auto&& uv : edges(g, u)) {
      auto   vkey = target_key(g, uv);
      auto&& v    = target(g, uv);
      os << "  --> [" << vkey << ' ' << vertex_value(g, v) << "] " << edge_value(g, uv) << "km" << std::endl;
    }
    ++ukey;
  }
#else
  for (typename routes_graph<G>::key_type ukey = 0; auto&& u : vertices(g)) {
    os << '[' << ukey << ' ' << vertex_value(g, u) << ']' << std::endl;
    //auto vw = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    for (auto&& uv : edges(g, u)) {
      auto   vkey = target_key(g, uv);
      auto&& v    = target(g, uv);
      os << "  --> [" << vkey << ' ' << vertex_value(g, v) << "] " << edge_value(g, uv) << "km" << std::endl;
    }
    ++ukey;
  }
#endif
  return os;
}

template <typename G>
auto load_graph(csv::string_view csv_file) {
  using namespace std::graph;

  using graph_type       = G;
  using vertex_key_type  = vertex_key_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using name_type        = vertex_value_t<graph_type>;
  using weight_type      = edge_value_t<graph_type, vertex_edge_range_t<graph_type>>;
  //static_assert(std::is_same_v<name_type, std::string>);
  //static_assert(std::is_arithmetic_v<weight_type>);

  // Scan the CSV to get the unique city names (cols 0 & 1)
  auto&& [city_names, csv_row_cnt] = unique_vertex_labels(csv_file, 0UL, 1UL);

  graph_type g;

  // Load vertices
  auto&& city_name_getter = [](auto& name) { return name; };
  g.load_vertices(city_names, city_name_getter);

  //
  auto vertex_to_name = [&g](vertex_reference u) { return vertex_value(g, u); }; // projection

  // find the vertex for the given city. The vertices are ordered by the city_name,
  // as they were loaded from the city_names vector.
  auto find_city_key = [&g, &vertex_to_name](const csv::string_view& city_name) {
    auto it = std::ranges::lower_bound(vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
    if (it != end(vertices(g)) && vertex_value(g, *it) != city_name)
      it = end(vertices(g));
    assert(it != end(vertices(g))); // city not found? unexpected
    return static_cast<vertex_key_type>(it - begin(vertices(g)));
  };

  auto ekey_fnc = [&g, &find_city_key](const csv::CSVRow& row) {
    auto from_key = find_city_key(row[0].get_sv());
    auto to_key   = find_city_key(row[1].get_sv());
    assert(from_key < size(vertices(g)) && to_key < size(vertices(g)));
    return std::pair{from_key, to_key};
  };
  auto evalue_fnc = [&g, &find_city_key](const csv::CSVRow& row) {
    auto to_key = find_city_key(row[1].get<std::string_view>());
    auto dist   = row[2].get<double>();
    assert(to_key < size(vertices(g)));
    return dist;
  };

  const vertex_key_type max_city_key = static_cast<vertex_key_type>(size(city_names)) - 1;
  csv::CSVReader        reader(csv_file); // CSV file reader
  g.load_edges(max_city_key, reader, ekey_fnc, evalue_fnc);

  return g;
}
