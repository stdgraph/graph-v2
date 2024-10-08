#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "graph/algorithm/breadth_first_search.hpp"
#include "graph/container/dynamic_graph.hpp"
#include <iostream>

#define GRAPH_BFS_ALGORITHM_HPP

#define TEST_OPTION_OUTPUT (1)
#define TEST_OPTION_GEN (2)
#define TEST_OPTION_TEST (3)
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;

using std::ranges::forward_range;
using std::forward_iterator;

using namespace graph;

void init_console(); // init cout for UTF-8

TEST_CASE("breadth_first_search algorithm test", "[bfs][single-source][algorithm]") {
  init_console();

  // Create graph instance

  SECTION("verify graph") {
    // verify graph
    REQUIRE(1 + 1 == 2);
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("bfs algo output") {
    // output graph
    cout << "graph" << endl;
  }
#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("bfs algo test generation") {
    // generate test code
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("bfs algo test") {
    // production test code
  }
#endif // TEST_OPTION
} // TEST_CASE"breadth_first_search algorithm test"
