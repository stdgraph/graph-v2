#pragma once

namespace std::graph::views {

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

template <class VKey, class VV>
struct neighbor_view<VKey, false, void, VV> {
  VKey target_key;
  VV   value;
};

template <class VKey>
struct neighbor_view<VKey, false, void, void> {
  VKey target_key;
};

template <class VKey, class V>
struct neighbor_view<VKey, true, V, void> {
  VKey source_key;
  VKey target_key;
  V    target;
};

template <class VKey, class VV>
struct neighbor_view<VKey, true, void, VV> {
  VKey source_key;
  VKey target_key;
  VV   value;
};

template <class VKey>
struct neighbor_view<VKey, true, void, void> {
  VKey source_key;
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


template <class G, bool Sourced>
class source_vertex {
public:
  using vertex_key_type = vertex_key_t<G>;

  source_vertex(vertex_key_type key) : key_(key) {}

  source_vertex()                         = default;
  source_vertex(const source_vertex& rhs) = default;
  source_vertex(source_vertex&&)          = default;
  ~source_vertex()                        = default;

  source_vertex& operator=(const source_vertex&) = default;
  source_vertex& operator=(source_vertex&&) = default;

  constexpr vertex_key_type source_vertex_key() const noexcept { return key_; }

protected:
  vertex_key_type key_ = 0;
};

template <class G>
class source_vertex<G, false> {
public:
  using vertex_key_type = vertex_key_t<G>;

  source_vertex(vertex_key_type key) {}
  source_vertex() = default;
};

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


} // namespace std::graph::views
