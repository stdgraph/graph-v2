/**
 * @file ch3_toposort.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#include "compressed.hpp"
#include "dfs_range.hpp"
#include "edge_list.hpp"
#include "mmio.hpp"

int main() {

  edge_list<directed> E = read_mm<directed>("makefile-dependencies.mmio");
  adjacency<0>        A(E);

  std::vector<std::string> name;
  std::ifstream            name_in("makefile-target-names.dat");
  while (!name_in.eof()) {
    std::string buffer;
    name_in >> buffer;
    name.push_back(buffer);
  }

  std::vector<size_t> order;
  for (auto&& j : dfs_range(A, 0)) {
    //    std::cout << j << std::endl;
    std::cout << name[j] << std::endl;
    order.push_back(j);
  }

  return 0;
}
