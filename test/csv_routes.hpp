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
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence_view.hpp"
#include <set>
#include <map>
#include <deque>
#include <algorithm>
#include <string_view>
#include <iomanip>
#include <ostream>

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

enum struct name_order_policy : int8_t {
  order_found,        // key assigned when first encountered as source or target
  source_order_found, // key assigned when first encountered as source only; names that are only targets appear at end
  alphabetical        // key assigned after all keys found, in name order
};

/// <summary>
/// Scans 2 columns in a CSV file and returns a map<string_view,size>, where the string_view is a
/// unique label in occurring in either column and size_t is it's unique key.
/// </summary>
/// <typeparam name="ColNumOrName">Column name or column number to specify the column in the CSV file</typeparam>
/// <param name="csv_file">CSV filename</param>
/// <param name="col1">First column to use</param>
/// <param name="col2">Second column to use</param>
/// <param name="order_policy">For order_policy=order_found, the label key is assigned to the row it was
/// first encountered in the first column. Labels that only occur in the second column will be assigned
/// a key that follows the other keys in the first column. For order_policy=alphabetical, the key will be
/// assigned based on the alphabetical ordering of the labels.</param>
/// <returns></returns>
template <typename ColNumOrName, typename VKey = uint32_t>
auto unique_vertex_labels2(csv::string_view        csv_file,
                           ColNumOrName            col1,
                           ColNumOrName            col2,
                           const name_order_policy order_policy) {
  csv::CSVReader reader(csv_file); // CSV file reader

  using label_key_map = std::map<std::string, VKey>; // label, vertex key
  label_key_map lbls;

  VKey row_order = 0;
  for (csv::CSVRow& row : reader) {
    std::string_view source_key = row[col1].get_sv();
    std::string_view target_key = row[col2].get_sv();
    auto&& [source_iter, source_inserted] =
          lbls.emplace(typename label_key_map::value_type(source_key, std::numeric_limits<VKey>::max()));
    auto&& [target_iter, target_inserted] =
          lbls.emplace(typename label_key_map::value_type(target_key, std::numeric_limits<VKey>::max()));

    if (order_policy == name_order_policy::order_found && source_iter->second == std::numeric_limits<VKey>::max())
      source_iter->second = row_order++;
  }

  // Assign unique keys to each label that doesn't have an order assigned yet.
  // The following behavior will occur for different order_policy:
  // 1. ==alphabetical: no order has been assigned yet and all values will be assigned to
  //    reflect the order in the map (alphabetical).
  // 2. ==order_found, then the order reflected will be defined as the first
  //    time the label was found as the source_key in the file. Anything that hasn't been
  //    assigned yet only appears in the target_key and will be assigned values at the end.
  // assign order to labels that were only targets
  for (auto&& [lbl, key] : lbls)
    if (key == std::numeric_limits<VKey>::max())
      key = row_order++;

  return std::pair(lbls, reader.n_rows());
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
  bool atEnd = (it == end(std::graph::vertices(g)));
  auto key   = it - begin(std::graph::vertices(g));
  if (it != end(std::graph::vertices(g)) && std::graph::vertex_value(g, *it) == city_name)
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

/// <summary>
/// Loads graph such that the vertices are ordered in the same order as the source_key on the edges.
/// The value of the source_key is not significant. Edges must be ordered by their source_key.
///
/// Uses 2 passes the the CSV file. The first is to get the set of unique vertex keys and to create
/// the vertices. The second pass is used to create the edges.
/// </summary>
/// <typeparam name="G"></typeparam>
/// <param name="csv_file"></param>
/// <returns></returns>
template <typename G>
auto load_graph(csv::string_view csv_file) {
  using namespace std::graph;

  using graph_type      = G;
  using vertex_key_type = vertex_key_t<graph_type>;

  const size_t col1 = 0;
  const size_t col2 = 1;

  // Scan the CSV to get the unique city names (cols 0 & 1)
  auto&& [city_names, csv_row_cnt] = unique_vertex_labels2(csv_file, col1, col2, name_order_policy::alphabetical);
  using city_key_map               = std::ranges::range_value_t<decltype(city_names)>;
  graph_type g;

  // Load vertices
  auto city_key_getter = [&city_names](const city_key_map& name_key) {
    using copyable_key_name = std::graph::views::copyable_vertex_t<vertex_key_type, std::string>;
    return copyable_key_name{name_key.second,
                             name_key.first}; // {key,name} don't move name b/c we need to keep it in the map
  };
  g.load_vertices(city_names, city_key_getter);

  // load edges
  auto eproj = [&g, col1, col2](const csv::CSVRow& row) {
    using edge_value_type    = std::remove_cvref_t<edge_value_t<G>>;
    using copyable_edge_type = views::copyable_edge_t<vertex_key_type, edge_value_type>;
    return copyable_edge_type{
          find_city_key(g, row[col1].get_sv()), // source_key
          find_city_key(g, row[col2].get_sv()), // target_key
          row[2].get<double>()                  // value (e.g. distance)
    };
  };

  csv::CSVReader reader(csv_file); // CSV file reader
  g.load_edges(reader, eproj, size(city_names), csv_row_cnt);

  return g;
}

/// <summary>
/// Loads a graph such that the vertices are ordered alphabetically by their label so that find_city()
/// can use std::lower_bound. Edges are shown in the same order as they appear in the CSV, relative
/// to the source city.
///
/// Requires a single pass throught the CSV file to build both a map of unique labels --> vertex_key,
/// and a "copy" of the rows. The rows have iterators to the unique labels for source and target keys
/// plus a copy of the values stored.
///
/// The resulting graph should give the order as load_graph(csv_file) for vertices but edges may be
/// ordered differently.
/// </summary>
/// <typeparam name="G"></typeparam>
/// <param name="csv_file"></param>
/// <returns></returns>
template <typename G>
auto load_ordered_graph(csv::string_view        csv_file,
                        const name_order_policy order_policy = name_order_policy::alphabetical) {
  using namespace std::graph;
  using std::ranges::range_value_t;
  using std::ranges::iterator_t;
  using std::string_view;
  using std::map;
  using std::vector;
  using std::deque;
  using std::move;
  using std::numeric_limits;
  using graph_type        = G;
  using vertex_key_type   = vertex_key_t<G>;
  using vertex_value_type = vertex_value_t<G>;
  using edge_value_type   = edge_value_t<G>; //std::remove_cvref<edge_value_t<G>>;

  csv::CSVReader reader(csv_file); // CSV file reader; string_views remain valid until the file is closed

  using labels_map   = map<string_view, vertex_key_type>; // label, vertex key (also output order, assigned later)
  using csv_row_type = views::copyable_edge_t<iterator_t<labels_map>, double>;
  using csv_row_deq  = deque<csv_row_type>;

  labels_map  lbls;    // unique labels for both source_key and target_key, ordered
  csv_row_deq row_deq; // all rows in the csv, though the labels only refer to entries in lbls

  // scan the CSV file. lbls has the only (unique) string_views into the file.
  vertex_key_type row_order = 0;
  for (csv::CSVRow& row : reader) {
    string_view source_key = row[0].get_sv();
    string_view target_key = row[1].get_sv();
    double      value      = row[2].get<double>();
    auto&& [source_iter, source_inserted] =
          lbls.emplace(range_value_t<labels_map>(source_key, numeric_limits<vertex_key_type>::max()));
    auto&& [target_iter, target_inserted] =
          lbls.emplace(range_value_t<labels_map>(target_key, numeric_limits<vertex_key_type>::max()));

    if (order_policy == name_order_policy::order_found || order_policy == name_order_policy::source_order_found) {
      if (source_iter->second == numeric_limits<vertex_key_type>::max())
        source_iter->second = row_order++;
      if (order_policy != name_order_policy::source_order_found &&
          target_iter->second == numeric_limits<vertex_key_type>::max())
        target_iter->second = row_order++;
    }

    row_deq.push_back({source_iter, target_iter, value});
  }

  // Assign unique vertex keys to each label (assigned in label order)
  // This only occurs for vertices where target_key is never a source
  for (auto&& [lbl, key] : lbls)
    if (key == std::numeric_limits<vertex_key_type>::max())
      key = row_order++;

  // Sort the rows based on the source_key, using it's key just assigned
  // row order is preserved within the same source_key (this should give the same order as load_ordered_graph)
  std::ranges::sort(row_deq, [](const csv_row_type& lhs, const csv_row_type& rhs) {
    return std::tie(lhs.source_key->second, lhs.target_key->second) <
           std::tie(rhs.source_key->second, rhs.target_key->second);
  });

  // Create sorted list of iterators to the cities/labels using the row_order value assigned
  using lbl_iter     = std::ranges::iterator_t<labels_map>;
  using lbl_iter_vec = vector<lbl_iter>;
  lbl_iter_vec ordered_cities;
  ordered_cities.reserve(lbls.size());
  for (auto it = lbls.begin(); it != lbls.end(); ++it)
    ordered_cities.emplace_back(it);
  std::ranges::sort(ordered_cities,
                    [](const lbl_iter& lhs, const lbl_iter& rhs) -> bool { return lhs->second < rhs->second; });

  // Create an empty graph
  graph_type g;

  // load vertices
  using copyable_label        = std::remove_reference_t<vertex_value_type>;
  using graph_copyable_vertex = std::graph::views::copyable_vertex_t<vertex_key_type, copyable_label>;
  auto city_name_getter       = [](lbl_iter& lbl) {
    graph_copyable_vertex retval{lbl->second, copyable_label(lbl->first)};
    return retval;
  };
  g.load_vertices(ordered_cities, city_name_getter);

  // load edges
  auto eproj = [&g](csv_row_type& row) {
    using graph_copyable_edge_type = views::copyable_edge_t<vertex_key_type, edge_value_type>;
    graph_copyable_edge_type retval{static_cast<vertex_key_type>(row.source_key->second),
                                    static_cast<vertex_key_type>(row.target_key->second), row.value};
    return retval;
  };
  g.load_edges(row_deq, eproj, lbls.size(), row_deq.size());

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
  for (auto&& [ukey, u] : views::vertexlist(g)) {
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


void        utf8_append(std::string& out, const char ch);
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

template <class OS>
OS& operator<<(OS& os, const ostream_indenter& indent) {
  for (int i = 0; i < indent.level(); ++i)
    os << "  ";
  return os;
}


/// <summary>
/// Outputs a graphviz file for the routes graph passed.
///
/// Example command line to generate image files for file "routes.gv"
/// dot -Tpdf -O routes.gv
/// dot -Tpng -O routes.gv
/// neato -Tpng -O routes.gv
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <param name="g">Grape instance</param>
/// <param name="filename">Graphviz filename to output</param>
template <class G>
void output_routes_graphviz(const G& g, std::string_view filename) {
  using namespace std::graph;
  std::ofstream of(filename.data());
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  splines = curved\n";

  for (auto&& [ukey, u] : views::vertexlist(g)) {
    of << "  " << ukey << " [shape=oval,label=\"" << vertex_value(g, u) << " [" << ukey << "]\"]\n";
    for (auto&& [vkey, uv] : views::edges_view(g, u)) {
      auto&& v = target(g, uv);
      of << "   " << ukey << " -> " << vkey << " [arrowhead=vee,xlabel=\"" << edge_value(g, uv)
         << " km\", fontcolor=blue]\n";
    }
    of << std::endl;
  }
  of << "}\n";
}


/// <summary>
/// Generates code that can be used for unit tests to validate graph contents haven't changed.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <param name="g">Graph instance</param>
/// <param name="name">Descriptive name of the graph</param>
template <class G>
void generate_routes_tests(const G& g, std::string_view name) {
  using namespace std::graph;
  using std::cout;
  using std::endl;
  ostream_indenter indent;
  cout << endl << indent << "auto ui = begin(vertices(g));" << endl;
  cout << indent << "vertex_key_t<G> ukey = 0;" << endl;
  for (vertex_key_t<G> ukey = 0; auto&& u : vertices(g)) {

    if (ukey > 0) {
      cout << indent << "if(++ui != end(vertices(g))) {" << endl;
    } else {
      cout << indent << "if(ui != end(vertices(g))) {" << endl;
    }
    ++indent;
    {
      if (ukey > 0)
        cout << indent << "REQUIRE(" << ukey << " == ++ukey);" << endl;
      else
        cout << indent << "REQUIRE(" << ukey << " == ukey);" << endl;

      size_t uv_cnt = 0;
      cout << indent << "REQUIRE(\"" << quoted_utf8(vertex_value(g, u)) << "\" == vertex_value(g,*ui));" << endl;
      cout << endl << indent << "auto uvi = begin(edges(g, *ui)); size_t uv_cnt = 0;" << endl;
      for (auto&& uv : edges(g, u)) {
        if (uv_cnt > 0) {
          cout << endl << indent << "++uvi;" << endl;
        }
        auto&& v = target(g, uv);
        cout << indent << "REQUIRE(" << target_key(g, uv) << " == target_key(g, *uvi));\n";
        cout << indent << "REQUIRE(\"" << quoted_utf8(vertex_value(g, target(g, uv)))
             << "\" == vertex_value(g, target(g, *uvi)));\n";
        cout << indent << "REQUIRE(" << edge_value(g, uv) << " == edge_value(g,*uvi));\n";
        cout << indent << "++uv_cnt;" << endl;
        ++uv_cnt;
      }
      cout << endl << indent << "REQUIRE(" << uv_cnt << " == uv_cnt);" << endl;
    }
    cout << "}" << endl;
    --indent;
    ++ukey;
  }

  cout << endl
       << indent << "REQUIRE(" << size(vertices(g)) << " == size(vertices(g))); // all vertices visited?" << endl;
}
