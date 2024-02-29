#include <catch2/catch.hpp>
#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include "graph/graph.hpp"
#include "graph/views/breadth_first_search.hpp"
#include "graph/container/compressed_graph.hpp"
#include "graph/views/co_dfs.hpp"

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_TEST

using std::vector;
using std::string;
using std::tuple;
using std::cout;
using std::endl;

using namespace std::graph::views;


const vector<string> actors{"Tom Cruise",      "Kevin Bacon",    "Hugo Weaving",      "Carrie-Anne Moss",
                            "Natalie Portman", "Jack Nicholson", "Kelly McGillis",    "Harrison Ford",
                            "Sebastian Stan",  "Mila Kunis",     "Michelle Pfeiffer", "Keanu Reeves",
                            "Julia Roberts"};

TEMPLATE_TEST_CASE("Kevin Bacon example",
                   "[example][bfs][basic_graph]",
                   vector<vector<size_t>>,
                   vector<vector<size_t>> const,
                   vector<vector<tuple<size_t>>>,
                   vector<vector<tuple<size_t>>> const) {
  using Graph = TestType;

  Graph costar_adjacency_list{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                              {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                              {1, 4}};

  vector<int> bacon_number(size(actors));
  //auto        evf = [](edge_reference_t<G> uv) { return uv; };

  // 1 -> Kevin Bacon
  for (auto&& [uid, vid, uv] : sourced_edges_breadth_first_search(costar_adjacency_list, 1)) {
    bacon_number[vid] = bacon_number[uid] + 1;
  }

#if TEST_OPTION == TEST_OPTION_OUTPUT
  SECTION("Kevin Bacon Example output") {
    for (size_t i = 0; i < size(actors); ++i) {
      cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
    }
  }

  //Tom Cruise has Bacon number 1
  //Kevin Bacon has Bacon number 0
  //Hugo Weaving has Bacon number 3
  //Carrie-Anne Moss has Bacon number 4
  //Natalie Portman has Bacon number 2
  //Jack Nicholson has Bacon number 1
  //Kelly McGillis has Bacon number 2
  //Harrison Ford has Bacon number 1
  //Sebastian Stan has Bacon number 3
  //Mila Kunis has Bacon number 3
  //Michelle Pfeiffer has Bacon number 1
  //Keanu Reeves has Bacon number 4
  //Julia Roberts has Bacon number 1

#elif TEST_OPTION == TEST_OPTION_GEN
  SECTION("Kevin Bacon Example generate") {

    cout << endl << "vector<int> expected = {";
    for (size_t i = 0; i < size(actors); ++i) {
      if (i > 0)
        cout << ",";
      cout << bacon_number[i];
    }
    cout << "};" << endl << endl;

    cout << "for (size_t i = 0; i < size(actors); ++i) {" << endl
         << "  REQUIRE(bacon_number[i] == expected[i]);" << endl
         << "}" << endl;
  }
#elif TEST_OPTION == TEST_OPTION_TEST
  SECTION("Kevin Bacon Example test") {}
  vector<int> expected = {1, 0, 3, 4, 2, 1, 2, 1, 3, 3, 1, 4, 1};

  for (size_t i = 0; i < size(actors); ++i) {
    REQUIRE(bacon_number[i] == expected[i]);
  }
#endif

} // TEMPLATE_TEST_CASE


#if 0
enum class bfs_event : int {
  none              = 0, // useful?
  initialize_vertex = 0x0001,
  examine_vertex    = 0x0002,
  examine_edge      = 0x0004,
  discover_vertex   = 0x0008,
  edge_relaxed      = 0x0010,
  edge_not_relaxed  = 0x0020,
  finish_vertex     = 0x0040,

  vertex_default = discover_vertex, // useful?
  edge_default   = examine_edge     // useful?
};


TEST_CASE("Kevin Bacon example with revised bfs", "[example][bfs][basic_graph]") {
  using Graph = vector<vector<size_t>>;

  Graph       costar_adjacency_list{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                                    {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                                    {1, 4}};
  vector<int> bacon_number(size(actors));

  // Only stop for one event in this example
  basic_breadth_first_search bfs(costar_adjacency_list, bfs_event::examine_edge);
  for (auto&& [event, uid, vid] : bfs) {
    // The switch statement is overkill for this example, but it's useful for more complex algorithms.
    switch (event) {
    case bfs_event::examine_edge: bacon_number[vid] = bacon_number[uid] + 1; break;
    default: assert(false); // Unhandled event
    }
  }
  for (size_t i = 0; i < size(actors); ++i)
	cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
}
#endif

