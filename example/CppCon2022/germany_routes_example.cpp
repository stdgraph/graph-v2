#include <catch2/catch.hpp>
#include "graphviz_output.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/container/compressed_graph.hpp"
#include "graph/algorithm/dijkstra_clrs.hpp"
#include "rr_adaptor.hpp"
#include <iostream>
#include <list>
#include <cassert>

using namespace std::ranges;
using namespace std::graph;
using namespace std::graph::container;
using namespace std::graph::views;
using namespace std::literals;

using std::cout;
using std::endl;
using std::string;
using std::string_view;
using std::pair;
using std::tie;
using std::vector;
using std::list;

void init_console(); // init cout for UTF-8

using city_id_type = uint32_t;

template <typename G>
struct city_id {
  G&             g;
  vertex_id_t<G> id;

  city_id(G& graph, vertex_id_t<G> uid) : g(graph), id(uid) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const city_id& rhs) {
    auto&& [_g, _id] = rhs;
    os << vertex_value(_g, *find_vertex(_g, _id)) << " [" << _id << "]";
    return os;
  }
};
// Given: G g, vertex_id_t<G> uid
//   cout << city_id(g,frankfurt_id) << endl;
// output: Frankfürt [0]

template <typename G>
struct city {
  G&             g;
  vertex_id_t<G> id;

  city(G& graph, vertex_id_t<G> uid) : g(graph), id(uid) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const city& rhs) {
    auto&& [_g, _id] = rhs;
    os << vertex_value(_g, *find_vertex(_g, _id));
    return os;
  }
};


TEST_CASE("Germany Routes Presentation", "[presentation][germany][routes][shortest_paths][csr]") {
  init_console();

  // city data (vertices)
  using city_name_type       = string;
  using city_names_type      = vector<city_name_type>;
  city_names_type city_names = {"Frankfürt", "Mannheim", "Karlsruhe", "Augsburg", "Würzburg",
                                "Nürnberg",  "Kassel",   "Erfurt",    "München",  "Stuttgart"};

  // edge data (edgelist)
  using route_data                  = copyable_edge_t<city_id_type, double>; // {source_id, target_id, value}
  vector<route_data> routes_doubled = {
        {0, 1, 85.0},  {0, 4, 217.0}, {0, 6, 173.0}, //
        {1, 0, 85.0},  {1, 2, 80.0},                 //
        {2, 1, 80.0},  {2, 3, 250.0},                //
        {3, 2, 250.0}, {3, 8, 84.0},                 //
        {4, 0, 217.0}, {4, 5, 103.0}, {4, 7, 186.0}, //
        {5, 4, 103.0}, {5, 8, 167.0}, {5, 9, 183.0}, //
        {6, 0, 173.0}, {6, 8, 502.0},                //
        {7, 4, 186.0},                               //
        {8, 3, 84.0},  {8, 5, 167.0}, {8, 6, 502.0}, //
        {9, 5, 183.0},
  };

  // Graph definition
  struct route { // edge
    city_id_type target_id = 0;
    double       distance  = 0.0; // km
  };
  using AdjList = vector<list<route>>;                  // range of ranges
  using G       = rr_adaptor<AdjList, city_names_type>; // graph

  G g(city_names, routes_doubled);

  // Useful demo values
  city_id_type          frankfurt_id = 0;
  vertex_reference_t<G> frankfurt    = *find_vertex(g, frankfurt_id);

  cout << "Traverse the vertices & outgoing edges" << endl;
  for (auto&& [uid, u] : vertexlist(g)) {                           // [id,vertex&]
    cout << city_id(g, uid) << endl;                                // city name [id]
    for (auto&& [vid, uv] : std::graph::views::incidence(g, uid)) { // [target_id,edge&]
      cout << "   --> " << city_id(g, vid) << endl;
      // "--> "target city" [target_id]
    }
  }

  // Shortest Paths (segments)
  {
    auto                        weight_1 = [](edge_reference_t<G> uv) -> int { return 1; };
    std::vector<int>            distances(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distances, predecessor, weight_1);

    cout << "Shortest distance (segments) from " << city_id(g, frankfurt_id) << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid)
      if (distances[uid] > 0)
        cout << "  --> " << city_id(g, uid) << " - " << distances[uid] << " segments" << endl;
  }

  // Shortest Paths (km)
  {
    auto                        weight = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); }; // { return 1; };
    std::vector<double>         distances(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distances, predecessor, weight);

    cout << "Shortest distance (km) from " << vertex_value(g, frankfurt) << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid) {
      if (distances[uid] > 0)
        cout << "  --> " << city_id(g, uid) << " - " << distances[uid] << "km" << endl;
    }

    // Find farthest city
    vertex_id_t<G> farthest_id   = frankfurt_id;
    double         farthest_dist = 0.0;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid) {
      if (distances[uid] > farthest_dist) {
        farthest_dist = distances[uid];
        farthest_id   = uid;
      }
    }

    // Output path for farthest distance
    cout << "The farthest city from " << city(g, frankfurt_id) << " is " << city(g, farthest_id) << " at "
         << distances[farthest_id] << "km" << endl;
    cout << "The shortest path from " << city(g, farthest_id) << " to " << city(g, frankfurt_id) << " is: " << endl
         << "  ";
    for (vertex_id_t<G> uid = farthest_id; uid != frankfurt_id; uid = predecessor[uid]) {
      if (uid != farthest_id)
        cout << " -- ";
      cout << city_id(g, uid);
    }
    cout << " -- " << city_id(g, frankfurt_id) << endl;
  }
}
