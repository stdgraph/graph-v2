#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/kruskal.hpp"
#include "graph/container/dynamic_graph.hpp"
#ifdef _MSC_VER
#  include "Windows.h"
#endif

using std::cout;
using std::endl;

using std::graph::vertex_t;
using std::graph::vertex_id_t;
using std::graph::vertex_reference_t;
using std::graph::vertex_iterator_t;
using std::graph::vertex_edge_range_t;
using std::graph::edge_t;

using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_value;
using std::graph::target_id;
using std::graph::target;
using std::graph::edge_value;
using std::graph::find_vertex;
using std::graph::vertex_id;

using routes_vol_graph_traits = std::graph::container::vol_graph_traits<double, std::string, std::string>;
using routes_vol_graph_type   = std::graph::container::dynamic_adjacency_graph<routes_vol_graph_traits>;

TEST_CASE("Kruskal Min ST Algorithm", "[min st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

    //Replace with whatever conforms to std::graph::edge_list
    std::vector<std::tuple<vertex_id_t<G>,vertex_id_t<G>,edge_t<G>>> t;
    std::graph::kruskal(g, std::back_inserter(t));
    /*for( auto&& [u,v,val] : t ) {
        cout << u << " " << v << " " << edge_value(g, val) << endl;
	}*/
}

TEST_CASE("Kruskal Max ST Algorithm", "[max st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

    //Replace with whatever conforms to std::graph::edge_list
    std::vector<std::tuple<vertex_id_t<G>,vertex_id_t<G>,edge_t<G>>> t;
    std::graph::kruskal(g, std::back_inserter(t),
    [](auto&& i, auto&& j){return i > j;});
    /*for( auto&& [u,v,val] : t ) {
        cout << u << " " << v << " " << edge_value(g, val) << endl;
	}*/
}
