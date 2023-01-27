#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/algorithm/mst.hpp"
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
using std::graph::edge_reference_t;
using std::graph::edge_value_t;

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

template<class VId, class EV>
class test_edge_list {
public:
  test_edge_list() { max = 0; }
  void push_back( std::tuple<VId, VId, EV> edge ) {
    if ( std::get<0>(edge) > max ) {
        max = std::get<0>(edge);
    }
    if ( std::get<1>(edge) > max ) {
        max = std::get<1>(edge);
    }
    e.push_back( edge );
  }
  auto begin() { return e.begin(); }
  auto end() { return e.end(); }
  VId size() { return max + 1; }
  using value_type = std::tuple<VId, VId, EV>;

private:
  std::vector<std::tuple<VId, VId, EV>> e;
  VId                                   max;
};

TEST_CASE("Kruskal Min ST Algorithm", "[min st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

    size_t N(size(vertices(g)));
    size_t M = 0;
    for (auto && [uid, u] : std::graph::views::vertexlist(g)) {
      M += std::graph::degree(g, u);
    }
    
    test_edge_list<vertex_id_t<G>, double> e, t;

    auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    for (auto && [uid,vid,uv] : std::graph::views::edgelist(g)) {
      e.push_back(std::make_tuple(uid,vid,evf(uv)));
    }
    
    //Replace with whatever conforms to std::graph::edge_list
    std::graph::kruskal<vertex_id_t<G>, edge_value_t<G>>(e, std::back_inserter(t));
    /*for( auto&& [u,v,val] : t ) {
      cout << u << " " << v << " " << val << endl;
	  }*/
}

TEST_CASE("Kruskal Max ST Algorithm", "[max st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv");

    size_t N(size(vertices(g)));
    size_t M = 0;
    for (auto && [uid, u] : std::graph::views::vertexlist(g)) {
      M += std::graph::degree(g, u);
    }
    
    test_edge_list<vertex_id_t<G>, double> e, t;

    auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    for (auto && [uid,vid,uv] : std::graph::views::edgelist(g)) {
      e.push_back(std::make_tuple(uid,vid,evf(uv)));
    }
    
    //Replace with whatever conforms to std::graph::edge_list
    std::graph::kruskal<vertex_id_t<G>,double>(e, std::back_inserter(t),
      [](auto&& i, auto&& j){return i > j;});
    /*for( auto&& [u,v,val] : t ) {
      cout << u << " " << v << " " << val << endl;
	  }*/
}

TEST_CASE("Prim Min ST Algorithm", "[prim min st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
    
    std::vector<vertex_id_t<G>> preds(size(vertices(g)));
    std::vector<double> weights(size(vertices(g)));
    std::graph::prim(g, preds, weights);
    /*for( auto && [uid, u] : std::graph::views::vertexlist(g)) {
        cout << "pred of " << uid << " is " << preds[uid] << " with val " << weights[uid] << endl;
    }*/
}

TEST_CASE("Prim Max ST Algorithm", "[prim max st]") {
    init_console();
    using G = routes_vol_graph_type;
    auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);

    std::vector<vertex_id_t<G>> preds(size(vertices(g)));
    std::vector<double> weights(size(vertices(g)));
    
    std::graph::prim(g, preds, weights,
      [](auto&& i, auto&& j){return i > j;}, 0);
    /*for( auto && [uid, u] : std::graph::views::vertexlist(g)) {
        cout << "pred of " << uid << " is " << preds[uid] << " with val " << weights[uid] << endl;
    }*/
}