#pragma once

#include <ranges>
#include <concepts>
#include <type_traits>

#include "detail/graph_access.hpp"


#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {
// Template parameters:
// G    - Graph
// GV   - Graph Value (user-defined or void)
// V    - Vertex type
// VKey - Vertex Key type
// VV   - Vertex Value (user-defined or void)
// VR   - Vertex Range
// E    - Edge type
// EV   - Edge Value (user-defined or void)
// ER   - Edge Range
//
// Parameters:
// g         - graph reference
// u,v,x,y   - vertex references
// ukey,vkey - vertex keys
// ui,vi     - vertex iterators
// uv        - edge reference
// uvi       - edge_iterator (use std::optional?)

// edge value types
template <class G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));

/// <summary>
/// Override for an edge type where source and target are unordered
/// For instance, given:
///      vertex_iterator_t<G> ui = ...;
///      for(auto&& uv : edges(g,*ui)) ...
/// if(source_key(g,u) != vertex_key(ui)) then target_key(g,u) == vertex_key(ui)
///
/// Example:
///  namespace my_namespace {
///      template<class X>
///      class my_graph { ... };
///  }
///  namespace std::graph {
///     template<class X>
///     inline constexpr bool is_unordered_edge_v<edge_t<my_namespace::my_graph<X>>> = true;
///  }
/// </summary>
/// <typeparam name="E">The edge type with unordered source and target</typeparam>
template <class E>
inline constexpr bool is_unordered_edge_v = false;

/// <summary>
/// Override for a graph type where edges are defined densely in a matrix to allow for
/// optimized algorithms can take advantage of the memory layout.
///
/// Example:
///  namespace my_namespace {
///      template<class X>
///      class my_graph { ... };
///  }
///  namespace std::graph {
///     template<class X>
///     inline constexpr bool is_adjacency_matrix_v<my_namespace::my_graph<X>> = true;
///  }
/// </summary>
/// <typeparam name="G"></typeparam>
template <class G>
inline constexpr bool is_adjacency_matrix_v = false;

//
// graph concepts
//
template <class G>
concept vertex_range = ranges::forward_range<vertex_range_t<G>> && ranges::sized_range<vertex_range_t<G>> &&
      requires(G&& g, ranges::iterator_t<vertex_range_t<G>> ui) {
  { vertices(g) } -> ranges::forward_range;
  {vertex_key(g, ui)};
};

template <class G, class ER>
concept edge_range = ranges::forward_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  target_key(g, uv);
  target(g, uv);
};

template <class G, class ER>
concept sourced_edge_range =
      requires(G&& g, ranges::range_reference_t<ER> uv, vertex_key_t<G> ukey, vertex_reference_t<G> u) {
  source_key(g, uv);
  source(g, uv);
  edge_key(g, uv);
#  ifdef ENABLE_OTHER_FNC
  other_key(g, uv, ukey);
  other_vertex(g, uv, u);
#  endif
};

template <class G>
concept incidence_graph = vertex_range<G> && edge_range<G, vertex_edge_range_t<G>>;
//!is_same_v<vertex_edge_range_t<G>, vertex_range_t<G>> &&
// CSR fails this condition b/c row_index & col_index are both index_vectors; common?

template <class G>
concept sourced_incidence_graph = incidence_graph<G> && sourced_edge_range<G, vertex_edge_range_t<G>>;

template <class G>
concept unordered_incidence_graph = sourced_incidence_graph<G> && is_unordered_edge_v<edge_t<G>>;

template <class G>
concept adjacency_matrix = is_adjacency_matrix_v<G>;

//
// property concepts
//
template <class G>
concept has_degree = requires(G&& g, vertex_reference_t<G> u) {
  {degree(g, u)};
};

//
// find/contains concepts
//
template <class G>
concept has_find_vertex = requires(G&& g, vertex_key_t<G> ukey) {
  { find_vertex(g, ukey) } -> forward_iterator;
};

template <class G>
concept has_find_vertex_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey, vertex_reference_t<G> u) {
  { find_vertex_edge(g, u, vkey) } -> forward_iterator;
  { find_vertex_edge(g, ukey, vkey) } -> forward_iterator;
};

template <class G>
concept has_contains_edge = requires(G&& g, vertex_key_t<G> ukey, vertex_key_t<G> vkey) {
  { contains_edge(g, ukey, vkey) } -> convertible_to<bool>;
};

namespace views {
  // experimental

  //
  // vertex_view
  // for(auto&& [ukey, u]        : vertexlist(g))
  // for(auto&& [ukey, u, value] : vertexlist(g, [](vertex_reference_t<G> u) { return ...; } )
  //
  template <class VKey, class V, class VV>
  struct vertex_view {
    VKey key;
    V    vertex;
    VV   value;
  };
  template <class VKey, class V>
  struct vertex_view<VKey, V, void> {
    VKey key;
    V    vertex;
  };
  template <class VKey, class VV>
  struct vertex_view<VKey, void, VV> {
    VKey key;
    VV   value;
  };
  template <class VKey>
  struct vertex_view<VKey, void, void> {
    VKey key;
  };

  template <class VKey, class VV>
  using copyable_vertex_t = vertex_view<VKey, void, VV>; // {key, value}

  //
  // edge_view
  //
  // for(auto&& [target_key, uv]        : incidence(g,u))
  // for(auto&& [target_key, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
  //
  // for(auto&& [source_key, target_key, uv]        : incidence(g,u))
  // for(auto&& [source_key, target_key, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
  //
  template <class VKey, bool Sourced, class E, class EV>
  struct edge_view {
    VKey source_key;
    VKey target_key;
    E    edge;
    EV   value;
  };

  template <class VKey, class E>
  struct edge_view<VKey, true, E, void> {
    VKey source_key;
    VKey target_key;
    E    edge;
  };
  template <class VKey>
  struct edge_view<VKey, true, void, void> {
    VKey source_key;
    VKey target_key;
  };
  template <class VKey, class EV>
  struct edge_view<VKey, true, void, EV> {
    VKey source_key;
    VKey target_key;
    EV   value;
  };

  template <class VKey, class E, class EV>
  struct edge_view<VKey, false, E, EV> {
    VKey target_key;
    E    edge;
    EV   value;
  };
  template <class VKey, class E>
  struct edge_view<VKey, false, E, void> {
    VKey target_key;
    E    edge;
  };

  template <class VKey, class EV>
  struct edge_view<VKey, false, void, EV> {
    VKey target_key;
    EV   value;
  };
  template <class VKey>
  struct edge_view<VKey, false, void, void> {
    VKey target_key;
  };

  //
  // targeted_edge
  // for(auto&& [vkey,uv,value] : edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
  // for(auto&& [vkey,uv]       : edges_view(g, u) )
  //
  template <class VKey, class E, class EV>
  using targeted_edge = edge_view<VKey, false, E, EV>; // {target_key, edge, [, value]}

  //
  // sourced_edge
  // for(auto&& [ukey,vkey,uv,value] : sourced_edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
  // for(auto&& [ukey,vkey,uv]       : sourced_edges_view(g, u) )
  //
  template <class VKey, class V, class E, class EV>
  using sourced_edge = edge_view<VKey, true, E, EV>; // {source_key, target_key, edge, [, value]}

  //
  // edgelist_edge
  // for(auto&& [ukey,vkey,uv,value] : edges_view(g, [](vertex_edge_reference_t<G> g) { return ...; } )
  // for(auto&& [ukey,vkey,uv]       : edges_view(g) )
  //
  template <class VKey, class E, class EV>
  using edgelist_edge = edge_view<VKey, true, E, EV>; // {source_key, target_key [, edge] [, value]}

  //
  // copyable_edge
  //
  template <class VKey, class EV>
  using copyable_edge_t = edge_view<VKey, true, void, EV>; // {source_key, target_key [, value]}

  //
  // neighbor_view (for adjacency)
  //
  template <class VKey, bool Sourced, class V, class VV>
  struct neighbor_view {
    VKey source_key;
    VKey target_key;
    V    target;
    VV   value;
  };

  template <class VKey, class V, class VV>
  struct neighbor_view<VKey, false, V, VV> {
    VKey target_key;
    V    target;
    VV   value;
  };

  template <class VKey, class V>
  struct neighbor_view<VKey, false, V, void> {
    VKey target_key;
    V    target;
  };

  template <class VKey, class V>
  struct neighbor_view<VKey, true, V, void> {
    VKey target_key;
    V    target;
  };

  template <class VKey>
  struct neighbor_view<VKey, true, void, void> {
    VKey target_key;
  };

  //
  // view concepts
  //
  template <class T, class VKey, class VV>
  concept copyable_vertex = convertible_to<T, copyable_vertex_t<VKey, VV>>;

  template <class T, class VKey, class EV>
  concept copyable_edge = convertible_to<T, copyable_edge_t<VKey, EV>>;

  //
  // is_sourced<G>
  //
  template <class T>
  inline constexpr bool is_sourced_v = false;
  template <class VKey, class V, class VV>
  inline constexpr bool is_sourced_v<edge_view<VKey, true, V, VV>> = true;
  template <class VKey, class V, class VV>
  inline constexpr bool is_sourced_v<neighbor_view<VKey, true, V, VV>> = true;

} // namespace views


/// <summary>
/// ref_to_ptr changes a reference to a pointer and stores it as a pointer in value.
/// Pointers and values are stored as-is.
///
/// ref_to_ptr is similar to reference_wrapper but there are a couple of
/// differences when used in a view iterator that are important:
/// 1.  It is default constructible, as long as the value being stored is default
///     constructible.
/// 2.  It stores a copy of the value if not a reference or a pointer.
/// </summary>
/// <typeparam name="T">The type to store</typeparam>
namespace _detail {
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

} // namespace _detail

} // namespace std::graph

#endif //GRAPH_HPP
