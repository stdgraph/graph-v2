#pragma once

namespace std::graph {

// Common types for DFS & BFS views
enum three_colors : int8_t { black, white, grey }; // { finished, undiscovered, discovered }
enum struct cancel_search : int8_t { continue_search, cancel_branch, cancel_all };


//
// vertex_view
// for(auto&& [uid, u]        : vertexlist(g))
// for(auto&& [uid, u, value] : vertexlist(g, [](vertex_reference_t<G> u) { return ...; } )
//
template <class VId, class V, class VV>
struct vertex_view {
  VId id;
  V   vertex;
  VV  value;
};
template <class VId, class V>
struct vertex_view<VId, V, void> {
  VId id;
  V   vertex;
};
template <class VId, class VV>
struct vertex_view<VId, void, VV> {
  VId id;
  VV  value;
};
template <class VId>
struct vertex_view<VId, void, void> {
  VId id;
};

template <class VId, class VV>
using copyable_vertex_t = vertex_view<VId, void, VV>; // {id, value}

//
// edge_view
//
// for(auto&& [target_id, uv]        : incidence(g,u))
// for(auto&& [target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
// for(auto&& [source_id, target_id, uv]        : incidence(g,u))
// for(auto&& [source_id, target_id, uv, value] : incidence(g,u, [](edge_reference_t<G> uv) { return ...; })
//
template <class VId, bool Sourced, class E, class EV>
struct edge_view {
  VId source_id;
  VId target_id;
  E   edge;
  EV  value;
};

template <class VId, class E>
struct edge_view<VId, true, E, void> {
  VId source_id;
  VId target_id;
  E   edge;
};
template <class VId>
struct edge_view<VId, true, void, void> {
  VId source_id;
  VId target_id;
};
template <class VId, class EV>
struct edge_view<VId, true, void, EV> {
  VId source_id;
  VId target_id;
  EV  value;
};

template <class VId, class E, class EV>
struct edge_view<VId, false, E, EV> {
  VId target_id;
  E   edge;
  EV  value;
};
template <class VId, class E>
struct edge_view<VId, false, E, void> {
  VId target_id;
  E   edge;
};

template <class VId, class EV>
struct edge_view<VId, false, void, EV> {
  VId target_id;
  EV  value;
};
template <class VId>
struct edge_view<VId, false, void, void> {
  VId target_id;
};

//
// targeted_edge
// for(auto&& [vid,uv,value] : edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [vid,uv]       : edges_view(g, u) )
//
//template <class VId, class E, class EV>
//using targeted_edge = edge_view<VId, false, E, EV>; // {target_id, edge, [, value]}

//
// sourced_edge
// for(auto&& [uid,vid,uv,value] : sourced_edges_view(g, u, [](vertex_edge_reference_t<G> uv) { return ...; } )
// for(auto&& [uid,vid,uv]       : sourced_edges_view(g, u) )
//
//template <class VId, class V, class E, class EV>
//using sourced_edge = edge_view<VId, true, E, EV>; // {source_id, target_id, edge, [, value]}

//
// edgelist_edge
// for(auto&& [uid,vid,uv,value] : edges_view(g, [](vertex_edge_reference_t<G> g) { return ...; } )
// for(auto&& [uid,vid,uv]       : edges_view(g) )
//
template <class VId, class E, class EV>
using edgelist_edge = edge_view<VId, true, E, EV>; // {source_id, target_id [, edge] [, value]}

//
// copyable_edge
//
template <class VId, class EV>
using copyable_edge_t = edge_view<VId, true, void, EV>; // {source_id, target_id [, value]}

//
// neighbor_view (for adjacency)
//
template <class VId, bool Sourced, class V, class VV>
struct neighbor_view {
  VId source_id;
  VId target_id;
  V   target;
  VV  value;
};

template <class VId, class V, class VV>
struct neighbor_view<VId, false, V, VV> {
  VId target_id;
  V   target;
  VV  value;
};

template <class VId, class V>
struct neighbor_view<VId, false, V, void> {
  VId target_id;
  V   target;
};

template <class VId, class VV>
struct neighbor_view<VId, false, void, VV> {
  VId target_id;
  VV  value;
};

template <class VId>
struct neighbor_view<VId, false, void, void> {
  VId target_id;
};

template <class VId, class V>
struct neighbor_view<VId, true, V, void> {
  VId source_id;
  VId target_id;
  V   target;
};

template <class VId, class VV>
struct neighbor_view<VId, true, void, VV> {
  VId source_id;
  VId target_id;
  VV  value;
};

template <class VId>
struct neighbor_view<VId, true, void, void> {
  VId source_id;
  VId target_id;
};

//
// view concepts
//
template <class T, class VId, class VV>
concept copyable_vertex = convertible_to<T, copyable_vertex_t<VId, VV>>;

template <class T, class VId, class EV>
concept copyable_edge = convertible_to<T, copyable_edge_t<VId, EV>>;

//
// is_sourced<G>
//
template <class T>
inline constexpr bool is_sourced_v = false;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<edge_view<VId, true, V, VV>> = true;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<neighbor_view<VId, true, V, VV>> = true;


template <class G, bool Sourced>
class source_vertex {
public:
  using vertex_id_type = vertex_id_t<G>;

  source_vertex(vertex_id_type id) : id_(id) {}

  source_vertex()                         = default;
  source_vertex(const source_vertex& rhs) = default;
  source_vertex(source_vertex&&)          = default;
  ~source_vertex()                        = default;

  source_vertex& operator=(const source_vertex&) = default;
  source_vertex& operator=(source_vertex&&)      = default;

  constexpr vertex_id_type source_vertex_id() const noexcept { return id_; }

protected:
  vertex_id_type id_ = 0;
};

template <class G>
class source_vertex<G, false> {
public:
  using vertex_id_type = vertex_id_t<G>;

  source_vertex(vertex_id_type id) {}
  source_vertex() = default;
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


  template <class A>
  concept is_allocator_v = is_copy_constructible_v<A> && requires(A alloc, size_t n) {
    { alloc.allocate(n) };
  };


} // namespace _detail


} // namespace std::graph
