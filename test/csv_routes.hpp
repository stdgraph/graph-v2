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
#include "graph/view/vertices_view.hpp"
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


template <typename G>
std::optional<std::graph::vertex_iterator_t<G>> find_city(G&& g, std::string_view city_name) {
  auto vertex_to_name = [&g](std::graph::vertex_reference_t<G> u) { return std::graph::vertex_value<G>(g, u); };
  auto it = std::ranges::lower_bound(std::graph::vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
  if (it != end(std::graph::vertices(g)) && std::graph::vertex_value(g, *it) != city_name)
    it = end(std::graph::vertices(g));
  if (it != end(std::graph::vertices(g)))
    return std::optional<std::graph::vertex_iterator_t<G>>(it);
  return std::optional<std::graph::vertex_iterator_t<G>>();
}

template <typename G>
std::graph::vertex_key_t<G> find_city_key(G&& g, std::string_view city_name) {
  auto vertex_to_name = [&g](std::graph::vertex_reference_t<G> u) { return std::graph::vertex_value<G>(g, u); };
  auto it = std::ranges::lower_bound(std::graph::vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
  if (it != end(std::graph::vertices(g)) && std::graph::vertex_value(g, *it) != city_name)
    it = end(std::graph::vertices(g));
  return static_cast<std::graph::vertex_key_t<G>>(it -
                                                  begin(std::graph::vertices(g))); // == size(vertices(g)) if not found
}

template <typename G>
auto load_graph(csv::string_view csv_file) {
  using namespace std::graph;

  using graph_type       = G;
  using vertex_key_type  = vertex_key_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;

  // Scan the CSV to get the unique city names (cols 0 & 1)
  auto&& [city_names, csv_row_cnt] = unique_vertex_labels(csv_file, 0UL, 1UL);

  graph_type g;

  // Load vertices, moving city name to vertex for ownership
  auto city_name_getter = [](auto&& name) -> std::string&& { return std::move(name); };
  g.load_vertices(city_names, city_name_getter);

  // load edges
  auto vertex_to_name = [&g](vertex_reference u) { return vertex_value(g, u); }; // projection

  auto ekey_fnc = [&g](const csv::CSVRow& row) { // get edge key
    auto from_key = find_city_key(g, row[0].get_sv());
    auto to_key   = find_city_key(g, row[1].get_sv());
    assert(from_key < size(vertices(g)) && to_key < size(vertices(g)));
    return std::pair{from_key, to_key};
  };
  auto evalue_fnc = [&g](const csv::CSVRow& row) { // get edge weight
    auto to_key = find_city_key(g, row[1].get<std::string_view>());
    auto dist   = row[2].get<double>();
    assert(to_key < size(vertices(g)));
    return dist;
  };

  const vertex_key_type max_city_key = static_cast<vertex_key_type>(size(city_names)) - 1;
  csv::CSVReader        reader(csv_file); // CSV file reader
  g.load_edges(max_city_key, reader, ekey_fnc, evalue_fnc);

  return g;
}

template <typename G>
struct routes_graph {
  const G& g;
  routes_graph(const G& graph) : g(graph) {}
  routes_graph(const routes_graph&) = default;
};

template <typename OStream, typename G>
OStream& operator<<(OStream& os, const routes_graph<G>& graph) {
  using namespace std::graph;
  const auto& g = graph.g;

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
  for (auto&& [ukey, u] : vertices_view(g)) {
    os << '[' << ukey << ' ' << vertex_value(g, u) << ']' << std::endl;
    //auto vw = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    for (auto&& uv : edges(g, u)) {
      auto   vkey = target_key(g, uv);
      auto&& v    = target(g, uv);
      os << "  --> [" << vkey << ' ' << vertex_value(g, v) << "] " << edge_value(g, uv) << "km" << std::endl;
    }
  }
#endif
  return os;
}


void utf8_append(std::string& out, const char ch);
std::string quoted_utf8(const std::string& s);
std::string quoted_utf8(const std::string_view& s);
std::string quoted_utf8(const char* s);

class ostream_indenter {
public:
  ostream_indenter(int level) : level_(level) {}
  ostream_indenter()                        = default;
  ostream_indenter(const ostream_indenter&) = default;
  ~ostream_indenter()                       = default;

  ostream_indenter& operator=(const ostream_indenter&) = default;

  int level() const { return level_; }

  ostream_indenter& operator++() {
    ++level_;
    return *this;
  }
  ostream_indenter operator++(int) {
    ostream_indenter tmp(*this);
    ++*this;
    return tmp;
  }
  ostream_indenter& operator--() {
    --level_;
    return *this;
  }
  ostream_indenter operator--(int) {
    ostream_indenter tmp(*this);
    --*this;
    return tmp;
  }

private:
  int level_ = 0;
};
