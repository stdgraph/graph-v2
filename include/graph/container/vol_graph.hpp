#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
#include "graph/graph.hpp"

namespace std::graph::container {

template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge;

template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_vertex;

template <typename EV    = void,
          typename VV    = void,
          typename GV    = void,
          bool Sourced   = false,
          typename VKey  = uint32_t,
          typename Alloc = allocator<char>>
class vol_graph;

// vol_vertices and vol_edges can be specialized to use different containers

// vol_vertices
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
using vol_vertices =
      vector<vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>, allocator<vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>>>;

// vol_edges
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
using vol_edges =
      forward_list<vol_edge<EV, VV, GV, Sourced, VKey, Alloc>, allocator<vol_edge<EV, VV, GV, Sourced, VKey, Alloc>>>;

//--------------------------------------------------------------------------------------------------
// vol_edge_target
//
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge_target {
public:
  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = vol_graph<EV, VV, GV, false, VKey, Alloc>;
  using vertex_type     = vol_vertex<EV, VV, GV, false, VKey, Alloc>;
  using edge_type       = vol_edge<EV, VV, GV, false, VKey, Alloc>;

public:
  constexpr vol_edge_target(vertex_key_type target_key) : target_key_(target_key) {}

  constexpr vol_edge_target()                       = default;
  constexpr vol_edge_target(const vol_edge_target&) = default;
  constexpr vol_edge_target(vol_edge_target&&)      = default;
  constexpr ~vol_edge_target()                      = default;

  constexpr vol_edge_target& operator=(const vol_edge_target&) = default;
  constexpr vol_edge_target& operator=(vol_edge_target&&) = default;

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

template <typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge_target<void, VV, GV, Sourced, VKey, Alloc> {};

//--------------------------------------------------------------------------------------------------
// vol_edge_source
//

//--------------------------------------------------------------------------------------------------
// vol_edge_value
//
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge_value {
public:
  using value_type  = EV;
  using graph_type  = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_type = vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>;
  using edge_type   = vol_edge_value<EV, VV, GV, Sourced, VKey, Alloc>;

public:
  vol_edge_value(const value_type& value) : value_(value) {}
  vol_edge_value(value_type&& value) : value_(move(value)) {}

  vol_edge_value()                      = default;
  vol_edge_value(const vol_edge_value&) = default;
  vol_edge_value(vol_edge_value&&)      = default;
  ~vol_edge_value()                     = default;

  vol_edge_value& operator=(const vol_edge_value&) = default;
  vol_edge_value& operator=(vol_edge_value&&) = default;

public:
  value_type&       value() { return value_; }
  const value_type& value() const { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend value_type& tag_invoke(::std::graph::access::edge_value_fn_t, graph_type& g, edge_type& uv) {
    return uv.value_;
  }
  friend const value_type& tag_invoke(::std::graph::access::edge_value_fn_t, const graph_type& g, const edge_type& uv) {
    return uv.value_;
  }
};

template <typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge_value<void, VV, GV, Sourced, VKey, Alloc> {};


//--------------------------------------------------------------------------------------------------
// vol_edge
//
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge
      : public vol_edge_target<EV, VV, GV, Sourced, VKey, Alloc>
      , public vol_edge_value<EV, VV, GV, Sourced, VKey, Alloc> {
public:
  using base_target_type = vol_edge_target<EV, VV, GV, Sourced, VKey, Alloc>;
  using base_value_type  = vol_edge_value<EV, VV, GV, Sourced, VKey, Alloc>;

  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_type     = vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>;
  using edge_type       = vol_edge<EV, VV, GV, Sourced, VKey, Alloc>;

public:
  vol_edge(vertex_key_type target_key) : base_target_type(target_key) {}
  vol_edge(vertex_key_type target_key, const value_type& val) : base_target_type(target_key), base_value_type(val) {}
  vol_edge(vertex_key_type target_key, value_type&& val) : base_target_type(target_key), base_value_type(move(val)) {}

  vol_edge()                = default;
  vol_edge(const vol_edge&) = default;
  vol_edge(vol_edge&&)      = default;
  ~vol_edge()               = default;

  vol_edge& operator=(const vol_edge&) = default;
  vol_edge& operator=(vol_edge&&) = default;

  //using base_value_type::value;
};

template <typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_edge<void, VV, GV, Sourced, VKey, Alloc>
      : public vol_edge_target<void, VV, GV, Sourced, VKey, Alloc>
      , public vol_edge_value<void, VV, GV, Sourced, VKey, Alloc> {
public:
  using base_target_type = vol_edge_target<void, VV, GV, Sourced, VKey, Alloc>;
  using vertex_key_type  = VKey;
  using value_type       = void;
  using graph_type       = vol_graph<void, VV, GV, Sourced, VKey, Alloc>;
  using vertex_type      = vol_vertex<void, VV, GV, Sourced, VKey, Alloc>;
  using edge_type        = vol_edge<void, VV, GV, Sourced, VKey, Alloc>;

public:
  vol_edge(vertex_key_type target_key) : base_target_type(target_key) {}

  vol_edge()                = default;
  vol_edge(const vol_edge&) = default;
  vol_edge(vol_edge&&)      = default;
  ~vol_edge()               = default;

  vol_edge& operator=(const vol_edge&) = default;
  vol_edge& operator=(vol_edge&&) = default;
};

//--------------------------------------------------------------------------------------------------
// vol_vertex
//
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class _vol_vertex_base {
public:
  using allocator_type  = Alloc;
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_type     = vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>;
  using edge_type       = vol_edge<EV, VV, GV, Sourced, VKey, Alloc>;
  using edges_type      = vol_edges<EV, VV, GV, Sourced, VKey, Alloc>;

public:
  _vol_vertex_base()                        = default;
  _vol_vertex_base(const _vol_vertex_base&) = default;
  _vol_vertex_base(_vol_vertex_base&&)      = default;
  ~_vol_vertex_base()                       = default;

  _vol_vertex_base& operator=(const _vol_vertex_base&) = default;
  _vol_vertex_base& operator=(_vol_vertex_base&&) = default;

  _vol_vertex_base(Alloc alloc) : edges_(alloc) {}

public:
  edges_type&       edges() noexcept { return edges_; }
  const edges_type& edges() const noexcept { return edges_; }

  auto begin() noexcept { return edges_.begin(); }
  auto begin() const noexcept { return edges_.begin(); }
  auto cbegin() const noexcept { return edges_.begin(); }

  auto end() noexcept { return edges_.end(); }
  auto end() const noexcept { return edges_.end(); }
  auto cend() const noexcept { return edges_.end(); }

private:
  edges_type edges_;

private: // tag_invoke properties
  friend vertex_key_type tag_invoke(::std::graph::access::vertex_key_fn_t, const graph_type& g, const vertex_type& u) {
    return static_cast<vertex_key_type>(&u - &g[0]); // works well when everything is contiguous in memory
  }

  friend edges_type& tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, vertex_type& u) { return u.edges_; }
  friend const edges_type& tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u.edges_;
  }

  edges_type::iterator
  tag_invoke(::std::graph::access::find_vertex_edge_fn_t, graph_type& g, vertex_key_type ukey, vertex_key_type vkey) {
    return ranges::find(g[ukey].edges_, [&g, &vkey](const edge_type& uv) -> bool { return target_key(g, uv) == vkey; });
  }
  edges_type::const_iterator tag_invoke(::std::graph::access::find_vertex_edge_fn_t,
                                        const graph_type& g,
                                        vertex_key_type   ukey,
                                        vertex_key_type   vkey) {
    return ranges::find(g[ukey].edges_, [&g, &vkey](const edge_type& uv) -> bool { return target_key(g, uv) == vkey; });
  }
};


template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_vertex : public _vol_vertex_base<EV, VV, GV, Sourced, VKey, Alloc> {
public:
  using base_type       = _vol_vertex_base<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_type     = vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>;
  using edge_type       = vol_edge<EV, VV, GV, Sourced, VKey, Alloc>;
  using edges_type      = vol_edges<EV, VV, GV, Sourced, VKey, Alloc>;

public:
  vol_vertex(const value_type& value, Alloc alloc = Alloc()) : base_type(alloc), value_(value) {}
  vol_vertex(value_type&& value, Alloc alloc = Alloc()) : base_type(alloc), value_(move(value)) {}
  vol_vertex(Alloc alloc) : base_type(alloc) {}

  vol_vertex()                  = default;
  vol_vertex(const vol_vertex&) = default;
  vol_vertex(vol_vertex&&)      = default;
  ~vol_vertex()                 = default;

  vol_vertex& operator=(const vol_vertex&) = default;
  vol_vertex& operator=(vol_vertex&&) = default;

public:
  using base_type::edges;

  value_type&       value() noexcept { return value_; }
  const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend value_type& tag_invoke(::std::graph::access::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    return u.value_;
  }
  friend const value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    return u.value_;
  }
};


template <typename EV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_vertex<EV, void, GV, Sourced, VKey, Alloc> : public _vol_vertex_base<EV, void, GV, Sourced, VKey, Alloc> {
public:
  using base_type       = _vol_vertex_base<EV, void, GV, Sourced, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = void;
  using graph_type      = vol_graph<EV, void, GV, Sourced, VKey, Alloc>;
  using vertex_type     = vol_vertex<EV, void, GV, Sourced, VKey, Alloc>;
  using edge_type       = vol_edge<EV, void, GV, Sourced, VKey, Alloc>;
  using edges_type      = vol_edges<EV, void, GV, Sourced, VKey, Alloc>;

public:
  vol_vertex()                  = default;
  vol_vertex(const vol_vertex&) = default;
  vol_vertex(vol_vertex&&)      = default;
  ~vol_vertex()                 = default;

  vol_vertex& operator=(const vol_vertex&) = default;
  vol_vertex& operator=(vol_vertex&&) = default;

  vol_vertex(Alloc alloc) : base_type(alloc) {}

public:
  using base_type::edges;

private:
};

//--------------------------------------------------------------------------------------------------
/// vol_graph - vector of [forward] list
///
/// A graph that supports the minimal functionality required for the algorithms, where vertices are
/// stored in a vector and edges are stored in a forward_list (which doesn't have a size). The
/// graph, vertex and edge types can optionally store a user-defined function; no space is reserved
/// if a property isn't used.
///
/// A possible change to test the lower bounds of requirements is to use a deque to store the
/// vertices instead of a vector.
///
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class _vol_graph_base {
public: // types
  using vertex_key_type = VKey;
  using vertex_type     = vol_vertex<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertices_type   = vol_vertices<EV, VV, GV, Sourced, VKey, Alloc>;
  using size_type       = vertices_type::size_type;

  using graph_type = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using edge_type  = vol_edge<EV, VV, GV, Sourced, VKey, Alloc>;

public: // Construction/Destruction/Assignment
  _vol_graph_base()                       = default;
  _vol_graph_base(const _vol_graph_base&) = default;
  _vol_graph_base(_vol_graph_base&&)      = default;
  ~_vol_graph_base()                      = default;

  _vol_graph_base& operator=(const _vol_graph_base&) = default;
  _vol_graph_base& operator=(_vol_graph_base&&) = default;

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
  template <typename ERng, typename EKeyFnc, typename EValueFnc, typename VRng, typename VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  _vol_graph_base(ERng&            erng,
                  VRng&            vrng,
                  const EKeyFnc&   ekey_fnc,
                  const EValueFnc& evalue_fnc,
                  const VValueFnc& vvalue_fnc,
                  Alloc            alloc)
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
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  _vol_graph_base(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc) : vertices_(alloc) {

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
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  _vol_graph_base(
        vertex_key_type max_row_idx, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc)
        : vertices_(alloc) {

    load_edges(max_row_idx, erng, ekey_fnc, evalue_fnc, alloc);
  }

public:
  template <typename VRng, typename VValueFnc>
  void load_vertices(VRng& vrng, const VValueFnc& vvalue_fnc, Alloc alloc = Alloc()) {
    vertices_.reserve(ranges::size(vrng));
    for (auto&& u : vrng)
      vertices_.emplace_back(vertex_type(std::move(vvalue_fnc(u)), alloc));
  }

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(vertex_key_type  max_row_idx,
                  ERng&            erng,
                  const EKeyFnc&   ekey_fnc,
                  const EValueFnc& evalue_fnc,
                  Alloc            alloc = Alloc()) {
    vertices_.resize(static_cast<size_t>(max_row_idx) + 1, vertex_type(alloc));

    // add edges
    for (auto& edge_data : erng) {
      auto&& [ukey, vkey] = ekey_fnc(edge_data);
      assert(static_cast<size_t>(ukey) < vertices_.size() && static_cast<size_t>(vkey) < vertices_.size());
      if constexpr (is_same_v<EV, void>) {
        vertices_[ukey].edges().emplace_front(edge_type(vkey));
      } else {
        vertices_[ukey].edges().emplace_front(edge_type(vkey, evalue_fnc(edge_data)));
      }
    }
  }
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc()) {
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

public:
  //constexpr vertices_type&       vertices() noexcept { return vertices_; }
  //constexpr const vertices_type& vertices() const noexcept { return vertices_; }

private: // Member Variables
  vertices_type vertices_;

private: // tag_invoke properties
  friend vertices_type& tag_invoke(::std::graph::access::vertices_fn_t, _vol_graph_base& g) { return g.vertices_; }
  friend const vertices_type& tag_invoke(::std::graph::access::vertices_fn_t, const _vol_graph_base& g) {
    return g.vertices_;
  }

  friend vertex_key_type
  tag_invoke(::std::graph::access::vertex_key_fn_t, const _vol_graph_base& g, vertices_type::const_iterator ui) {
    return ui - g.vertices_.begin();
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
template <typename EV, typename VV, typename GV, bool Sourced, typename VKey, typename Alloc>
class vol_graph : public _vol_graph_base<EV, VV, GV, Sourced, VKey, Alloc> {
public:
  using base_type       = _vol_graph_base<EV, VV, GV, Sourced, VKey, Alloc>;
  using graph_type      = vol_graph<EV, VV, GV, Sourced, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = GV;
  using allocator_type  = Alloc;

  vol_graph()                 = default;
  vol_graph(const vol_graph&) = default;
  vol_graph(vol_graph&&)      = default;
  ~vol_graph()                = default;

  vol_graph& operator=(const vol_graph&) = default;
  vol_graph& operator=(vol_graph&&) = default;

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(vertex_key_type  max_vertex_key,
            ERng&            erng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(const GV& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(g) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(vertex_key_type  max_vertex_key,
            const GV&        gv,
            ERng&            erng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc), value_(gv) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(GV&& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(move(g)) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(vertex_key_type  max_vertex_key,
            GV&&             gv,
            ERng&            erng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc), value_(move(gv)) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc, typename VRng, typename VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  vol_graph(const GV&        gv,
            const ERng&      erng,
            const VRng&      vrng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            const VValueFnc& vvalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc), value_(gv) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc, typename VRng, typename VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  vol_graph(GV&&             gv,
            const ERng&      erng,
            const VRng&      vrng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            const VValueFnc& vvalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc), value_(move(gv)) {}

public:
  value_type&       value() { return value_; }
  const value_type& value() const { return value_; }

private:
  value_type value_;

private: // tag_invoke properties
  friend value_type&       tag_invoke(::std::graph::access::graph_value_fn_t, graph_type& g) { return g.value_; }
  friend const value_type& tag_invoke(::std::graph::access::graph_value_fn_t, const graph_type& g) { return g.value_; }
};

// a specialization for vol_graph<...> that doesn't have a graph value_type
template <typename EV, typename VV, bool Sourced, typename VKey, typename Alloc>
class vol_graph<EV, VV, void, Sourced, VKey, Alloc> : public _vol_graph_base<EV, VV, void, Sourced, VKey, Alloc> {
public:
  using base_type       = _vol_graph_base<EV, VV, void, Sourced, VKey, Alloc>;
  using graph_type      = vol_graph<EV, VV, void, Sourced, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = void;
  using allocator_type  = Alloc;
  using base_type::vertices_type;

  vol_graph()                 = default;
  vol_graph(const vol_graph&) = default;
  vol_graph(vol_graph&&)      = default;
  ~vol_graph()                = default;

  vol_graph& operator=(const vol_graph&) = default;
  vol_graph& operator=(vol_graph&&) = default;

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  vol_graph(vertex_key_type  max_vertex_key,
            ERng&            erng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc, typename VRng, typename VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  vol_graph(ERng&            erng,
            VRng&            vrng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            const VValueFnc& vvalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc) {}
};

} // namespace std::graph::container
