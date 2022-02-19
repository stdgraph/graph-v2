#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <cstdint>
#include "graph/graph.hpp"

// NOTES
//  have public load_edges(...), load_vertices(...), and load()
//  allow separation of construction and load
//  allow multiple calls to load edges as long as subsequent edges have ukey >= last vertex (append)

// load_vertices(vrng, vvalue_fnc) -> [ukey,vval]
//
// load_edges(erng, eproj) -> [ukey,vkey]
// load_edges(erng, eproj) -> [ukey,vkey, eval]
//
// load_edges(erng, eproj) -> [ukey,vkey]
// load_edges(erng, eproj) -> [ukey,vkey, eval]
//
// load_edges(erng, eproj, vrng, vproj) -> [ukey,vkey],       [ukey,vval]
// load_edges(erng, eproj, vrng, vproj) -> [ukey,vkey, eval], [ukey,vval]
//
// load_edges(initializer_list<[ukey,vkey]>
// load_edges(initializer_list<[ukey,vkey,eval]>
//
// [ukey,vval]      <-- copyable_vertex<VKey,VV>
// [ukey,vkey]      <-- copyable_edge<VKey,void>
// [ukey,vkey,eval] <-- copyable_edge<VKey,EV>
//
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
class csr_row_values {
public:
  using graph_type = csr_graph<EV, VV, GV, VKey, Alloc>;

  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<VKey>;
  using index_vector_type    = std::vector<VKey, index_allocator_type>;

  using vertex_key_type             = VKey;
  using vertex_type                 = vertex_key_type;
  using vertex_value_type           = VV;
  using vertex_value_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_value_type>;
  using values_type                 = vector<vertex_value_type, vertex_value_allocator_type>;

  using size_type = ranges::range_size_t<values_type>;

  constexpr csr_row_values(Alloc& alloc) : values_(alloc) {}

  constexpr csr_row_values()                      = default;
  constexpr csr_row_values(const csr_row_values&) = default;
  constexpr csr_row_values(csr_row_values&&)      = default;
  constexpr ~csr_row_values()                     = default;

  constexpr csr_row_values& operator=(const csr_row_values&) = default;
  constexpr csr_row_values& operator=(csr_row_values&&) = default;

  template <ranges::forward_range VRng, class VValueFnc>
  // VValueFnc is a projection with default of identity
  constexpr void load_values(const VRng& vrng, const VValueFnc& vvalue_fnc) {
    if constexpr (ranges::sized_range<VRng>)
      values_.reserve(vrng.size());
    for (auto&& vvalue : vrng)
      values_.push_back(vvalue_fnc(vvalue));
  }

  constexpr size_type size() const noexcept { return values_.size(); }

  constexpr void reserve(size_type new_cap) { values_.reserve(new_cap); }
  constexpr void resize(size_type n) { values_.resize(n); }

private: // Member variables
  values_type values_;

private: // tag_invoke properties
  friend constexpr vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>,
                  "row_index_ must be a contiguous range to evaluate uidx");
    auto uidx = &u - g.row_index_.data();
    return g.values_[uidx];
  }
  friend constexpr const vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>,
                  "row_index_ must be a contiguous range to evaluate uidx");
    auto uidx = &u - g.row_index_.data();
    return g.values_[uidx];
  }
};

template <class EV, class GV, integral VKey, class Alloc>
class csr_row_values<EV, void, GV, VKey, Alloc> {
public:
  using size_type = size_t;

  constexpr csr_row_values(Alloc& alloc) {}

  constexpr csr_row_values()                      = default;
  constexpr csr_row_values(const csr_row_values&) = default;
  constexpr csr_row_values(csr_row_values&&)      = default;
  constexpr ~csr_row_values()                     = default;

  constexpr csr_row_values& operator=(const csr_row_values&) = default;
  constexpr csr_row_values& operator=(csr_row_values&&) = default;

  constexpr size_type size() const noexcept { return 0; }

  constexpr void reserve(size_type new_cap) {}
  constexpr void resize(size_type n) {}

  template <ranges::forward_range VRng, class VValueFnc>
  constexpr void load_values(const VRng& vrng, const VValueFnc& vvalue_fnc) {
    //static_assert(false, "vertex values being loaded when VV is void for csr_graph");
  }
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
class csr_graph_base {
  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<VKey>;
  using v_allocator_type     = typename allocator_traits<Alloc>::template rebind_alloc<EV>;

  using index_vector_type = std::vector<VKey, index_allocator_type>;
  using v_vector_type     = std::vector<EV, v_allocator_type>;

  using csr_vertex_range_type       = index_vector_type;
  using const_csr_vertex_range_type = const index_vector_type;

  using csr_vertex_edge_range_type       = index_vector_type;
  using const_csr_vertex_edge_range_type = const index_vector_type;

  using row_values_type = csr_row_values<EV, VV, GV, VKey, Alloc>;

public: // Types
  using graph_type = csr_graph_base<EV, VV, GV, VKey, Alloc>;

  using vertex_key_type     = VKey;
  using vertex_type         = vertex_key_type;
  using vertex_value_type   = VV;
  using vertices_type       = ranges::subrange<ranges::iterator_t<index_vector_type>>;
  using const_vertices_type = ranges::subrange<ranges::iterator_t<const index_vector_type>>;

  using edge_value_type  = EV;
  using edge_type        = VKey; // index into v_
  using edges_type       = ranges::subrange<ranges::iterator_t<index_vector_type>>;
  using const_edges_type = ranges::subrange<ranges::iterator_t<const index_vector_type>>;

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
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng))
      return;

    // Evaluate edge_count and max vertex key needed
    auto [max_key, edge_count] = max_vertex_key(erng, ekey_fnc);

    load_edges(erng, ekey_fnc, evalue_fnc, max_key, edge_count);
    row_value_.resize(row_index_.size());
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
  /// @param erng       The container of edge data.
  /// @param ekey_fnc   The edge key extractor functor:
  ///                   ekey_fnc(ERng::value_type) -> directed_adjacency_vector::edge_key_type
  /// @param evalue_fnc The edge value extractor functor:
  ///                   evalue_fnc(ERng::value_type) -> edge_value_t<G> (or a value convertible
  ///                   edge_value_t<G>).
  /// @param alloc      The allocator to use for internal containers for
  ///                   vertices & edges.
  ///
  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph_base(ERng&            erng,
                           const EKeyFnc&   ekey_fnc,
                           const EValueFnc& evalue_fnc,
                           VRng&            vrng,
                           const VValueFnc& vvalue_fnc,
                           Alloc            alloc = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng))
      return;

    // Evaluate edge_count and max vertex key needed
    auto [max_key, edge_count] = max_vertex_key(erng, ekey_fnc);

    if (ranges::sized_range<VRng>)
      max_key = max(max_key, static_cast<vertex_key_type>(ranges::size(vrng) - 1));

    load_edges(erng, ekey_fnc, evalue_fnc, max_key, edge_count);
    row_value_.load_values(vrng, vvalue_fnc);
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
  constexpr csr_graph_base(ERng&            erng,
                           const EKeyFnc&   ekey_fnc,
                           const EValueFnc& evalue_fnc,
                           vertex_key_type  max_vertex_key,
                           size_t           edge_count = 0,
                           Alloc            alloc      = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    load_edges(erng, ekey_fnc, evalue_fnc, max_vertex_key, edge_count);
    row_value_.resize(row_index_.size() - 1);
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
  template <class ERng, class EKeyFnc, class EValueFnc, class VRng, class VValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph_base(ERng&            erng,
                           const EKeyFnc&   ekey_fnc,
                           const EValueFnc& evalue_fnc,
                           VRng&            vrng,
                           const VValueFnc& vvalue_fnc,
                           vertex_key_type  max_vertex_key,
                           size_t           edge_count = 0,
                           Alloc            alloc      = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    load_edges(erng, ekey_fnc, evalue_fnc, max_vertex_key, edge_count);
    row_value_.load_values(vrng, vvalue_fnc);
    //this->vertex_values_.resize(row_index_.size() - 1);
    //assert(this->vertex_values_.size() == row_index_.size() - 1); // what to do if not the same?
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
                alloc) {}

protected:
  template <ranges::forward_range ERng, class EKeyFnc>
  constexpr pair<vertex_key_type, size_t> max_vertex_key(const ERng& erng, const EKeyFnc& ekey_fnc) {
    size_t          edge_count = 0;
    vertex_key_type max_key    = 0;
    for (auto&& [source_key, target_key] : erng) {
      max_key = max(max_key, max(source_key, target_key));
      ++edge_count;
    }
    return pair(max_key, edge_count);
  }

  template <class VRng, class VValueFnc>
  constexpr void load_vertices(VRng& vrng, const VValueFnc& vvalue_fnc) {
    if constexpr (!is_void_v<VV>) {
      using fnc_value_t = decltype(vvalue_fnc(declval<ranges::range_value_t<VRng>>()));
      static_assert(is_convertible_v<fnc_value_t, VV>);
      load_values(vrng, vvalue_fnc);
    }
  }

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr void load_edges(ERng&            erng,
                            const EKeyFnc&   ekey_fnc,
                            const EValueFnc& evalue_fnc,
                            vertex_key_type  max_vertex_key = 0,
                            size_t           edge_count     = 0) {
    if (ranges::sized_range<ERng>)
      edge_count = max(edge_count, ranges::size(erng));

    row_index_.reserve(static_cast<size_t>(max_vertex_key) + 2); // +1 for zero-based-index and +1 for terminating row
    col_index_.reserve(edge_count);
    v_.reserve(edge_count);

    // add edges
    vertex_key_type last_ukey = 0;
    for (auto&& edge_data : erng) {
      auto&& [ukey, vkey] = ekey_fnc(edge_data);
      auto&& value        = evalue_fnc(edge_data);

      assert(ukey >= last_ukey); // ordered by ukey?
      row_index_.resize(static_cast<size_t>(ukey) + 1, static_cast<vertex_key_type>(v_.size()));
      col_index_.push_back(vkey);
      v_.emplace_back(value);
      last_ukey = ukey;
    }
    assert(max_vertex_key >= row_index_.size() - 1);
    row_index_.resize(static_cast<size_t>(max_vertex_key) + 1,
                      static_cast<vertex_key_type>(v_.size())); // add terminating row
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
  row_values_type   row_value_; // row_value_[r] holds the value for row_index_[r], for VV!=void

private: // tag_invoke properties
  friend constexpr vertices_type tag_invoke(::std::graph::access::vertices_fn_t, csr_graph_base& g) {
    if (g.row_index_.empty())
      return vertices_type(g.row_index_); // really empty
    else
      return vertices_type(g.row_index_.begin(), g.row_index_.end() - 1); // don't include terminating row
  }
  friend constexpr const_vertices_type tag_invoke(::std::graph::access::vertices_fn_t, const csr_graph_base& g) {
    if (g.row_index_.empty())
      return const_vertices_type(g.row_index_); // really empty
    else
      return const_vertices_type(g.row_index_.begin(), g.row_index_.end() - 1); // don't include terminating row
  }

  friend vertex_key_type tag_invoke(::std::graph::access::vertex_key_fn_t, const csr_graph_base& g, const_iterator ui) {
    return static_cast<vertex_key_type>(ui - g.row_index_.begin());
  }

  friend constexpr edges_type tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>, "row_index_ must be a contiguous range to get next row");
    vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size()); // in row_index_ bounds?
    assert(static_cast<size_t>(u) < g.col_index_.size() &&
           static_cast<size_t>(*u2) < g.col_index_.size()); // in col_index_ bounds?
    return edges_type(g.col_index_.begin() + u, g.col_index_.begin() + *u2);
  }
  friend constexpr const edges_type
  tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<index_vector_type>, "row_index_ must be a contiguous range to get next row");
    const vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size()); // in row_index_ bounds?
    assert(static_cast<size_t>(u) < g.col_index_.size() &&
           static_cast<size_t>(*u2) < g.col_index_.size()); // in col_index_ bounds?
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
