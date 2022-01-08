#include <catch2/catch.hpp>
#include "csv_parser.hpp"

using namespace csv;

TEST_CASE("Dummy CSV test", "[csv]") {
  CSVReader reader(TEST_DATA_ROOT_DIR "germany_routes.csv");

  for (CSVRow& row : reader) { // Input iterator
    for (CSVField& field : row) {
      // By default, get<>() produces a std::string.
      // A more efficient get<string_view>() is also available, where the resulting
      // string_view is valid as long as the parent CSVRow is alive
      //std::cout << field.get<>() << ...
    }
  }
}
