#include <catch2/catch.hpp>

#include "csv_parser.hpp"
#include "graph/container/csr_adjacency.hpp"
#include <set>
#include <algorithm>
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


class csr_german_routes_graph {
public:
  using key_type   = uint32_t;
  using name_type  = string;
  using cities_vec = std::vector<name_type>;

  struct route {
    key_type target = 0;
    double   weight = 0;
    route(key_type targt, double wght = 0) : target(targt), weight(wght) {}
  };

  using graph_type = std::graph::container::csr_adjacency<route, key_type>;

public: // Construction/Destruction
  csr_german_routes_graph(const string& csv_file) : cities_(), g_(load_graph(csv_file, cities_)) {}
  ~csr_german_routes_graph() = default;

public: // Properties
  graph_type&       graph() { return g_; }
  const graph_type& graph() const { return g_; }

  cities_vec&       cities() { return cities_; }
  const cities_vec& cities() const { return cities_; }

public: // Operations
  cities_vec::iterator find_city(const string_view& city_name) {
    auto it = std::ranges::lower_bound(cities_, city_name);
    if (it != end(cities_) && *it == city_name)
      return it;
    return end(cities_);
  }
  key_type find_city_key(const string_view& city_name) {
    return static_cast<key_type>(end(cities_) - find_city(city_name));
  }

  void output_routes() { // used to validate core functionality
  }


private: // construction helpers
  graph_type load_graph(const string& csv_file, cities_vec& cities) {
    setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
    CSVReader reader(csv_file);
    load_cities(reader, cities);
    return load_routes(reader);
  }

  void load_cities(CSVReader& reader, cities_vec& cities) {
    std::set<string_view> city_set;
    for (CSVRow& row : reader) { // Input iterator
      string_view from = row[0].get<string_view>();
      string_view to   = row[1].get<string_view>();
      city_set.insert(from);
      city_set.insert(to);
    }
    cities.reserve(city_set.size());
    for (const string_view& city_name : city_set)
      cities.emplace_back(string(city_name));
  }

  graph_type load_routes(CSVReader& reader) {
    auto ekey_fnc = [this](const CSVRow& row) {
      auto from_key = find_city_key(row[0].get<string_view>());
      auto to_key   = find_city_key(row[1].get<string_view>());
      return std::pair{from_key, to_key};
    };
    auto evalue_fnc = [this](const CSVRow& row) {
      auto to_key = find_city_key(row[1].get<string_view>());
      auto dist   = row[2].get<double>();
      return route(to_key, dist);
    };
    graph_type cc(reader, ekey_fnc, evalue_fnc);

    for (CSVRow& row : reader) { // Input iterator
      string_view from = row[0].get<string_view>();
      string_view to   = row[1].get<string_view>();
      auto        dist = row[2].get<double>();
    }
    return cc;
  }

private:              // Member Variables
  cities_vec cities_; // ordered city names; must be before g_
  graph_type g_;      // csr graph
};


#if 0
TEST_CASE("Dummy CSV test", "[csv]") {
#  ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);
#  endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator
    string_view from = row[0].get<string_view>();
    string_view to   = row[1].get<string_view>();
    auto        dist = row[2].get<double>();
    cout << from << "," << to << "," << dist << endl;
  }
}
#endif

TEST_CASE("German CSV test", "[csv]") {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);
#endif
  csr_german_routes_graph routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
}
