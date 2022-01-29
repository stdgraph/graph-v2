#pragma once
#include "csv_routes.hpp"
#include "graph/container/dynamic_graph.hpp"
#include <iomanip>

class routes_vol_graph : public routes_base<uint32_t> {
public:
  using base_type                      = routes_base<uint32_t>;
  using key_type                       = base_type::key_type;
  constexpr inline static bool sourced = false;
  using name_view                      = std::string_view;
  using weight_type                    = double;

  using graph_traits = std::graph::container::vofl_graph_traits<weight_type, name_view, void, sourced, key_type>;
  using graph_type   = std::graph::container::dynamic_adjacency_graph<graph_traits>;

public: // Construction/Destruction/Assignment
  routes_vol_graph(csv::string_view csv_file) : base_type(csv_file), g_(load_routes(csv_file)) {}

  routes_vol_graph()                        = default;
  routes_vol_graph(const routes_vol_graph&) = default;
  routes_vol_graph(routes_vol_graph&&)      = default;
  ~routes_vol_graph()                       = default;

  routes_vol_graph& operator=(const routes_vol_graph&) = default;
  routes_vol_graph& operator=(routes_vol_graph&&) = default;

public: // Properties
  constexpr graph_type&       graph() { return g_; }
  constexpr const graph_type& graph() const { return g_; }

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

template <typename OStream>
OStream& operator<<(OStream& os, const routes_vol_graph& graph) {
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
  for (routes_vol_graph::key_type ukey = 0; auto&& u : vertices(g)) {
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

#ifdef FUTURE
// This is an attempt to consolidate routes_vol_graph & routes_base<uint32_t> into a single function.
// The intent was to move the city_names strings into the vertex nodes as the vertices were added,
// and then to have the edges use those. It's close-ish, but the code to move the names is having an
// issue.

TEST_CASE("Germany routes CSV+vol test", "[csv][vol][germany][freefnc]") {
  auto g = load_vol_graph<uint32_t>(TEST_DATA_ROOT_DIR "germany_routes.csv");
}

template <typename VKey>
auto load_vol_graph(csv::string_view csv_file) {
  using key_type    = VKey;
  using name_type   = std::string;
  using weight_type = double;
  using graph_type  = std::graph::container::vol_graph<weight_type, name_type, void, key_type>;

  using namespace std::graph;

  graph_type g;

  // Scan the CSV to get the unique city names (cols 0 & 1)
  auto&& [city_names, row_cnt] = unique_vertex_labels(csv_file, 0UL, 1UL);

  // Load the vertices
  std::allocator<char> alloc;
  auto&&               vvalue_fnc = [](std::string&& name) {
    return std::move(name);
  }; // moves city names into the vertices of the graph
  g.load_vertices(city_names, vvalue_fnc, alloc);

  auto vertex_to_name = [&g](std::graph::vertex_reference_t<graph_type> u) {
    const std::string& name = vertex_value(g, u);
    return std::string_view(name);
  }; // projection

  // find the vertex for the given city. The vertices are ordered by the city_name,
  // as they were loaded from the city_names vector.
  auto find_city_key = [&g, &vertex_to_name](const csv::string_view& city_name) {
    auto it = std::ranges::lower_bound(vertices(g), city_name, std::less<std::string_view>(), vertex_to_name);
    if (it != end(vertices(g)) && vertex_value(g, *it) != city_name)
      it = end(vertices(g));
    assert(it != end(vertices(g))); // unexpected
    return static_cast<key_type>(it - begin(vertices(g)));
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

  const key_type max_city_key = static_cast<key_type>(size(city_names)) - 1;
  csv::CSVReader reader(csv_file); // CSV file reader
  g.load_edges(max_city_key, reader, ekey_fnc, evalue_fnc, alloc);

  return g;
}
#endif // FUTURE
