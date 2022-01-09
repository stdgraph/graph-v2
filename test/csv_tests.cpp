#include <catch2/catch.hpp>
#include "csv_parser.hpp"
#include "graph/container/csr_adjacency.hpp"
#include <set>
#include <algorithm>
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using namespace csv;
using std::string;
using std::string_view;
using std::cout;
using std::endl;
using std::move;
using std::ranges::iterator_t;

void init_console() {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8); // Change console from OEM to UTF-8 in Windows
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
}

template <typename KeyT>
class routes_base {
public: // types
  using key_type   = KeyT;
  using name_type  = std::string;
  using cities_vec = std::vector<name_type>;

public: // Construction/Destruction/Assignment
  routes_base(csv::string_view csv_file) { load_cities(csv_file); }
  routes_base()                        = delete;
  routes_base(const routes_base&)      = delete;
  constexpr routes_base(routes_base&&) = default;
  ~routes_base()                       = default;

  routes_base&           operator=(const routes_base&) = delete;
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

private: // Operations
  void load_cities(csv::string_view csv_file) {
    CSVReader reader(csv_file);                 // CSV file reader
    assert(reader.get_col_names().size() >= 2); // expecting from_city, to_city [, weight]

    // build set of unique city names
    // string_views remain valid while reader is open
    std::set<string_view> city_set;
    for (CSVRow& row : reader) { // Input iterator
      string_view from = row[0].get<string_view>();
      string_view to   = row[1].get<string_view>();
      city_set.insert(from);
      city_set.insert(to);
    }

    // Preserve the city names in an ordered vector
    cities_.reserve(city_set.size());
    for (const string_view& city_name : city_set)
      cities_.emplace_back(city_name);
  }

private:              // Member Variables
  cities_vec cities_; ///< Ordered city names
};


class routes_csr_graph : public routes_base<uint32_t> {
public:
  using base_type   = routes_base<uint32_t>;
  using key_type    = base_type::key_type;
  using name_type   = base_type::name_type;
  using weight_type = double;

  struct route {
    key_type    target = 0;
    weight_type weight = 0;
    route(key_type targt, double wght = 0) : target(targt), weight(wght) {}
  };

  using graph_type = std::graph::container::csr_adjacency<route, key_type>;

public: // Construction/Destruction/Assignment
  routes_csr_graph(csv::string_view csv_file) : base_type(csv_file), g_(load_routes(csv_file)) {}

  routes_csr_graph()                        = delete;
  routes_csr_graph(const routes_csr_graph&) = delete;
  routes_csr_graph(routes_csr_graph&&)      = default;
  ~routes_csr_graph()                       = default;

  routes_csr_graph& operator=(const routes_csr_graph&) = delete;
  routes_csr_graph& operator=(routes_csr_graph&&) = default;

public: // Properties
  constexpr graph_type&       graph() { return g_; }
  constexpr const graph_type& graph() const { return g_; }

public: // Operations
  // used to visually validate core functionality
  void output_routes() {}


private: // construction helpers
  graph_type load_routes(csv::string_view csv_file) {
    CSVReader reader(csv_file); // CSV file reader

    auto ekey_fnc = [this](const CSVRow& row) {
      auto from_key = find_city_key(row[0].get<string_view>());
      auto to_key   = find_city_key(row[1].get<string_view>());
      assert(from_key < cities().size() && to_key < cities().size());
      return std::pair{from_key, to_key};
    };
    auto evalue_fnc = [this](const CSVRow& row) {
      auto to_key = find_city_key(row[1].get<string_view>());
      auto dist   = row[2].get<double>();
      assert(to_key < cities().size());
      return route(to_key, dist);
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


#if 0
TEST_CASE("Dummy CSV test", "[csv]") {
  init_console();
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator
    string_view from = row[0].get<string_view>();
    string_view to   = row[1].get<string_view>();
    auto        dist = row[2].get<double>();
    cout << from << "," << to << "," << dist << endl;
  }
}
#endif

TEST_CASE("Germany routes CSV test", "[csr][csv]") {
  init_console();
  routes_csr_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
}
