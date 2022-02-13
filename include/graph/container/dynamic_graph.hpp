#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
#include <list>
#include "graph/graph.hpp"

namespace std::graph::container {

//--------------------------------------------------------------------------------------------------
// dynamic_graph forward references
//

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VKey = uint32_t>
struct vofl_graph_traits;

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VKey = uint32_t>
struct vol_graph_traits;

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VKey = uint32_t>
struct vov_graph_traits;


template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_vertex;

template <class EV     = void,
          class VV     = void,
          class GV     = void,
          bool Sourced = false,
          class VKey   = uint32_t,
          class Traits = vofl_graph_traits<EV, VV, GV, Sourced, VKey>>
class dynamic_graph;

template <class EV, class VV, class GV, bool Sourced, class VKey>
struct vofl_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_key_type                      = VKey;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VKey, vofl_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VKey, vofl_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VKey, vofl_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = forward_list<edge_type>;
};

template <class EV, class VV, class GV, bool Sourced, class VKey>
struct vol_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_key_type                      = VKey;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VKey, vol_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VKey, vol_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VKey, vol_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = list<edge_type>;
};

template <class EV, class VV, class GV, bool Sourced, class VKey>
struct vov_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_key_type                      = VKey;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VKey, vov_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VKey, vov_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VKey, vov_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = vector<edge_type>;
};

template <class Traits>
using dynamic_adjacency_graph = dynamic_graph<typename Traits::edge_value_type,
                                              typename Traits::vertex_value_type,
                                              typename Traits::graph_value_type,
                                              Traits::sourced,
                                              typename Traits::vertex_key_type,
                                              Traits>;

//--------------------------------------------------------------------------------------------------
// utility functions
//

template <class C>
concept reservable = requires(C& container, typename C::size_type n) {
  {container.reserve(n)};
};
template <class C>
concept resizable = requires(C& container, typename C::size_type n) {
  {container.resize(n)};
};

template <class C>
concept has_emplace_back = requires(C& container, typename C::value_type&& value) {
  {container.emplace_back(move(value))};
};
template <class C>
concept has_push_back = requires(C& container, const typename C::value_type& value) {
  {container.push_back(value)};
};
template <class C>
concept has_emplace_front = requires(C& container, typename C::value_type&& value) {
  {container.emplace_front(move(value))};
};
template <class C>
concept has_push_front = requires(C& container, const typename C::value_type& value) {
  {container.push_front(value)};
};
template <class C>
concept has_emplace = requires(C& container, typename C::value_type&& value) {
  {container.emplace(move(value))};
};
template <class C>
concept has_insert = requires(C& container, const typename C::value_type& value) {
  {container.insert(value)};
};

// return a lambda to push/insert/emplace an element in a container
template <class C>
constexpr auto push_or_insert(C& container) {
  // favor pushing to the back over the front for things list list & deque
  if constexpr (has_emplace_back<C>)
    return [&container](C::value_type&& value) { container.emplace_back(move(value)); };
  else if constexpr (has_push_back<C>) {
    return [&container](const C::value_type& value) { container.push_back(value); };
  } else if constexpr (has_emplace_front<C>)
    return [&container](C::value_type&& value) { container.emplace_front(move(value)); };
  else if constexpr (has_push_front<C>) {
    return [&container](const C::value_type& value) { container.push_front(value); };
  } else if constexpr (has_emplace<C>)
    return [&container](C::value_type&& value) { container.emplace(move(value)); };
  else if constexpr (has_insert<C>) {
    return [&container](const C::value_type& value) { container.insert(value); };
  } else {
#ifdef _MSC_VER
    static_assert(false,
                  "The container doesn't have emplace_back, push_back, emplace_front, push_front, emplace or insert");
#endif
  }
}

//--------------------------------------------------------------------------------------------------
// dynamic_edge_target
//
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_target {
public:
  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = dynamic_graph<EV, VV, GV, false, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, false, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, false, VKey, Traits>;

public:
  constexpr dynamic_edge_target(vertex_key_type target_key) : target_key_(target_key) {}

  constexpr dynamic_edge_target()                           = default;
  constexpr dynamic_edge_target(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target(dynamic_edge_target&&)      = default;
  constexpr ~dynamic_edge_target()                          = default;

  constexpr dynamic_edge_target& operator=(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target& operator=(dynamic_edge_target&&) = default;

public:
  //constexpr vertex_key_type target_key() const { return target_key_; }
  //constexpr vertex_key_type source_key() const { return source_key_; }

private:
  vertex_key_type target_key_ = vertex_key_type();

private:
  // target_key(g,uv), target(g)
  friend constexpr vertex_key_type
  tag_invoke(::std::graph::access::target_key_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.target_key_;
  }
  friend constexpr vertex_type& tag_invoke(::std::graph::access::target_fn_t, graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_key_];
  }
  friend constexpr const vertex_type&
  tag_invoke(::std::graph::access::target_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_key_];
  }
};

template <class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_target<void, VV, GV, Sourced, VKey, Traits> {};

//--------------------------------------------------------------------------------------------------
// dynamic_edge_source
//
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_source {
public:
  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = dynamic_graph<EV, VV, GV, false, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, false, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, false, VKey, Traits>;

public:
  constexpr dynamic_edge_source(vertex_key_type source_key) : source_key_(source_key) {}

  constexpr dynamic_edge_source()                           = default;
  constexpr dynamic_edge_source(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source(dynamic_edge_source&&)      = default;
  constexpr ~dynamic_edge_source()                          = default;

  constexpr dynamic_edge_source& operator=(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source& operator=(dynamic_edge_source&&) = default;

public:
  //constexpr vertex_key_type source_key() const { return source_key_; }
  //constexpr vertex_key_type source_key() const { return source_key_; }

private:
  vertex_key_type source_key_ = vertex_key_type();

private:
  // source_key(g,uv), source(g)
  friend constexpr vertex_key_type
  tag_invoke(::std::graph::access::source_key_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.source_key_;
  }
  friend constexpr vertex_type& tag_invoke(::std::graph::access::source_fn_t, graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_key_];
  }
  friend constexpr const vertex_type&
  tag_invoke(::std::graph::access::source_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_key_];
  }
};

template <class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_source<void, VV, GV, Sourced, VKey, Traits> {};

//--------------------------------------------------------------------------------------------------
// dynamic_edge_value
//
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_value {
public:
  using value_type  = EV;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VKey, Traits>;
  using edge_type   = dynamic_edge_value<EV, VV, GV, Sourced, VKey, Traits>;

public:
  constexpr dynamic_edge_value(const value_type& value) : value_(value) {}
  constexpr dynamic_edge_value(value_type&& value) : value_(move(value)) {}

  constexpr dynamic_edge_value()                          = default;
  constexpr dynamic_edge_value(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value(dynamic_edge_value&&)      = default;
  constexpr ~dynamic_edge_value()                         = default;

  constexpr dynamic_edge_value& operator=(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value& operator=(dynamic_edge_value&&) = default;

public:
  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type&
  tag_invoke(::std::graph::access::edge_value_fn_t, graph_type& g, edge_type& uv) noexcept {
    return uv.value_;
  }
  friend constexpr const value_type&
  tag_invoke(::std::graph::access::edge_value_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.value_;
  }
};

template <class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge_value<void, VV, GV, Sourced, VKey, Traits> {};


//--------------------------------------------------------------------------------------------------
// dynamic_edge
//
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_edge
      : public dynamic_edge_target<EV, VV, GV, Sourced, VKey, Traits>
      , public dynamic_edge_source<EV, VV, GV, Sourced, VKey, Traits>
      , public dynamic_edge_value<EV, VV, GV, Sourced, VKey, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, Sourced, VKey, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, Sourced, VKey, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, Sourced, VKey, Traits>;

  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, Sourced, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, Sourced, VKey, Traits>;

public:
  constexpr dynamic_edge(vertex_key_type target_key, vertex_key_type source_key)
        : base_target_type(target_key), base_source_type(source_key) {}
  constexpr dynamic_edge(vertex_key_type target_key, vertex_key_type source_key, const value_type& val)
        : base_target_type(target_key), base_source_type(source_key), base_value_type(val) {}
  constexpr dynamic_edge(vertex_key_type target_key, vertex_key_type source_key, value_type&& val)
        : base_target_type(target_key), base_source_type(source_key), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&) = default;
};

template <class VV, class GV, class VKey, class Traits>
class dynamic_edge<void, VV, GV, true, VKey, Traits>
      : public dynamic_edge_target<void, VV, GV, true, VKey, Traits>
      , public dynamic_edge_source<void, VV, GV, true, VKey, Traits>
      , public dynamic_edge_value<void, VV, GV, true, VKey, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, true, VKey, Traits>;
  using base_source_type = dynamic_edge_source<void, VV, GV, true, VKey, Traits>;
  using base_value_type  = dynamic_edge_value<void, VV, GV, true, VKey, Traits>;

  using vertex_key_type = VKey;
  using value_type      = void;
  using graph_type      = dynamic_graph<void, VV, GV, true, VKey, Traits>;
  using vertex_type     = dynamic_vertex<void, VV, GV, true, VKey, Traits>;
  using edge_type       = dynamic_edge<void, VV, GV, true, VKey, Traits>;

public:
  constexpr dynamic_edge(vertex_key_type target_key, vertex_key_type source_key)
        : base_target_type(target_key), base_source_type(source_key) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&) = default;
};


template <class EV, class VV, class GV, class VKey, class Traits>
class dynamic_edge<EV, VV, GV, false, VKey, Traits>
      : public dynamic_edge_target<EV, VV, GV, false, VKey, Traits>
      , public dynamic_edge_source<EV, VV, GV, false, VKey, Traits>
      , public dynamic_edge_value<EV, VV, GV, false, VKey, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, false, VKey, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, false, VKey, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, false, VKey, Traits>;

  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = dynamic_graph<EV, VV, GV, false, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, false, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, false, VKey, Traits>;

public:
  constexpr dynamic_edge(vertex_key_type target_key) : base_target_type(target_key) {}
  constexpr dynamic_edge(vertex_key_type target_key, const value_type& val)
        : base_target_type(target_key), base_value_type(val) {}
  constexpr dynamic_edge(vertex_key_type target_key, value_type&& val)
        : base_target_type(target_key), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&) = default;
};

template <class VV, class GV, class VKey, class Traits>
class dynamic_edge<void, VV, GV, false, VKey, Traits>
      : public dynamic_edge_target<void, VV, GV, false, VKey, Traits>
      , public dynamic_edge_source<void, VV, GV, false, VKey, Traits>
      , public dynamic_edge_value<void, VV, GV, false, VKey, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, false, VKey, Traits>;
  using vertex_key_type  = VKey;
  using value_type       = void;
  using graph_type       = dynamic_graph<void, VV, GV, false, VKey, Traits>;
  using vertex_type      = dynamic_vertex<void, VV, GV, false, VKey, Traits>;
  using edge_type        = dynamic_edge<void, VV, GV, false, VKey, Traits>;

public:
  constexpr dynamic_edge(vertex_key_type target_key) : base_target_type(target_key) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&) = default;
};

//--------------------------------------------------------------------------------------------------
// dynamic_vertex
//
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_vertex_base {
public:
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, Sourced, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, Sourced, VKey, Traits>;
  using edges_type      = typename Traits::edges_type;
  using allocator_type  = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex_base()                           = default;
  constexpr dynamic_vertex_base(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base(dynamic_vertex_base&&)      = default;
  constexpr ~dynamic_vertex_base()                          = default;

  constexpr dynamic_vertex_base& operator=(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base& operator=(dynamic_vertex_base&&) = default;

  constexpr dynamic_vertex_base(allocator_type alloc) : edges_(alloc) {}

public:
  constexpr edges_type&       edges() noexcept { return edges_; }
  constexpr const edges_type& edges() const noexcept { return edges_; }

  constexpr auto begin() noexcept { return edges_.begin(); }
  constexpr auto begin() const noexcept { return edges_.begin(); }
  constexpr auto cbegin() const noexcept { return edges_.begin(); }

  constexpr auto end() noexcept { return edges_.end(); }
  constexpr auto end() const noexcept { return edges_.end(); }
  constexpr auto cend() const noexcept { return edges_.end(); }

private:
  edges_type edges_;

private: // tag_invoke properties
  friend constexpr vertex_key_type
  tag_invoke(::std::graph::access::vertex_key_fn_t, const graph_type& g, const vertex_type& u) {
    return static_cast<vertex_key_type>(&u - &g[0]); // works well when everything is contiguous in memory
  }

  friend constexpr edges_type& tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, vertex_type& u) {
    return u.edges_;
  }
  friend constexpr const edges_type&
  tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u.edges_;
  }

  friend constexpr edges_type::iterator
  tag_invoke(::std::graph::access::find_vertex_edge_fn_t, graph_type& g, vertex_key_type ukey, vertex_key_type vkey) {
    return ranges::find(g[ukey].edges_, [&g, &vkey](const edge_type& uv) -> bool { return target_key(g, uv) == vkey; });
  }
  friend constexpr edges_type::const_iterator tag_invoke(::std::graph::access::find_vertex_edge_fn_t,
                                                         const graph_type& g,
                                                         vertex_key_type   ukey,
                                                         vertex_key_type   vkey) {
    return ranges::find(g[ukey].edges_, [&g, &vkey](const edge_type& uv) -> bool { return target_key(g, uv) == vkey; });
  }
};


template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_vertex : public dynamic_vertex_base<EV, VV, GV, Sourced, VKey, Traits> {
public:
  using base_type       = dynamic_vertex_base<EV, VV, GV, Sourced, VKey, Traits>;
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, VV, GV, Sourced, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, VV, GV, Sourced, VKey, Traits>;
  using edges_type      = typename Traits::edges_type;
  using allocator_type  = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex(const value_type& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(value) {}
  constexpr dynamic_vertex(value_type&& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(move(value)) {}
  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}

  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&) = default;

public:
  using base_type::edges;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type& tag_invoke(::std::graph::access::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    return u.value_;
  }
  friend constexpr const value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    return u.value_;
  }
};


template <class EV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_vertex<EV, void, GV, Sourced, VKey, Traits>
      : public dynamic_vertex_base<EV, void, GV, Sourced, VKey, Traits> {
public:
  using base_type       = dynamic_vertex_base<EV, void, GV, Sourced, VKey, Traits>;
  using vertex_key_type = VKey;
  using value_type      = void;
  using graph_type      = dynamic_graph<EV, void, GV, Sourced, VKey, Traits>;
  using vertex_type     = dynamic_vertex<EV, void, GV, Sourced, VKey, Traits>;
  using edge_type       = dynamic_edge<EV, void, GV, Sourced, VKey, Traits>;
  using edges_type      = typename Traits::edges_type;
  using allocator_type  = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&) = default;

  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}
};

//--------------------------------------------------------------------------------------------------
/// dynamic_graph - vector of [forward] list
///
/// A graph that supports the minimal functionality required for the algorithms, where vertices are
/// stored in a vector and edges are stored in a forward_list (which doesn't have a size). The
/// graph, vertex and edge types can optionally store a user-defined function; no space is reserved
/// if a property isn't used.
///
/// A possible change to test the lower bounds of requirements is to use a deque to store the
/// vertices instead of a vector.
///
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_graph_base {
public: // types
  using graph_type   = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using graph_traits = Traits;

  using vertex_key_type       = VKey;
  using vertex_type           = dynamic_vertex<EV, VV, GV, Sourced, VKey, Traits>;
  using vertices_type         = typename Traits::vertices_type;
  using vertex_allocator_type = typename vertices_type::allocator_type;
  using size_type             = vertices_type::size_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, Sourced, VKey, Traits>;

public: // Construction/Destruction/Assignment
  constexpr dynamic_graph_base()                          = default;
  constexpr dynamic_graph_base(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base(dynamic_graph_base&&)      = default;
  constexpr ~dynamic_graph_base()                         = default;

  constexpr dynamic_graph_base& operator=(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base& operator=(dynamic_graph_base&&) = default;

  /// <summary>
  /// Create an empty graph using the allocator passed.
  /// </summary>
  /// <param name="alloc">The allocator to use for internal containers for vertices and edges</param>
  dynamic_graph_base(vertex_allocator_type alloc) : vertices_(alloc) {}

  /// Constructor that takes edge & vertex ranges to create the graph.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EKeyFnc   Function object to return edge_key_type of the
  ///                   ERng::value_type.
  /// @tparam EValueFnc Function object to return the edge_value_type, or
  ///                   a type that edge_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   edge_value_type default constructor will be used
  ///                   to initialize the value.
  /// @tparam VRng      The vertex data range.
  /// @tparam VValueFnc Function object to return the vertex_value_type,
  ///                   or a type that vertex_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   vertex_value_type default constructor will be
  ///                   used to initialize the value.
  ///
  /// @param erng       The container of edge data.
  /// @param vrng       The container of vertex data.
  /// @param ekey_fnc   The edge key extractor functor:
  ///                   ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G>.
  /// @param vvalue_fnc The vertex value extractor functor:
  ///                   vvalue_fnc(VRng::value_type) -> vertex_value_t<G>.
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph_base(ERng&                 erng,
                     VRng&                 vrng,
                     const EKeyFnc&        ekey_fnc,
                     const EValueFnc&      evalue_fnc,
                     const VValueFnc&      vvalue_fnc,
                     vertex_allocator_type alloc)
        : vertices_(alloc) {
    load_vertices(vrng, vvalue_fnc, alloc);
    load_edges(static_cast<vertex_key_type>(vertices_.size() - 1), erng, ekey_fnc, evalue_fnc, alloc);
  }

  /// Constructor that takes edge ranges to create the graph. Edges are scanned to determine the
  /// largest vertex key needed.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EKeyFnc   Function object to return edge_key_type of the
  ///                   ERng::value_type.
  /// @tparam EValueFnc Function object to return the edge_value_type, or
  ///                   a type that edge_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   edge_value_type default constructor will be used
  ///                   to initialize the value.
  ///
  /// @param erng       The container of edge data.
  /// @param ekey_fnc   The edge key extractor functor:
  ///                   ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                   edge_value_t<G>).
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph_base(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, vertex_allocator_type alloc)
        : vertices_(alloc) {

    load_edges(erng, ekey_fnc, evalue_fnc, alloc);
  }

  /// Constructor that takes edge ranges to create the csr graph.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EKeyFnc   Function object to return edge_key_type of the
  ///                   ERng::value_type.
  /// @tparam EValueFnc Function object to return the edge_value_type, or
  ///                   a type that edge_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   edge_value_type default constructor will be used
  ///                   to initialize the value.
  ///
  /// @param max_row_idx The maximum row index needed.
  /// @param erng       The container of edge data.
  /// @param ekey_fnc   The edge key extractor functor:
  ///                   ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                   edge_value_t<G>).
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph_base(vertex_key_type       max_row_idx,
                     ERng&                 erng,
                     const EKeyFnc&        ekey_fnc,
                     const EValueFnc&      evalue_fnc,
                     vertex_allocator_type alloc)
        : vertices_(alloc) {

    load_edges(max_row_idx, erng, ekey_fnc, evalue_fnc, alloc);
  }

public:
  template <class VRng, class VValueFnc>
  void load_vertices(VRng& vrng, VValueFnc& vvalue_fnc, vertex_allocator_type alloc = vertex_allocator_type()) {
    if constexpr (reservable<vertices_type>)
      vertices_.reserve(ranges::size(vrng));
    auto add_vertex = push_or_insert(vertices_);
    for (auto&& u : vrng)
      add_vertex(vertex_type(std::move(vvalue_fnc(u)), alloc));
    //vertices_.emplace_back(vertex_type(std::move(vvalue_fnc(u)), alloc));
  }

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(vertex_key_type     max_row_idx,
                  ERng&               erng,
                  const EKeyFnc&      ekey_fnc,
                  const EValueFnc&    evalue_fnc,
                  edge_allocator_type alloc = edge_allocator_type()) {
    if constexpr (resizable<vertices_type>) {
      if (vertices_.size() < static_cast<size_t>(max_row_idx) + 1)
        vertices_.resize(static_cast<size_t>(max_row_idx) + 1, vertex_type(alloc));
    }

    // add edges
    for (auto& edge_data : erng) {
      auto&& [ukey, vkey] = ekey_fnc(edge_data);
      assert(static_cast<size_t>(ukey) < vertices_.size() && static_cast<size_t>(vkey) < vertices_.size());
      if (static_cast<size_t>(ukey) >= vertices_.size())
        throw runtime_error("source key exceeds the number of vertices in load_edges");
      if (static_cast<size_t>(vkey) >= vertices_.size())
        throw runtime_error("target key exceeds the number of vertices in load_edges");

      auto&& add_edge = push_or_insert(vertices_[ukey].edges());
      if constexpr (is_same_v<EV, void>) {
        add_edge(edge_type(vkey));
        //vertices_[ukey].edges().push_front(edge_type(vkey));
      } else {
        add_edge(edge_type(vkey, evalue_fnc(edge_data)));
        //vertices_[ukey].edges().push_front(edge_type(vkey, evalue_fnc(edge_data)));
      }
    }
  }
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(ERng&               erng,
                  const EKeyFnc&      ekey_fnc,
                  const EValueFnc&    evalue_fnc,
                  edge_allocator_type alloc = edge_allocator_type()) {
    // Nothing to do?
    if (erng.begin() == erng.end())
      return;

    // Evaluate max vertex key needed
    size_t          erng_size   = 0;
    vertex_key_type max_row_idx = 0;
    for (auto& edge_data : erng) {
      auto&& [uidx, vidx] = ekey_fnc(edge_data);
      max_row_idx         = max(max_row_idx, max(uidx, vidx));
      ++erng_size;
    }

    load_edges(max_row_idx, erng, ekey_fnc, evalue_fnc, alloc);
  }

public: // Properties
  constexpr auto begin() noexcept { return vertices_.begin(); }
  constexpr auto begin() const noexcept { return vertices_.begin(); }
  constexpr auto cbegin() const noexcept { return vertices_.begin(); }

  constexpr auto end() noexcept { return vertices_.end(); }
  constexpr auto end() const noexcept { return vertices_.end(); }
  constexpr auto cend() const noexcept { return vertices_.end(); }

  constexpr auto size() const noexcept { return vertices_.size(); }

  constexpr vertices_type::value_type&       operator[](size_type i) noexcept { return vertices_[i]; }
  constexpr const vertices_type::value_type& operator[](size_type i) const noexcept { return vertices_[i]; }

private: // Member Variables
  vertices_type vertices_;

private: // tag_invoke properties
  friend constexpr vertices_type& tag_invoke(::std::graph::access::vertices_fn_t, dynamic_graph_base& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_type& tag_invoke(::std::graph::access::vertices_fn_t, const dynamic_graph_base& g) {
    return g.vertices_;
  }

  friend vertex_key_type
  tag_invoke(::std::graph::access::vertex_key_fn_t, const dynamic_graph_base& g, vertices_type::const_iterator ui) {
    return static_cast<vertex_key_type>(ui - g.vertices_.begin());
  }
};

/// <summary>
/// Vector-of-List defines a minimal representation for an incidence graph, while also allowing
/// for user-defined property types for edges, vertices and the graph itself.
/// </summary>
/// @tparam EV    [default=void] The edge value type. If "void" is used no user value is stored
///               on the edge.
/// @tparam VV    [default=void] The vertex value type. If "void" is used no user value is stored
///               on the vertex.
/// @tparam GV    [default=void] The graph value type. If "void" is used no user value is stored
///               on the graph.
/// @tparam VKey  [default=uint32_t] The type used for the vertex key.
/// @tparam Alloc The allocator used for storing the vertices and edges.
///
template <class EV, class VV, class GV, bool Sourced, class VKey, class Traits>
class dynamic_graph : public dynamic_graph_base<EV, VV, GV, Sourced, VKey, Traits> {
public:
  using base_type       = dynamic_graph_base<EV, VV, GV, Sourced, VKey, Traits>;
  using graph_type      = dynamic_graph<EV, VV, GV, Sourced, VKey, Traits>;
  using graph_traits    = Traits;
  using vertex_key_type = VKey;
  using value_type      = GV;
  using allocator_type  = typename Traits::vertices_type::allocator_type;

  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&) = default;

  // gv&,  alloc
  // gv&&, alloc
  //       alloc

  dynamic_graph(allocator_type alloc) : base_type(alloc) {}
  dynamic_graph(const GV& gv, allocator_type alloc) : base_type(alloc), value_(gv) {}
  dynamic_graph(GV&& gv, allocator_type alloc) : base_type(alloc), value_(move(gv)) {}


  // erng, ekey_fnc, evalue_fnc, vvalue_fnc,       alloc
  // erng, ekey_fnc, evalue_fnc, vvalue_fnc, gv&,  alloc
  // erng, ekey_fnc, evalue_fnc, vvalue_fnc, gv&&, alloc

  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(const ERng&      erng,
                const VRng&      vrng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const VValueFnc& vvalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc) {}

  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(const ERng&      erng,
                const VRng&      vrng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const VValueFnc& vvalue_fnc,
                const GV&        gv,
                allocator_type   alloc = allocator_type())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc), value_(gv) {}

  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(const ERng&      erng,
                const VRng&      vrng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const VValueFnc& vvalue_fnc,
                GV&&             gv,
                allocator_type   alloc = allocator_type())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc), value_(move(gv)) {}


  // max_vertex_key, erng, ekey_fnc, evalue_fnc,       alloc
  // max_vertex_key, erng, ekey_fnc, evalue_fnc, gv&,  alloc
  // max_vertex_key, erng, ekey_fnc, evalue_fnc, gv&&, alloc

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(vertex_key_type  max_vertex_key,
                ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(vertex_key_type  max_vertex_key,
                ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const GV&        gv,
                allocator_type   alloc = allocator_type())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc), value_(gv) {}

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(vertex_key_type  max_vertex_key,
                ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                GV&&             gv,
                allocator_type   alloc = allocator_type())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc), value_(move(gv)) {}

  // erng, ekey_fnc, evalue_fnc,       alloc
  // erng, ekey_fnc, evalue_fnc, gv&,  alloc
  // erng, ekey_fnc, evalue_fnc, gv&&, alloc

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const GV&        gv,
                allocator_type   alloc = allocator_type())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(gv) {}

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                GV&&             gv,
                allocator_type   alloc = allocator_type())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(move(gv)) {}

public:
  constexpr value_type&       value() { return value_; }
  constexpr const value_type& value() const { return value_; }

private:
  value_type value_;

private: // tag_invoke properties
  friend constexpr value_type& tag_invoke(::std::graph::access::graph_value_fn_t, graph_type& g) { return g.value_; }
  friend constexpr const value_type& tag_invoke(::std::graph::access::graph_value_fn_t, const graph_type& g) {
    return g.value_;
  }
};

// a specialization for dynamic_graph<...> that doesn't have a graph value_type
template <class EV, class VV, bool Sourced, class VKey, class Traits>
class dynamic_graph<EV, VV, void, Sourced, VKey, Traits>
      : public dynamic_graph_base<EV, VV, void, Sourced, VKey, Traits> {
public:
  using base_type       = dynamic_graph_base<EV, VV, void, Sourced, VKey, Traits>;
  using graph_type      = dynamic_graph<EV, VV, void, Sourced, VKey, Traits>;
  using graph_traits    = Traits;
  using vertex_key_type = VKey;
  using value_type      = void;
  using allocator_type  = typename Traits::vertices_type::allocator_type;
  using base_type::vertices_type;

  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&) = default;

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

  template <class ERng, typename EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  dynamic_graph(vertex_key_type  max_vertex_key,
                ERng&            erng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(ERng&            erng,
                VRng&            vrng,
                const EKeyFnc&   ekey_fnc,
                const EValueFnc& evalue_fnc,
                const VValueFnc& vvalue_fnc,
                allocator_type   alloc = allocator_type())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc) {}
};

} // namespace std::graph::container
