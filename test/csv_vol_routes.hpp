#pragma once
#include "csv_routes.hpp"
#include "graph/container/vol.hpp"


class routes_vol_graph : public routes_base<uint32_t> {
public:
  using base_type   = routes_base<uint32_t>;
  using key_type    = base_type::key_type;
  using name_type   = base_type::name_type;
  using weight_type = double;

  using graph_type = std::graph::container::vol<weight_type, void, void, key_type>;

public: // Construction/Destruction/Assignment
  routes_vol_graph(csv::string_view csv_file) : routes_base<uint32_t>(csv_file), g_(load_routes(csv_file)) {}

  routes_vol_graph()                        = default;
  routes_vol_graph(const routes_vol_graph&) = default;
  routes_vol_graph(routes_vol_graph&&)      = default;
  ~routes_vol_graph()                       = default;

  routes_vol_graph& operator=(const routes_vol_graph&) = default;
  routes_vol_graph& operator=(routes_vol_graph&&) = default;

public: // Properties
  constexpr graph_type&       graph() { return g_; }
  constexpr const graph_type& graph() const { return g_; }

public: // Operations
  // used to visually validate core functionality
  void output_routes() {}


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
    graph_type cc(reader, ekey_fnc, evalue_fnc);
#if 0
    for (CSVRow& row : reader()) { // Input iterator-ish
      string_view from = row[0].get<string_view>();
      string_view to   = row[1].get<string_view>();
      weight_type dist = row[2].get<weight_type>();
    }
#endif
    return cc;
  }

private:         // Member Variables
  graph_type g_; // CSR graph of routes
};
