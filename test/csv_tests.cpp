#include <catch2/catch.hpp>
#include "csv_parser.hpp"
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using namespace csv;
using std::string_view;
using std::cout;
using std::endl;

TEST_CASE("Dummy CSV test", "[csv]") {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator
    string_view from = row[0].get<string_view>();
    string_view to   = row[1].get<string_view>();
    auto        dist = row[2].get<double>();
    cout << from << "," << to << "," << dist << endl;
  }
}
