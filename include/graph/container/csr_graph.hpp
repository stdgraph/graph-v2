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
//  VKey must be large enough for the total edges and the total vertices.

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
/// Wrapper struct for the row index to distinguish it from a vertex_key_type (VKey).
/// </summary>
template <integral VKey>
struct csr_row {
  using vertex_key_type = VKey;
  vertex_key_type index = 0;
};

/// <summary>
/// Wrapper struct for the col (edge) index to distinguish it from a vertex_key_type (VKey).
/// </summary>
template <integral VKey>
struct csr_col {
  using vertex_key_type = VKey;
  vertex_key_type index = 0;
};


/// <summary>
/// Class to hold vertex values in a vector that is the same size as row_index_.
/// If is_void_v<VV> then the class is empty with a single
/// constructor that accepts (and ignores) an allocator.
/// </summary>
/// <typeparam name="EV">Edge Value type, or void if there is none</typeparam>
/// <typeparam name="VV">Vertex Value type, or void if there is none</typeparam>
/// <typeparam name="GV">Graph Value type, or void if there is none</typeparam>
/// <typeparam name="VKey">Vertex Key type. This must be large enough for the total edges and the total vertices.</typeparam>
/// <typeparam name="Alloc">Allocator type</typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_row_values {
public:
  using graph_type        = csr_graph<EV, VV, GV, VKey, Alloc>;
  using vertex_value_type = VV;
  using allocator_type    = typename allocator_traits<Alloc>::template rebind_alloc<vertex_value_type>;
  using vector_type       = vector<vertex_value_type, allocator_type>;

  using value_type      = VV;
  using size_type       = size_t; //VKey;
  using difference_type = typename vector_type::difference_type;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = typename vector_type::pointer;
  using const_pointer   = typename vector_type::const_pointer;
  using iterator        = typename vector_type::iterator;
  using const_iterator  = typename vector_type::const_iterator;

  constexpr csr_row_values(const Alloc& alloc) : v_(alloc) {}

  constexpr csr_row_values()                      = default;
  constexpr csr_row_values(const csr_row_values&) = default;
  constexpr csr_row_values(csr_row_values&&)      = default;
  constexpr ~csr_row_values()                     = default;

  constexpr csr_row_values& operator=(const csr_row_values&) = default;
  constexpr csr_row_values& operator=(csr_row_values&&) = default;


public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return static_cast<size_type>(v_.size()); }
  [[nodiscard]] constexpr bool      empty() const noexcept { return v_.empty(); }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return static_cast<size_type>(v_.size()); }

public: // Operations
  constexpr void reserve(size_type new_cap) { v_.reserve(new_cap); }
  constexpr void resize(size_type new_size) { v_.resize(new_size); }

  constexpr void clear() noexcept { v_.clear(); }
  constexpr void push_back(const value_type& value) { v_.push_back(value); }
  constexpr void emplace_back(value_type&& value) { v_.emplace_back(forward<value_type>(value)); }

  constexpr void swap(csr_row_values& other) noexcept { swap(v_, other.v_); }

  template <ranges::forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_row_values(const VRng& vrng, VProj projection, size_type vertex_count) {
    if constexpr (ranges::sized_range<VRng>)
      vertex_count = max(vertex_count, ranges::size(vrng));
    resize(ranges::size(vrng));

    for (auto&& vtx : vrng) {
      const auto& [key, value] = move(projection(vtx));

      // if an unsized vrng is passed, the caller is responsible to call
      // resize_vertices(n) with enough entries for all the values.
      assert(static_cast<size_t>(key) < size());

      (*this)[key] = value;
    }
  }

  template <ranges::forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_row_values(VRng&& vrng, VProj projection, size_type vertex_count) {
    if constexpr (ranges::sized_range<VRng>)
      vertex_count = max(vertex_count, ranges::size(vrng));
    resize(ranges::size(vrng));

    for (auto&& vtx : vrng) {
      auto&& [key, value] = projection(vtx);

      // if an unsized vrng is passed, the caller is responsible to call
      // resize_vertices(n) with enough entries for all the values.
      assert(static_cast<size_t>(key) < size());

      (*this)[key] = move(value);
    }
  }

public:
  constexpr reference       operator[](size_type pos) { return v_[pos]; }
  constexpr const_reference operator[](size_type pos) const { return v_[pos]; }

private:
  vector_type v_;
};

template <class EV, class GV, integral VKey, class Alloc>
class csr_row_values<EV, void, GV, VKey, Alloc> {
public:
  constexpr csr_row_values(const Alloc& alloc) {}
  constexpr csr_row_values() = default;

  using value_type = void;
  using size_type  = size_t; //VKey;

public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve(size_type new_cap) {}
  constexpr void resize(size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap(csr_row_values& other) noexcept {}
};


/// <summary>
/// Class to hold vertex values in a vector that is the same size as col_index_.
/// If is_void_v<EV> then the class is empty with a single
/// constructor that accepts (and ignores) an allocator.
/// </summary>
/// <typeparam name="EV">Edge Value type, or void if there is none</typeparam>
/// <typeparam name="VV">Vertex Value type, or void if there is none</typeparam>
/// <typeparam name="GV">Graph Value type, or void if there is none</typeparam>
/// <typeparam name="VKey">Vertex Key type. This must be large enough for the total edges and the total vertices.</typeparam>
/// <typeparam name="Alloc">Allocator type</typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_col_values {
public:
  using graph_type      = csr_graph<EV, EV, GV, VKey, Alloc>;
  using edge_value_type = EV;
  using allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_value_type>;
  using vector_type     = vector<edge_value_type, allocator_type>;

  using value_type      = EV;
  using size_type       = size_t; //VKey;
  using difference_type = typename vector_type::difference_type;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = typename vector_type::pointer;
  using const_pointer   = typename vector_type::const_pointer;
  using iterator        = typename vector_type::iterator;
  using const_iterator  = typename vector_type::const_iterator;

  constexpr csr_col_values(const Alloc& alloc) : v_(alloc) {}

  constexpr csr_col_values()                      = default;
  constexpr csr_col_values(const csr_col_values&) = default;
  constexpr csr_col_values(csr_col_values&&)      = default;
  constexpr ~csr_col_values()                     = default;

  constexpr csr_col_values& operator=(const csr_col_values&) = default;
  constexpr csr_col_values& operator=(csr_col_values&&) = default;


public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return static_cast<size_type>(v_.size()); }
  [[nodiscard]] constexpr bool      empty() const noexcept { return v_.empty(); }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return static_cast<size_type>(v_.size()); }

public: // Operations
  constexpr void reserve(size_type new_cap) { v_.reserve(new_cap); }
  constexpr void resize(size_type new_size) { v_.resize(new_size); }

  constexpr void clear() noexcept { v_.clear(); }
  constexpr void push_back(const value_type& value) { v_.push_back(value); }
  constexpr void emplace_back(value_type&& value) { v_.emplace_back(forward<value_type>(value)); }

  constexpr void swap(csr_col_values& other) noexcept { swap(v_, other.v_); }

public:
  constexpr reference       operator[](size_type pos) { return v_[pos]; }
  constexpr const_reference operator[](size_type pos) const { return v_[pos]; }

private:
  vector_type v_;
};

template <class VV, class GV, integral VKey, class Alloc>
class csr_col_values<void, VV, GV, VKey, Alloc> {
public:
  constexpr csr_col_values(const Alloc& alloc) {}
  constexpr csr_col_values() = default;

  using value_type = void;
  using size_type  = size_t; //VKey;

public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve(size_type new_cap) {}
  constexpr void resize(size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap(csr_col_values& other) noexcept {}
};


/// <summary>
/// csr_graph_base - base class for compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV">Edge value type</typeparam>
/// <typeparam name="VV">Vertex value type</typeparam>
/// <typeparam name="GV">Graph value type</typeparam>
/// <typeparam name="VKey">Vertex Key type. This must be large enough for the total edges and the total vertices.</typeparam>
/// <typeparam name="Alloc">Allocator</typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_graph_base
      : public csr_row_values<EV, VV, GV, VKey, Alloc>
      , public csr_col_values<EV, VV, GV, VKey, Alloc> {
  using row_values_base = csr_row_values<EV, VV, GV, VKey, Alloc>;
  using col_values_base = csr_col_values<EV, VV, GV, VKey, Alloc>;

  using row_type           = csr_row<VKey>; // index into col_index_
  using row_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<row_type>;
  using row_index_vector   = vector<row_type, row_allocator_type>;

  using col_type           = csr_col<VKey>; // target_key
  using col_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<col_type>;
  using col_index_vector   = vector<col_type, col_allocator_type>;

public: // Types
  using graph_type = csr_graph_base<EV, VV, GV, VKey, Alloc>;

  using vertex_key_type     = VKey;
  using vertex_type         = row_type;
  using vertex_value_type   = VV;
  using vertices_type       = ranges::subrange<ranges::iterator_t<row_index_vector>>;
  using const_vertices_type = ranges::subrange<ranges::iterator_t<const row_index_vector>>;

  using edge_value_type  = EV;
  using edge_type        = col_type; // index into v_
  using edges_type       = ranges::subrange<ranges::iterator_t<col_index_vector>>;
  using const_edges_type = ranges::subrange<ranges::iterator_t<const col_index_vector>>;

  using const_iterator = typename row_index_vector::const_iterator;
  using iterator       = typename row_index_vector::iterator;

  using size_type = ranges::range_size_t<row_index_vector>;

public: // Construction/Destruction
  constexpr csr_graph_base()                      = default;
  constexpr csr_graph_base(const csr_graph_base&) = default;
  constexpr csr_graph_base(csr_graph_base&&)      = default;
  constexpr ~csr_graph_base()                     = default;

  constexpr csr_graph_base& operator=(const csr_graph_base&) = default;
  constexpr csr_graph_base& operator=(csr_graph_base&&) = default;

  constexpr csr_graph_base(const Alloc& alloc)
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc) {}

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
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc) {

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
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc) {

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
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc) {
    load_edges(ilist, identity());
  }

public:
public: // Operations
  void reserve_vertices(size_type count) {
    row_index_.reserve(count + 1); // +1 for terminating row
    row_values_base::reserve(count);
  }
  void reserve_edges(size_type count) {
    col_index_.reserve(count);
    static_cast<col_values_base&>(*this).reserve(count);
  }

  void resize_vertices(size_type count) {
    row_index_.resize(count + 1); // +1 for terminating row
    row_values_resize(count);
  }
  void resize_edges(size_type count) {
    col_index_.reserve(count);
    static_cast<col_values_base&>(*this).reserve(count);
  }

  /// <summary>
  /// Load vertex values. This can be called either before or after load_edges(erng,eproj).
  ///
  /// If load_edges(vrng,vproj) has been called before this, the row_values_ vector will be
  /// extended to match the number of row_index_.size()-1 to avoid out-of-bounds errors when
  /// accessing vertex values.
  /// </summary>
  /// <typeparam name="VProj">Projection function for vrng values</typeparam>
  /// <param name="vrng">Range of values to load for vertices. The order of the values is preserved in the internal vector.</param>
  /// <param name="vprojection">Projection function for vrng values</param>
  template <ranges::forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_vertices(const VRng& vrng, VProj vprojection, size_type vertex_count = 0) {
    row_values_base::load_row_values(max(vrng, vprojection, vertex_count, ranges::size(vrng)));
  }

  /// <summary>
  /// Load vertex values. This can be called either before or after load_edges(erng,eproj).
  ///
  /// If load_edges(vrng,vproj) has been called before this, the row_values_ vector will be
  /// extended to match the number of row_index_.size()-1 to avoid out-of-bounds errors when
  /// accessing vertex values.
  /// </summary>
  /// <typeparam name="VProj">Projection function for vrng values</typeparam>
  /// <param name="vrng">Range of values to load for vertices. The order of the values is preserved in the internal vector.</param>
  /// <param name="vprojection">Projection function for vrng values</param>
  template <ranges::forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load_vertices(VRng& vrng, VProj vprojection = {}, size_type vertex_count = 0) {
    row_values_base::load_row_values(vrng, vprojection, max(vertex_count, ranges::size(vrng)));
  }

  /// <summary>
  /// Load the edges for the graph. This can be called either before or after load_vertices(erng,eproj).
  ///
  /// erng must be ordered by source_key (copyable_edge_t) and is enforced by assertion. target_key
  /// can be unordered within a source_key.
  ///
  /// If erng is bi-directional, the source_key in the last entry is used to determine the maximum
  /// number of rows and is used to reserve space in the internal row_index and row_value vectors.
  /// If erng is an input_range or forward_range that evaluation can't be done and the internal
  /// row_index vector is grown and resized normally as needed (the row_value vector is updated by
  /// load_vertices(vrng,vproj)). If the caller knows the number of rows/vertices, they can call
  /// reserve_vertices(n) to reserve the space.
  ///
  /// If erng is a sized_range, size(erng) is used to reserve space for the internal col_index and
  /// v vectors. If it isn't a sized range, the vectors will be grown and resized normally as needed
  /// as new indexes and values are added. If the caller knows the number of columns/edges, they
  /// can call reserve_edges(n) to reserve the space.
  ///
  /// If row indexes have been referenced in the edges but there are no edges defined for them
  /// (with source_key), rows will be added to fill out the row_index vector to avoid out-of-bounds
  /// references.
  ///
  /// If load_vertices(vrng,vproj) has been called before this, the row_values_ vector will be
  /// extended to match the number of row_index_.size()-1 to avoid out-of-bounds errors when
  /// accessing vertex values.
  ///
  /// TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
  /// </summary>
  /// <typeparam name="EProj">Edge Projection</typeparam>
  /// <param name="erng">Input range for edges.</param>
  /// <param name="eprojection">Edge projection function that returns a copyable_edge_t<VKey,EV> for an element in erng</param>
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr void load_edges(ERng&& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    static_assert(!is_void_v<EV>);
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng)) {
      return;
    }

    // We can get the last vertex key from the list because erng is required to be ordered by
    // the source key. It's possible a target_key could have a larger key also, which is taken
    // care of at the end of this function.
    vertex_count = std::max(vertex_count,
                            static_cast<size_type>(last_erng_key(erng, eprojection) + 1)); // +1 for zero-based index
    reserve_vertices(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (ranges::sized_range<ERng>)
      edge_count = max(edge_count, ranges::size(erng));
    reserve_edges(edge_count);

    // Add edges
    vertex_key_type last_ukey = 0, max_vkey = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // csr_graph requires EV!=void
      assert(edge.source_key >= last_ukey); // ordered by ukey? (requirement)
      row_index_.resize(static_cast<size_t>(edge.source_key) + 1,
                        vertex_type{static_cast<vertex_key_type>(static_cast<col_values_base&>(*this).size())});
      col_index_.push_back(edge_type{edge.target_key});
      if (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).emplace_back(std::move(edge.value));
      last_ukey = edge.source_key;
      max_vkey  = max(max_vkey, edge.target_key);
    }

    // ukey and vkey may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vkey + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1,
                      vertex_type{static_cast<vertex_key_type>(static_cast<col_values_base&>(*this).size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 1 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
  }

  // The only diff with this and ERng&& is v_.push_back vs. v_.emplace_back
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV>
  constexpr void
  load_edges(const ERng& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (ranges::begin(erng) == ranges::end(erng)) {
      return;
    }

    // We can get the last vertex key from the list because erng is required to be ordered by
    // the source key. It's possible a target_key could have a larger key also, which is taken
    // care of at the end of this function.
    vertex_count = std::max(vertex_count,
                            static_cast<size_type>(last_erng_key(erng, eprojection) + 1)); // +1 for zero-based index
    reserve_vertices(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (ranges::sized_range<ERng>)
      edge_count = max(edge_count, ranges::size(erng));
    reserve_edges(edge_count);

    // Add edges
    vertex_key_type last_ukey = 0, max_vkey = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // csr_graph requires EV!=void
      assert(edge.source_key >= last_ukey); // ordered by ukey? (requirement)
      row_index_.resize(static_cast<size_t>(edge.source_key) + 1,
                        vertex_type{static_cast<vertex_key_type>(static_cast<col_values_base&>(*this).size())});
      col_index_.push_back(edge_type{edge.target_key});
      if (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).push_back(edge.value);
      last_ukey = edge.source_key;
      max_vkey  = max(max_vkey, edge.target_key);
    }

    // ukey and vkey may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vkey + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1,
                      vertex_type{static_cast<vertex_key_type>(static_cast<col_values_base&>(*this).size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 0 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
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
  //requires views::copyable_edge<invoke_result<EProj, ranges::range_value_t<ERng>>, VKey, EV> &&
  //      views::copyable_vertex<invoke_result<VProj, ranges::range_value_t<VRng>>, VKey, VV>
  constexpr void load(const ERng& erng, const VRng& vrng, EProj eprojection = {}, VProj vprojection = {}) {
    load_edges(erng, eprojection);
    load_vertices(vrng, vprojection); // load the values
  }

protected:
  template <class ERng, class EProj>
  constexpr vertex_key_type last_erng_key(ERng&& erng, EProj eprojection) const {
    vertex_key_type last_key = vertex_key_type();
    if constexpr (ranges::bidirectional_range<ERng>) {
      if (ranges::begin(erng) != ranges::end(erng)) {
        auto lastIt = ranges::end(erng);
        --lastIt;
        auto&& e = eprojection(*lastIt); // copyable_edge
        last_key = max(e.source_key, e.target_key);
      }
    }
    return last_key;
  }

public: // Operations
  constexpr ranges::iterator_t<row_index_vector> find_vertex(vertex_key_type key) noexcept {
    return row_index_.begin() + key;
  }
  constexpr ranges::iterator_t<const row_index_vector> find_vertex(vertex_key_type key) const noexcept {
    return row_index_.begin() + key;
  }

public: // Operators
  constexpr vertex_type&       operator[](vertex_key_type key) noexcept { return row_index_[key]; }
  constexpr const vertex_type& operator[](vertex_key_type key) const noexcept { return row_index_[key]; }

private:                       // Member variables
  row_index_vector row_index_; // starting index into col_index_ and v_; holds +1 extra terminating row
  col_index_vector col_index_; // col_index_[n] holds the column index (aka target)
  //v_vector_type    v_;         // v_[n]         holds the edge value for col_index_[n]
  //row_values_type  row_value_; // row_value_[r] holds the value for row_index_[r], for VV!=void

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

  friend constexpr vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to evaluate uidx");
    auto             uidx     = static_cast<size_t>(&u - g.row_index_.data());
    row_values_base& row_vals = g;
    return row_vals[uidx];
  }
  friend constexpr const vertex_value_type&
  tag_invoke(::std::graph::access::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to evaluate uidx");
    auto                   uidx     = static_cast<size_t>(&u - g.row_index_.data());
    const row_values_base& row_vals = g;
    return row_vals[uidx];
  }

  friend constexpr edges_type tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, vertex_type& u) {
    static_assert(ranges::contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to get next row");
    vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size()); // in row_index_ bounds?
    assert(static_cast<size_t>(u.index) <= g.col_index_.size() &&
           static_cast<size_t>(u2->index) <= g.col_index_.size()); // in col_index_ bounds?
    return edges_type(g.col_index_.begin() + u.index, g.col_index_.begin() + u2->index);
  }
  friend constexpr const_edges_type
  tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_type& u) {
    static_assert(ranges::contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to get next row");
    const vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size()); // in row_index_ bounds?
    assert(static_cast<size_t>(u.index) <= g.col_index_.size() &&
           static_cast<size_t>(u2->index) <= g.col_index_.size()); // in col_index_ bounds?
    return const_edges_type(g.col_index_.begin() + u.index, g.col_index_.begin() + u2->index);
  }

  friend constexpr edges_type tag_invoke(::std::graph::access::edges_fn_t, graph_type& g, const vertex_key_type ukey) {
    assert(static_cast<size_t>(ukey + 1) < g.row_index_.size());                      // in row_index_ bounds?
    assert(static_cast<size_t>(g.row_index_[ukey + 1].index) <= g.col_index_.size()); // in col_index_ bounds?
    return edges_type(g.col_index_.begin() + g.row_index_[ukey].index,
                      g.col_index_.begin() + g.row_index_[ukey + 1].index);
  }
  friend constexpr const_edges_type
  tag_invoke(::std::graph::access::edges_fn_t, const graph_type& g, const vertex_key_type ukey) {
    assert(static_cast<size_t>(ukey + 1) < g.row_index_.size());                      // in row_index_ bounds?
    assert(static_cast<size_t>(g.row_index_[ukey + 1].index) <= g.col_index_.size()); // in col_index_ bounds?
    return const_edges_type(g.col_index_.begin() + g.row_index_[ukey].index,
                            g.col_index_.begin() + g.row_index_[ukey + 1].index);
  }


  // target_key(g,uv), target(g,uv)
  friend constexpr vertex_key_type
  tag_invoke(::std::graph::access::target_key_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return uv.index;
  }
  friend constexpr vertex_type& tag_invoke(::std::graph::access::target_fn_t, graph_type& g, edge_type& uv) noexcept {
    return g.row_index_[uv.index];
  }
  friend constexpr const vertex_type&
  tag_invoke(::std::graph::access::target_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return g.row_index_[uv.index];
  }

  // edge_value(g,uv)
  friend constexpr edge_value_type&
  tag_invoke(::std::graph::access::edge_value_fn_t, graph_type& g, edge_type& uv) noexcept {
    size_t uv_idx = static_cast<size_t>(&uv - g.col_index_.data());
    return static_cast<col_values_base&>(g)[uv_idx];
  }
  friend constexpr const edge_value_type&
  tag_invoke(::std::graph::access::edge_value_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    size_t uv_idx = static_cast<size_t>(&uv - g.col_index_.data());
    return static_cast<const col_values_base&>(g)[uv_idx];
  }
};


/// <summary>
/// csr_graph - compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV">Edge value type</typeparam>
/// <typeparam name="VV">Vertex value type</typeparam>
/// <typeparam name="GV">Graph value type</typeparam>
/// <typeparam name="VKey">Vertex Key type. This must be large enough for the total edges and the total vertices.</typeparam>
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
/// <typeparam name="VKey">Vertex Key type. This must be large enough for the total edges and the total vertices.</typeparam>
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
