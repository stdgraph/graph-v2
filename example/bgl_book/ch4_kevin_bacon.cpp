/**
 * @file ch4_kevin_bacon.cpp
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

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "bfs_range.hpp"
#include "compressed.hpp"
#include "edge_list.hpp"

auto parse_buffer(const std::string& buffer) {
  std::stringstream link(buffer);
  std::string       actor_one, movie_name, actor_two;
  getline(link, actor_one, ';');
  getline(link, movie_name, ';');
  getline(link, actor_two);
  return std::make_tuple(actor_one, movie_name, actor_two);
}

auto read_imdb(const std::string& path, std::map<std::string, vertex_index_t>& actor_id_map) {
  std::string    buffer;
  vertex_index_t actor_vertex_counter = 0;
  std::ifstream  datastream(path);

  edge_list<undirected, std::string> imdb(0);
  imdb.open_for_push_back();
  while (getline(datastream, buffer)) {
    auto&& [actor_one, movie_name, actor_two] = parse_buffer(buffer);

    auto&& [key_val_one, inserted_one] = actor_id_map.insert({actor_one, actor_vertex_counter});
    if (inserted_one) ++actor_vertex_counter;
    auto&& [dummy_one, index_one] = *key_val_one;

    auto&& [key_val_two, inserted_two] = actor_id_map.insert({actor_two, actor_vertex_counter});
    if (inserted_two) ++actor_vertex_counter;
    auto&& [dummy_two, index_two] = *key_val_two;

    imdb.push_back(index_one, index_two, movie_name);
  }
  imdb.close_for_push_back();

  adjacency<0, std::string> A(imdb);
  return A;
}

int main(int argc, char* argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " scsv_file" << std::endl;
    return -1;
  }
  std::map<std::string, vertex_index_t> actor_id_map;
  auto&&                                A = read_imdb(argv[1], actor_id_map);

  size_t              N = A.size();
  std::vector<size_t> bacon_number(N);

  auto&& [kb, kb_id] = *(actor_id_map.find("Kevin Bacon"));

  for (auto&& [v, u] : bfs_edge_range(A, kb_id)) {
    bacon_number[u] = bacon_number[v] + 1;
  }

  for (auto&& [actor, id] : actor_id_map) {
    std::cout << actor << " has Bacon number of " << bacon_number[id] << std::endl;
  }

  return 0;
}
