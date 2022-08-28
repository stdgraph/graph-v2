#pragma once
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/depth_first_search.hpp"
#include <fstream>

enum struct directedness : int8_t {
  directed,   // a single edge joins 2 vertices
  directed2,  // 2 edges join 2 vertices, each with different directions; needed for graphviz
  undirected, // one or more edges exist between vertices with no direction
  bidirected  // a single edge between vertices with direction both ways (similar to undirected, but with arrows)
};


/// <summary>
/// Outputs a graphviz file for the routes graph passed.
///
/// Example command line to generate image files for file "routes.gv"
/// dot -Tpdf -O routes.gv
/// dot -Tpng -O routes.gv
/// neato -Tpng -O routes.gv
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <param name="g">Grape instance</param>
/// <param name="filename">Graphviz filename to output</param>
template <class G>
void output_routes_graphviz(
      G&                 g, // bug: "const G&" causes vertexlist iterator to believe vertex_id is int64
      std::string_view   filename,
      const directedness dir,
      std::string_view   bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace std::graph;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help
  std::string_view arrows, rev_arrows = "dir=back,arrowhead=vee,";
  switch (dir) {
  case directedness::bidirected: arrows = "dir=both,arrowhead=vee,arrowtail=vee"; break;
  case directedness::directed: arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::directed2: arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::undirected: arrows = "dir=none"; break;
  }

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  splines = curved\n"
     << "  node[shape=oval]\n"
     << "  edge[" << arrows << ", fontcolor=blue]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [label=\"" << vertex_value(g, u) << " [" << uid << "]\"]\n";
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      auto&&           v   = target(g, uv);
      std::string_view arw = (dir == directedness::directed2 && vid < uid) ? rev_arrows : "";
      of << "   " << uid << " -> " << vid << " [" << arw << "xlabel=\"" << edge_value(g, uv) << " km\"]\n";
    }
    of << std::endl;
  }
  of << "}\n";
}

template <class G>
void output_routes_graphviz_adjlist(
      const G&         g,
      std::string_view filename,
      std::string_view bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace std::graph;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  graph[rankdir=LR]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [shape=Mrecord, label=\"{<f0>" << uid << "|<f1>" << vertex_value(g, u) << "}\"]\n";
    std::string from = std::to_string(uid);
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      auto&&      v  = target(g, uv);
      std::string to = "e"s + std::to_string(uid) + "_"s + std::to_string(vid);
      of << "    " << to << " [shape=record, label=\"{<f0>" << vid << "|<f1>" << edge_value(g, uv) << "km}\"]\n";
      of << "    " << from << " -> " << to;
      from = to;
    }
    of << std::endl;
  }
  of << "}\n";
}

template <class G>
void output_routes_graphviz_dfs_vertices(
      G&                         g,
      std::string_view           filename,
      std::graph::vertex_id_t<G> seed,
      std::string_view bgcolor = std::string_view() // "transparent" or see http://graphviz.org/docs/attr-types/color/
) {
  using namespace std::graph;
  using namespace std::graph::views;
  using namespace std::literals;
  std::string   fn(filename);
  std::ofstream of(fn);
  assert(of.is_open());
  //of << "\xEF\xBB\xBF"; // UTF-8 lead chars for UTF-8 including BOM
  //of << "\xBB\xBF"; // UTF-8 lead chars for UTF-8

  // nodesep=0.5; doesn't help
  std::vector<bool> visited;
  visited.resize(size(vertices(g)));

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     //<< "  graph[rankdir=LR]\n"
     << "  node[shape=oval]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  // output seed
  //of << "  " << seed << " [shape=Mrecord, label=\"{<f0>" << seed << "|<f1>" << vertex_value(g, *find_vertex(g, seed))
  //   << "}\"]\n";
  of << "  " << seed << " [label=\"" << vertex_value(g, *find_vertex(g, seed)) << " [" << seed << "]\"]\n";
  visited[seed] = true;

  // output descendents
  for (auto&& [uid, vid, uv] : std::graph::views::sourced_edges_depth_first_search(g, seed)) {
    // Output newly discovered vertex
    if (!visited[vid]) {
      //of << "  " << vid << " [shape=Mrecord, label=\"{<f0>" << vid << "|<f1>" << vertex_value(g, *find_vertex(g, uid))
      //   << "}\"]\n";
      auto& v = *find_vertex(g, vid);
      of << "  " << vid << " [label=\"" << vertex_value(g, v) << " [" << vid << "]\"]\n";
      visited[vid] = true;
    }
    of << "  " << uid << " -> " << vid << "\n"; // output the edge
  }
  of << "}\n";
}
