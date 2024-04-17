#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/edgelist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views/breadth_first_search.hpp"
#include "graph/container/compressed_graph.hpp"
#include <cassert>
#include <iostream>

#define TEST_OPTION_OUTPUT (1) // output tests for visual inspection
#define TEST_OPTION_GEN (2)    // generate unit test code to be pasted into this file
#define TEST_OPTION_TEST (3)   // run unit tests
#define TEST_OPTION TEST_OPTION_TEST

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::string_view;

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;
using std::graph::edge_value_t;

using std::graph::graph_value;
using std::graph::vertices;
using std::graph::num_vertices;
using std::graph::num_edges;
using std::graph::has_edge;
using std::graph::edges;
using std::graph::vertex_id;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::degree;
using std::graph::find_vertex;
using std::graph::find_vertex_edge;

using std::graph::partition_id;

using std::graph::views::edgelist;
using std::graph::views::sourced_edges_breadth_first_search;

using G = std::graph::container::compressed_graph<void, void, void>;

vector<string> actors{"Tom Cruise",        "Kevin Bacon",    "Hugo Weaving",  "Carrie-Anne Moss", "Natalie Portman",
                      "Jack Nicholson",    "Kelly McGillis", "Harrison Ford", "Sebastian Stan",   "Mila Kunis",
                      "Michelle Pfeiffer", "Keanu Reeves",   "Julia Roberts"};


//G costar_adjacency_list{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
//                        {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
//                        {1, 4}};

using costar_type = std::graph::copyable_edge_t<vertex_id_t<G>>;

/*
G costar_adjacency_list({{0, 1},  {0, 5},  {0, 6},                    //
                         {1, 7},  {1, 10}, {1, 0},  {1, 5},  {1, 12}, //
                         {2, 4},  {2, 3},  {2, 11},                   //
                         {3, 2},  {3, 11},                            //
                         {4, 2},  {4, 11}, {4, 12}, {4, 9},           //
                         {5, 8},  {5, 9},  {5, 2},  {5, 12},          //
                         {6, 7},  {6, 0},                             //
                         {7, 6},  {7, 1},  {7, 10},                   //
                         {8, 4},  {8, 9},                             //
                         {9, 8},  {9, 4},  {10, 7}, {10, 1},          //
                         {11, 2}, {11, 3},                            //
                         {12, 1}, {12, 4}, {12, 5}});

TEST_CASE("CSR Bacon Test", "[csr][bacon]") {
  SECTION("Print Costar Adjacency List") {
    for (auto&& [uid, vid, uv] : edgelist(costar_adjacency_list)) {
      cout << actors[uid] << " -> " << actors[vid] << endl;
    }
  }

  SECTION("Evaluate Bacon Number") {
    std::vector<int> bacon_number(size(actors));

    vertex_id_t<G> kevin_bacon_id = 1;
    for (auto&& [uid, vid, uv] : sourced_edges_breadth_first_search(costar_adjacency_list, kevin_bacon_id)) {
      bacon_number[vid] = bacon_number[uid] + 1;
    }

    for (int i = 0; i < size(actors); ++i) {
      std::cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
    }
  }
}
*/
