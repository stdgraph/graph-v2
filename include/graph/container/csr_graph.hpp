#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <cstdint>
#include "graph/graph.hpp"

namespace std::graph::container {

template <class EV      = empty_value,
          class VV      = void,
          class GV      = void,
          integral VKey = uint32_t,
          class Alloc   = allocator<uint32_t>>
class csr_graph;


template <class EV, class VV, class GV, integral VKey, class Alloc>
class const_csr_vertex_iterator {
public: // types
  using graph_type        = csr_graph<EV, VV, GV, VKey, Alloc>;
  using index_vector_type = graph_type::index_vector_type;

  using key_type        = graph_type::vertex_key_type;
  using value_type      = graph_type::vertex_value_type;
  using allocator_type  = Alloc;
  using size_type       = index_vector_type::size_type;
  using difference_type = index_vector_type::difference_type;
  using reference       = const value_type&;
  using const_reference = const value_type&;
  using pointer         = allocator_traits<Alloc>::const_pointer;
  using const_pointer   = allocator_traits<Alloc>::const_pointer;

  using iterator_category = ranges::iterator_t<index_vector_type>::iterator_category; // contiguous

protected:
  using index_iterator = ranges::iterator_t<typename graph_type::index_vector_type>;

public: // Construction/Destruction/Assignment
  constexpr const_csr_vertex_iterator(graph_type& g, index_iterator vi) : g_(g), vi_(vi) {}

  constexpr const_csr_vertex_iterator()                                 = default;
  constexpr const_csr_vertex_iterator(const const_csr_vertex_iterator&) = default;
  constexpr ~const_csr_vertex_iterator()                                = default;

  constexpr const_csr_vertex_iterator& operator=(const const_csr_vertex_iterator&) = default;

public: // Properties
  reference operator*() const noexcept { return *vi_; }
  pointer   operator->() const noexcept { return vi_.operator->(); }

public: // Operators
  constexpr const_csr_vertex_iterator& operator++() noexcept {
    ++vi_;
    return *this;
  }
  constexpr const_csr_vertex_iterator operator++(int) noexcept {
    const_csr_vertex_iterator tmp(*this);
    ++*this;
    return tmp;
  }
  constexpr const_csr_vertex_iterator& operator+=(const difference_type offset) noexcept {
    vi_ += offset;
    return *this;
  }
  [[nodiscard]] constexpr const_csr_vertex_iterator operator+(const difference_type offset) const noexcept {
    const_csr_vertex_iterator tmp(*this);
    tmp += offset;
    return tmp;
  }

  constexpr const_csr_vertex_iterator& operator--() noexcept {
    --vi_;
    return *this;
  }
  constexpr const_csr_vertex_iterator operator--(int) noexcept {
    const_csr_vertex_iterator tmp(*this);
    --*this;
    return tmp;
  }
  constexpr const_csr_vertex_iterator& operator-=(const difference_type offset) noexcept {
    vi_ -= offset;
    return *this;
  }
  [[nodiscard]] constexpr const_csr_vertex_iterator operator-(const difference_type offset) const noexcept {
    const_csr_vertex_iterator tmp(*this);
    tmp -= offset;
    return tmp;
  }
  [[nodiscard]] difference_type operator-(const const_csr_vertex_iterator& rhs) const noexcept { return vi_ - rhs.vi_; }

  //[[nodiscard]] const_csr_vertex_iterator operator-(const const_csr_vertex_iterator& rhs) const noexcept {
  //  return const_csr_vertex_iterator(g_, vi_ - rhs.vi_);
  //}

  [[nodiscard]] constexpr reference operator[](const difference_type offset) const noexcept {
    return *(*this + offset);
  }

public: // Relational Operators
  [[nodiscard]] constexpr bool operator==(const const_csr_vertex_iterator& rhs) const noexcept {
    return g_ == rhs.g_ && vi_ == rhs.vi_;
  }
  [[nodiscard]] constexpr strong_ordering operator<=>(const const_csr_vertex_iterator& rhs) const noexcept {
    if (g_ != rhs.g_)
      return g_ <=> rhs.g_;
    return vi_ <=> rhs.vi_;
  }


protected: // Member Variables
  graph_type*    g_ = nullptr;
  index_iterator vi_;
};

template <class EV, class VV, class GV, integral VKey, class Alloc>
const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc>
operator+(typename const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc>::difference_type offset,
          const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc>                           iter) noexcept {
  iter += offset;
  return iter;
}


template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_vertex_iterator : public const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc> {
public: // types
  using base_type         = const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc>;
  using graph_type        = base_type::graph_type;
  using index_vector_type = base_type::index_vector_type;

  using key_type        = base_type::key_type;
  using value_type      = base_type::value_type;
  using allocator_type  = base_type::base_type;
  using size_type       = base_type::size_type;
  using difference_type = base_type::difference_type;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = allocator_traits<Alloc>::pointer;
  using const_pointer   = allocator_traits<Alloc>::const_pointer;

  using iterator_category = base_type::iterator_category; // contiguous

protected:
  using base_type::g_;
  using base_type::vi_;

public:
  using base_type::base_type; // constructors

  constexpr csr_vertex_iterator& operator=(const csr_vertex_iterator&) noexcept = default;

public: // Properties
  reference operator*() const noexcept { return *vi_; }
  pointer   operator->() const noexcept { return vi_.operator->(); }

public: // Operators
  constexpr csr_vertex_iterator& operator++() noexcept {
    ++vi_;
    return *this;
  }
  constexpr csr_vertex_iterator operator++(int) noexcept {
    csr_vertex_iterator tmp(*this);
    ++*this;
    return tmp;
  }
  constexpr csr_vertex_iterator& operator+=(const difference_type offset) noexcept {
    vi_ += offset;
    return *this;
  }
  [[nodiscard]] constexpr csr_vertex_iterator operator+(const difference_type offset) const noexcept {
    csr_vertex_iterator tmp(*this);
    tmp += offset;
    return tmp;
  }

  constexpr csr_vertex_iterator& operator--() noexcept {
    --vi_;
    return *this;
  }
  constexpr csr_vertex_iterator operator--(int) noexcept {
    csr_vertex_iterator tmp(*this);
    --*this;
    return tmp;
  }
  constexpr csr_vertex_iterator& operator-=(const difference_type offset) noexcept {
    vi_ -= offset;
    return *this;
  }
  [[nodiscard]] constexpr csr_vertex_iterator operator-(const difference_type offset) const noexcept {
    csr_vertex_iterator tmp(*this);
    tmp -= offset;
    return tmp;
  }
  [[nodiscard]] constexpr difference_type operator-(const csr_vertex_iterator& rhs) const noexcept {
    return vi_ - rhs.vi_;
  }

  //[[nodiscard]] constexpr csr_vertex_iterator operator-(const csr_vertex_iterator& rhs) const noexcept {
  //  return csr_vertex_iterator(g_, vi_ - rhs.vi_);
  //}

  [[nodiscard]] constexpr reference operator[](const difference_type offset) const noexcept {
    return *(*this + offset);
  }

public: // Relational Operators
  [[nodiscard]] constexpr bool operator==(const csr_vertex_iterator& rhs) const noexcept {
    return g_ == rhs.g_ && vi_ == rhs.vi_;
  }
  [[nodiscard]] constexpr strong_ordering operator<=>(const csr_vertex_iterator& rhs) const noexcept {
    if (g_ != rhs.g_)
      return g_ <=> rhs.g_;
    return vi_ <=> rhs.vi_;
  }
};

template <class EV, class VV, class GV, integral VKey, class Alloc>
csr_vertex_iterator<EV, VV, GV, VKey, Alloc>
operator+(typename csr_vertex_iterator<EV, VV, GV, VKey, Alloc>::difference_type offset,
          csr_vertex_iterator<EV, VV, GV, VKey, Alloc>                           iter) noexcept {
  iter += offset;
  return iter;
}


/// <summary>
/// csr_graph - compressed sparse row adjacency graph
///
/// </summary>
/// <typeparam name="EV"></typeparam>
/// <typeparam name="VKey"></typeparam>
/// <typeparam name="Alloc"></typeparam>
template <class EV, class VV, class GV, integral VKey, class Alloc>
class csr_graph {
  using index_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<VKey>;
  using v_allocator_type     = typename allocator_traits<Alloc>::template rebind_alloc<EV>;

  using index_vector_type = std::vector<VKey, index_allocator_type>;
  using v_vector_type     = std::vector<EV, v_allocator_type>;

  using csr_vertex_range_type       = index_vector_type;
  using const_csr_vertex_range_type = const index_vector_type;

  using csr_vertex_edge_range_type       = index_vector_type;
  using const_csr_vertex_edge_range_type = const index_vector_type;

public: // Types
  using graph_type = csr_graph<EV, VKey, Alloc>;

  using vertex_key_type   = VKey;
  using vertex_value_type = vertex_key_type;

  using edge_key_type   = pair<VKey, VKey>;
  using edge_value_type = EV;
  using edge_type       = VKey; // index into v_

  using const_iterator = typename index_vector_type::const_iterator;
  using iterator       = typename index_vector_type::iterator;

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
        : row_index_(alloc), col_index_(alloc), v_(alloc) {

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

  template <class ERng, class EKeyFnc, class EValueFnc>
  //requires edge_value_extractor<ERng, EKeyFnc, EValueFnc>
  constexpr csr_graph(vertex_key_type  max_vertex_key,
                      size_t           max_edges,
                      ERng&            erng,
                      const EKeyFnc&   ekey_fnc,
                      const EValueFnc& evalue_fnc,
                      Alloc            alloc = Alloc())
        : row_index_(alloc), col_index_(alloc), v_(alloc) {

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
  constexpr csr_graph(const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
                      const Alloc&                                                                      alloc = Alloc())
        : csr_graph(
                ilist,
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) {
                  return pair{get<0>(e), get<1>(e)};
                },
                [](const tuple<vertex_key_type, vertex_key_type, edge_value_type>& e) { return get<2>(e); }) {}

protected:
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

private: // Iterators
  iterator       begin() { return row_index_.begin(); }
  const_iterator begin() const { return row_index_.begin(); }
  const_iterator cbegin() const { return row_index_.begin(); }

  iterator       end() { return row_index_.end(); }
  const_iterator end() const { return row_index_.end(); }
  const_iterator cend() const { return row_index_.end(); }

private:                        // Member variables
  index_vector_type row_index_; // starting index into col_index_ and v_
  index_vector_type col_index_; // col_index_[n] holds the column index (aka target)
  v_vector_type     v_;         // v_[n]         holds the edge value for col_index_[n]

private: // tag_invoke properties
  friend constexpr index_vector_type& tag_invoke(::std::graph::access::vertices_fn_t, csr_graph& g) {
    return g.row_index_;
  }
  friend constexpr const index_vector_type& tag_invoke(::std::graph::access::vertices_fn_t, const csr_graph& g) {
    return g.row_index_;
  }

  friend vertex_key_type tag_invoke(::std::graph::access::vertex_key_fn_t, const csr_graph& g, const_iterator ui) {
    return static_cast<vertex_key_type>(ui - g.row_index_.begin());
  }

private: //friends
  friend const_csr_vertex_iterator<EV, VV, GV, VKey, Alloc>;
  friend csr_vertex_iterator<EV, VV, GV, VKey, Alloc>;
};

} // namespace std::graph::container
