/**
 * @file ch3_toposort.cpp
 *
 * Adapted from NWGraph:
 *   @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 *   @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *   @license  BSD-3-Clause
 *   @authors  Andrew Lumsdaine
 *
 */

#include <map>
#include <graph/graph.hpp>
#include <graph/container/compressed_graph.hpp>

//****************************************************************************
using vv_type  = std::string;
using vid_type = uint32_t;
using eid_type = size_t;
using G = graph::container::compressed_graph<void,vv_type,void,  // EV,VV,GV
                                             vid_type,eid_type>; // VId,EIndex
//using vid_type = graph::vertex_id_t<G>;
//using eid_type = typename G::edge_index_type; //graph::edge_id_t<G>;

/// @todo Need a matrix-market reader for "makefile-dependencies.mmio"
std::vector<std::tuple<std::string const, std::string const>> const file_dependencies{
    {"dax.h","bar.o"},
    {"dax.h","foo.o"},
    {"dax.h","zag.o"},
    {"yow.h","bar.o"},
    {"yow.h","zag.o"},
    {"boz.h","bar.o"},
    {"boz.h","zig.o"},
    {"boz.h","zag.o"},
    {"zow.h","foo.o"},
    {"bar.cpp","bar.o"},
    {"bar.o","libfoobar.a"},
    {"foo.cpp","foo.o"},
    {"foo.o","libfoobar.a"},
    {"zig.cpp","zig.o"},
    {"zig.o","libzigzag.a"},
    {"zag.cpp","zag.o"},
    {"zag.o","libzigzag.a"},
    {"libzigzag.a","killerapp"},
    {"libfoobar.a","libzigzag.a"},
};

//****************************************************************************
template <typename G>
graph::vertex_id_t<G> get_vid(G&& g, std::string_view const &vlabel)
{
  auto it = std::ranges::find_if(graph::vertices(g),
      [&g, &vlabel](auto& u) { return graph::vertex_value(g, u) == vlabel; });

  // return size(vertices(g)) if not found
  return static_cast<graph::vertex_id_t<G>>(it - begin(graph::vertices(g)));
}

//****************************************************************************
G load_graph(std::vector<std::tuple<vv_type const, vv_type const>> const &edges)
{
    eid_type num_edges{edges.size()};
    vid_type curr_id = 0;

    // Parse and load vertices from label pairs
    using label_id_container_type = std::map<std::string,vid_type>;
    label_id_container_type vertex_label_id;
    for (auto& [u, v] : edges) {
        if (vertex_label_id.find(u) == vertex_label_id.end()) {
            vertex_label_id[u] = curr_id++;
        }
    }
    for (auto& [u, v] : edges) {
        if (vertex_label_id.find(v) == vertex_label_id.end()) {
            vertex_label_id[v] = curr_id++;
        }
    }

    auto swap_label_id =
        [&vertex_label_id](label_id_container_type::value_type const &label_id)
        {
            using copyable_id_label = graph::copyable_vertex_t<vid_type,std::string>;
            return copyable_id_label{label_id.second, label_id.first};   // id,name
        };

    G g;
    g.load_vertices(vertex_label_id, swap_label_id);
    
    // Parse and load edges from label pairs
    auto eproj =
        [&g,&vertex_label_id](std::tuple<vv_type const,vv_type const> const &edge)
        {
            using edge_value_type    = std::remove_cvref_t<graph::edge_value_t<G>>;
            using copyable_edge_type = graph::copyable_edge_t<vid_type, edge_value_type>;
            std::cout << std::get<0>(edge) << ","
                      << std::get<1>(edge) << std::endl;
            std::cout << vertex_label_id[std::get<0>(edge)] << ","
                      << vertex_label_id[std::get<1>(edge)] << std::endl;
            // return copyable_edge_type {
            //     get_vid(g, std::get<0>(edge)),  // source_id
            //     get_vid(g, std::get<1>(edge)) }; // target_id

            return copyable_edge_type {
                vertex_label_id[std::get<0>(edge)],
                vertex_label_id[std::get<1>(edge)]};
        };

    std::cout << "size(e): " << edges.size() << std::endl;
    std::cout << "num_edges:" << num_edges << std::endl;
    g.load_edges(edges, eproj, size(vertex_label_id), num_edges);

    // get_vid will only work after load_edges apparently
    for ( auto& edge : edges ) {
      std::cout << get_vid(g, std::get<0>(edge)) << ","
		<< get_vid(g, std::get<1>(edge)) << std::endl;
    }
    
    return g;
}

//****************************************************************************
//****************************************************************************
int main()
{
    auto g = load_graph(file_dependencies);

    std::cout << "Print vertices?\n";
    for (auto& v : graph::vertices(g))
    {
        auto vv{graph::vertex_value(g, v)};
        std::cout << "Vertex: " << vv << std::endl;
    }

    for (auto& v : graph::vertices(g))
    {
        for (auto&& uv : graph::edges(g, v)) {
	  std::cout << "Edge from " << graph::vertex_value(g, v)
		    << " to "
		    << graph::vertex_value(g, graph::target(g, uv))
		    << std::endl;
	}
    }

    return 0;
}


#if 0
// #include "compressed.hpp"
// #include "dfs_range.hpp"
// #include "edge_list.hpp"
// #include "mmio.hpp"

int main()
{
  edge_list<directed> E = read_mm<directed>("makefile-dependencies.mmio");
  adjacency<0>        A(E);

  std::vector<std::string> name;
  std::ifstream            name_in(BGL_DATA_ROOT_DIR "makefile-target-names.dat");
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
#endif
