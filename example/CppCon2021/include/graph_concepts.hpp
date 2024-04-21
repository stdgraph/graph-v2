//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

#ifndef NWGRAPH_GRAPH_CONCEPTS_HPP
#define NWGRAPH_GRAPH_CONCEPTS_HPP

#include <concepts>
#include <ranges>

#if 0
template <class T>
class graph_traits {
  using vertex_id_type = typename T::vertex_id_type;
};

template <typename G>
using vertex_id_t = typename graph_traits<G>::vertex_id_type;

template <typename G>
concept graph = std::semiregular<G> && requires(G g) {
  typename vertex_id_t<G>;
};

template <typename G>
using inner_range_t = std::ranges::range_value_t<G>;

template <typename G>
using inner_value_t = std::ranges::range_value_t<inner_range_t<G>>;

DECL_TAG_INVOKE(degree);
DECL_TAG_INVOKE(target);
DECL_TAG_INVOKE(untarget);

template <typename G>
concept adjacency_list = graph<G> && std::ranges::random_access_range<G> && std::ranges::forward_range<inner_range_t<G>> &&
    std::convertible_to<vertex_id_t<G>, std::ranges::range_difference_t<G>> && requires(G g, vertex_id_t<G> u, inner_value_t<G> e) {
  { g[u] } -> std::convertible_to<inner_range_t<G>>;
  { target(g, e) } -> std::convertible_to<vertex_id_t<G>>;
};

template <typename G>
concept degree_enumerable = adjacency_list<G> && requires(G g, vertex_id_t<G> u) {
  { degree(g[u]) } -> std::convertible_to<std::ranges::range_difference_t<G>>;
};

template <typename G>
concept edge_range = graph<G> && requires(G g, std::ranges::range_value_t<G> e) {
  { source(g, e) } -> std::convertible_to<vertex_id_t<G>>;
  { target(g, e) } -> std::convertible_to<vertex_id_t<G>>;
};

template <template <class> class Outer, template <class> class Inner, std::integral Index, typename... Attributes>
requires std::ranges::random_access_range<Outer<Inner<std::tuple<Index, Attributes...>>>> &&
    std::ranges::forward_range<Inner<std::tuple<Index, Attributes...>>>
struct graph_traits<Outer<Inner<std::tuple<Index, Attributes...>>>> {
  using vertex_id_type = Index;
};

template <template <class> class Outer, template <class> class Inner, std::integral Index>
requires std::ranges::random_access_range<Outer<Inner<Index>>> && std::ranges::forward_range<Inner<Index>>
struct graph_traits<Outer<Inner<Index>>> {
  using vertex_id_type = Index;
};

template <typename R>
concept vertex_list_c = std::ranges::forward_range<R> && !std::is_compound_v<std::ranges::range_value_t<R>>;

template <typename R>
concept edge_list_c = std::ranges::forward_range<R> && requires(std::ranges::range_value_t<R> e) {
  std::get<0>(e);
};

template <typename R>
concept property_edge_list_c = std::ranges::forward_range<R> && requires(std::ranges::range_value_t<R> e) {
  std::get<1>(e);
};

template <typename G>
concept min_idx_adjacency_list = std::ranges::random_access_range<G> && vertex_list_c<inner_range_t<G>> &&
    std::is_convertible_v<inner_value_t<G>, std::ranges::range_difference_t<G>> && requires(G g, inner_value_t<G> u) {
  { g[u] } -> std::convertible_to<inner_range_t<G>>;
};

template <typename G>
concept idx_adjacency_list = std::ranges::random_access_range<G> && edge_list_c<inner_range_t<G>> &&
    std::is_convertible_v<std::tuple_element_t<0, inner_value_t<G>>, std::ranges::range_difference_t<G>> &&
    requires(G g, std::tuple_element_t<0, inner_value_t<G>> u) {
  { g[u] } -> std::convertible_to<inner_range_t<G>>;
};

template <idx_adjacency_list G>
struct graph_traits<G> {
  using vertex_id_type = std::tuple_element_t<0, std::ranges::range_value_t<std::ranges::range_value_t<G>>>;
};

template <min_idx_adjacency_list T, class U>
auto tag_invoke(const target_tag, const T& graph, const U& e) {
  return e;
}

template <idx_adjacency_list T, class U>
auto tag_invoke(const target_tag, const T& graph, const U& e) {
  return std::get<0>(e);
}

template <min_idx_adjacency_list T, class U>
auto tag_invoke(const untarget_tag, const T& graph, const U& e) {
  return std::make_tuple(e);
}

template <idx_adjacency_list T, class U>
auto tag_invoke(const untarget_tag, const T& graph, const U& e) {
  return e;
}

#endif // 0

#endif    // NWGRAPH_GRAPH_CONCEPTS_HPP
