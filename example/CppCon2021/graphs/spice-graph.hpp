//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

// Data and graph from Spice example in Cppcon slides

#include <vector>
#include <string>
#include <tuple>

// clang-format off

std::vector<std::string> spice_vertices {
  "GND",
  "n0",
  "Vdd",
  "n1",
  "out",
  "n2",
};

std::vector<std::string> spice_elements {
  "C1",
  "AC",
  "R2",
  "R0",
  "R1",
  "L1",
  "C0",
  "R3",
};

std::vector<std::tuple<std::string, double>> spice_elements_values {
  { "C1", 1.e-6 },
  { "AC", 3     },
  { "R2", 1.e4  },
  { "R0", 2.7e3 },
  { "R1", 3.3e4 },
  { "L1", 1.e-4 },
  { "C0", 10.e-9},
  { "R3", 1.e3  },
};

std::vector<std::tuple<std::string, std::string, std::string>> spice_edges {
  {  "n0",   "n1", "C1" },
  { "Vdd",  "GND", "AC" },
  {  "n0",  "Vdd", "R2" },
  {  "n2",  "Vdd", "R0" },
  { "GND",   "n2", "R1" },
  {  "n2", "Vout", "L1" },
  { "GND",   "n0", "C0"},
  {  "n2",   "n1", "R3"},
};

std::vector<std::tuple<std::string, std::string, std::string, double>> spice_edges_values {
  {  "n0",   "n1", "C1", 1.e-6 },
  { "Vdd",  "GND", "AC", 3     },
  {  "n0",  "Vdd", "R2", 1.e4  },
  {  "n2",  "Vdd", "R0", 2.7e3 },
  { "GND",   "n2", "R1", 3.3e4 },
  {  "n2", "Vout", "L1", 1.e-4 },
  { "GND",   "n0", "C0", 10.e-9},
  {  "n2",   "n1", "R3", 1.e3  },
};

std::vector<std::tuple<size_t, size_t, size_t>> spice_index_edge_list {
  { 1, 3, 0 },
  { 2, 0, 1 },
  { 1, 2, 2 },
  { 5, 2, 3 },
  { 0, 5, 4 }, 
  { 5, 4, 5 },
  { 0, 1, 6 },
  { 5, 3, 7 },
};

std::vector<std::tuple<size_t, size_t, std::string>> spice_property_edge_list {
  { 1, 3, "C1" },
  { 2, 0, "AC" },
  { 1, 2, "R2" },
  { 5, 2, "R0" },
  { 0, 5, "R1" },
  { 5, 4, "L1" },
  { 0, 1, "C0" },
  { 5, 3, "R3" },
};

std::vector<std::tuple<size_t, size_t, std::string, double>> spice_property_edge_list_values {
  { 1, 3, "C1", 1.e-6 },
  { 1, 0, "AC", 3     },
  { 1, 2, "R2", 1.e4  },
  { 5, 2, "R0", 2.7e3 },
  { 0, 5, "R1", 3.3e4 },
  { 5, 4, "L1", 1.e-4 },
  { 0, 1, "C0", 10.e-9},
  { 5, 3, "R3", 1.e3  },
};

std::vector<std::vector<std::tuple<size_t, size_t>>> spice_index_adjacency_list {
  /* 0 */ { { 1, 6 }, { 5, 4 }           },
  /* 1 */ { { 2, 2 }, { 3, 0 }           },
  /* 2 */ { { 0, 1 }                     },
  /* 3 */ {                              },
  /* 4 */ {                              },
  /* 5 */ { { 2, 3 }, { 4, 5 }, { 3, 7 } },
};

std::vector<std::vector<std::tuple<size_t, std::string>>> spice_property_adjacency_list {
  /* 0 */ { { 1, "C0" }, { 5, "R1" },             },
  /* 1 */ { { 2, "R2" }, { 3, "C1" }              },
  /* 2 */ { { 0, "AC" }                           },
  /* 3 */ {                                       },
  /* 4 */ {                                       },
  /* 5 */ { { 2, "R0" }, { 4, "L1" }, { 3, "R3" } },
};

std::vector<std::vector<std::tuple<size_t, std::string, double>>> spice_property_adjacency_list_values {
  /* 0 */ { { 1, "C0", 10.e-9 }, { 5, "R1", 3.3e4 }                   },
  /* 1 */ { { 2, "R2", 1.e-4 },  { 3, "C1", 1.e-6 }                   },
  /* 2 */ { { 0, "AC", 3 }                                            },
  /* 3 */ {                                                           },
  /* 4 */ {                                                           },
  /* 5 */ { { 2, "R0", 2.7e3 }, { 4, "L1", 1.e-4 }, { 3, "R3", 1.e3 } },
};

// clang-format on
