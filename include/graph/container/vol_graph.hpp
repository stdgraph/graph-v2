#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
//#include "container_utility.hpp"
#include "graph/detail/graph_access.hpp"

namespace std::graph::container {

template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_edge;

template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_vertex;

template <typename EV    = void,
          typename VV    = void,
          typename GV    = void,
          typename VKey  = uint32_t,
          typename Alloc = allocator<char>>
class vol_graph;

//--------------------------------------------------------------------------------------------------
// _vol_edge
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_edge_base {
public:
  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = vol_graph<EV, VV, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<EV, VV, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<EV, VV, GV, VKey, Alloc>;

public:
  _vol_edge_base(vertex_key_type target_key) : target_key_(target_key) {}

  _vol_edge_base()                      = default;
  _vol_edge_base(const _vol_edge_base&) = default;
  _vol_edge_base(_vol_edge_base&&)      = default;
  ~_vol_edge_base()                     = default;

  _vol_edge_base& operator=(const _vol_edge_base&) = default;
  _vol_edge_base& operator=(_vol_edge_base&&) = default;

public:
  vertex_key_type target_key() const { return target_key_; }

private:
  vertex_key_type target_key_ = vertex_key_type();

private: // tag_invoke properties
  friend const vertex_key_type
  tag_invoke(::std::graph::access::target_key_fn_t, const graph_type& g, const edge_type& uv) {
    return uv.target_key_;
  }
  friend vertex_type& tag_invoke(::std::graph::access::target_fn_t, graph_type& g, edge_type& uv) {
    return g[uv.target_key_];
  }
  friend const vertex_type& tag_invoke(::std::graph::access::target_fn_t, const graph_type& g, const edge_type& uv) {
    return g[uv.target_key_];
  }
};

template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_edge : public _vol_edge_base<EV, VV, GV, VKey, Alloc> {
public:
  using base_type       = _vol_edge_base<EV, VV, GV, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = EV;
  using graph_type      = vol_graph<EV, VV, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<EV, VV, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<EV, VV, GV, VKey, Alloc>;

public:
  _vol_edge(vertex_key_type target_key) : base_type(target_key) {}
  _vol_edge(vertex_key_type target_key, const value_type& value) : base_type(target_key), value_(value) {}
  _vol_edge(vertex_key_type target_key, value_type&& value) : base_type(target_key), value_(move(value)) {}

  _vol_edge()                 = default;
  _vol_edge(const _vol_edge&) = default;
  _vol_edge(_vol_edge&&)      = default;
  ~_vol_edge()                = default;

  _vol_edge& operator=(const _vol_edge&) = default;
  _vol_edge& operator=(_vol_edge&&) = default;

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

template <typename VV, typename GV, typename VKey, typename Alloc>
class _vol_edge<void, VV, GV, VKey, Alloc> : public _vol_edge_base<void, VV, GV, VKey, Alloc> {
public:
  using base_type       = _vol_edge_base<void, VV, GV, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = void;
  using graph_type      = vol_graph<void, VV, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<void, VV, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<void, VV, GV, VKey, Alloc>;

public:
  _vol_edge(vertex_key_type target_key) : base_type(target_key) {}

  _vol_edge()                 = default;
  _vol_edge(const _vol_edge&) = default;
  _vol_edge(_vol_edge&&)      = default;
  ~_vol_edge()                = default;

  _vol_edge& operator=(const _vol_edge&) = default;
  _vol_edge& operator=(_vol_edge&&) = default;

public:
private:
};

//--------------------------------------------------------------------------------------------------
// _vol_edges_
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
using _vol_edges = forward_list<_vol_edge<EV, VV, GV, VKey, Alloc>, allocator<_vol_edge<EV, VV, GV, VKey, Alloc>>>;

//--------------------------------------------------------------------------------------------------
// _vol_vertex
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_vertex_base {
public:
  using allocator_type  = Alloc;
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = vol_graph<EV, VV, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<EV, VV, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<EV, VV, GV, VKey, Alloc>;
  using edges_type      = _vol_edges<EV, VV, GV, VKey, Alloc>;

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


template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_vertex : public _vol_vertex_base<EV, VV, GV, VKey, Alloc> {
public:
  using base_type       = _vol_vertex_base<EV, VV, GV, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = VV;
  using graph_type      = vol_graph<EV, VV, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<EV, VV, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<EV, VV, GV, VKey, Alloc>;
  using edges_type      = _vol_edges<EV, VV, GV, VKey, Alloc>;

public:
  _vol_vertex(const value_type& value, Alloc alloc = Alloc()) : base_type(alloc), value_(value) {}
  _vol_vertex(value_type&& value, Alloc alloc = Alloc()) : base_type(alloc), value_(forward(value)) {}
  _vol_vertex(Alloc alloc) : base_type(alloc) {}

  _vol_vertex()                   = default;
  _vol_vertex(const _vol_vertex&) = default;
  _vol_vertex(_vol_vertex&&)      = default;
  ~_vol_vertex()                  = default;

  _vol_vertex& operator=(const _vol_vertex&) = default;
  _vol_vertex& operator=(_vol_vertex&&) = default;

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


template <typename EV, typename GV, typename VKey, typename Alloc>
class _vol_vertex<EV, void, GV, VKey, Alloc> : public _vol_vertex_base<EV, void, GV, VKey, Alloc> {
public:
  using base_type       = _vol_vertex_base<EV, void, GV, VKey, Alloc>;
  using vertex_key_type = VKey;
  using value_type      = void;
  using graph_type      = vol_graph<EV, void, GV, VKey, Alloc>;
  using vertex_type     = _vol_vertex<EV, void, GV, VKey, Alloc>;
  using edge_type       = _vol_edge<EV, void, GV, VKey, Alloc>;
  using edges_type      = _vol_edges<EV, void, GV, VKey, Alloc>;

public:
  _vol_vertex()                   = default;
  _vol_vertex(const _vol_vertex&) = default;
  _vol_vertex(_vol_vertex&&)      = default;
  ~_vol_vertex()                  = default;

  _vol_vertex& operator=(const _vol_vertex&) = default;
  _vol_vertex& operator=(_vol_vertex&&) = default;

  _vol_vertex(Alloc alloc) : base_type(alloc) {}

public:
  using base_type::edges;

private:
};

//--------------------------------------------------------------------------------------------------
// _vol_vertices
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
using _vol_vertices = vector<_vol_vertex<EV, VV, GV, VKey, Alloc>, allocator<_vol_vertex<EV, VV, GV, VKey, Alloc>>>;


//--------------------------------------------------------------------------------------------------
// vol_graph - vector of list
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class _vol_graph_base {
public: // types
  using vertex_key_type = VKey;
  using vertex_type     = _vol_vertex<EV, VV, GV, VKey, Alloc>;
  using vertices_type   = _vol_vertices<EV, VV, GV, VKey, Alloc>;
  using size_type       = vertices_type::size_type;

  using graph_type = vol_graph<EV, VV, GV, VKey, Alloc>;
  using edge_type  = _vol_edge<EV, VV, GV, VKey, Alloc>;

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
  _vol_graph_base(const ERng&      erng,
                  const VRng&      vrng,
                  const EKeyFnc&   ekey_fnc,
                  const EValueFnc& evalue_fnc,
                  const VValueFnc& vvalue_fnc,
                  Alloc            alloc)
        : vertices_(alloc) {
    load_vertices(vrng, vvalue_fnc, alloc);
    load_edges(erng, ekey_fnc, evalue_fnc, alloc);
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
  constexpr _vol_graph_base(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc)
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
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr _vol_graph_base(
        vertex_key_type max_row_idx, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc)
        : vertices_(alloc) {

    load_edges(max_row_idx, erng, ekey_fnc, evalue_fnc, alloc);
  }

protected:
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(
        vertex_key_type max_row_idx, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc) {
    vertices_.resize(static_cast<size_t>(max_row_idx) + 1, vertex_type(alloc));

    // add edges
    for (auto& edge_data : erng) {
      auto&& [ukey, vkey] = ekey_fnc(edge_data);
      if constexpr (is_same_v<EV, void>) {
        vertices_[ukey].edges().emplace_front(edge_type(vkey));
      } else {
        vertices_[ukey].edges().emplace_front(edge_type(vkey, evalue_fnc(edge_data)));
      }
    }
  }
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  void load_edges(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc) {
    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng))
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

  template <typename VRng, typename VValueFnc>
  void load_vertices(VRng& vrng, const VValueFnc& vvalue_fnc, Alloc alloc) {
    vertices_.reserve(ranges::size(vrng));
    for (auto&& u : vrng)
      vertices_.emplace_back(vertex_type(vvalue_fnc(u), alloc));
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
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc>
class vol_graph : public _vol_graph_base<EV, VV, GV, VKey, Alloc> {
public:
  using base_type       = _vol_graph_base<EV, VV, GV, VKey, Alloc>;
  using graph_type      = vol_graph<EV, VV, GV, VKey, Alloc>;
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
  constexpr vol_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(vertex_key_type  max_vertex_key,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(
        const GV& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(g) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(vertex_key_type  max_vertex_key,
                      const GV&        gv,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc), value_(gv) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(GV&& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(move(g)) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(vertex_key_type  max_vertex_key,
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
  friend const value_type& tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g) { return g.value_; }
};

// a specialization for vol_graph<...> that doesn't have a graph value_type
template <typename EV, typename VV, typename VKey, typename Alloc>
class vol_graph<EV, VV, void, VKey, Alloc> : public _vol_graph_base<EV, VV, void, VKey, Alloc> {
public:
  using base_type       = _vol_graph_base<EV, VV, void, VKey, Alloc>;
  using graph_type      = vol_graph<EV, VV, void, VKey, Alloc>;
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
  constexpr vol_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol_graph(vertex_key_type  max_vertex_key,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : base_type(max_vertex_key, erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc, typename VRng, typename VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc> && vertex_value_extractor<VRng, VValueFnc>
  vol_graph(const ERng&      erng,
            const VRng&      vrng,
            const EKeyFnc&   ekey_fnc,
            const EValueFnc& evalue_fnc,
            const VValueFnc& vvalue_fnc,
            Alloc            alloc = Alloc())
        : base_type(erng, vrng, ekey_fnc, evalue_fnc, vvalue_fnc, alloc) {}
};

} // namespace std::graph::container
