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

#include "csv.hpp"

#ifdef _MSC_VER
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/depth_first_search.hpp"
#include "graph/views//depth_first_search.hpp"
#include <set>
#include <map>
#include <deque>
#include <algorithm>
#include <string_view>
#include <iomanip>
#include <ostream>
#include <optional>

void init_console(); // init cout for UTF-8

/**
 * @brief Scans 2 columns in a CSV file and returns all the unique values in an ordered vector.
 * 
 * This is used to get all the unique labels for vertices.
 *
 * @tparam ColNumOrName A name or integer for a column in the CSV file
 * 
 * @param csv_file  The CSV file name (path)
 * @param col1      The first column to get labels from. If the column doesn't exist the function is undefined.
 * @param col2      The second column to get labels from. If the column doesn't exist the function is undefined.
 * 
 * @return @c pair<vector<string>,size_t> as unique labels, number of rows read
*/
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

enum struct directedness : int8_t {
  directed,   // a single edge joins 2 vertices
  directed2,  // 2 edges join 2 vertices, each with different directions; needed for graphviz
  undirected, // one or more edges exist between vertices with no direction
  bidirected  // a single edge between vertices with direction both ways (similar to undirected, but with arrows)
};

enum struct name_order_policy : int8_t {
  order_found,        // id assigned when first encountered as source or target
  source_order_found, // id assigned when first encountered as source only; names that are only targets appear at end
  alphabetical        // id assigned after all ids found, in name order
};

/**
 * @brief Scans 2 columns in a CSV file and returns a @c map<string_view,size>, where the @c string_view is a
 *        unique label in occurring in either column and @c size_t is it's unique id.
 *
 * @tparam ColNumOrName Column name or column number to specify the column in the CSV file
 * 
 * @param csv_file      CSV filename
 * @param col1          First column to use
 * @param col2          Second column to use
 * @param order_policy  For @c order_policy=order_found, the label id is assigned to the row it was first 
 *                      encountered in the first column. Labels that only occur in the second column will 
 *                      be assigned a id that follows the other ids in the first column. For 
 *                      @c order_policy=alphabetical, the id will be assigned based on the alphabetical 
 *                      ordering of the labels.
 *
 * @return @c pair<vector<string>,size_t> as unique labels, number of rows read
*/
template <typename ColNumOrName, typename VId = uint32_t>
auto unique_vertex_labels2(csv::string_view        csv_file,
                           ColNumOrName            col1,
                           ColNumOrName            col2,
                           const name_order_policy order_policy) {
  csv::CSVReader reader(csv_file); // CSV file reader

  using label_id_map = std::map<std::string, VId>; // label, vertex id
  label_id_map lbls;

  VId row_order = 0;
  for (csv::CSVRow& row : reader) {
    std::string_view source_id = row[col1].get_sv();
    std::string_view target_id = row[col2].get_sv();
    auto&& [source_iter, source_inserted] =
          lbls.emplace(typename label_id_map::value_type(source_id, std::numeric_limits<VId>::max()));
    auto&& [target_iter, target_inserted] =
          lbls.emplace(typename label_id_map::value_type(target_id, std::numeric_limits<VId>::max()));

    if (order_policy == name_order_policy::order_found && source_iter->second == std::numeric_limits<VId>::max())
      source_iter->second = row_order++;
  }

  // Assign unique ids to each label that doesn't have an order assigned yet.
  // The following behavior will occur for different order_policy:
  // 1. ==alphabetical: no order has been assigned yet and all values will be assigned to
  //    reflect the order in the map (alphabetical).
  // 2. ==order_found, then the order reflected will be defined as the first
  //    time the label was found as the source_id in the file. Anything that hasn't been
  //    assigned yet only appears in the target_id and will be assigned values at the end.
  // assign order to labels that were only targets
  for (auto&& [lbl, id] : lbls)
    if (id == std::numeric_limits<VId>::max())
      id = row_order++;

  return std::pair(lbls, reader.n_rows());
}


#ifdef FUTURE
/**
 * @brief Gets the maximum value of two columns in a CSV file with integral values.
 *
 * @tparam T            The integer type that is used to hold the maximum value found
 * @tparam ColNumOrName A name or integer for a column in the CSV file
 * 
 * @param csv_file  The CSV file name (path)
 * @param col1      The first column to get labels from. If the column doesn't exist the function is undefined
 * @param col2      The second column to get labels from. If the column doesn't exist the function is undefined
 * 
 * @return pair<T, size_t> as max_id, number of rows read
*/
template <std::integral T, typename ColNumOrName>
auto max_vertex_id(csv::string_view csv_file, ColNumOrName col1, ColNumOrName col2) {
  csv::CSVReader reader(csv_file); // CSV file reader
  T              max_id = std::numeric_limits<T>::min();
  for (csv::CSVRow& row : reader)
    max_id = std::max(max_id, std::max(row[col1].get<T>(), row[col2].get<T>()));

  return std::pair(max_id, reader.n_rows()); // return (max_id, num rows read)
}
#endif // FUTURE


template <typename G>
std::optional<graph::vertex_iterator_t<G>> find_city(G&& g, std::string_view city_name) {
#if 1
  auto it = std::ranges::find_if(graph::vertices(g),
                                 [&g, &city_name](auto& u) { return graph::vertex_value(g, u) == city_name; });
  if (it != end(graph::vertices(g)))
    return std::optional<graph::vertex_iterator_t<G>>(it);
  return std::optional<graph::vertex_iterator_t<G>>();
#else
  auto vertex_to_name = [&g](graph::vertex_reference_t<G> u) { return graph::vertex_value<G>(g, u); };
  auto it    = std::ranges::lower_bound(graph::vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
  bool atEnd = (it == end(graph::vertices(g)));
  auto id    = it - begin(graph::vertices(g));
  if (it != end(graph::vertices(g)) && graph::vertex_value(g, *it) == city_name)
    return std::optional<graph::vertex_iterator_t<G>>(it);
  return std::optional<graph::vertex_iterator_t<G>>();
#endif
}

template <typename G>
graph::vertex_id_t<G> find_city_id(G&& g, std::string_view city_name) {
#if 1
  auto it = std::ranges::find_if(graph::vertices(g),
                                 [&g, &city_name](auto& u) { return graph::vertex_value(g, u) == city_name; });
#else
  auto vertex_to_name = [&g](graph::vertex_reference_t<G> u) { return graph::vertex_value<G>(g, u); };
  auto it = std::ranges::lower_bound(graph::vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
  if (it != end(graph::vertices(g)) && graph::vertex_value(g, *it) != city_name)
    it = end(graph::vertices(g));
#endif
  return static_cast<graph::vertex_id_t<G>>(it - begin(graph::vertices(g))); // == size(vertices(g)) if not found
}

/**
 * @brief Loads graph such that the vertices are ordered in the same order as the source_id on the edges.
 * 
 * The value of the source_id is not significant. Edges must be ordered by their source_id.
 *
 * Uses 2 passes the the CSV file. The first is to get the set of unique vertex ids and to create
 * the vertices. The second pass is used to create the edges.
 *
 * @tparam G Graph type
 * 
 * @param csv_file CSV filename
 * 
 * @return A new graph of type G with vertices and edges defined by @c csv_file.
*/
template <typename G>
auto load_graph(csv::string_view csv_file) {
  using namespace graph;

  using graph_type     = G;
  using vertex_id_type = vertex_id_t<graph_type>;

  const size_t col1 = 0;
  const size_t col2 = 1;

  // Scan the CSV to get the unique city names (cols 0 & 1)
  auto&& [city_names, csv_row_cnt] = unique_vertex_labels2(csv_file, col1, col2, name_order_policy::alphabetical);
  using city_id_map                = std::ranges::range_value_t<decltype(city_names)>;
  graph_type g;

  // Load vertices
  auto&& cnames         = city_names; // Clang-15 not finding city_names for following capture
  auto   city_id_getter = [&cnames](const city_id_map& name_id) {
    using copyable_id_name = graph::copyable_vertex_t<vertex_id_type, std::string>;
    return copyable_id_name{name_id.second,
                            name_id.first}; // {id,name} don't move name b/c we need to keep it in the map
  };
  g.load_vertices(city_names, city_id_getter);

  // load edges
  auto eproj = [&g, col1, col2](const csv::CSVRow& row) {
    using edge_value_type    = std::remove_cvref_t<edge_value_t<G>>;
    using copyable_edge_type = copyable_edge_t<vertex_id_type, edge_value_type>;
    return copyable_edge_type{
          find_city_id(g, row[col1].get_sv()), // source_id
          find_city_id(g, row[col2].get_sv()), // target_id
          row[2].get<double>()                 // value (e.g. distance)
    };
  };

  csv::CSVReader reader(csv_file); // CSV file reader
  g.load_edges(reader, eproj, size(city_names), csv_row_cnt);

  return g;
}

/**
 * @brief Loads a graph such that the vertices are ordered alphabetically by their label so that 
 *        @c find_city() can use @c std::lower_bound(). 
 *
 * Edges are shown in the same order as they appear in the CSV, relative to the source city.
 *
 * Requires a single pass throught the CSV file to build both a map of unique labels --> vertex_id,
 * and a "copy" of the rows. The rows have iterators to the unique labels for source and target ids
 * plus a copy of the values stored.
 *
 * The resulting graph should give the order as @c load_graph(csv_file) for vertices but edges may be
 * ordered differently.
 * 
 * @tparam G Graph type
 * 
 * @param csv_file              CSV filename
 * @param order_policy          How should id's be defined? When a vertex name is first encountered or 
 *                              overall alphabetical order?
 * @param add_reversed_src_tgt  Should an edge be duplicated with its source_id and target_id reversed?
 *                              Useful to represent unordered graphs.
 * 
 * @return A new graph of type G with vertices and edges defined by @c csv_file.
*/
template <typename G>
auto load_ordered_graph(csv::string_view        csv_file,
                        const name_order_policy order_policy         = name_order_policy::alphabetical,
                        const bool              add_reversed_src_tgt = false) {
  using namespace graph;
  using std::ranges::range_value_t;
  using std::ranges::iterator_t;
  using std::string_view;
  using std::map;
  using std::vector;
  using std::deque;
  using std::move;
  using std::numeric_limits;
  using graph_type        = G;
  using vertex_id_type    = vertex_id_t<G>;
  using vertex_value_type = vertex_value_t<G>;
  using edge_value_type   = edge_value_t<G>; //std::remove_cvref<edge_value_t<G>>;

  //assert(!add_reversed_src_tgt || (add_reversed_src_tgt && order_policy != name_order_policy::source_order_found));

  csv::CSVReader reader(csv_file); // CSV file reader; string_views remain valid until the file is closed

  using labels_map   = map<string_view, vertex_id_type>; // label, vertex id (also output order, assigned later)
  using csv_row_type = copyable_edge_t<iterator_t<labels_map>, double>;
  using csv_row_deq  = deque<csv_row_type>;

  labels_map  lbls;    // unique labels for both source_id and target_id, ordered
  csv_row_deq row_deq; // all rows in the csv, though the labels only refer to entries in lbls

  // scan the CSV file. lbls has the only (unique) string_views into the file.
  vertex_id_type row_order = 0;
  for (csv::CSVRow& row : reader) {
    string_view source_id = row[0].get_sv();
    string_view target_id = row[1].get_sv();
    double      value     = row[2].get<double>();
    auto&& [source_iter, source_inserted] =
          lbls.emplace(range_value_t<labels_map>(source_id, numeric_limits<vertex_id_type>::max()));
    auto&& [target_iter, target_inserted] =
          lbls.emplace(range_value_t<labels_map>(target_id, numeric_limits<vertex_id_type>::max()));

    if (order_policy == name_order_policy::order_found || order_policy == name_order_policy::source_order_found) {
      if (source_iter->second == numeric_limits<vertex_id_type>::max())
        source_iter->second = row_order++;
      if (order_policy != name_order_policy::source_order_found &&
          target_iter->second == numeric_limits<vertex_id_type>::max())
        target_iter->second = row_order++;
    }

    row_deq.push_back({source_iter, target_iter, value});
    if (add_reversed_src_tgt)
      row_deq.push_back({target_iter, source_iter, value});
  }

  // Assign unique vertex ids to each label (assigned in label order)
  // This only occurs for vertices where target_id is never a source
  for (auto&& [lbl, id] : lbls)
    if (id == std::numeric_limits<vertex_id_type>::max())
      id = row_order++;

  // Sort the rows based on the source_id, using it's id just assigned
  // row order is preserved within the same source_id (this should give the same order as load_ordered_graph)
  std::ranges::sort(row_deq, [](const csv_row_type& lhs, const csv_row_type& rhs) {
    return std::tie(lhs.source_id->second, lhs.target_id->second) <
           std::tie(rhs.source_id->second, rhs.target_id->second);
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
  using graph_copyable_vertex = graph::copyable_vertex_t<vertex_id_type, copyable_label>;
  auto city_name_getter       = [](lbl_iter& lbl) {
    graph_copyable_vertex retval{lbl->second, copyable_label(lbl->first)};
    return retval;
  };
  g.load_vertices(ordered_cities, city_name_getter);

  // load edges
  auto eproj = [](csv_row_type& row) {
    using graph_copyable_edge_type = copyable_edge_t<vertex_id_type, edge_value_type>;
    graph_copyable_edge_type retval{static_cast<vertex_id_type>(row.source_id->second),
                                    static_cast<vertex_id_type>(row.target_id->second), row.value};
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
  using namespace graph;
  const auto& g = graph.g;

#if 0
  // experiment with transform
  using namespace std::ranges;
  auto rng = vertices(g) | views::transform([&g](auto&& u) { return std::tuple(u, vertex_value(g, u)); });

  //auto transform_incidence_edge = [&g](auto&& uv) { return std::tuple(target(g, uv), uv); };
  for (routes_vol_graph::id_type uid = 0; auto&& u : vertices(g)) {
    os << '[' << uid << ' ' << vertex_value(g, u) << ']' << std::endl;
    //auto vw = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    //auto x = [&g](auto&& uv) { return std::tuple(uv, target(g, uv); };

    //auto rng = edges(g, u) | views::transform([&g](auto&& uv) { return std::tuple(target(g, uv), uv); });

    for (auto&& uv : edges(g, u)) {
      auto   vid = target_id(g, uv);
      auto&& v    = target(g, uv);
      os << "  --> [" << vid << ' ' << vertex_value(g, v) << "] " << edge_value(g, uv) << "km" << std::endl;
    }
    ++uid;
  }
#else
  for (auto&& [uid, u] : views::vertexlist(g)) {
    os << '[' << uid << ' ' << vertex_value(g, u) << ']' << std::endl;
    //auto vw = std::ranges::transform_view(edges(g, u), transform_incidence_edge);
    for (auto&& uv : edges(g, u)) {
      auto   vid = target_id(g, uv);
      auto&& v   = target(g, uv);
      os << "  --> [" << vid << ' ' << vertex_value(g, v) << "] " << edge_value(g, uv) << "km" << std::endl;
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
  explicit ostream_indenter(int level) : level_(level) {}
  explicit ostream_indenter(size_t level) : level_(static_cast<int>(level)) {}
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


/**
 * @brief Outputs a graphviz file for the routes graph passed.
 *
 * Example command line to generate image files for file "routes.gv"
 * dot -Tpdf -O routes.gv
 * dot -Tpng -O routes.gv
 * neato -Tpng -O routes.gv
 *
 * @tparam G Graph type
 * 
 * @param g     Graph instance
 * @filename    Filename to write to. If the file already exists it will be overwritten.
*/
template <class G>
void output_routes_graphviz(
      const G&           g,
      std::string_view   filename,
      const directedness dir,
      std::string_view   bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace graph;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help
  std::string_view arrows, rev_arrows = "dir=back,arrowhead=vee,";
  switch (dir) {
  case directedness::bidirected: arrows = "dir=both,arrowhead=vee,arrowtail=vee"; break;
  case directedness::directed: arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::directed2: arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::undirected: arrows = "dir=none"; break;
  }

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  splines = curved\n"
     << "  node[shape=oval]\n"
     << "  edge[" << arrows << ", fontcolor=blue]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";


  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [label=\"" << vertex_value(g, u) << " [" << uid << "]\"]\n";
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      auto&&           v   = target(g, uv);
      std::string_view arw = (dir == directedness::directed2 && vid < uid) ? rev_arrows : "";
      of << "   " << uid << " -> " << vid << " [" << arw << "xlabel=\"" << edge_value(g, uv) << " km\"]\n";
    }
    of << std::endl;
  }
  of << "}\n";
}

template <class G>
void output_routes_graphviz_adjlist(
      const G&         g,
      std::string_view filename,
      std::string_view bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace graph;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  graph[rankdir=LR]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [shape=Mrecord, label=\"{<f0>" << uid << "|<f1>" << vertex_value(g, u) << "}\"]\n";
    std::string from = std::to_string(uid);
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      auto&&      v  = target(g, uv);
      std::string to = "e"s + std::to_string(uid) + "_"s + std::to_string(vid);
      of << "    " << to << " [shape=record, label=\"{<f0>" << vid << "|<f1>" << edge_value(g, uv) << "km}\"]\n";
      of << "    " << from << " -> " << to;
      from = to;
    }
    of << std::endl;
  }
  of << "}\n";
}


template <class G>
void output_routes_graphviz_dfs_vertices(
      const G&              g,
      std::string_view      filename,
      graph::vertex_id_t<G> seed,
      std::string_view bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace graph;
  using namespace graph::views;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help
  std::vector<bool> visited;
  visited.resize(size(vertices(g)));

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  graph[rankdir=LR]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  // output seed
  of << "  " << seed << " [shape=Mrecord, label=\"{<f0>" << seed << "|<f1>" << vertex_value(g, *find_vertex(g, seed))
     << "}\"]\n";

  // output descendents
  for (auto&& [uid, vid, uv] : graph::views::sourced_edges_depth_first_search(g, seed)) {
    // Output newly discovered vertex
    if (!visited[vid]) {
      of << "  " << vid << " [shape=Mrecord, label=\"{<f0>" << vid << "|<f1>" << vertex_value(g, *find_vertex(g, uid))
         << "}\"]\n";
      visited[vid] = true;
    }
    of << "  " << uid << " -> " << vid << "\n"; // output the edge
  }
  of << "}\n";
}


/**
 * @brief Generates code that can be used for unit tests to validate graph contents haven't changed.
 * 
 * @tparam G Graph type
 * 
 * @param g     Graph instance
 * @param name  Descriptive name of the graph
*/
template <class G>
void generate_routes_tests(const G& g, std::string_view name) {
  using namespace graph;
  using std::cout;
  using std::endl;
  ostream_indenter indent;
  cout << endl << indent << "auto ui = begin(vertices(g));" << endl;
  cout << indent << "vertex_id_t<G> uid = 0;" << endl;
  for (vertex_id_t<G> uid = 0; auto&& u : vertices(g)) {

    if (uid > 0) {
      cout << indent << "if(++ui != end(vertices(g))) {" << endl;
    } else {
      cout << indent << "if(ui != end(vertices(g))) {" << endl;
    }
    ++indent;
    {
      if (uid > 0)
        cout << indent << "REQUIRE(" << uid << " == ++uid);" << endl;
      else
        cout << indent << "REQUIRE(" << uid << " == uid);" << endl;

      size_t uv_cnt = 0;
      cout << indent << "REQUIRE(\"" << quoted_utf8(vertex_value(g, u)) << "\" == vertex_value(g,*ui));" << endl;
      cout << endl << indent << "auto uvi = begin(edges(g, *ui)); size_t uv_cnt = 0;" << endl;
      for (auto&& uv : edges(g, u)) {
        if (uv_cnt > 0) {
          cout << endl << indent << "++uvi;" << endl;
        }
        auto&& v = target(g, uv);
        cout << indent << "REQUIRE(" << target_id(g, uv) << " == target_id(g, *uvi));\n";
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
    ++uid;
  }

  cout << endl
       << indent << "REQUIRE(" << size(vertices(g)) << " == size(vertices(g))); // all vertices visited?" << endl;
}
