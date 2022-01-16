#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
#include "container_utility.hpp"

namespace std::graph::container {

template <typename EV, typename VKey>
class _vol_edge;

template <typename EV, typename VV, typename VKey, typename Alloc = allocator<_vol_edge<EV, VKey>>>
class _vol_vertex;

template <typename EV    = void,
          typename VV    = void,
          typename GV    = void,
          typename VKey  = uint32_t,
          typename Alloc = allocator<_vol_vertex<EV, VV, VKey>>>
class vol;

//--------------------------------------------------------------------------------------------------
// _vol_edge
//
template <typename EV, typename VKey>
class _vol_edge {
public:
  using vertex_key_type = VKey;
  using value_type      = EV;

public:
  _vol_edge(vertex_key_type target_key) : target_key_(target_key) {}
  _vol_edge(vertex_key_type target_key, const value_type& value) : target_key_(target_key), value_(value) {}
  _vol_edge(vertex_key_type target_key, value_type&& value) : target_key_(target_key), value_(move(value)) {}

  _vol_edge()                 = default;
  _vol_edge(const _vol_edge&) = default;
  _vol_edge(_vol_edge&&)      = default;
  ~_vol_edge()                = default;

  _vol_edge& operator=(const _vol_edge&) = default;
  _vol_edge& operator=(_vol_edge&&) = default;

public:
  vertex_key_type target_key() const { return target_key; }

  value_type&       value() { return value_; }
  const value_type& value() const { return value_; }

private:
  vertex_key_type target_key_ = vertex_key_type();
  value_type      value_      = value_type();
};

template <typename VKey>
class _vol_edge<void, VKey> {
public:
  using vertex_key_type = VKey;
  using value_type      = void;

public:
  _vol_edge(vertex_key_type target_key) : target_key_(target_key) {}

  _vol_edge()                 = default;
  _vol_edge(const _vol_edge&) = default;
  _vol_edge(_vol_edge&&)      = default;
  ~_vol_edge()                = default;

  _vol_edge& operator=(const _vol_edge&) = default;
  _vol_edge& operator=(_vol_edge&&) = default;

public:
  vertex_key_type target_key() const { return target_key; }

private:
  vertex_key_type target_key_ = vertex_key_type();
};

//--------------------------------------------------------------------------------------------------
// _vol_edges_
//
template <typename EV, typename VKey, typename Alloc = allocator<_vol_edge<EV, VKey>>>
using _vol_edges = forward_list<_vol_edge<EV, VKey>, Alloc>;

//--------------------------------------------------------------------------------------------------
// _vol_vertex
//
template <typename EV, typename VV, typename VKey, typename Alloc>
class _vol_vertex {
public:
  using value_type = VV;
  using edges_type = _vol_edges<EV, VKey, Alloc>;

public:
  _vol_vertex(const value_type& value, Alloc alloc = Alloc()) : edges_(alloc), value_(value) {}
  _vol_vertex(value_type&& value, Alloc alloc = Alloc()) : edges_(alloc), value_(forward(value)) {}
  _vol_vertex(Alloc alloc) : edges_(alloc) {}

  _vol_vertex()                   = default;
  _vol_vertex(const _vol_vertex&) = default;
  _vol_vertex(_vol_vertex&&)      = default;
  ~_vol_vertex()                  = default;

  _vol_vertex& operator=(const _vol_vertex&) = default;
  _vol_vertex& operator=(_vol_vertex&&) = default;

public:
  edges_type&       edges() { return edges_; }
  const edges_type& edges() const { return edges_; }

  value_type&       value() { return value_; }
  const value_type& value() const { return value_; }

private:
  edges_type edges_ = edges_type(Alloc());
  value_type value_ = value_type();
};


template <typename EV, typename VKey, typename Alloc>
class _vol_vertex<EV, void, VKey, Alloc> {
public:
  using value_type = void;
  using edges_type = _vol_edges<EV, VKey, Alloc>;

public:
  _vol_vertex()                   = default;
  _vol_vertex(const _vol_vertex&) = default;
  _vol_vertex(_vol_vertex&&)      = default;
  ~_vol_vertex()                  = default;

  _vol_vertex& operator=(const _vol_vertex&) = default;
  _vol_vertex& operator=(_vol_vertex&&) = default;

  _vol_vertex(Alloc alloc) : edges_(alloc) {}

public:
  edges_type&       edges() { return edges_; }
  const edges_type& edges() const { return edges_; }

private:
  edges_type edges_ = edges_type();
};

//--------------------------------------------------------------------------------------------------
// _vol_vertices
//
template <typename EV, typename VV, typename VKey, typename Alloc = allocator<_vol_vertex<EV, VV, VKey>>>
using _vol_vertices = vector<_vol_vertex<EV, VV, VKey>, Alloc>;


//--------------------------------------------------------------------------------------------------
// vol - vector of list
//
template <typename EV, typename VV, typename GV, typename VKey, typename Alloc = allocator<_vol_vertex<EV, VV, VKey>>>
class _vol_base {
public: // types
  using vertex_key_type = VKey;
  using vertex_type     = _vol_vertex<EV, VV, VKey>;
  using vertices_type   = _vol_vertices<EV, VV, VKey>;

  using edge_type = _vol_edge<EV, VKey>;

public: // Construction/Destruction/Assignment
  _vol_base()                 = default;
  _vol_base(const _vol_base&) = default;
  _vol_base(_vol_base&&)      = default;
  ~_vol_base()                = default;

  _vol_base& operator=(const _vol_base&) = default;
  _vol_base& operator=(_vol_base&&) = default;

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
  constexpr _vol_base(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : vertices_(alloc) {

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

public: // Properties
  vertices_type&       vertices() { return vertices_; }
  const vertices_type& vertices() const { return vertices_; }

private: // Member Variables
  vertices_type vertices_;
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
class vol : public _vol_base<EV, VV, GV, VKey> {
public:
  using base_type  = _vol_base<EV, VV, GV, VKey, Alloc>;
  using value_type = GV;

  vol()           = default;
  vol(const vol&) = default;
  vol(vol&&)      = default;
  ~vol()          = default;

  vol& operator=(const vol&) = default;
  vol& operator=(vol&&) = default;

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol(const GV& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(g) {}
  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol(GV&& g, ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(move(g)) {}

  value_type&       value() { return value_; }
  const value_type& value() const { return value_; }

private:
  value_type value_;
};

// a specialization for vol<...> that doesn't have a value_type
template <typename EV, typename VV, typename VKey, typename Alloc>
class vol<EV, VV, void, VKey, Alloc> : public _vol_base<EV, VV, void, VKey, Alloc> {
public:
  using base_type  = _vol_base<EV, VV, void, VKey, Alloc>;
  using value_type = void;

  vol()           = default;
  vol(const vol&) = default;
  vol(vol&&)      = default;
  ~vol()          = default;

  vol& operator=(const vol&) = default;
  vol& operator=(vol&&) = default;

  template <typename ERng, typename EKeyFnc, typename EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr vol(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}
};

} // namespace std::graph::container
