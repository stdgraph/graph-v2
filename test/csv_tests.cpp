#include <catch2/catch.hpp>
#include "csv_parser.hpp"
#ifdef _MSC_VER
#include "Windows.h"
#endif

using namespace csv;
using std::string_view;
using std::cout;
using std::endl;

TEST_CASE("Dummy CSV test", "[csv]") {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000);
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator

#if 1
    string_view from = row[0].get<string_view>();
    string_view to   = row[1].get<string_view>();
    int         dist = row[2].get<int>();
    cout << from << "," << to << "," << dist << endl;
#else
    bool first = true;
    for (CSVField& field : row) {
      if (first) {
        first = false;
      } else {
        cout << ",";
      }
      // By default, get<>() produces a std::string.
      // A more efficient get<string_view>() is also available, where the resulting
      // string_view is valid as long as the parent CSVRow is alive
      cout << field.get<>();
    }
    cout << endl;
#endif
  }
}
