#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <functional>
#include <ranges>
#include <cstdint>
#include "graph/graph.hpp"

// NOTES
//  have public load_edges(...), load_vertices(...), and load()
//  allow separation of construction and load
//  allow multiple calls to load edges as long as subsequent edges have ukey >= last vertex (append)

// load_vertices(vrng, vproj) <- [ukey,vval]
// load_edges(erng, eproj) <- [ukey, vkey, eval]
// load(erng, eproj, vrng, vproj): load_edges(erng,eproj), load_vertices(vrng,vproj)
//
// csr_graph(initializer_list<[ukey,vkey,eval]>) : load_edges(erng,eproj)
// csr_graph(erng, eproj) : load_edges(erng,eproj)
// csr_graph(erng, eproj, vrng, vproj): load(erng, eproj, vrng, vprog)
//
// [ukey,vval]      <-- copyable_vertex<VKey,VV>
// [ukey,vkey,eval] <-- copyable_edge<VKey,EV>
//
namespace std::graph::container {

/// <summary>
/// Scans a range used for input for loading edges to determine the largest vertex key used.
/// </summary>
/// <typeparam name="EProj">Edge Projection to convert from the input object to a copyable_edge_t<VKey,EV></typeparam>
/// <typeparam name="VKey">Vertex Key type</typeparam>
/// <typeparam name="EV">Edge Value type</typeparam>
/// <param name="erng">Input range to scan</param>
/// <param name="eprojection">Projects (converts) a value in erng to a copyable_edge_t<VKey,EV></param>
/// <returns>A pair<VKey,size_t> with the max vertex key used and the number of edges scanned.</returns>
template <class VKey, class EV, ranges::forward_range ERng, class EProj = identity>
requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
constexpr auto max_vertex_key(const ERng& erng, const EProj& eprojection) {
  size_t edge_count = 0;
  VKey   max_key    = 0;
  for (auto&& edge_data : erng) {
    const views::copyable_edge_t<VKey, EV>& uv = eprojection(edge_data);
    max_key = max(max_key, max(static_cast<VKey>(uv.source_key), static_cast<VKey>(uv.target_key)));
    ++edge_count;
  }
  return pair(max_key, edge_count);
}


//
// forward declarations
//
template <class EV      = bool,
          class VV      = void,
          class GV      = void,
          integral VKey = uint32_t,
          class Alloc   = allocator<uint32_t>>
requires(!is_void_v<EV>) //
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

  template <ranges::forward_range VRng, class VProj = identity>
  requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_values(const VRng& vrng, VProj projection, size_type vertex_count = 0) {
    if constexpr (ranges::sized_range<VRng>)
      vertex_count = max(vertex_count, ranges::size(vrng));
    values_.reserve(max(ranges::size(vrng), vertex_count));
    for (auto&& vvalue : vrng)
      values_.push_back(projection(vvalue));
    if (values_.size() < vertex_count)
      values_.resize(vertex_count);
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

  template <ranges::forward_range VRng, class VProj = identity>
  requires views::copyable_vertex < invoke_result<VProj, ranges::range_value_t<VRng>>, VKey,
  void > constexpr void load_values(const VRng& vrng, VProj projection, size_type vertex_count = 0) {
    // do nothing when VV=void
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

  using size_type = ranges::range_size_t<index_vector_type>;

public: // Construction/Destruction
  constexpr csr_graph_base()                      = default;
  constexpr csr_graph_base(const csr_graph_base&) = default;
  constexpr csr_graph_base(csr_graph_base&&)      = default;
  constexpr ~csr_graph_base()                     = default;

  constexpr csr_graph_base& operator=(const csr_graph_base&) = default;
  constexpr csr_graph_base& operator=(csr_graph_base&&) = default;

  constexpr csr_graph_base(const Alloc& alloc) : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {}

  /// <summary>
  /// Constructor that takes a edge range to create the CSR graph.
  /// Edges must be ordered by source_key (enforced by asssertion).
  /// </summary>
  /// <typeparam name="EProj">Edge projection function</typeparam>
  /// <param name="erng">The input range of edges</param>
  /// <param name="eprojection">Projection function that creates a copyable_edge_t<VKey,EV> from an erng value</param>
  /// <param name="alloc">Allocator to use for internal containers</param>
  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr csr_graph_base(const ERng& erng, EProj eprojection = {}, const Alloc& alloc = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    load_edges(erng, eprojection);
  }

  /// <summary>
  /// Constructor that takes edge range and vertex range to create the CSR graph.
  /// Edges must be ordered by source_key (enforced by asssertion).
  /// </summary>
  /// <typeparam name="EProj">Edge projection function</typeparam>
  /// <typeparam name="VProj">Vertex projection function</typeparam>
  /// <param name="erng">The input range of edges</param>
  /// <param name="vrng">The input range of vertices</param>
  /// <param name="eprojection">Projection function that creates a copyable_edge_t<VKey,EV> from an erng value</param>
  /// <param name="vprojection">Projection function that creates a copyable_vertex_t<VKey,EV> from a vrng value</param>
  /// <param name="alloc">Allocator to use for internal containers</param>
  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr csr_graph_base(const ERng&  erng,
                           const VRng&  vrng,
                           EProj        eprojection = {},
                           VProj        vprojection = {},
                           const Alloc& alloc       = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc), row_value_(alloc) {

    load(erng, vrng, eprojection, vprojection);
  }

  /// <summary>
  /// Constructor for easy creation of a graph that takes an initializer list
  /// of copyable_edge_t<VKey,EV> -> [source_key, target_key, value].
  /// </summary>
  /// <param name="ilist">Initializer list of copyable_edge_t<VKey,EV> -> [source_key, target_key, value]</param>
  /// <param name="alloc">Allocator to use for internal containers</param>
  constexpr csr_graph_base(const initializer_list<views::copyable_edge_t<VKey, EV>>& ilist,
                           const Alloc&                                              alloc = Alloc())
        : csr_graph_base(ilist, identity(), alloc) {}

public:
  /// <summary>
  /// Load vertex values. This should be called after load_edges() to assure that there are
  /// at least as many values as there are rows.
  /// 
  /// After this is called, the number of values will match the number of rows loaded if
  /// there are fewer values than rows that exist. If more values are passed than rows
  /// created by load_edges() then the extra values are loaded.
  /// </summary>
  /// <typeparam name="VProj">Projection function for vrng values</typeparam>
  /// <param name="vrng">Range of values to load for vertices. The order of the values is preserved in the internal vector.</param>
  /// <param name="vprojection">Projection function for vrng values</param>
  template <ranges::forward_range VRng, class VProj = identity>
  requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_vertices(const VRng& vrng, VProj vprojection) {
    size_type vertex_count = row_index_.empty() ? 0 : row_index_.size() - 1; // edges loaded?
    row_value_.load_values(vrng, vprojection, vertex_count);
  }

  /// <summary>
  /// Load the edges for the graph. 
  /// 
  /// Space for the col_index and v vectors is reserved if it can easily be evaluated (if the erng
  /// is a sized range or it is a random access range). The edge_count parameter can also be supplied
  /// to determine the number of edges to reserved. When both exist, the maximum of the two values 
  /// is used. If the edge count still can't be pre-determined the normal processing to periodically
  /// reallocate the internal vectors will occur.
  /// 
  /// Space for the row_index vector is reserved if a vertex_count > 0 is passed. If it is zero then
  /// the normal processing to periodically reallocated the internal vectors will occur.
  /// </summary>
  /// <typeparam name="EProj">Edge Projection</typeparam>
  /// <param name="erng">Input range for edges</param>
  /// <param name="eprojection">Edge projection function</param>
  /// <param name="vertex_count">Number of vertices to reserve</param>
  /// <param name="edge_count">Number of edges to reserve</param>
  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr void
  load_edges(const ERng& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng)) {
      row_index_.resize(vertex_count + 1, static_cast<vertex_key_type>(v_.size())); // add terminating row
      return;
    }

    // reserve space for rows if caller has provided it
    if (vertex_count > 0)
      row_index_.reserve(vertex_count + 1); // +1 for terminating row

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (ranges::sized_range<ERng>)
      edge_count = max(edge_count, ranges::size(erng));
    else if constexpr (ranges::random_access_range<ERng>)
      edge_count = max(edge_count, (ranges::end(erng) - ranges::begin(erng)));
    col_index_.reserve(edge_count);
    v_.reserve(edge_count);

    // Add edges
    vertex_key_type last_ukey = 0, max_vkey = 0;
    for (auto&& edge_data : erng) {
      auto&& [ukey, vkey, value] = eprojection(edge_data); // csr_graph requires EV!=void
      assert(ukey >= last_ukey);                           // ordered by ukey? (requirement)
      row_index_.resize(static_cast<size_t>(ukey) + 1, static_cast<vertex_key_type>(v_.size()));
      col_index_.push_back(vkey);
      v_.emplace_back(std::move(value));
      last_ukey = ukey;
      max_vkey  = max(max_vkey, vkey);
    }

    // ukey and vkey may refer to rows that exceed the value passed in vertex_count
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vkey + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1, static_cast<vertex_key_type>(v_.size()));
  }

  /// <summary>
  /// Load edges and then vertices for the graph. See load_edges() and load_vertices() for more
  /// information.
  /// </summary>
  /// <typeparam name="EProj">Edge Projection Function Type</typeparam>
  /// <typeparam name="VProj">Vertex Projectiong Function Type</typeparam>
  /// <param name="erng">Input edge range</param>
  /// <param name="vrng">Input vertex range</param>
  /// <param name="eprojection">Edge projection function object</param>
  /// <param name="vprojection">Vertex projection function object</param>
  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load(const ERng& erng, const VRng& vrng, EProj eprojection = {}, VProj vprojection = {}) {
    size_type vertex_count = 0;
    if constexpr (ranges::sized_range<VRng>)
      vertex_count = ranges::size(vrng);
    else if constexpr (ranges::random_access_range<VRng>)
      vertex_count = max(vertex_count, (ranges::end(vrng) - ranges::begin(vrng)));
    load_edges(erng, eprojection, vertex_count);
    load_vertices(vrng, vprojection);
  }

public: // Operations
  constexpr ranges::iterator_t<index_vector_type> find_vertex(vertex_key_type key) noexcept {
    return row_index_.begin() + key;
  }
  constexpr ranges::iterator_t<const index_vector_type> find_vertex(vertex_key_type key) const noexcept {
    return row_index_.begin() + key;
  }

private:                        // Member variables
  index_vector_type row_index_; // starting index into col_index_ and v_; holds +1 extra terminating row
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
requires(!is_void_v<EV>) //
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

  // csr_graph(      alloc)
  // csr_graph(gv&,  alloc)
  // csr_graph(gv&&, alloc)

  constexpr csr_graph(const Alloc& alloc) : base_type(alloc) {}
  constexpr csr_graph(const graph_value_type& value, const Alloc& alloc = Alloc()) : base_type(alloc), value_(value) {}
  constexpr csr_graph(graph_value_type&& value, const Alloc& alloc = Alloc()) : base_type(alloc), value_(move(value)) {}

  // csr_graph(      erng, eprojection, alloc)
  // csr_graph(gv&,  erng, eprojection, alloc)
  // csr_graph(gv&&, erng, eprojection, alloc)

  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr csr_graph(const ERng& erng, EProj eprojection, const Alloc& alloc = Alloc())
        : base_type(erng, eprojection, alloc) {}

  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr csr_graph(const graph_value_type& value, const ERng& erng, EProj eprojection, const Alloc& alloc = Alloc())
        : base_type(erng, eprojection, alloc), value_(value) {}

  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr csr_graph(graph_value_type&& value, const ERng& erng, EProj eprojection, const Alloc& alloc = Alloc())
        : base_type(erng, eprojection, alloc), value_(move(value)) {}

  // csr_graph(      erng, vrng, eprojection, vprojection, alloc)
  // csr_graph(gv&,  erng, vrng, eprojection, vprojection, alloc)
  // csr_graph(gv&&, erng, vrng, eprojection, vprojection, alloc)

  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr csr_graph(const ERng&  erng,
                      const VRng&  vrng,
                      EProj        eprojection = {},
                      VProj        vprojection = {},
                      const Alloc& alloc       = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, alloc) {}

  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr csr_graph(const graph_value_type& value,
                      const ERng&             erng,
                      const VRng&             vrng,
                      EProj                   eprojection = {},
                      VProj                   vprojection = {},
                      const Alloc&            alloc       = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, alloc), value_(value) {}

  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr csr_graph(graph_value_type&& value,
                      const ERng&        erng,
                      const VRng&        vrng,
                      EProj              eprojection = {},
                      VProj              vprojection = {},
                      const Alloc&       alloc       = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, alloc), value_(move(value)) {}


  constexpr csr_graph(const initializer_list<views::copyable_edge_t<VKey, EV>>& ilist, const Alloc& alloc = Alloc())
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
requires(!is_void_v<EV>) //
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

  template <ranges::forward_range ERng, class EProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr csr_graph(const ERng& erng, EProj eprojection, const Alloc& alloc = Alloc())
        : base_type(erng, eprojection, alloc) {}

  template <ranges::forward_range ERng, ranges::forward_range VRng, class EProj = identity, class VProj = identity>
  requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
        views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr csr_graph(const ERng&  erng,
                      const VRng&  vrng,
                      EProj        eprojection = {},
                      VProj        vprojection = {},
                      const Alloc& alloc       = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, alloc) {}

  constexpr csr_graph(const initializer_list<views::copyable_edge_t<VKey, EV>>& ilist, const Alloc& alloc = Alloc())
        : base_type(ilist, alloc) {}


public:  // Operations
private: // tag_invoke properties
};

} // namespace std::graph::container
