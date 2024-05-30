#pragma once
#include <map>
#include <ranges>
#include <tuple>
#include <vector>
#include "graph/edgelist.hpp"

namespace std::graph {

// Common types for DFS & BFS views
enum three_colors : int8_t { black, white, gray }; // { finished, undiscovered, discovered }
enum struct cancel_search : int8_t { continue_search, cancel_branch, cancel_all };


/**
 * Tuple utilities to get the "cdr" of a tuple (for our purposes)
 */

template <size_t N, typename... Ts>
auto nth_cdr(tuple<Ts...> t) {
  return [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
    return tuple{std::get<Ns + N>(t)...};
  }(std::make_index_sequence<sizeof...(Ts) - N>());
}

template <typename... Ts>
auto props(tuple<Ts...> t) {
  return nth_cdr<2>(t);
}

template <typename... Ts>
auto graph_edge(tuple<Ts...> t) {
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
template <ranges::random_access_range R>
auto make_index_map(const R& range) {
  using value_type = ranges::range_value_t<R>;

  std::map<value_type, size_t> the_map;
  for (size_t i = 0; i < size(range); ++i) {
    the_map[range[i]] = i;
  }
  return the_map;
}

/**
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
 */
//template <template <class> class I = vector, class M, ranges::random_access_range E>
template <class M, ranges::random_access_range E, class EdgeList = vector<decltype(tuple_cat(tuple<size_t, size_t>()))>>
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
          ranges::random_access_range E,
          class EdgeList = vector<decltype(tuple_cat(tuple<size_t, size_t>(), props(E()[0])))>>
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
template <class I = vector<tuple<size_t, size_t, size_t>>, class M, edgelist::basic_sourced_edgelist E>
auto make_index_edges(M& map, const E& edges) {

  auto index_edges = I();

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = source_id(edges[i]);
    auto right = target_id(edges[i]);

    index_edges.push_back(std::make_tuple(map[left], map[right], i));
  }

  return index_edges;
}

/**  
 *  Make a plain graph from data, e.g., vector<vector<index>>
 */
template <ranges::random_access_range V,
          ranges::random_access_range E,
          basic_adjacency_list        Graph = vector<vector<size_t>>>
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
template <ranges::random_access_range V,
          ranges::random_access_range E,
          basic_adjacency_list        Graph = vector<vector<tuple<size_t, size_t>>>>
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
template <
      ranges::random_access_range V,
      ranges::forward_range       E,
      basic_adjacency_list Graph = vector<vector<decltype(tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
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
template <class I = vector<tuple<size_t, size_t>>, ranges::random_access_range V, edgelist::basic_sourced_edgelist E>
auto data_to_graph_edge_list(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto left_map  = make_index_map(left_vertices);
  auto right_map = make_index_map(right_vertices);

  vector<tuple<size_t, size_t>> index_edges;

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = source_id(edges[i]);
    auto right = target_id(edges[i]);

    index_edges.push_back({left_map[left], right_map[right]});
  }

  return index_edges;
}

template <
      ranges::random_access_range V1,
      ranges::random_access_range V2,
      ranges::random_access_range E,
      basic_adjacency_list Graph = vector<vector<decltype(tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_plain_bipartite_graph(const V1& left_vertices, const V2& right_vertices, const E& edges, size_t idx = 0) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_plain_fill(index_edges, G, true, idx);

  return G;
}

template <ranges::random_access_range V1,
          ranges::random_access_range V2,
          ranges::random_access_range E,
          basic_adjacency_list        Graph = vector<vector<size_t>>>
auto make_plain_bipartite_graphs(const V1& left_vertices, const V2& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_plain_fill(index_edges, G, true, 0);
  push_back_plain_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <size_t                      idx   = 0,
          basic_adjacency_list        Graph = vector<vector<size_t>>,
          ranges::random_access_range V,
          ranges::random_access_range E>
auto make_bipartite_graph(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_fill(index_edges, G, true, idx);

  return G;
}

template <
      ranges::random_access_range V,
      ranges::random_access_range E,
      basic_adjacency_list Graph = vector<vector<decltype(tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_bipartite_graphs(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_fill(index_edges, G, true, 0);
  push_back_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <basic_adjacency_list       Graph1,
          basic_adjacency_list       Graph2,
          basic_index_adjacency_list IndexGraph = vector<vector<tuple<size_t, size_t>>>>
auto join(const Graph1& G, const Graph2& H) {
  vector<tuple<size_t, size_t, size_t>> s_overlap;

  for (size_t i = 0; i < H.size(); ++i) {
    for (auto&& k : H[i]) {
      for (auto&& j : G[target_id(H, k)]) {
        if (target_id(G, j) != i) {
          s_overlap.push_back({i, target_id(G, j), target_id(H, k)});
        }
      }
    }
  }

  IndexGraph L(size(H));
  push_back_fill(s_overlap, L, true, 0);

  return L;
}


namespace _detail {
  template <class G, bool Sourced>
  class _source_vertex {
  public:
    using vertex_id_type = vertex_id_t<G>;

    _source_vertex(vertex_id_type id) : id_(id) {}

    _source_vertex()                          = default;
    _source_vertex(const _source_vertex& rhs) = default;
    _source_vertex(_source_vertex&&)          = default;
    ~_source_vertex()                         = default;

    _source_vertex& operator=(const _source_vertex&) = default;
    _source_vertex& operator=(_source_vertex&&)      = default;

    constexpr vertex_id_type _source_vertex_id() const noexcept { return id_; }

  protected:
    vertex_id_type id_ = 0;
  };

  template <class G>
  class _source_vertex<G, false> {
  public:
    using vertex_id_type = vertex_id_t<G>;

    _source_vertex(vertex_id_type id) {}
    _source_vertex() = default;
  };

  /**
   * @brief ref_to_ptr changes a reference to a pointer and stores it as a pointer in value.
   *        Pointers and values are stored as-is.
   *
   * @c ref_to_ptr is similar to @c reference_wrapper but there are a couple of
   * differences when used in a view iterator that are important:
   * 1.  It is default constructible, as long as the value being stored is default
   *     constructible.
   * 2.  It stores a copy of the value if not a reference or a pointer.
   *
   * @tparam T    The type to store
  */
  template <class T>
  class ref_to_ptr {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");

    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(const T& rhs) : value(rhs) {}
    constexpr ~ref_to_ptr() = default;

    constexpr ref_to_ptr& operator=(const T& rhs) {
      value = rhs;
      return *this;
    }

    constexpr operator bool() const noexcept { return true; }

    constexpr T*       get() noexcept { return &value; }
    constexpr const T* get() const noexcept { return &value; }

    constexpr operator T&() noexcept { return value; }
    constexpr operator const T&() const noexcept { return value; }

  private:
    T value = {};
  };

  template <class T>
  class ref_to_ptr<T&> {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");
    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(T& rhs) noexcept : value(&rhs) {}
    constexpr ~ref_to_ptr() = default;

    ref_to_ptr& operator=(T& rhs) noexcept {
      value = &rhs;
      return *this;
    }

    constexpr T*       get() noexcept { return value; }
    constexpr const T* get() const noexcept { return value; }

    constexpr operator bool() const noexcept { return value != nullptr; }

    constexpr operator T&() noexcept {
      assert(value);
      return *value;
    }
    constexpr operator const T&() const noexcept {
      assert(value);
      return *value;
    }

  private:
    T* value = nullptr;
  };

  template <class T>
  class ref_to_ptr<T*> {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");
    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(T* rhs) noexcept : value(rhs) {}
    constexpr ~ref_to_ptr() = default;

    constexpr ref_to_ptr& operator=(T* rhs) noexcept {
      value = rhs;
      return *this;
    }

    constexpr T*       get() noexcept { return value; }
    constexpr const T* get() const noexcept { return value; }

    constexpr operator bool() const noexcept { return value != nullptr; }

    constexpr operator T&() noexcept {
      assert(value);
      return *value;
    }
    constexpr operator const T&() const noexcept {
      assert(value);
      return *value;
    }

  private:
    T* value = nullptr;
  };


  template <class A>
  concept is_allocator_v = is_copy_constructible_v<A> && requires(A alloc, size_t n) {
    { alloc.allocate(n) };
  };


} // namespace _detail


} // namespace std::graph
