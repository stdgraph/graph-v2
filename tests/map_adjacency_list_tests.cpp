#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <vector>
#include <map>
#include <string>

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"


namespace my {
    
struct ID
{
    std::string value;
    ID() = default;
    ID(const char* s) : value(s) {}
    friend auto operator<=>(ID const&, ID const&) = default;
};

using Graph = std::map<ID, std::vector<ID>>;

const auto& vertices(Graph const& g) { return g; }

ID vertex_id(Graph const& g, Graph::const_iterator it) { 
  return it->first;
}

std::vector<ID> const& edges(Graph const&, const Graph::const_reference& u) { return u.second; }
std::vector<ID> const& edges(Graph const& g, ID uid) { return edges(g, *g.find(uid)); }
 
ID target_id(Graph const& g, ID const& uid) { return uid; }

auto find_vertex(Graph const& g, ID const& uid) { return g.find(uid); }

Graph::const_reference target(Graph const& g, ID const& uv) { return *g.find(uv); }

template <typename T>
T* vertex_record(std::map<my::ID, T>& cont, my::ID id)
{
    return &cont[id]; // possibly creating an object
}

}


static_assert(graph::vertex_range<my::Graph>);
static_assert(graph::targeted_edge<my::Graph>);
static_assert(graph::adjacency_list<my::Graph>);


struct one_t
{
    double operator()(auto const&) const noexcept { return 1.0; }
};
constexpr one_t one {};

struct Visit
{
    void on_initialize_vertex(my::ID const& id, graph::vertex_t<my::Graph> const&) const {}
};
    

namespace graph {
    template <class G, class Sources, class Distances, class Predecessors, 
              class WF, class Visitor, class Compare, class Combine>
    concept dijkstra_requirements =
        adjacency_list<G>
        && std::ranges::input_range<Sources>
        && record_for<Distances, G>
        && record_for<Predecessors, G>
        && convertible_to<range_value_t<Sources>, vertex_id_t<G>>
        && convertible_to<vertex_id_t<G>, record_t<Predecessors, G>>
        && basic_edge_weight_function<G, WF, record_t<Distances, G>, Compare, Combine>;
}

TEST_CASE("index-based adjacency list test", "[index][concept]") {
    
    using GG = std::vector<std::vector<int>>;
    GG g {{1, 2}, {0, 2}, {0, 1}};
    static_assert(graph::adjacency_list<GG>);
    
    std::vector<int> predecessors(3);
    std::vector<double> distances(3); 
    std::vector<int> sources = {0};
    graph::dijkstra_shortest_paths(g, 0, distances, predecessors, one, Visit{});
    graph::dijkstra_shortest_paths(g, predecessors, distances, predecessors, one, Visit{});
    graph::dijkstra_shortest_distances(g, 0, distances, one, Visit{});
    graph::dijkstra_shortest_distances(g, distances, predecessors, one, Visit{});
    
    static_assert(graph::dijkstra_requirements<
      decltype(g), decltype(sources), decltype(distances), decltype(predecessors), one_t,
      graph::empty_visitor, 
      std::less<graph::record_t<decltype(distances), GG>>,
      std::plus<graph::record_t<decltype(distances), GG>>
    >);
    static_assert(graph::record_for<std::vector<double>, GG>);
}


TEST_CASE("lookup-based adjacency list test", "[map][concept]") {
  
    my::Graph g {
        {"A", {"B", "C"}}
    };
    
    std::map<my::ID, my::ID> predecessors; // we will store the predecessor of each vertex here

    std::map<my::ID, double> distances; 
    std::vector<my::ID> sources = {my::ID("A")};

  
  auto inc = graph::views::incidence(g, my::ID("A"), one);
  graph::dijkstra_shortest_paths(g, sources, distances, predecessors, one, Visit{});
  
  std::vector<int> v;
  std::vector<std::vector<int>> gg;
  int id;
  (void)graph::vertex_record(v, id);
  
  static_assert(graph::record_for<std::vector<int>, std::vector<std::vector<int>>>);
    static_assert(graph::dijkstra_requirements<
      decltype(g), decltype(sources), decltype(distances), decltype(predecessors), one_t,
      graph::empty_visitor, 
      std::less<graph::record_t<decltype(distances), decltype(g)>>,
      std::plus<graph::record_t<decltype(distances), decltype(g)>>
    >);

  SECTION("vertices_breadth_first_search_view is an input view") {
    REQUIRE(9 == 9);
  }


}
