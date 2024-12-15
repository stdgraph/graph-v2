//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

#include "graph_concepts.hpp"
#include <map>
#include <ranges>
#include <tuple>
#include <vector>

/**
 * Tuple utilities to get the "cdr" of a tuple (for our purposes)
 */

template <size_t N, typename... Ts>
auto nth_cdr(std::tuple<Ts...> t) {
  return [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
    return std::tuple{std::get<Ns + N>(t)...};
  }(std::make_index_sequence<sizeof...(Ts) - N>());
}

template <typename... Ts>
auto props(std::tuple<Ts...> t) {
  return nth_cdr<2>(t);
}

template <typename... Ts>
auto graph_edge(std::tuple<Ts...> t) {
  return nth_cdr<1>(t);
}

/**
 * Fill a plain graph from edge list
 */
template <class EdgeList, class Adjacency>
void push_back_plain_fill(const EdgeList& edge_list, Adjacency& adj, bool directed, size_t idx) {
  const size_t jdx = (idx + 1) % 2;

  for (auto&& e : edge_list) {

    if (idx == 0) {
      std::apply([&](size_t u, size_t v) { adj[u].emplace_back(v); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v) { adj[v].emplace_back(u); }, e);
      }
    } else {
      std::apply([&](size_t u, size_t v) { adj[v].emplace_back(u); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v) { adj[u].emplace_back(v); }, e);
      }
    }
  }
}

/**
 * Fill a non-plain graph from edge list
 */
template <class EdgeList, class Adjacency>
void push_back_fill(const EdgeList& edge_list, Adjacency& adj, bool directed, size_t idx) {
  const size_t jdx = (idx + 1) % 2;

  for (auto&& e : edge_list) {

    if (idx == 0) {
      std::apply([&](size_t u, size_t v, auto... props) { adj[u].emplace_back(v, props...); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v, auto... props) { adj[v].emplace_back(u, props...); }, e);
      }
    } else {
      std::apply([&](size_t u, size_t v, auto... props) { adj[v].emplace_back(u, props...); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v, auto... props) { adj[u].emplace_back(v, props...); }, e);
      }
    }
  }
}

/**
 *  Make a map from data to the index value of each element in its container
 */
template <std::ranges::random_access_range R>
auto make_index_map(const R& range) {
  using value_type = std::ranges::range_value_t<R>;

  std::map<value_type, size_t> the_map;
  for (size_t i = 0; i < size(range); ++i) {
    the_map[range[i]] = i;
  }
  return the_map;
}

/**
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
 */
//template <template <class> class I = std::vector, class M, std::ranges::random_access_range E>
template <class M,
          std::ranges::random_access_range E,
          class EdgeList = std::vector<decltype(std::tuple_cat(std::tuple<size_t, size_t>()))>>
auto make_plain_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v])); }, e);
  }

  return index_edges;
}

/**
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
 */
template <class M,
          std::ranges::random_access_range E,
          class EdgeList = std::vector<decltype(std::tuple_cat(std::tuple<size_t, size_t>(), props(E()[0])))>>
auto make_property_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v,
                   auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v], props_...)); },
               e);
  }

  return index_edges;
}

/**
 * Make an edge list indexing back to the original data, e.g., vector<tuple<size_t, size_t, size_t>>
 */
template <class I = std::vector<std::tuple<size_t, size_t, size_t>>, class M, std::ranges::random_access_range E>
auto make_index_edges(M& map, const E& edges) {

  auto index_edges = I();

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = std::get<0>(edges[i]);
    auto right = std::get<1>(edges[i]);

    index_edges.push_back(std::make_tuple(map[left], map[right], i));
  }

  return index_edges;
}

/**  
 *  Make a plain graph from data, e.g., vector<vector<index>>
 */
template <std::ranges::random_access_range V,
          std::ranges::random_access_range E,
          graph::basic_adjacency_list      Graph = std::vector<std::vector<size_t>>>
auto make_plain_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {
  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_plain_edges(vertex_map, edges);

  Graph G(size(vertices));
  push_back_plain_fill(index_edges, G, directed, idx);

  return G;
}

/**  
 *  Make an index graph from data, e.g., vector<vector<tuple<index, index>>>
 */
template <std::ranges::random_access_range V,
          std::ranges::random_access_range E,
          graph::basic_adjacency_list      Graph = std::vector<std::vector<std::tuple<size_t, size_t>>>>
auto make_index_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_index_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(index_edges, G, directed, idx);

  return G;
}

/**  
 *  Make a property graph from data, e.g., vector<vector<tuple<index, properties...>>>
 */
template <std::ranges::random_access_range V,
          std::ranges::forward_range       E,
          graph::basic_adjacency_list      Graph =
                std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_property_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map     = make_index_map(vertices);
  auto property_edges = make_property_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(property_edges, G, directed, idx);

  return G;
}

/**  
 *  Functions for building bipartite graphs
 */
template <class I = std::vector<std::tuple<size_t, size_t>>,
          std::ranges::random_access_range V,
          std::ranges::random_access_range E>
auto data_to_graph_edge_list(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto left_map  = make_index_map(left_vertices);
  auto right_map = make_index_map(right_vertices);

  std::vector<std::tuple<size_t, size_t>> index_edges;

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = std::get<0>(edges[i]);
    auto right = std::get<1>(edges[i]);

    index_edges.push_back({left_map[left], right_map[right]});
  }

  return index_edges;
}

template <std::ranges::random_access_range V1,
          std::ranges::random_access_range V2,
          std::ranges::random_access_range E,
          graph::basic_adjacency_list      Graph =
                std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_plain_bipartite_graph(const V1& left_vertices, const V2& right_vertices, const E& edges, size_t idx = 0) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(graph_size);
  push_back_plain_fill(index_edges, G, true, idx);

  return G;
}

template <std::ranges::random_access_range V1,
          std::ranges::random_access_range V2,
          std::ranges::random_access_range E,
          class Graph = std::vector<std::vector<size_t>>>
auto make_plain_bipartite_graphs(const V1& left_vertices, const V2& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_plain_fill(index_edges, G, true, 0);
  push_back_plain_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <size_t idx  = 0,
          class Graph = std::vector<std::vector<size_t>>,
          std::ranges::random_access_range V,
          std::ranges::random_access_range E>
auto make_bipartite_graph(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_fill(index_edges, G, true, idx);

  return G;
}

template <std::ranges::random_access_range V,
          std::ranges::random_access_range E,
          graph::basic_adjacency_list      Graph =
                std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_bipartite_graphs(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_fill(index_edges, G, true, 0);
  push_back_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <class Graph1, class Graph2, class IndexGraph = std::vector<std::vector<std::tuple<size_t, size_t>>>>
auto join(const Graph1& G, const Graph2& H) {

  std::vector<std::tuple<size_t, size_t, size_t>> s_overlap;

  for (size_t i = 0; i < H.size(); ++i) {
    for (auto&& k : H[i]) {
      for (auto&& j : G[graph::target_id(H, k)]) {
        if (graph::target_id(G, j) != i) {
          s_overlap.push_back({i, graph::target_id(G, j), graph::target_id(H, k)});
        }
      }
    }
  }

  IndexGraph L(size(H));
  push_back_fill(s_overlap, L, true, 0);

  return L;
}
