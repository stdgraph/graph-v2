#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views//depth_first_search.hpp"
#include "graph/container/csr_graph.hpp"
#include "graph/algorithm/dijkstra_clrs.hpp"
#include <forward_list>
#include <cassert>

using namespace std::literals;
using namespace std::graph;
using namespace std::graph::views;

using std::cout;
using std::endl;

using routes_csr_graph_type = std::graph::container::csr_graph<double, std::string, std::string>;

template <typename G>
struct out_city {
  const G&                    g;
  vertex_id_t<G>              city_id;
  vertex_reference_t<const G> city;

  out_city(const G& graph, vertex_id_t<G> uid, vertex_reference_t<G> u) : g(graph), city_id(uid), city(u) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const out_city& rhs) {
    auto&& [g, city_id, city] = rhs;
    os << vertex_value(g, city) << " [" << city_id << "]";
    return os;
  }
};

template <typename G>
constexpr auto find_frankfurt_id(const G& g) {
  return find_city_id(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

TEST_CASE("Germany Routes Example", "[example][germany][routes][shortest_paths][csr]") {
  init_console();

  using G                         = routes_csr_graph_type;
  csv::string_view csv_file       = TEST_DATA_ROOT_DIR "germany_routes.csv";
  csv::string_view undir_out_file = TEST_OUTPUT_ROOT_DIR "germany_routes_undir.gv";
  csv::string_view dir_out_file   = TEST_OUTPUT_ROOT_DIR "germany_routes_dir.gv";
  csv::string_view bidir_out_file = TEST_OUTPUT_ROOT_DIR "germany_routes_bidir.gv";
  csv::string_view final_out_file = TEST_OUTPUT_ROOT_DIR "germany_routes_final.gv";

  // Load for graphviz
  {
    auto&& g = load_ordered_graph<G>(csv_file, name_order_policy::order_found, false);
    output_routes_graphviz(g, undir_out_file, directedness::undirected, "transparent");
    output_routes_graphviz(g, dir_out_file, directedness::directed, "transparent");
    output_routes_graphviz(g, bidir_out_file, directedness::bidirected, "transparent");
    // name_order_policy::source_order_found gives best output with least overlap for germany routes

    csv::string_view g3 = TEST_OUTPUT_ROOT_DIR "g3.gv";
    output_routes_graphviz_adjlist(g, g3, "transparent");
  }

#if 1
  auto  g            = load_ordered_graph<G>(csv_file, name_order_policy::source_order_found, true);
  auto& frankfurt    = **find_frankfurt(g);
  auto  frankfurt_id = find_frankfurt_id(g);
  output_routes_graphviz(g, final_out_file, directedness::directed2, "transparent");

  //std::string_view munchen_name = "M\xC3\xBCnchen";
  //auto&& u   = **find_city(g, munchen_name);
  //auto   uid = find_city_id(g, munchen_name);

  cout << "DFS Path Segments (depth):" << endl;
  for (auto&& [uid, u] : vertexlist(g)) {
    cout << "From " << out_city(g, uid, u) << "\n";
    auto dfs = vertices_depth_first_search(g, uid);
    for (auto&& [vid, v] : dfs) {
      cout << "   --> " << out_city(g, vid, v) << " - " << dfs.depth() << " segments" << endl;
    }
  }

  // Shortest Paths (segments)
  {
    auto                        weight_1 = [](edge_reference_t<G> uv) -> int { return 1; };
    std::vector<int>            distance(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distance, predecessor, weight_1);

    cout << "Shortest paths from " << vertex_value(g, frankfurt) << " by segment" << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid)
      if (distance[uid] > 0)
        cout << "  --> " << out_city(g, uid, *find_vertex(g, uid)) << " - " << distance[uid] << " segments" << endl;
  }

  // Shortest Paths (km)
  {
    auto                        weight = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    std::vector<double>         distance(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distance, predecessor, weight);

    cout << "Shortest paths from " << vertex_value(g, frankfurt) << " by km" << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid)
      if (distance[uid] > 0)
        cout << "  --> " << out_city(g, uid, *find_vertex(g, uid)) << " - " << distance[uid] << "km" << endl;
  }
#endif

  //using inv_vec = std::vector<int>;
  //using sr = std::ranges::subrange<inv_vec::iterator>;
  //cout << "\nUsing CPO functions" << endl;
  //auto frantfurt = germany_routes.frankfurt();
  //REQUIRE(frantfurt != end(germany_routes.cities()));

  // (uncomment to generate a graphviz file)
}

// https://www.reddit.com/r/cpp/comments/4yp7fv/c17_structured_bindings_convert_struct_to_a_tuple/
// https://gist.github.com/utilForever/1a058050b8af3ef46b58bcfa01d5375d
template <class T, class... TArgs>
decltype(void(T{std::declval<TArgs>()...}), std::true_type{}) test_is_braces_constructible(int);
template <class, class...>
std::false_type test_is_braces_constructible(...);
template <class T, class... TArgs>
using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

struct any_type {
  template <class T>
  constexpr operator T(); // non explicit
};

template <class T>
auto to_tuple(T&& object) noexcept {
  using type = std::decay_t<T>;
  if constexpr (is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3, p4] = object;
    return std::make_tuple(p1, p2, p3, p4);
  } else if constexpr (is_braces_constructible<type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3] = object;
    return std::make_tuple(p1, p2, p3);
  } else if constexpr (is_braces_constructible<type, any_type, any_type>{}) {
    auto&& [p1, p2] = object;
    return std::make_tuple(p1, p2);
  } else if constexpr (is_braces_constructible<type, any_type>{}) {
    auto&& [p1] = object;
    return std::make_tuple(p1);
  } else {
    return std::make_tuple();
  }
}

template <class T>
using to_tuple_t = decltype(to_tuple(std::declval<T>()));

template <class C>
concept has_push_back = requires(C& container, std::ranges::range_value_t<C>& val) {
  {container.push_back(val)};
};
template <class C>
concept has_push_front = requires(C& container, std::ranges::range_value_t<C>& val) {
  {container.push_front(val)};
};

template <class Outer>
concept range_of_ranges = std::ranges::random_access_range<Outer> &&   // outer range is an random_access_range
      std::ranges::forward_range<std::ranges::range_value_t<Outer>> && // inner range is forward_range
      std::integral<std::tuple_element_t<0,
                                         to_tuple_t<std::ranges::range_value_t<std::ranges::range_value_t<
                                               Outer>>>>>; // first element of edge is integral (target_id)

template <class Outer, class VVR>
requires range_of_ranges<Outer> && std::ranges::contiguous_range<Outer> && std::ranges::random_access_range<VVR>
// contiguous so we can calculate vertex_id, given vertex_value(g,u), where u is a vertex reference
class rr_adaptor {
public:
  using graph_type        = rr_adaptor<Outer, VVR>;
  using vertices_range    = Outer;
  using vertex_type       = std::ranges::range_value_t<vertices_range>;
  using edges_range       = std::ranges::range_value_t<vertices_range>; // Inner range
  using edge_type         = std::ranges::range_value_t<edges_range>;    //
  using vertex_id_type    = std::tuple_element_t<0, to_tuple_t<edge_type>>;
  using edge_value_type   = std::tuple_element_t<1, to_tuple_t<edge_type>>;
  using vertex_value_type = std::ranges::range_value_t<VVR>;

public:
  template <class InputEdges, class EdgeIdFnc>
  rr_adaptor(VVR& vertex_values, const InputEdges& input, const EdgeIdFnc& edge_id_fn, bool dup_edges = true)
        : vertex_values_(vertex_values) {
    size_t max_vid = max_vertex_id(input, edge_id_fn);

    // assure vertices & vertex_values are big enough for the elements & sized the same
    size_t vcnt = std::max(max_vid + 1, vertex_values_.size());
    vertices_.resize(vcnt);
    vertex_values_.resize(vcnt);

    // Add edges
    for (auto&& e : input) {
      auto edge_key = edge_id_fn(e); // pair<VId,VId>
      push(vertices_[edge_key.first], edge_key.second);
      if (dup_edges)
        vertices_[edge_key.second].push_back(edge_key.first);
    }
  }

  template <class InputEdges, class EdgeIdFnc, class EdgeValFnc>
  rr_adaptor(VVR&              vertex_values,
             const InputEdges& input,
             const EdgeIdFnc&  edge_id_fn,
             const EdgeValFnc& edge_val_fn,
             bool              dup_edges = true)
        : vertex_values_(vertex_values) {
    vertex_id_type max_vid = max_vertex_id(input, edge_id_fn);

    // assure vertices & vertex_values are big enough for the elements & sized the same
    size_t vcnt = std::max(static_cast<size_t>(max_vid + 1), vertex_values_.size());
    vertices_.resize(vcnt);
    vertex_values_.resize(vcnt);

    // Add edges
    for (auto&& e : input) {
      auto edge_key = edge_id_fn(e); // pair<VId,VId>
      push(vertices_[static_cast<size_t>(edge_key.first)],
           {static_cast<vertex_id_type>(edge_key.second), edge_val_fn(e)});
      if (dup_edges)
        push(vertices_[static_cast<size_t>(edge_key.second)],
             {static_cast<vertex_id_type>(edge_key.first), edge_val_fn(e)});
    }
  }

private:
  // scan for the max vertex id used in input edges
  template <class InputEdges, class EdgeIdFnc>
  vertex_id_type max_vertex_id(const InputEdges& input, const EdgeIdFnc& edge_id_fn) const noexcept {
    vertex_id_type max_vid = 0;
    for (auto&& e : input) {
      auto edge_key = edge_id_fn(e); // pair<VId,VId>
      max_vid       = std::max(max_vid, std::max(edge_key.first, edge_key.second));
    }
    return max_vid;
  }

  void push(edges_range& edges, const edge_type& val) {
    if constexpr (has_push_back<edges_range>)
      edges.push_back(val);
    else if constexpr (has_push_front<edges_range>)
      edges.push_front(val);
  }

private:
  friend constexpr vertices_range& tag_invoke(::std::graph::tag_invoke::vertices_fn_t, graph_type& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_range& tag_invoke(::std::graph::tag_invoke::vertices_fn_t, const graph_type& g) {
    return g.vertices_;
  }

  friend vertex_id_type
  tag_invoke(std::graph::tag_invoke::vertex_id_fn_t, const graph_type& g, std::ranges::iterator_t<vertices_range> ui) {
    return static_cast<vertex_id_type>(ui - std::ranges::begin(g.vertices_)); // overriden to assure correct type returned
  }

  friend constexpr edges_range& tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, vertex_type& u) {
    return u;
  }
  friend constexpr const edges_range&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u;
  }

  friend constexpr edges_range&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid];
  }
  friend constexpr const edges_range&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid];
  }

  friend constexpr vertex_id_type
  tag_invoke(::std::graph::tag_invoke::target_id_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return get<0>(to_tuple(uv));
  }

  friend constexpr vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    size_t uidx = &u - g.vertices_.data();
    return g.vertex_values_[uidx];
  }
  friend constexpr const vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    size_t uidx = &u - g.vertices_.data();
    return g.vertex_values_[uidx];
  }

  // edge_value(g,uv)
  friend constexpr edge_value_type& tag_invoke(std::graph::tag_invoke::edge_value_fn_t, graph_type& g, edge_type& uv) {
    auto t = to_tuple(uv);
    return get<1>(t);
  }
  friend constexpr const edge_value_type&
  tag_invoke(std::graph::tag_invoke::edge_value_fn_t, const graph_type& g, const edge_type& uv) {
    auto t = to_tuple(uv);
    return get<1>(t);
  }

private:
  vertices_range vertices_;
  VVR&           vertex_values_;
};

using city_id_type = int32_t;
struct city {
  std::string name;
};
struct route {
  city_id_type target_id = 0;
  double       distance  = 0.0; // km
};

TEST_CASE("Germany with shortest paths", "[example][germany][routes][shortest_paths][rr]") {
  using city_names_type      = std::vector<std::string_view>;
  city_names_type city_names = {"Frankfürt", "Mannheim", "Karlsruhe", "Augsburg", "Würzburg",
                                "Nürnberg",  "Kassel",   "Erfurt",    "München",  "Stuttgart"};
  struct route_input {
    city_id_type from     = 0;
    city_id_type to       = 0;
    double       distance = 0.0; // km
  };
  std::vector<route_input> segments = {{0, 1, 85.0},  {0, 4, 217.0}, {0, 6, 173.0}, {1, 2, 80.0},
                                       {2, 3, 250.0}, {3, 8, 84.0},  {4, 5, 103.0}, {4, 7, 186.0},
                                       {5, 8, 167.0}, {5, 9, 183.0}, {6, 8, 502.0}};

  using RR = std::vector<std::forward_list<route>>;
  using G  = rr_adaptor<RR, city_names_type>;

  auto edge_id_fn  = [](const route_input& r) { return std::pair(r.from, r.to); };
  auto edge_val_fn = [](const route_input& r) { return r.distance; };
  G    g(city_names, segments, edge_id_fn, edge_val_fn, true);

  auto& frankfurt    = **find_frankfurt(g);
  auto  frankfurt_id = find_frankfurt_id(g);

  // Shortest Paths (segments)
  {
    auto                        weight_1 = [](edge_reference_t<G> uv) { return 1.0; };
    std::vector<double>         distance(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distance, predecessor, weight_1);

    cout << "Shortest paths from " << vertex_value(g, frankfurt) << " by segment" << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid)
      if (distance[uid] > 0)
        cout << "  --> " << out_city(g, uid, *find_vertex(g, uid)) << " - " << distance[uid] << " segments" << endl;
  }

  // Shortest Paths (km)
  {
    auto                        weight = [&g](edge_reference_t<G> uv) { return edge_value(g, uv); };
    std::vector<double>         distance(size(vertices(g)));
    std::vector<vertex_id_t<G>> predecessor(size(vertices(g)));
    dijkstra_clrs(g, frankfurt_id, distance, predecessor, weight);

    cout << "Shortest paths from " << vertex_value(g, frankfurt) << " by km" << endl;
    for (vertex_id_t<G> uid = 0; uid < size(vertices(g)); ++uid)
      if (distance[uid] > 0)
        cout << "  --> " << out_city(g, uid, *find_vertex(g, uid)) << " - " << distance[uid] << "km" << endl;
  }
}
