#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <cstdint>
#include "graph/graph.hpp"

namespace std::graph::container {

//
// forward declarations
//
template <class EV      = empty_value,
          class VV      = void,
          class GV      = void,
          integral VKey = uint32_t,
          class Alloc   = allocator<uint32_t>>
class csr_graph;

/// <summary>
/// Class to hold vertex values in a vector that is the same size as row_index_. 
/// If is_void_v<VV> then the class is empty with a single
/// constructor that accepts (and ignores) an allocator.
/// </summary>
/// <typeparam name="EV"></typeparam>
/// <typeparam name="VV"></typeparam>
/// <typeparam name="GV"></typeparam>
/// <typeparam name="Alloc"></typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_vertex_values {
protected:
  using graph_type = csr_graph<EV, VV, GV, VKey, Alloc>;

  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<VKey>;
  using index_vector_type    = std::vector<VKey, index_allocator_type>;

  using vertex_key_type             = VKey;
  using vertex_type                 = vertex_key_type;
  using vertex_value_type           = VV;
  using vertex_value_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_value_type>;
  using vertex_values_type          = vector<vertex_value_type, vertex_value_allocator_type>;

  constexpr csr_vertex_values(Alloc& alloc) : vertex_values_(alloc) {}

  constexpr csr_vertex_values()                         = default;
  constexpr csr_vertex_values(const csr_vertex_values&) = default;
  constexpr csr_vertex_values(csr_vertex_values&&)      = default;
  constexpr ~csr_vertex_values()                        = default;

  constexpr csr_vertex_values& operator=(const csr_vertex_values&) = default;
  constexpr csr_vertex_values& operator=(csr_vertex_values&&) = default;

  template <ranges::forward_range VRng, class VValueFnc>
  // VValueFnc is a projection with default of identity
  constexpr void load_vertex_values(const VRng& vrng, const VValueFnc& vvalue_fnc) {
    if constexpr (ranges::sized_range<VRng>)
      vertex_values_.reserve(vrng.size());
    for (auto&& vvalue : vrng)
      vertex_values_.push_back(vvalue_fnc(vvalue));
  }

protected: // Member variables
  vertex_values_type vertex_values_;

private: // tag_invoke properties
  friend constexpr vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>,
                  "row_index_ must be a contiguous range to evaluate uidx");
    auto uidx = &u - g.row_index_.data();
    return g.vertex_values_[uidx];
  }
  friend constexpr const vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>,
                  "row_index_ must be a contiguous range to evaluate uidx");
    auto uidx = &u - g.row_index_.data();
    return g.vertex_values_[uidx];
  }
};

template <class EV, class GV, integral VKey, class Alloc>
class csr_vertex_values<EV, void, GV, VKey, Alloc> {
  constexpr csr_vertex_values(Alloc& alloc) {}
  constexpr csr_vertex_values()                         = default;
  constexpr csr_vertex_values(const csr_vertex_values&) = default;
  constexpr csr_vertex_values(csr_vertex_values&&)      = default;
  constexpr ~csr_vertex_values()                        = default;

  constexpr csr_vertex_values& operator=(const csr_vertex_values&) = default;
  constexpr csr_vertex_values& operator=(csr_vertex_values&&) = default;
};


/// <summary>
/// csr_graph_base - base class for compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV">Edge value type</typeparam>
/// <typeparam name="VV">Vertex value type</typeparam>
/// <typeparam name="GV">Graph value type</typeparam>
/// <typeparam name="VKey">Vertex Key type</typeparam>
/// <typeparam name="Alloc">Allocator</typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_graph_base : protected csr_vertex_values<EV, VV, GV, VKey, Alloc> {
  using base_type = csr_vertex_values<EV, VV, GV, VKey, Alloc>;

  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<VKey>;
  using v_allocator_type     = typename allocator_traits<Alloc>::template rebind_alloc<EV>;

  using index_vector_type = std::vector<VKey, index_allocator_type>;
  using v_vector_type     = std::vector<EV, v_allocator_type>;

  using csr_vertex_range_type       = index_vector_type;
  using const_csr_vertex_range_type = const index_vector_type;

  using csr_vertex_edge_range_type       = index_vector_type;
  using const_csr_vertex_edge_range_type = const index_vector_type;

public: // Types
  using graph_type = csr_graph_base<EV, VV, GV, VKey, Alloc>;

  using vertex_key_type   = VKey;
  using vertex_type       = vertex_key_type;
  using vertex_value_type = VV;

  using edges_type       = ranges::subrange<ranges::iterator_t<index_vector_type>>;
  using const_edges_type = ranges::subrange<ranges::iterator_t<const index_vector_type>>;
  using edge_value_type  = EV;
  using edge_type        = VKey; // index into v_

  using const_iterator = typename index_vector_type::const_iterator;
  using iterator       = typename index_vector_type::iterator;

public: // Construction/Destruction
  constexpr csr_graph_base()                      = default;
  constexpr csr_graph_base(const csr_graph_base&) = default;
  constexpr csr_graph_base(csr_graph_base&&)      = default;
  constexpr ~csr_graph_base()                     = default;

  constexpr csr_graph_base& operator=(const csr_graph_base&) = default;
  constexpr csr_graph_base& operator=(csr_graph_base&&) = default;

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
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph_base(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(alloc), row_index_(alloc), col_index_(alloc), v_(alloc) {

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

    load_edges(max_row_idx, erng_size, erng, ekey_fnc, evalue_fnc);
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
  /// @param max_vertex_key Number of vertices to reserve before loading the graph
  /// @param max_edges      Number of edges to reserve before loading the graph
  /// @param erng           The container of edge data.
  /// @param ekey_fnc       The edge key extractor functor:
  ///                       ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc     The edge value extractor functor:
  ///                       evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                       edge_value_t<G>).
  /// @param alloc          The allocator to use for internal containers for
  ///                       vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph_base(vertex_key_type  max_vertex_key,
                           size_t           max_edges,
                           ERng&            erng,
                           const EKeyFnc&   ekey_fnc,
                           const EValueFnc& evalue_fnc,
                           Alloc            alloc = Alloc())
        : base_type(alloc), row_index_(alloc), col_index_(alloc), v_(alloc) {

    load_edges(max_vertex_key, max_edges, erng, ekey_fnc, evalue_fnc);
  }

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with a tuple with 3 edge elements: source_vertex_key,
  /// target_vertex_key and edge_value.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_key,
  ///              target_vertex_key and the edge value.
  /// @param alloc Allocator.
  ///
  constexpr csr_graph_base(const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
                           const Alloc& alloc = Alloc())
        : csr_graph_base(
                ilist,
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) {
                  return pair{get<0>(e), get<1>(e)};
                },
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) { return get<2>(e); },
                alloc) {
    if (!is_void_v<vertex_value_type>)
      this->vertex_values_.resize(row_index_.size());
  }

protected:
  template <ranges::forward_range ERng, class EKeyFnc>
  constexpr vertex_key_type max_vertex_key(const ERng& erng, const EKeyFnc& ekey_fnc) {
    vertex_key_type max_key = 0;
    for (auto&& [source_key, target_key] : erng)
      max_key = max(max_key, max(source_key, target_key));
    return max_key;
  }

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr void load_edges(vertex_key_type  max_vertex_key,
                            size_t           max_edges,
                            ERng&            erng,
                            const EKeyFnc&   ekey_fnc,
                            const EValueFnc& evalue_fnc) {
    // copy edge key+val & sort in row/col order (CSR def allows cols to be unsorted)
    using EKey       = decltype(ekey_fnc(declval<std::ranges::range_value_t<ERng>>()));
    using EVal       = decltype(evalue_fnc(declval<std::ranges::range_value_t<ERng>>()));
    using EKeyVal    = std::pair<EKey, EVal>;
    using EKeyValVec = std::vector<EKeyVal>;
    EKeyValVec edges;
    edges.reserve(max_edges);
    for (auto& edge_data : erng)
      edges.emplace_back(EKeyVal(ekey_fnc(edge_data), evalue_fnc(edge_data)));
    auto ecmp = [](auto&& lhs, auto&& rhs) { return lhs.first < rhs.first; };
    std::ranges::sort(edges, ecmp);
    auto unique_edges = std::ranges::unique(edges, ecmp);

    max_edges = std::ranges::size(unique_edges);
    row_index_.reserve(static_cast<size_t>(max_vertex_key) + 1);
    col_index_.reserve(max_edges);
    v_.reserve(max_edges);

    // add edges
    for (auto& [key, val] : unique_edges) {
      auto& [ukey, vkey] = key;

      row_index_.resize(static_cast<size_t>(ukey) + 1, static_cast<vertex_key_type>(v_.size()));
      col_index_.push_back(vkey);
      v_.emplace_back(val);
    }
    row_index_.resize(static_cast<size_t>(max_vertex_key) + 1, static_cast<vertex_key_type>(v_.size()));
  }


public: // Operations
  constexpr ranges::iterator_t<index_vector_type> find_vertex(vertex_key_type key) noexcept {
    return row_index_.begin() + key;
  }
  constexpr ranges::iterator_t<const index_vector_type> find_vertex(vertex_key_type key) const noexcept {
    return row_index_.begin() + key;
  }

private:                        // Member variables
  index_vector_type row_index_; // starting index into col_index_ and v_
  index_vector_type col_index_; // col_index_[n] holds the column index (aka target)
  v_vector_type     v_;         // v_[n]         holds the edge value for col_index_[n]

private: // tag_invoke properties
  friend constexpr index_vector_type& tag_invoke(::std::graph::access::vertices_fn_t, csr_graph_base& g) {
    return g.row_index_;
  }
  friend constexpr const index_vector_type& tag_invoke(::std::graph::access::vertices_fn_t, const csr_graph_base& g) {
    return g.row_index_;
  }

  friend vertex_key_type tag_invoke(::std::graph::access::vertex_key_fn_t, const csr_graph_base& g, const_iterator ui) {
    return static_cast<vertex_key_type>(ui - g.row_index_.begin());
  }

  friend constexpr edges_type tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>, "row_index_ must be a contiguous range to get next row");
    vertex_type* u2 = &u + 1;
    return edges_type(g.col_index_.begin() + u, g.col_index_.begin() + *u2);
  }
  friend constexpr const edges_type
  tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>, "row_index_ must be a contiguous range to get next row");
    const vertex_type* u2 = &u + 1;
    return const_edges_type(g.col_index_.begin() + u, g.col_index_.begin() + *u2);
  }
};


/// <summary>
/// csr_graph - compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV">Edge value type</typeparam>
/// <typeparam name="VV">Vertex value type</typeparam>
/// <typeparam name="GV">Graph value type</typeparam>
/// <typeparam name="VKey">Vertex Key type</typeparam>
/// <typeparam name="Alloc">Allocator</typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_graph : public csr_graph_base<EV, VV, GV, VKey, Alloc> {
public: // Types
  using graph_type = csr_graph<EV, VV, GV, VKey, Alloc>;
  using base_type  = csr_graph_base<EV, VV, GV, VKey, Alloc>;

  using edge_value_type = EV;

  using vertex_key_type   = VKey;
  using vertex_value_type = VV;

  using graph_value_type = GV;
  using value_type       = GV;

public: // Construction/Destruction
  constexpr csr_graph()                 = default;
  constexpr csr_graph(const csr_graph&) = default;
  constexpr csr_graph(csr_graph&&)      = default;
  constexpr ~csr_graph()                = default;

  constexpr csr_graph& operator=(const csr_graph&) = default;
  constexpr csr_graph& operator=(csr_graph&&) = default;

  // gv&,  alloc
  // gv&&, alloc
  //       alloc


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
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

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
  /// @param value      The graph value.
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
  constexpr csr_graph(const graph_value_type& value,
                      ERng&                   erng,
                      const EKeyFnc&          ekey_fnc,
                      const EValueFnc&        evalue_fnc,
                      Alloc                   alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(value) {}

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
  /// @param value      The graph value.
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
  constexpr csr_graph(graph_value_type&& value,
                      ERng&              erng,
                      const EKeyFnc&     ekey_fnc,
                      const EValueFnc&   evalue_fnc,
                      Alloc              alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc), value_(move(value)) {}


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
  /// @param max_vertex_key Number of vertices to reserve before loading the graph
  /// @param max_edges      Number of edges to reserve before loading the graph
  /// @param erng           The container of edge data.
  /// @param ekey_fnc       The edge key extractor functor:
  ///                       ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc     The edge value extractor functor:
  ///                       evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                       edge_value_t<G>).
  /// @param alloc          The allocator to use for internal containers for
  ///                       vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(vertex_key_type  max_vertex_key,
                      size_t           max_edges,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : base_type(max_vertex_key, max_edges, erng, ekey_fnc, evalue_fnc, alloc) {}

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
  /// @param value          Graph value.
  /// @param max_vertex_key Number of vertices to reserve before loading the graph
  /// @param max_edges      Number of edges to reserve before loading the graph
  /// @param erng           The container of edge data.
  /// @param ekey_fnc       The edge key extractor functor:
  ///                       ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc     The edge value extractor functor:
  ///                       evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                       edge_value_t<G>).
  /// @param alloc          The allocator to use for internal containers for
  ///                       vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(const graph_value_type& value,
                      vertex_key_type         max_vertex_key,
                      size_t                  max_edges,
                      ERng&                   erng,
                      const EKeyFnc&          ekey_fnc,
                      const EValueFnc&        evalue_fnc,
                      Alloc                   alloc = Alloc())
        : base_type(max_vertex_key, max_edges, erng, ekey_fnc, evalue_fnc, alloc), value_(value) {}

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
  /// @param value          Graph value.
  /// @param max_vertex_key Number of vertices to reserve before loading the graph
  /// @param max_edges      Number of edges to reserve before loading the graph
  /// @param erng           The container of edge data.
  /// @param ekey_fnc       The edge key extractor functor:
  ///                       ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc     The edge value extractor functor:
  ///                       evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                       edge_value_t<G>).
  /// @param alloc          The allocator to use for internal containers for
  ///                       vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(graph_value_type&& value,
                      vertex_key_type    max_vertex_key,
                      size_t             max_edges,
                      ERng&              erng,
                      const EKeyFnc&     ekey_fnc,
                      const EValueFnc&   evalue_fnc,
                      Alloc              alloc = Alloc())
        : base_type(max_vertex_key, max_edges, erng, ekey_fnc, evalue_fnc, alloc), value_(move(value)) {}

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with a tuple with 3 edge elements: source_vertex_key,
  /// target_vertex_key and edge_value.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_key,
  ///              target_vertex_key and the edge value.
  /// @param alloc Allocator.
  ///
  constexpr csr_graph(const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
                      const Alloc&                                                                      alloc = Alloc())
        : base_type(ilist, alloc) {}

private: // tag_invoke properties
  friend constexpr value_type& tag_invoke(::std::graph::access::graph_value_fn_t, graph_type& g) { return g.value_; }
  friend constexpr const value_type& tag_invoke(::std::graph::access::graph_value_fn_t, const graph_type& g) {
    return g.value_;
  }

private: // Member variables
  graph_value_type value_ = graph_value_type();
};

/// <summary>
/// csr_graph - compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV">Edge value type</typeparam>
/// <typeparam name="VV">Vertex value type</typeparam>
/// <typeparam name="GV">Graph value type</typeparam>
/// <typeparam name="VKey">Vertex Key type</typeparam>
/// <typeparam name="Alloc">Allocator</typeparam>
template <class EV, class VV, integral VKey, class Alloc>
class csr_graph<EV, VV, void, VKey, Alloc> : public csr_graph_base<EV, VV, void, VKey, Alloc> {
public: // Types
  using graph_type = csr_graph<EV, VV, void, VKey, Alloc>;
  using base_type  = csr_graph_base<EV, VV, void, VKey, Alloc>;

  using edge_value_type = EV;

  using vertex_key_type   = VKey;
  using vertex_value_type = VV;

  using graph_value_type = void;
  using value_type       = void;

public: // Construction/Destruction
  constexpr csr_graph()                 = default;
  constexpr csr_graph(const csr_graph&) = default;
  constexpr csr_graph(csr_graph&&)      = default;
  constexpr ~csr_graph()                = default;

  constexpr csr_graph& operator=(const csr_graph&) = default;
  constexpr csr_graph& operator=(csr_graph&&) = default;

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
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : base_type(erng, ekey_fnc, evalue_fnc, alloc) {}

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
  /// @param max_vertex_key Number of vertices to reserve before loading the graph
  /// @param max_edges      Number of edges to reserve before loading the graph
  /// @param erng           The container of edge data.
  /// @param ekey_fnc       The edge key extractor functor:
  ///                       ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc     The edge value extractor functor:
  ///                       evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                       edge_value_t<G>).
  /// @param alloc          The allocator to use for internal containers for
  ///                       vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(vertex_key_type  max_vertex_key,
                      size_t           max_edges,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : base_type(max_vertex_key, max_edges, erng, ekey_fnc, evalue_fnc, alloc) {}

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with a tuple with 3 edge elements: source_vertex_key,
  /// target_vertex_key and edge_value.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_key,
  ///              target_vertex_key and the edge value.
  /// @param alloc Allocator.
  ///
  constexpr csr_graph(const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
                      const Alloc&                                                                      alloc = Alloc())
        : base_type(ilist, alloc) {}


public:  // Operations
private: // tag_invoke properties
};

} // namespace std::graph::container
