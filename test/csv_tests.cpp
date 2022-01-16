#include <catch2/catch.hpp>
#include "csv_csr_routes.hpp"
#include "csv_vol_routes.hpp"
#include <cassert>
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using std::cout;
using std::endl;



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
  //routes_csr_graph germany_routes(TEST_DATA_ROOT_DIR "germany_routes.csv");
}
