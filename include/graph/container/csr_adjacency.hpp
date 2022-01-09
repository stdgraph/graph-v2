#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>

namespace std::graph::container {

/// <summary>
/// csr_adjacency - compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV"></typeparam>
/// <typeparam name="KeyT"></typeparam>
/// <typeparam name="Alloc"></typeparam>
template <typename EV = empty_value, integral KeyT = uint32_t, typename Alloc = allocator<uint32_t>>
class csr_adjacency {
  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<KeyT>;
  using v_allocator_type     = typename allocator_traits<Alloc>::template rebind_alloc<EV>;

  using index_vector_type = std::vector<KeyT, index_allocator_type>;
  using v_vector_type     = std::vector<EV, v_allocator_type>;

  index_vector_type row_index_; // row_index into col_index_
  index_vector_type col_index_; //
  v_vector_type     v_;

public: // Types
  using vertex_key_type = KeyT;
  using edge_key_type   = pair<KeyT, KeyT>;
  using edge_value_type = EV;

  using vertex_range_type            = index_vector_type;
  using const_vertex_range_type      = const index_vector_type;
  using vertex_edge_range_type       = index_vector_type;
  using const_vertex_edge_range_type = const index_vector_type;

public: // Construction/Destruction
  constexpr csr_adjacency()                     = default;
  constexpr csr_adjacency(const csr_adjacency&) = default;
  constexpr csr_adjacency(csr_adjacency&&)      = default;
  constexpr ~csr_adjacency()                    = default;

  constexpr csr_adjacency& operator=(const csr_adjacency&) = default;
  constexpr csr_adjacency& operator=(csr_adjacency&&) = default;

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
  constexpr csr_adjacency(ERng& erng, const EKeyFnc& ekey_fnc, const EValueFnc& evalue_fnc, Alloc alloc = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc) {

    using edata_type  = ranges::range_value_t<ERng>;
    using evalue_type = decltype(evalue_fnc(declval<edata_type>())); // evalue_type==void suppresses loading values

    // Nothing to do?
    //if (size(erng) == 0)
    //  return;

    // Evaluate max vertex key needed
    size_t          erng_size   = 0;
    vertex_key_type max_row_idx = 0;
    for (auto& edge_data : erng) {
      auto&& [uidx, vidx] = ekey_fnc(edge_data);
      max_row_idx         = max(max_row_idx, max(uidx, vidx));
      ++erng_size;
    }
    row_index_.reserve(static_cast<size_t>(max_row_idx) + 1);
    col_index_.reserve(erng_size);
    v_.reserve(erng_size);

    // add edges
    auto [last_ukey, last_vkey] = ekey_fnc(*begin(erng));
    for (auto& edge_data : erng) {
      auto&& [ukey, vkey] = ekey_fnc(edge_data);
      if (ukey == last_ukey + 1 || size(row_index_) == 0) {
        row_index_.push_back(static_cast<vertex_key_type>(col_index_.size()));
      } else if (ukey == last_ukey) {
        if (vkey < last_vkey)
          throw_unordered_col();
        else if (vkey == last_vkey)
          throw_duplicate_col();
      } else if (ukey < last_ukey) {
        throw_unordered_row();
      } else if (ukey > last_ukey + 1) {
        throw_empty_row(); // could be supported if we create temp map from input_row_idx to internal_row_idx
      }

      // set col index & associated value
      col_index_.push_back(static_cast<vertex_key_type>(v_.size()));
      v_.emplace_back(evalue_fnc(edge_data));
      last_ukey = ukey;
      last_vkey = vkey;
    }
  }

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with a tuple with 3 edge elements: source_vertex_key,
  /// target_vertex_key and edge_value.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_key,
  ///              target_vertex_key and the edge value.
  /// @param alloc Allocator.
  ///
  constexpr csr_adjacency(const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
                          const Alloc& alloc = Alloc())
        : csr_adjacency(
                ilist,
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) {
                  return pair{get<0>(e), get<1>(e)};
                },
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) { return get<2>(e); }) {}

public: // Operations
  constexpr ranges::iterator_t<index_vector_type> find_vertex(vertex_key_type key) { return row_index_.begin() + key; }
  constexpr ranges::iterator_t<const index_vector_type> find_vertex(vertex_key_type key) const {
    return row_index_.begin() + key;
  }

private:
  constexpr void throw_unordered_row() const { throw domain_error("rows not ordered"); }
  constexpr void throw_unordered_col() const { throw domain_error("columns not ordered on a row"); }
  constexpr void throw_duplicate_col() const { throw domain_error("duplicate column on a row"); }
  constexpr void throw_empty_row() const { throw domain_error("no columns defined for a row"); }
};


} // namespace std::graph::container
