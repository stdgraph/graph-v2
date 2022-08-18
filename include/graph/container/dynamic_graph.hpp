#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
#include <list>
#include "graph/graph.hpp"
#include "container_utility.hpp"

// load_vertices(vrng, vvalue_fnc) -> [uid,vval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid],       [uid,vval]
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid, eval], [uid,vval]
//
// load_edges(initializer_list<[uid,vid]>
// load_edges(initializer_list<[uid,vid,eval]>
//
// [uid,vval]      <-- copyable_vertex<VId,VV>
// [uid,vid]      <-- copyable_edge<VId,void>
// [uid,vid,eval] <-- copyable_edge<VId,EV>
//

namespace std::graph::container {

//--------------------------------------------------------------------------------------------------
// dynamic_graph forward references
//

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VId = uint32_t>
struct vofl_graph_traits;

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VId = uint32_t>
struct vol_graph_traits;

template <class EV = void, class VV = void, class GV = void, bool Sourced = false, class VId = uint32_t>
struct vov_graph_traits;


template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_vertex;

template <class EV     = void,
          class VV     = void,
          class GV     = void,
          bool Sourced = false,
          class VId    = uint32_t,
          class Traits = vofl_graph_traits<EV, VV, GV, Sourced, VId>>
class dynamic_graph;

template <class EV, class VV, class GV, bool Sourced, class VId>
struct vofl_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VId, vofl_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VId, vofl_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VId, vofl_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = forward_list<edge_type>;
};

template <class EV, class VV, class GV, bool Sourced, class VId>
struct vol_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VId, vol_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VId, vol_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VId, vol_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = list<edge_type>;
};

template <class EV, class VV, class GV, bool Sourced, class VId>
struct vov_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, Sourced, VId, vov_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VId, vov_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VId, vov_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = vector<edge_type>;
};

template <class Traits>
using dynamic_adjacency_graph = dynamic_graph<typename Traits::edge_value_type,
                                              typename Traits::vertex_value_type,
                                              typename Traits::graph_value_type,
                                              Traits::sourced,
                                              typename Traits::vertex_id_type,
                                              Traits>;

//--------------------------------------------------------------------------------------------------
// dynamic_edge_target
//
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge_target {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;

public:
  constexpr dynamic_edge_target(vertex_id_type target_id) : target_id_(target_id) {}

  constexpr dynamic_edge_target()                           = default;
  constexpr dynamic_edge_target(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target(dynamic_edge_target&&)      = default;
  constexpr ~dynamic_edge_target()                          = default;

  constexpr dynamic_edge_target& operator=(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target& operator=(dynamic_edge_target&&)      = default;

public:
  //constexpr vertex_id_type target_id() const { return target_id_; }
  //constexpr vertex_id_type source_id() const { return source_id_; }

private:
  vertex_id_type target_id_ = vertex_id_type();

private:
  // target_id(g,uv), target(g,uv)
  friend constexpr vertex_id_type
  tag_invoke(::std::graph::tag_invoke::target_id_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.target_id_;
  }
  friend constexpr vertex_type&
  tag_invoke(::std::graph::tag_invoke::target_fn_t, graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_id_];
  }
  friend constexpr const vertex_type&
  tag_invoke(::std::graph::tag_invoke::target_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_id_];
  }
};

//--------------------------------------------------------------------------------------------------
// dynamic_edge_source
//
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge_source {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;

public:
  constexpr dynamic_edge_source(vertex_id_type source_id) : source_id_(source_id) {}

  constexpr dynamic_edge_source()                           = default;
  constexpr dynamic_edge_source(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source(dynamic_edge_source&&)      = default;
  constexpr ~dynamic_edge_source()                          = default;

  constexpr dynamic_edge_source& operator=(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source& operator=(dynamic_edge_source&&)      = default;

public:
  //constexpr vertex_id_type source_id() const { return source_id_; }
  //constexpr vertex_id_type source_id() const { return source_id_; }

private:
  vertex_id_type source_id_ = vertex_id_type();

private:
  // source_id(g,uv), source(g)
  friend constexpr vertex_id_type
  tag_invoke(::std::graph::tag_invoke::source_id_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.source_id_;
  }
  friend constexpr vertex_type&
  tag_invoke(::std::graph::tag_invoke::source_fn_t, graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_id_];
  }
  friend constexpr const vertex_type&
  tag_invoke(::std::graph::tag_invoke::source_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_id_];
  }
};

template <class EV, class VV, class GV, class VId, class Traits>
class dynamic_edge_source<EV, VV, GV, false, VId, Traits> {};

//--------------------------------------------------------------------------------------------------
// dynamic_edge_value
//
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge_value {
public:
  using value_type  = EV;
  using graph_type  = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type   = dynamic_edge_value<EV, VV, GV, Sourced, VId, Traits>;

public:
  constexpr dynamic_edge_value(const value_type& value) : value_(value) {}
  constexpr dynamic_edge_value(value_type&& value) : value_(move(value)) {}

  constexpr dynamic_edge_value()                          = default;
  constexpr dynamic_edge_value(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value(dynamic_edge_value&&)      = default;
  constexpr ~dynamic_edge_value()                         = default;

  constexpr dynamic_edge_value& operator=(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value& operator=(dynamic_edge_value&&)      = default;

public:
  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type&
  tag_invoke(::std::graph::tag_invoke::edge_value_fn_t, graph_type& g, edge_type& uv) noexcept {
    return uv.value_;
  }
  friend constexpr const value_type&
  tag_invoke(::std::graph::tag_invoke::edge_value_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.value_;
  }
};

template <class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge_value<void, VV, GV, Sourced, VId, Traits> {};


//--------------------------------------------------------------------------------------------------
// dynamic_edge
//
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_edge
      : public dynamic_edge_target<EV, VV, GV, Sourced, VId, Traits>
      , public dynamic_edge_source<EV, VV, GV, Sourced, VId, Traits>
      , public dynamic_edge_value<EV, VV, GV, Sourced, VId, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, Sourced, VId, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, Sourced, VId, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, Sourced, VId, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id)
        : base_target_type(target_id), base_source_type(source_id) {}
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id, const value_type& val)
        : base_target_type(target_id), base_source_type(source_id), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id, value_type&& val)
        : base_target_type(target_id), base_source_type(source_id), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

template <class VV, class GV, class VId, class Traits>
class dynamic_edge<void, VV, GV, true, VId, Traits>
      : public dynamic_edge_target<void, VV, GV, true, VId, Traits>
      , public dynamic_edge_source<void, VV, GV, true, VId, Traits>
      , public dynamic_edge_value<void, VV, GV, true, VId, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, true, VId, Traits>;
  using base_source_type = dynamic_edge_source<void, VV, GV, true, VId, Traits>;
  using base_value_type  = dynamic_edge_value<void, VV, GV, true, VId, Traits>;

  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<void, VV, GV, true, VId, Traits>;
  using vertex_type    = dynamic_vertex<void, VV, GV, true, VId, Traits>;
  using edge_type      = dynamic_edge<void, VV, GV, true, VId, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id)
        : base_target_type(target_id), base_source_type(source_id) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};


template <class EV, class VV, class GV, class VId, class Traits>
class dynamic_edge<EV, VV, GV, false, VId, Traits>
      : public dynamic_edge_target<EV, VV, GV, false, VId, Traits>
      , public dynamic_edge_source<EV, VV, GV, false, VId, Traits>
      , public dynamic_edge_value<EV, VV, GV, false, VId, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, false, VId, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, false, VId, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, false, VId, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, false, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, false, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, false, VId, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type target_id) : base_target_type(target_id) {}
  constexpr dynamic_edge(vertex_id_type target_id, const value_type& val)
        : base_target_type(target_id), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type target_id, value_type&& val)
        : base_target_type(target_id), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

template <class VV, class GV, class VId, class Traits>
class dynamic_edge<void, VV, GV, false, VId, Traits>
      : public dynamic_edge_target<void, VV, GV, false, VId, Traits>
      , public dynamic_edge_source<void, VV, GV, false, VId, Traits>
      , public dynamic_edge_value<void, VV, GV, false, VId, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, false, VId, Traits>;
  using vertex_id_type   = VId;
  using value_type       = void;
  using graph_type       = dynamic_graph<void, VV, GV, false, VId, Traits>;
  using vertex_type      = dynamic_vertex<void, VV, GV, false, VId, Traits>;
  using edge_type        = dynamic_edge<void, VV, GV, false, VId, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type target_id) : base_target_type(target_id) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

//--------------------------------------------------------------------------------------------------
// dynamic_vertex
//
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_vertex_base {
public:
  using vertex_id_type = VId;
  using value_type     = VV;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex_base()                           = default;
  constexpr dynamic_vertex_base(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base(dynamic_vertex_base&&)      = default;
  constexpr ~dynamic_vertex_base()                          = default;

  constexpr dynamic_vertex_base& operator=(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base& operator=(dynamic_vertex_base&&)      = default;

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
  friend constexpr vertex_id_type
  tag_invoke(::std::graph::tag_invoke::vertex_id_fn_t, const graph_type& g, const vertex_type& u) {
    return static_cast<vertex_id_type>(&u - &g[0]); // works well when everything is contiguous in memory
  }

  friend constexpr edges_type& tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, vertex_type& u) {
    return u.edges_;
  }
  friend constexpr const edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u.edges_;
  }

  friend constexpr edges_type::iterator
  tag_invoke(::std::graph::tag_invoke::find_vertex_edge_fn_t, graph_type& g, vertex_id_type uid, vertex_id_type vid) {
    return ranges::find(g[uid].edges_, [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  }
  friend constexpr edges_type::const_iterator tag_invoke(::std::graph::tag_invoke::find_vertex_edge_fn_t,
                                                         const graph_type& g,
                                                         vertex_id_type    uid,
                                                         vertex_id_type    vid) {
    return ranges::find(g[uid].edges_, [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  }
};


template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_vertex : public dynamic_vertex_base<EV, VV, GV, Sourced, VId, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_id_type = VId;
  using value_type     = remove_cvref_t<VV>;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

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
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

public:
  using base_type::edges;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type& tag_invoke(::std::graph::tag_invoke::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    return u.value_;
  }
  friend constexpr const value_type&
  tag_invoke(::std::graph::tag_invoke::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    return u.value_;
  }
};


template <class EV, class GV, bool Sourced, class VId, class Traits>
class dynamic_vertex<EV, void, GV, Sourced, VId, Traits>
      : public dynamic_vertex_base<EV, void, GV, Sourced, VId, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, void, GV, Sourced, VId, Traits>;
  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<EV, void, GV, Sourced, VId, Traits>;
  using vertex_type    = dynamic_vertex<EV, void, GV, Sourced, VId, Traits>;
  using edge_type      = dynamic_edge<EV, void, GV, Sourced, VId, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

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
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_graph_base {
public: // types
  using graph_type   = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using graph_traits = Traits;

  using vertex_id_type        = VId;
  using vertex_type           = dynamic_vertex<EV, VV, GV, Sourced, VId, Traits>;
  using vertices_type         = typename Traits::vertices_type;
  using vertex_allocator_type = typename vertices_type::allocator_type;
  using size_type             = vertices_type::size_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;

public: // Construction/Destruction/Assignment
  constexpr dynamic_graph_base()                          = default;
  constexpr dynamic_graph_base(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base(dynamic_graph_base&&)      = default;
  constexpr ~dynamic_graph_base()                         = default;

  constexpr dynamic_graph_base& operator=(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base& operator=(dynamic_graph_base&&)      = default;

  /// <summary>
  /// Create an empty graph using the allocator passed.
  /// </summary>
  /// <param name="alloc">The allocator to use for internal containers for vertices and edges</param>
  dynamic_graph_base(vertex_allocator_type alloc) : vertices_(alloc) {}

  /// Constructor that takes edge & vertex ranges to create the graph.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EIdFnc   Function object to return edge_id_type of the
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
  /// @param eid_fnc   The edge id extractor functor:
  ///                   eid_fnc(ERng::value_type) -> directed_adjacency_vector::edge_id_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G>.
  /// @param vvalue_fnc The vertex value extractor functor:
  ///                   vvalue_fnc(VRng::value_type) -> vertex_value_t<G>.
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class VRng, class EProj = identity, class VProj>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph_base(ERng&& erng, VRng&& vrng, EProj eproj, VProj vproj, vertex_allocator_type alloc)
        : vertices_(alloc) {
    load_vertices(vrng, vproj);
    load_edges(vertices_.size(), 0, erng, eproj);
  }

  /// Constructor that takes edge ranges to create the graph. Edges are scanned to determine the
  /// largest vertex id needed.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EIdFnc   Function object to return edge_id_type of the
  ///                   ERng::value_type.
  /// @tparam EValueFnc Function object to return the edge_value_type, or
  ///                   a type that edge_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   edge_value_type default constructor will be used
  ///                   to initialize the value.
  ///
  /// @param erng       The container of edge data.
  /// @param eid_fnc   The edge id extractor functor:
  ///                   eid_fnc(ERng::value_type) -> directed_adjacency_vector::edge_id_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                   edge_value_t<G>).
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EProj>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph_base(ERng&& erng, EProj eproj, vertex_allocator_type alloc) : vertices_(alloc) {

    load_edges(move(erng), eproj);
  }

  /// Constructor that takes edge ranges to create the csr graph.
  ///
  /// @tparam ERng      The edge data range.
  /// @tparam EIdFnc   Function object to return edge_id_type of the
  ///                   ERng::value_type.
  /// @tparam EValueFnc Function object to return the edge_value_type, or
  ///                   a type that edge_value_type is constructible
  ///                   from. If the return type is void or empty_value the
  ///                   edge_value_type default constructor will be used
  ///                   to initialize the value.
  ///
  /// @param max_row_idx The maximum row index needed.
  /// @param erng       The container of edge data.
  /// @param eid_fnc   The edge id extractor functor:
  ///                   eid_fnc(ERng::value_type) -> directed_adjacency_vector::edge_id_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                   edge_value_t<G>).
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EProj>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph_base(size_type vertex_count, ERng&& erng, EProj eproj, vertex_allocator_type alloc) : vertices_(alloc) {

    load_edges(vertex_count, 0, move(erng), eproj);
  }

  dynamic_graph_base(const initializer_list<copyable_edge_t<VId, EV>>& il,
                     edge_allocator_type                               alloc = edge_allocator_type())
        : vertices_(alloc) {
    size_t last_id = 0;
    for (auto&& e : il)
      last_id = max(last_id, static_cast<size_t>(max(e.source_id, e.target_id)));
    resize_vertices(last_id + 1);
    load_edges(il);
  }

public:
  template <class VRng, class VProj = identity>
  void load_vertices(const VRng& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (ranges::sized_range<VRng> && resizable<vertices_type>) {
      vertex_count = max(vertex_count, ranges::size(vertices_));
      resize_vertices(max(vertex_count, ranges::size(vrng)));
    }
    for (auto&& v : vrng) {
      auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
      size_t k           = static_cast<size_t>(id);
      if constexpr (ranges::random_access_range<vertices_type>)
        assert(k < vertices_.size());
      vertices_[k].value() = value;
    }
  }

  template <class VRng, class VProj = identity>
  void load_vertices(VRng&& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (ranges::sized_range<VRng> && resizable<vertices_type>) {
      vertex_count = max(vertex_count, ranges::size(vertices_));
      resize_vertices(max(vertex_count, ranges::size(vrng)));
    }
    for (auto&& v : vrng) {
      auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
      size_t k           = static_cast<size_t>(id);
      if constexpr (ranges::random_access_range<vertices_type>)
        assert(k < vertices_.size());
      vertices_[k].value() = move(value);
    }
  }

  /// TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  void load_edges(const ERng& erng, EProj eproj = {}, size_type vertex_count = 0, size_type edge_count_hint = 0) {
    if constexpr (resizable<vertices_type>) {
      if (vertices_.size() < vertex_count)
        vertices_.resize(vertex_count, vertex_type(vertices_.get_allocator()));
    }

    // add edges
    for (auto&& edge_data : erng) {
      auto&& e = eproj(edge_data); //views::copyable_edge_t<VId, EV>

      if (static_cast<size_t>(e.source_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("source id exceeds the number of vertices in load_edges");
      }
      if (static_cast<size_t>(e.target_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("target id exceeds the number of vertices in load_edges");
      }

      auto&& edge_adder = push_or_insert(vertices_[e.source_id].edges());
      if constexpr (Sourced) {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(move(e.source_id), move(e.target_id)));
        } else {
          edge_adder(edge_type(move(e.source_id), move(e.target_id), move(e.value)));
        }
      } else {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(move(e.target_id)));
        } else {
          edge_adder(edge_type(move(e.target_id), move(e.value)));
        }
      }
    }
  }

  /// TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  void load_edges(ERng&& erng, EProj eproj = {}, size_type vertex_count = 0, size_type edge_count_hint = 0) {
    if constexpr (resizable<vertices_type>) {
      if (vertices_.size() < vertex_count)
        vertices_.resize(vertex_count, vertex_type(vertices_.get_allocator()));
    }

    // add edges
    for (auto&& edge_data : erng) {
      auto&& e = eproj(edge_data); //views::copyable_edge_t<VId, EV>

      if (static_cast<size_t>(e.source_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("source id exceeds the number of vertices in load_edges");
      }
      if (static_cast<size_t>(e.target_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("target id exceeds the number of vertices in load_edges");
      }

      auto&& edge_adder = push_or_insert(vertices_[e.source_id].edges());
      if constexpr (Sourced) {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(move(e.source_id), move(e.target_id)));
        } else {
          edge_adder(edge_type(move(e.source_id), move(e.target_id), move(e.value)));
        }
      } else {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(move(e.target_id)));
        } else {
          edge_adder(edge_type(move(e.target_id), move(e.value)));
        }
      }
    }
  }

#if 0
  /// TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  void load_edges(ERng&& erng, EProj eproj = {}) {
    // Nothing to do?
    if (erng.begin() == erng.end())
      return;

    // Evaluate max vertex id needed
    size_type       erng_size   = 0;
    vertex_id_type max_row_idx = 0;
    for (auto& edge_data : erng) {
      auto&& e    = eproj(edge_data);
      max_row_idx = max(max_row_idx, max(e.source_id, e.target_id));
      ++erng_size;
    }

    load_edges(static_cast<size_type>(max_row_idx + 1), erng_size, erng, eproj);
  }
#endif

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

public: // Operations
  void reserve_vertices(size_type count) {
    if constexpr (reservable<vertices_type>) // reserve if we can; otherwise ignored
      vertices_.reserve(count);
  }
  void reserve_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

  void resize_vertices(size_type count) {
    if constexpr (resizable<vertices_type>) // resize if we can; otherwise ignored
      vertices_.resize(count);
  }
  void resize_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

private: // Member Variables
  vertices_type vertices_;

private: // tag_invoke properties
  friend constexpr vertices_type& tag_invoke(::std::graph::tag_invoke::vertices_fn_t, dynamic_graph_base& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_type& tag_invoke(::std::graph::tag_invoke::vertices_fn_t,
                                                   const dynamic_graph_base& g) {
    return g.vertices_;
  }

  friend vertex_id_type
  tag_invoke(::std::graph::tag_invoke::vertex_id_fn_t, const dynamic_graph_base& g, vertices_type::const_iterator ui) {
    return static_cast<vertex_id_type>(ui - g.vertices_.begin());
  }

  friend constexpr edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid].edges();
  }
  friend constexpr const edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid].edges();
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
/// @tparam VId  [default=uint32_t] The type used for the vertex id.
/// @tparam Alloc The allocator used for storing the vertices and edges.
///
template <class EV, class VV, class GV, bool Sourced, class VId, class Traits>
class dynamic_graph : public dynamic_graph_base<EV, VV, GV, Sourced, VId, Traits> {
public:
  using base_type      = dynamic_graph_base<EV, VV, GV, Sourced, VId, Traits>;
  using graph_type     = dynamic_graph<EV, VV, GV, Sourced, VId, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = GV;
  using allocator_type = typename Traits::vertices_type::allocator_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, Sourced, VId, Traits>;

  constexpr inline const static bool sourced = Sourced;


  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  // gv&,  alloc
  // gv&&, alloc
  //       alloc

  dynamic_graph(allocator_type alloc) : base_type(alloc) {}
  dynamic_graph(const GV& gv, allocator_type alloc = allocator_type()) : base_type(alloc), value_(gv) {}
  dynamic_graph(GV&& gv, allocator_type alloc = allocator_type()) : base_type(alloc), value_(move(gv)) {}


  //       erng, eproj, vproj, alloc
  // gv&,  erng, eproj, vproj, alloc
  // gv&&, erng, eproj, vproj, alloc

  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(
        const ERng& erng, const VRng& vrng, EProj eproj = {}, VProj vproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc) {}

  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(const GV&      gv,
                const ERng&    erng,
                const VRng&    vrng,
                EProj          eproj = {},
                VProj          vproj = {},
                allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc), value_(gv) {}

  template <class ERng, class EProj, class VRng, class VProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(
        const ERng& erng, const VRng& vrng, EProj eproj, VProj vproj, GV&& gv, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc), value_(move(gv)) {}


  //       max_vertex_id, erng, eproj, alloc
  // gv&,  max_vertex_id, erng, eproj, alloc
  // gv&&, max_vertex_id, erng, eproj, alloc

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc) {}

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(const GV&      gv,
                vertex_id_type max_vertex_id,
                ERng&          erng,
                EProj          eproj = {},
                allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc), value_(gv) {}

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(
        GV&& gv, vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc), value_(move(gv)) {}

  // erng, eproj,       alloc
  // erng, eproj, gv&,  alloc
  // erng, eproj, gv&&, alloc

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc) {}

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(const GV& gv, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc), value_(gv) {}

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(GV&& gv, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc), value_(move(gv)) {}


  dynamic_graph(const initializer_list<copyable_edge_t<VId, EV>>& il, edge_allocator_type alloc = edge_allocator_type())
        : base_type(il, alloc) {}

public:
  constexpr value_type&       value() { return value_; }
  constexpr const value_type& value() const { return value_; }

private:
  value_type value_;

private: // tag_invoke properties
  friend constexpr value_type& tag_invoke(::std::graph::tag_invoke::graph_value_fn_t, graph_type& g) {
    return g.value_;
  }
  friend constexpr const value_type& tag_invoke(::std::graph::tag_invoke::graph_value_fn_t, const graph_type& g) {
    return g.value_;
  }
};

// a specialization for dynamic_graph<...> that doesn't have a graph value_type
template <class EV, class VV, bool Sourced, class VId, class Traits>
class dynamic_graph<EV, VV, void, Sourced, VId, Traits>
      : public dynamic_graph_base<EV, VV, void, Sourced, VId, Traits> {
public:
  using base_type      = dynamic_graph_base<EV, VV, void, Sourced, VId, Traits>;
  using graph_type     = dynamic_graph<EV, VV, void, Sourced, VId, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = void;
  using allocator_type = typename Traits::vertices_type::allocator_type;
  using base_type::vertices_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, void, Sourced, VId, Traits>;

  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  template <class ERng, class EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc) {}

  template <class ERng, typename EProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc>
  dynamic_graph(vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc) {}

  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  //requires edge_value_extractor<ERng, EIdFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  dynamic_graph(ERng& erng, VRng& vrng, EProj eproj = {}, VProj vproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc) {}

  dynamic_graph(const initializer_list<copyable_edge_t<VId, EV>>& il, edge_allocator_type alloc = edge_allocator_type())
        : base_type(il, alloc) {}
};


} // namespace std::graph::container
