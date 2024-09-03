#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <functional>
#include <algorithm>
#include <ranges>
#include <cstdint>
#include <cassert>
#include "graph/graph.hpp"
#include <fmt/format.h>

// NOTES
//  have public load_edges(...), load_vertices(...), and load()
//  allow separation of construction and load
//  allow multiple calls to load edges as long as subsequent edges have uid >= last vertex (append)
//  VId must be large enough for the total edges and the total vertices.

// load_vertices(vrng, vproj) <- [uid,vval]
// load_edges(erng, eproj) <- [uid, vid, eval]
// load(erng, eproj, vrng, vproj): load_edges(erng,eproj), load_vertices(vrng,vproj)
//
// compressed_graph(initializer_list<[uid,vid,eval]>) : load_edges(erng,eproj)
// compressed_graph(erng, eproj) : load_edges(erng,eproj)
// compressed_graph(erng, eproj, vrng, vproj): load(erng, eproj, vrng, vprog)
//
// [uid,vval]     <-- copyable_vertex<VId,VV>
// [uid,vid,eval] <-- copyable_edge<VId,EV>
//
namespace graph::container {

/**
 * @ingroup graph_containers
 * @brief Scans a range used for input for loading edges to determine the largest vertex id used.
 * 
 * @tparam VId   Vertex Id type.
 * @tparam EV    Edge value type. It may be void.
 * @tparam ERng  Edge data range type.
 * @tparam EProj Edge Projection function type to convert from an @c ERng value type to a @c copyable_edge_t<VId,EV>.
 *               If the @c ERng value type is already copyable_edge_t<VId,EV> identity can be used.
 * 
 * @param erng        The edge data range type.
 * @param eprojection The edge projection function that converts a @c ERng value type to a @c copyable_edge_t<VId,EV>.
 *                    If @c erng value type is already copyable_edge_t<VId,EV>, identity() can be used.
 * 
 * @return A @c pair<VId,size_t> with the max vertex id used and the number of edges scanned.
*/
template <class VId, class EV, forward_range ERng, class EProj = identity>
requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
constexpr auto max_vertex_id(const ERng& erng, const EProj& eprojection) {
  size_t edge_count = 0;
  VId    max_id     = 0;
  for (auto&& edge_data : erng) {
    const copyable_edge_t<VId, EV>& uv = eprojection(edge_data);
    max_id = max(max_id, max(static_cast<VId>(uv.source_id), static_cast<VId>(uv.target_id)));
    ++edge_count;
  }
  return pair(max_id, edge_count);
}


//
// forward declarations
//
template <class EV        = void,            // edge value type
          class VV        = void,            // vertex value type
          class GV        = void,            // graph value type
          integral VId    = uint32_t,        // vertex id type
          integral EIndex = uint32_t,        // edge index type
          class Alloc     = std::allocator<VId>> // for internal containers
class compressed_graph;

/**
 * @ingroup graph_containers
 * @brief Wrapper struct for the row index to distinguish it from a vertex_id_type (VId).
 * 
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
*/
template <integral EIndex>
struct csr_row {
  using edge_index_type = EIndex;
  edge_index_type index = 0;
};

/**
 * @ingroup graph_containers
 * @brief Wrapper struct for the col (edge) index to distinguish it from a vertex_id_type (VId).
 * 
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
*/
template <integral VId>
struct csr_col {
  using vertex_id_type = VId;
  vertex_id_type index = 0;
};


/**
 * @ingroup graph_containers
 * @brief Holds vertex values in a vector that is the same size as @c row_index_.
 * 
 * If @c is_void_v<VV> then a specialization class is defined that is empty with a single 
 * constructor that accepts (and ignores) an allocator.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_row_values {
  using row_type           = csr_row<EIndex>; // index into col_index_
  using row_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<row_type>;
  using row_index_vector   = std::vector<row_type, row_allocator_type>;

public:
  using graph_type        = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using vertex_type       = row_type;
  using vertex_value_type = VV;
  using allocator_type    = typename std::allocator_traits<Alloc>::template rebind_alloc<vertex_value_type>;
  using vector_type       = std::vector<vertex_value_type, allocator_type>;

  using value_type      = VV;
  using size_type       = size_t; //VId;
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
  constexpr csr_row_values& operator=(csr_row_values&&)      = default;


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

  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_row_values(const VRng& vrng, VProj projection, size_type vertex_count) {
    if constexpr (sized_range<VRng>)
      vertex_count = max(vertex_count, size(vrng));
    resize(size(vrng));

    for (auto&& vtx : vrng) {
      const auto& [id, value] = projection(vtx);

      // if an unsized vrng is passed, the caller is responsible to call
      // resize_vertices(n) with enough entries for all the values.
      assert(static_cast<size_t>(id) < size());

      (*this)[static_cast<size_t>(id)] = value;
    }
  }

  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_row_values(VRng&& vrng, VProj projection, size_type vertex_count) {
    if constexpr (sized_range<VRng>)
      vertex_count = max(vertex_count, std::ranges::size(vrng));
    resize(std::ranges::size(vrng));

    for (auto&& vtx : vrng) {
      auto&& [id, value] = projection(vtx);

      // if an unsized vrng is passed, the caller is responsible to call
      // resize_vertices(n) with enough entries for all the values.
      assert(static_cast<size_t>(id) < size());

      (*this)[id] = std::move(value);
    }
  }

public:
  constexpr reference       operator[](size_type pos) { return v_[pos]; }
  constexpr const_reference operator[](size_type pos) const { return v_[pos]; }

private:
  friend constexpr vertex_value_type& vertex_value(graph_type& g, vertex_type& u) {
    static_assert(contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to evaluate uidx");
    auto            uidx     = g.index_of(u);
    csr_row_values& row_vals = g;
    return row_vals.v_[uidx];
  }
  friend constexpr const vertex_value_type& vertex_value(const graph_type& g, const vertex_type& u) {
    static_assert(contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to evaluate uidx");
    auto                  uidx     = g.index_of(u);
    const csr_row_values& row_vals = g;
    return row_vals.v_[uidx];
  }

private:
  vector_type v_;
};

template <class EV, class GV, integral VId, integral EIndex, class Alloc>
class csr_row_values<EV, void, GV, VId, EIndex, Alloc> {
public:
  constexpr csr_row_values(const Alloc& alloc) {}
  constexpr csr_row_values() = default;

  using value_type = void;
  using size_type  = size_t; //VId;

public:                      // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve(size_type new_cap) {}
  constexpr void resize(size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap(csr_row_values& other) noexcept {}
};


/**
 * @ingroup graph_containers
 * @brief Class to hold vertex values in a vector that is the same size as col_index_.
 * 
 * If is_void_v<EV> then the class is empty with a single
 * constructor that accepts (and ignores) an allocator.
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_col_values {
  using col_type           = csr_col<VId>; // target_id
  using col_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<col_type>;
  using col_index_vector   = std::vector<col_type, col_allocator_type>;

public:
  using graph_type      = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using edge_type       = col_type; // index into v_
  using edge_value_type = EV;
  using allocator_type  = typename std::allocator_traits<Alloc>::template rebind_alloc<edge_value_type>;
  using vector_type     = std::vector<edge_value_type, allocator_type>;

  using value_type      = EV;
  using size_type       = size_t; //VId;
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
  constexpr csr_col_values& operator=(csr_col_values&&)      = default;


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
  // edge_value(g,uv)
  friend constexpr edge_value_type& edge_value(graph_type& g, edge_type& uv) {
    auto            uv_idx   = g.index_of(uv);
    csr_col_values& col_vals = g;
    return col_vals.v_[uv_idx];
  }
  friend constexpr const edge_value_type& edge_value(const graph_type& g, const edge_type& uv) {
    auto                  uv_idx   = g.index_of(uv);
    const csr_col_values& col_vals = g;
    return col_vals.v_[uv_idx];
  }

private:
  vector_type v_;
};

template <class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_col_values<void, VV, GV, VId, EIndex, Alloc> {
public:
  constexpr csr_col_values(const Alloc& alloc) {}
  constexpr csr_col_values() = default;

  using value_type = void;
  using size_type  = size_t; //VId;

public:                      // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve(size_type new_cap) {}
  constexpr void resize(size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap(csr_col_values& other) noexcept {}
};


/**
 * @ingroup graph_containers
 * @brief Base class for compressed sparse row adjacency graph
 * 
 * For constructors that accept a partition function, the function must return a partition id for a vertex id.
 * When used, the range of input edges must be ordered by the partition id they're in. Partions may be skipped
 * (empty) but the partition id must be in increasing order.
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class compressed_graph_base
      : public csr_row_values<EV, VV, GV, VId, EIndex, Alloc>
      , public csr_col_values<EV, VV, GV, VId, EIndex, Alloc> {
  using row_values_base = csr_row_values<EV, VV, GV, VId, EIndex, Alloc>;
  using col_values_base = csr_col_values<EV, VV, GV, VId, EIndex, Alloc>;

  using row_type           = csr_row<EIndex>; // index into col_index_
  using row_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<row_type>;
  using row_index_vector   = std::vector<row_type, row_allocator_type>;

  using col_type           = csr_col<VId>; // target_id
  using col_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<col_type>;
  using col_index_vector   = std::vector<col_type, col_allocator_type>;

public: // Types
  using graph_type = compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc>;

  using partition_id_type = VId;
  using partition_vector  = std::vector<VId>;

  using vertex_id_type      = VId;
  using vertex_type         = row_type;
  using vertex_value_type   = VV;
  using vertices_type       = subrange<iterator_t<row_index_vector>>;
  using const_vertices_type = subrange<iterator_t<const row_index_vector>>;

  using edge_type        = col_type; // index into v_
  using edge_value_type  = EV;
  using edge_index_type  = EIndex;
  using edges_type       = subrange<iterator_t<col_index_vector>>;
  using const_edges_type = subrange<iterator_t<const col_index_vector>>;

  using const_iterator = typename row_index_vector::const_iterator;
  using iterator       = typename row_index_vector::iterator;

  using size_type = range_size_t<row_index_vector>;

public: // Construction/Destruction
  constexpr compressed_graph_base()                             = default;
  constexpr compressed_graph_base(const compressed_graph_base&) = default;
  constexpr compressed_graph_base(compressed_graph_base&&)      = default;
  constexpr ~compressed_graph_base()                            = default;

  constexpr compressed_graph_base& operator=(const compressed_graph_base&) = default;
  constexpr compressed_graph_base& operator=(compressed_graph_base&&)      = default;

  constexpr compressed_graph_base(const Alloc& alloc)
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc), partition_(alloc) {
    terminate_partitions();
  }

  /**
   * @brief Constructor that takes a edge range to create the CSR graph.
   * 
   * Edges must be ordered by source_id (enforced by asssertion).
   * 
   * @tparam ERng    Edge range type
   * @tparam EProj   Edge projection function type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                 The input range of edges
   * @param eprojection          Projection function that creates a @c copyable_edge_t<VId,EV> from an erng value
   * @param partition_start_ids  Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc                Allocator to use for internal containers
  */
  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  //requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> && //
  //               convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph_base(const ERng&    erng,
                                  EProj          eprojection         = {},
                                  const PartRng& partition_start_ids = std::vector<VId>(),
                                  const Alloc&   alloc               = Alloc())
        : row_values_base(alloc)
        , col_values_base(alloc)
        , row_index_(alloc)
        , col_index_(alloc)
        , partition_(partition_start_ids, alloc) {
    load_edges(erng, eprojection);
    terminate_partitions();
  }

  /**
   * @brief Constructor that takes edge range and vertex range to create the CSR graph.
   * 
   * Edges must be ordered by source_id (enforced by asssertion).
   *
   * @tparam ERng    Edge Range type
   * @tparam VRng    Vetex range type
   * @tparam EProj   Edge projection function type
   * @tparam VProj   Vertex projection function type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The input range of edges
   * @param vrng                The input range of vertices
   * @param eprojection         Projection function that creates a @c copyable_edge_t<VId,EV> from an @c erng value
   * @param vprojection         Projection function that creates a @c copyable_vertex_t<VId,EV> from a @c vrng value
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               Allocator to use for internal containers
  */
  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  //requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
  //      copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr compressed_graph_base(const ERng&    erng,
                                  const VRng&    vrng,
                                  EProj          eprojection = {}, // eproj(eval) -> {source_id,target_id [,value]}
                                  VProj          vprojection = {}, // vproj(vval) -> {target_id [,value]}
                                  const PartRng& partition_start_ids = std::vector<VId>(),
                                  const Alloc&   alloc               = Alloc())
        : row_values_base(alloc)
        , col_values_base(alloc)
        , row_index_(alloc)
        , col_index_(alloc)
        , partition_(partition_start_ids, alloc) {

    load(erng, vrng, eprojection, vprojection);
    terminate_partitions();
  }

  /**
   * @brief Constructor for easy creation of a graph that takes an initializer list 
   *        of @c copyable_edge_t<VId,EV> -> [source_id, target_id, value].
   *
   * @param ilist   Initializer list of @c copyable_edge_t<VId,EV> -> [source_id, target_id, value]
   * @param alloc   Allocator to use for internal containers
  */
  constexpr compressed_graph_base(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist,
                                  const Alloc&                                           alloc = Alloc())
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc), partition_(alloc) {
    auto part_fnc = [](vertex_id_t<graph_type>) -> partition_id_t<graph_type> { return 0; };
    load_edges(ilist, identity());
    terminate_partitions();
  }

public:
public:                            // Operations
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

  /**
   * @brief Load vertex values, callable either before or after @c load_edges(erng,eproj).
   *
   * If @c load_edges(vrng,vproj) has been called before this, the @c row_values_ vector will be
   * extended to match the number of @c row_index_.size()-1 to avoid out-of-bounds errors when
   * accessing vertex values.
   *
   * @tparam VRng   Vertex range type
   * @tparam VProj  Vertex projection function type
   * 
   * @param  vrng           Range of values to load for vertices. The order of the values is 
   *                        preserved in the internal vector.
   * @param  vprojection    Projection function for @c vrng values
  */
  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_vertices(const VRng& vrng, VProj vprojection, size_type vertex_count = 0) {
    row_values_base::load_row_values(vrng, vprojection, max(vertex_count, size(vrng)));
  }

  /**
   * Load vertex values, callable either before or after @c load_edges(erng,eproj).
   *
   * If @c load_edges(vrng,vproj) has been called before this, the @c row_values_ vector will be
   * extended to match the number of @c row_index_.size()-1 to avoid out-of-bounds errors when
   * accessing vertex values.
   *
   * @tparam VRng   Vertex range type
   * @tparam VProj  Projection function type
   * 
   * @param vrng        Range of values to load for vertices. The order of the values is preserved in the internal vector.
   * @param vprojection Projection function for @c vrng values
  */
  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_vertices(VRng& vrng, VProj vprojection = {}, size_type vertex_count = 0) {
    row_values_base::load_row_values(vrng, vprojection, max(vertex_count, size(vrng)));
  }

  /**
   * @brief Load the edges for the graph, callable either before or after @c load_vertices(erng,eproj).
   *
   * @c erng must be ordered by source_id (copyable_edge_t) and is enforced by assertion. target_id
   * can be unordered within a source_id.
   *
   * If @c erng is bi-directional, the source_id in the last entry is used to determine the maximum
   * number of rows and is used to reserve space in the internal row_index and row_value vectors.
   * If @c erng is an input_range or forward_range that evaluation can't be done and the internal
   * row_index vector is grown and resized normally as needed (the row_value vector is updated by
   * @c load_vertices(vrng,vproj)). If the caller knows the number of rows/vertices, they can call
   * @c reserve_vertices(n) to reserve the space.
   *
   * If @c erng is a sized_range, @c size(erng) is used to reserve space for the internal col_index and
   * v vectors. If it isn't a sized range, the vectors will be grown and resized normally as needed
   * as new indexes and values are added. If the caller knows the number of columns/edges, they
   * can call @c reserve_edges(n) to reserve the space.
   *
   * If row indexes have been referenced in the edges but there are no edges defined for them
   * (with source_id), rows will be added to fill out the row_index vector to avoid out-of-bounds
   * references.
   *
   * If @c load_vertices(vrng,vproj) has been called before this, the row_values_ vector will be
   * extended to match the number of @c row_index_.size()-1 to avoid out-of-bounds errors when
   * accessing vertex values.
   *
   * @todo @c ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   * 
   * @tparam ERng   Edge range type
   * @tparam EProj  Edge projection function type
   * 
   * @param erng         Input range for edges
   * @param eprojection  Edge projection function that returns a @ copyable_edge_t<VId,EV> for an element in @c erng
   * @param part_fnc     Partition function that returns a partition id for a vertex id
   * @param vertex_count The number of vertices in the graph. If 0, the number of vertices is determined by the
   *                     largest vertex id in the edge range.
   * @param edge_count   The number of edges in the graph. If 0, the number of edges is determined by the size of the
   *                     edge range.
  */
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr void load_edges(ERng&& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (begin(erng) == end(erng)) {
      terminate_partitions();
      return;
    }

    // We can get the last vertex id from the list because erng is required to be ordered by
    // the source id. It's possible a target_id could have a larger id also, which is taken
    // care of at the end of this function.
    vertex_count =
          max(vertex_count, static_cast<size_type>(last_erng_id(erng, eprojection) + 1)); // +1 for zero-based index
    reserve_vertices(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (sized_range<ERng>)
      edge_count = max(edge_count, size(erng));
    reserve_edges(edge_count);

    // Add edges
    vertex_id_type last_uid = 0, max_vid = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // compressed_graph requires EV!=void
      assert(edge.source_id >= last_uid);   // ordered by uid? (requirement)
      row_index_.resize(static_cast<size_t>(edge.source_id) + 1,
                        vertex_type{static_cast<vertex_id_type>(col_index_.size())});
      col_index_.push_back(edge_type{edge.target_id});
      if constexpr (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).emplace_back(move(edge.value));
      last_uid = edge.source_id;
      max_vid  = max(max_vid, edge.target_id);
    }

    // uid and vid may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vid + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1, vertex_type{static_cast<vertex_id_type>(col_index_.size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 1 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
  }

  // The only diff with this and ERng&& is v_.push_back vs. v_.emplace_back
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr void
  load_edges(const ERng& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (begin(erng) == end(erng)) {
      terminate_partitions();
      return;
    }

    // We can get the last vertex id from the list because erng is required to be ordered by
    // the source id. It's possible a target_id could have a larger id also, which is taken
    // care of at the end of this function.
    vertex_count =
          max(vertex_count, static_cast<size_type>(last_erng_id(erng, eprojection) + 1)); // +1 for zero-based index
    reserve_vertices(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (sized_range<ERng>)
      edge_count = max(edge_count, size(erng));
    reserve_edges(edge_count);

    // Add edges
    size_t         debug_count = 0;
    vertex_id_type last_uid = 0, max_vid = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // compressed_graph requires EV!=void
      if (edge.source_id < last_uid) {      // ordered by uid? (requirement)
        std::string msg = fmt::format(
              "source id of {} on line {} of the data input is not ordered after source id of {} on the previous line",
              edge.source_id, debug_count, last_uid);
        fmt::print("\n{}\n", msg);
        assert(false);
        throw graph_error(move(msg));
      }
      row_index_.resize(static_cast<size_t>(edge.source_id) + 1,
                        vertex_type{static_cast<vertex_id_type>(col_index_.size())});
      col_index_.push_back(edge_type{edge.target_id});
      if constexpr (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).push_back(edge.value);
      last_uid = edge.source_id;
      max_vid  = max(max_vid, edge.target_id);
      ++debug_count;
    }

    // uid and vid may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vid + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1, vertex_type{static_cast<vertex_id_type>(col_index_.size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 0 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
  }

  /**
   * @brief Load edges and then vertices for the graph. 
   *
   * See @c load_edges() and @c load_vertices() for more information.
   *
   * @tparam EProj   Edge Projection Function type
   * @tparam VProj   Vertex Projectiong Function type
   * @tparam ERng    Edge Range type
   * @tparam VRng    Vertex Range type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng            Input edge range
   * @param vrng            Input vertex range
   * @param eprojection     Edge projection function object
   * @param vprojection     Vertex projection function object
   * @param partition_start_ids  Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
  */
  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
  //      views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load(const ERng& erng, const VRng& vrng, EProj eprojection = {}, VProj vprojection = {}) {
    load_edges(erng, eprojection);
    load_vertices(vrng, vprojection); // load the values
  }

protected:
  template <class ERng, class EProj>
  constexpr vertex_id_type last_erng_id(ERng&& erng, EProj eprojection) const {
    vertex_id_type last_id = vertex_id_type();
    if constexpr (bidirectional_range<ERng>) {
      if (begin(erng) != end(erng)) {
        auto lastIt = end(erng);
        --lastIt;
        auto&& e = eprojection(*lastIt); // copyable_edge
        last_id  = max(e.source_id, e.target_id);
      }
    }
    return last_id;
  }

  constexpr void terminate_partitions() {
    if (partition_.empty())
      partition_.push_back(0);
    else
      assert(partition_[0] == 0 &&
             is_sorted(partition_.begin(), partition_.end())); // must start with vertex_id 0 and be in increasing order

    partition_.push_back(static_cast<partition_id_type>(row_index_.size()));
  }

public: // Operations
  constexpr iterator_t<row_index_vector> find_vertex(vertex_id_type id) noexcept { return row_index_.begin() + id; }
  constexpr iterator_t<const row_index_vector> find_vertex(vertex_id_type id) const noexcept {
    return row_index_.begin() + id;
  }

  constexpr edge_index_type index_of(const row_type& u) const noexcept {
    return static_cast<edge_index_type>(&u - row_index_.data());
  }
  constexpr vertex_id_type index_of(const col_type& v) const noexcept {
    return static_cast<vertex_id_type>(&v - col_index_.data());
  }

public: // Operators
  constexpr vertex_type&       operator[](vertex_id_type id) noexcept { return row_index_[id]; }
  constexpr const vertex_type& operator[](vertex_id_type id) const noexcept { return row_index_[id]; }

private:                       // Member variables
  row_index_vector row_index_; // starting index into col_index_ and v_; holds +1 extra terminating row
  col_index_vector col_index_; // col_index_[n] holds the column index (aka target)
  partition_vector partition_; // partition_[n] holds the first vertex id for each partition n
                               // holds +1 extra terminating partition

  //v_vector_type    v_;         // v_[n]         holds the edge value for col_index_[n]
  //row_values_type  row_value_; // row_value_[r] holds the value for row_index_[r], for VV!=void

private: // CPO properties
  friend constexpr vertices_type vertices(compressed_graph_base& g) {
    if (g.row_index_.empty())
      return vertices_type(g.row_index_);                                 // really empty
    else
      return vertices_type(g.row_index_.begin(), g.row_index_.end() - 1); // don't include terminating row
  }
  friend constexpr const_vertices_type vertices(const compressed_graph_base& g) {
    if (g.row_index_.empty())
      return const_vertices_type(g.row_index_);                                 // really empty
    else
      return const_vertices_type(g.row_index_.begin(), g.row_index_.end() - 1); // don't include terminating row
  }

  friend constexpr auto num_edges(const compressed_graph_base& g) {
    return static_cast<size_type>(g.col_index_.size());
  }
  friend constexpr bool has_edge(const compressed_graph_base& g) { return g.col_index_.size() > 0; }

  friend vertex_id_type vertex_id(const compressed_graph_base& g, const_iterator ui) {
    return static_cast<vertex_id_type>(ui - g.row_index_.begin());
  }

  friend constexpr edges_type edges(graph_type& g, vertex_type& u) {
    static_assert(contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to get next row");
    vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size());    // in row_index_ bounds?
    assert(static_cast<size_t>(u.index) <= g.col_index_.size() &&
           static_cast<size_t>(u2->index) <= g.col_index_.size()); // in col_index_ bounds?
    return edges_type(g.col_index_.begin() + u.index, g.col_index_.begin() + u2->index);
  }
  friend constexpr const_edges_type edges(const graph_type& g, const vertex_type& u) {
    static_assert(contiguous_range<row_index_vector>, "row_index_ must be a contiguous range to get next row");
    const vertex_type* u2 = &u + 1;
    assert(static_cast<size_t>(u2 - &u) < g.row_index_.size());    // in row_index_ bounds?
    assert(static_cast<size_t>(u.index) <= g.col_index_.size() &&
           static_cast<size_t>(u2->index) <= g.col_index_.size()); // in col_index_ bounds?
    return const_edges_type(g.col_index_.begin() + u.index, g.col_index_.begin() + u2->index);
  }

  friend constexpr edges_type edges(graph_type& g, const vertex_id_type uid) {
    assert(static_cast<size_t>(uid + 1) < g.row_index_.size()); // in row_index_ bounds?
    assert(static_cast<size_t>(g.row_index_[static_cast<size_t>(uid) + 1].index) <=
           g.col_index_.size());                                // in col_index_ bounds?
    return edges_type(g.col_index_.begin() + g.row_index_[static_cast<size_type>(uid)].index,
                      g.col_index_.begin() + g.row_index_[static_cast<size_type>(uid + 1)].index);
  }
  friend constexpr const_edges_type edges(const graph_type& g, const vertex_id_type uid) {
    assert(static_cast<size_t>(uid + 1) < g.row_index_.size());                      // in row_index_ bounds?
    assert(static_cast<size_t>(g.row_index_[uid + 1].index) <= g.col_index_.size()); // in col_index_ bounds?
    return const_edges_type(g.col_index_.begin() + g.row_index_[static_cast<size_type>(uid)].index,
                            g.col_index_.begin() + g.row_index_[static_cast<size_type>(uid + 1)].index);
  }


  // target_id(g,uv), target(g,uv)
  friend constexpr vertex_id_type target_id(const graph_type& g, const edge_type& uv) noexcept { return uv.index; }

  friend constexpr vertex_type& target(graph_type& g, edge_type& uv) noexcept {
    return g.row_index_[static_cast<size_type>(uv.index)];
  }
  friend constexpr const vertex_type& target(const graph_type& g, const edge_type& uv) noexcept {
    return g.row_index_[static_cast<size_type>(uv.index)];
  }

  friend constexpr auto num_partitions(const compressed_graph_base& g) {
    return static_cast<partition_id_type>(g.partition_.size() - 1);
  }

  friend constexpr auto partition_id(const compressed_graph_base& g, vertex_id_type uid) {
    auto it = std::upper_bound(g.partition_.begin(), g.partition_.end(), uid);
    return static_cast<partition_id_type>(it - g.partition_.begin() - 1);
  }

  friend constexpr auto num_vertices(const compressed_graph_base& g, partition_id_type pid) {
    assert(static_cast<size_t>(pid) < g.partition_.size() - 1);
    g.partition_[pid + 1] - g.partition_[pid];
  }

  friend constexpr auto vertices(const compressed_graph_base& g, partition_id_type pid) {
    assert(static_cast<size_t>(pid) < g.partition_.size() - 1);
    return subrange(g.row_index_.begin() + g.partition_[pid], g.row_index_.begin() + g.partition_[pid + 1]);
  }

  friend row_values_base;
  friend col_values_base;
};


/**
 * @ingroup graph_containers
 * @brief Compressed Sparse Row adjacency graph container.
 *
 * When defining multiple partitions, the partition_start_ids[] must be in increasing order.
 * If the partition_start_ids[] is empty, all vertices are in partition 0. If partition_start_ids[0]!=0,
 * 0 will be inserted as the start of the first partition id.
 * 
 * @tparam EV Edge value type
 * @tparam VV Vertex value type
 * @tparam GV Graph value type
 * @tparam VI Vertex Id type. This must be large enough for the count of vertices.
 * @tparam EIndex Edge Index type. This must be large enough for the count of edges.
 * @tparam Alloc Allocator type
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class compressed_graph : public compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc> {
public: // Types
  using graph_type = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using base_type  = compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc>;

  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using value_type        = GV;

  using vertex_id_type = VId;

public: // Construction/Destruction
  constexpr compressed_graph()                        = default;
  constexpr compressed_graph(const compressed_graph&) = default;
  constexpr compressed_graph(compressed_graph&&)      = default;
  constexpr ~compressed_graph()                       = default;

  constexpr compressed_graph& operator=(const compressed_graph&) = default;
  constexpr compressed_graph& operator=(compressed_graph&&)      = default;

  // compressed_graph(      alloc)
  // compressed_graph(gv&,  alloc)
  // compressed_graph(gv&&, alloc)

  constexpr compressed_graph(const Alloc& alloc) : base_type(alloc) {}
  constexpr compressed_graph(const graph_value_type& value, const Alloc& alloc = Alloc())
        : base_type(alloc), value_(value) {}
  constexpr compressed_graph(graph_value_type&& value, const Alloc& alloc = Alloc())
        : base_type(alloc), value_(move(value)) {}

  // compressed_graph(      erng, eprojection, alloc)
  // compressed_graph(gv&,  erng, eprojection, alloc)
  // compressed_graph(gv&&, erng, eprojection, alloc)

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             EProj          eprojection,
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc) {}

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const graph_value_type& value,
                             const ERng&             erng,
                             EProj                   eprojection,
                             const PartRng&          partition_start_ids = std::vector<VId>(),
                             const Alloc&            alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc), value_(value) {}

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(graph_value_type&& value,
                             const ERng&        erng,
                             EProj              eprojection,
                             const PartRng&     partition_start_ids = std::vector<VId>(),
                             const Alloc&       alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc), value_(move(value)) {}

  // compressed_graph(      erng, vrng, eprojection, vprojection, alloc)
  // compressed_graph(gv&,  erng, vrng, eprojection, vprojection, alloc)
  // compressed_graph(gv&&, erng, vrng, eprojection, vprojection, alloc)

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             const VRng&    vrng,
                             EProj          eprojection         = {},
                             VProj          vprojection         = {},
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc) {}

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const graph_value_type& value,
                             const ERng&             erng,
                             const VRng&             vrng,
                             EProj                   eprojection         = {},
                             VProj                   vprojection         = {},
                             const PartRng&          partition_start_ids = std::vector<VId>(),
                             const Alloc&            alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc), value_(value) {}

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(graph_value_type&& value,
                             const ERng&        erng,
                             const VRng&        vrng,
                             EProj              eprojection         = {},
                             VProj              vprojection         = {},
                             const PartRng&     partition_start_ids = std::vector<VId>(),
                             const Alloc&       alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc), value_(move(value)) {}

  constexpr compressed_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist, const Alloc& alloc = Alloc())
        : base_type(ilist, alloc) {}

private: // CPO properties
  friend constexpr value_type&       graph_value(graph_type& g) { return g.value_; }
  friend constexpr const value_type& graph_value(const graph_type& g) { return g.value_; }

private: // Member variables
  graph_value_type value_ = graph_value_type();
};

/**
 * @ingroup graph_containers
 * @brief Compressed Sparse Row adjacency graph container.
 *
 * For constructors that accept a partition function, the function must return a partition id for a vertex id.
 * When used, the range of input edges must be ordered by the partition id they're in. Partions may be skipped
 * (empty) but the partition id must be in increasing order.
 *
 * @tparam EV Edge value type
 * @tparam VV Vertex value type
 * @tparam VI Vertex Id type. This must be large enough for the count of vertices.
 * @tparam EIndex Edge Index type. This must be large enough for the count of edges.
 * @tparam Alloc Allocator type
*/
template <class EV, class VV, integral VId, integral EIndex, class Alloc>
class compressed_graph<EV, VV, void, VId, EIndex, Alloc>
      : public compressed_graph_base<EV, VV, void, VId, EIndex, Alloc> {
public: // Types
  using graph_type = compressed_graph<EV, VV, void, VId, EIndex, Alloc>;
  using base_type  = compressed_graph_base<EV, VV, void, VId, EIndex, Alloc>;

  using vertex_id_type    = VId;
  using vertex_value_type = VV;

  using graph_value_type = void;
  using value_type       = void;

public: // Construction/Destruction
  constexpr compressed_graph()                        = default;
  constexpr compressed_graph(const compressed_graph&) = default;
  constexpr compressed_graph(compressed_graph&&)      = default;
  constexpr ~compressed_graph()                       = default;

  constexpr compressed_graph& operator=(const compressed_graph&) = default;
  constexpr compressed_graph& operator=(compressed_graph&&)      = default;

  // edge-only construction
  template <forward_range ERng, class EProj = identity, forward_range PartRng = std::vector<VId>>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr compressed_graph(const ERng&    erng,
                             EProj          eprojection         = identity(),
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc) {}

  // edge and vertex value construction
  template <forward_range ERng,
            forward_range VRng,
            class EProj           = identity,
            class VProj           = identity,
            forward_range PartRng = std::vector<VId>>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             const VRng&    vrng,
                             EProj          eprojection         = {},
                             VProj          vprojection         = {},
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc) {}

  // initializer list using edge_descriptor<VId,true,void,EV>
  constexpr compressed_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist, const Alloc& alloc = Alloc())
        : base_type(ilist, alloc) {}
};

} // namespace graph::container
