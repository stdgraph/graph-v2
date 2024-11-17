#pragma once
#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"

//
// neighbors(g,uid)       -> neighbor_info<VId,false,E,VV>    -> {target_id, vertex&}
// neighbors(g,uid,vvf)   -> neighbor_info<VId,false,E,VV>    -> {target_id, vertex&, value]}
//
// basic_neighbors(g,uid) -> neighbor_info<VId,false,void,EV> -> {target_id}
//
// given:    auto vvf = [&g](vertex_reference_t<G> v) { return vertex_value(g,v); }
//
// examples: for([vid, v]             : neighbors(g,uid))
//           for([vid, v, value]      : neighbors(g,uid,vvf))
//
//           for([vid]      : basic_neighbors(g,uid))
//
// Since uid is passed to neighbors(), there's no need to include Sourced versions of
// incidence().
// basic_neighbors(g,uid) is the same as basic_incidence(g,uid). It is retained for
// symmatry and to avoid confusion.
//
namespace graph {


template <adjacency_list G, bool Sourced = false, class VVF = void>
class neighbor_iterator;


/**
 * @brief Iterator for an neighbors range of edges for a vertex.
 *
 * @tparam G    Graph type
 * @tparam VVF  Edge Value Function type
*/
template <adjacency_list G, bool Sourced, class VVF>
class neighbor_iterator
      : public _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)> {
public:
  using base_type = _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)>;

  using graph_type            = remove_reference_t<G>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = invoke_result_t<VVF, vertex_reference_type>;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_info<const vertex_id_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter, const VVF& value_fn)
        : base_type(vertex_id(g, ui)), g_(&g), iter_(iter), value_fn_(&value_fn) {}
  neighbor_iterator(graph_type& g, vertex_id_type uid, const VVF& value_fn)
        : base_type(uid), g_(&g), iter_(begin(edges(g, uid))), value_fn_(&value_fn) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&)      = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type =
        neighbor_info<vertex_id_type, Sourced, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    internal_value() : shadow_{} {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (unordered_edge<G>) {
      static_assert(sourced_adjacency_list<G>);
      if (target_id(*g_, *iter_) != this->source_vertex_id()) {
        value_.shadow_.source_id = source_id(*g_.*iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
        value_.shadow_.target    = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
      } else {
        value_.shadow_.source_id = target_id(*g_.*iter_);
        value_.shadow_.target_id = source_id(*g_, *iter_);
        value_.shadow_.target    = const_cast<shadow_vertex_type*>(&source(*g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.shadow_.source_id = source_id(*g_, *iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = this->source_vertex_id();
        value_.shadow_.target_id = target_id(*g_, *iter_);
      }
      value_.shadow_.target = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
    } else {
      value_.shadow_.target_id = target_id(*g_, *iter_);
      value_.shadow_.target    = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
    }
    value_.shadow_.value =
          invoke(*value_fn_, *value_.shadow_.target); // 'value' undeclared identifier (.value not in struct?)
    return value_.value_;
  }

  constexpr neighbor_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr neighbor_iterator operator++(int) const {
    neighbor_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable internal_value value_;
  graph_type*            g_ = nullptr;
  edge_iterator          iter_;
  const VVF*             value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const neighbor_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, bool Sourced>
class neighbor_iterator<G, Sourced, void>
      : public _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)> {
public:
  using base_type = _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)>;

  using graph_type            = remove_reference_t<G>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<G>;
  using vertex_value_type     = void;
  using vertex_iterator       = vertex_iterator_t<G>;

  using edge_range    = vertex_edge_range_t<graph_type>;
  using edge_iterator = vertex_edge_iterator_t<graph_type>;
  using edge_type     = edge_t<graph_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = neighbor_info<const vertex_id_type, Sourced, vertex_reference_type, vertex_value_type>;
  using difference_type   = range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type  = neighbor_info<vertex_id_type, Sourced, shadow_vertex_type*, vertex_value_type>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    internal_value() : shadow_{} {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  neighbor_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter)
        : base_type(vertex_id(g, ui)), g_(&g), iter_(iter) {}
  neighbor_iterator(graph_type& g, vertex_id_type uid) : base_type(uid), g_(&g), iter_(begin(edges(g, uid))) {}

  constexpr neighbor_iterator()                         = default;
  constexpr neighbor_iterator(const neighbor_iterator&) = default;
  constexpr neighbor_iterator(neighbor_iterator&&)      = default;
  constexpr ~neighbor_iterator()                        = default;

  constexpr neighbor_iterator& operator=(const neighbor_iterator&) = default;
  constexpr neighbor_iterator& operator=(neighbor_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    // const in this functions signature causes target() to always return a const value, which isn't always what we want
    // shadow_vertex_type has correct constness based on the G template parameter

    if constexpr (unordered_edge<G>) {
      static_assert(sourced_adjacency_list<G>);
      if (target_id(*g_, *iter_) != this->source_vertex_id()) {
        value_.shadow_.source_id = source_id(*g_.*iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
        value_.shadow_.target    = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
      } else {
        value_.shadow_.source_id = target_id(*g_.*iter_);
        value_.shadow_.target_id = source_id(*g_, *iter_);
        value_.shadow_.target    = const_cast<shadow_vertex_type*>(&source(*g_, *iter_));
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.shadow_.source_id = source_id(*g_, *iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = this->source_vertex_id();
        value_.shadow_.target_id = target_id(*g_, *iter_);
      }
      value_.shadow_.target = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
    } else {
      value_.shadow_.target_id = target_id(*g_, *iter_);
      value_.shadow_.target    = const_cast<shadow_vertex_type*>(&target(*g_, *iter_));
    }
    return value_.value_;
  }

  constexpr neighbor_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr neighbor_iterator operator++(int) const {
    neighbor_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const neighbor_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable internal_value value_;
  graph_type*            g_ = nullptr;
  edge_iterator          iter_;

  friend bool operator==(const edge_iterator& lhs, const neighbor_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, bool Sourced, class VVF>
using neighbors_view = subrange<neighbor_iterator<G, Sourced, VVF>, vertex_edge_iterator_t<G>>;


namespace views {

  //
  // neighbors(g,uid)            -> edges[vid,v]
  // neighbors(g,uid,fn)         -> edges[vid,v,value]
  //
  namespace _Neighbors {
#if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void neighbors() = delete;             // Block unqualified name lookup
#else                                      // ^^^ no workaround / workaround vvv
    void neighbors();
#endif                                     // ^^^ workaround ^^^

    template <class _G>
    concept _Has_id_ADL = adjacency_list<_G> && requires(_G&& __g, const vertex_id_t<_G>& uid) {
      { _Fake_copy_init(neighbors(__g, uid)) }; // intentional ADL
    };
    template <class _G>
    concept _Can_id_eval = adjacency_list<_G> && requires(_G&& __g, vertex_id_t<_G>& uid) {
      { _Fake_copy_init(edges(__g, uid)) };
    };

    template <class _G, class VVF>
    concept _Has_id_vvf_ADL = adjacency_list<_G> && invocable<VVF, vertex_reference_t<_G>> &&
                              requires(_G&& __g, const vertex_id_t<_G>& uid, const VVF& vvf) {
                                { _Fake_copy_init(neighbors(__g, uid, vvf)) }; // intentional ADL
                              };
    template <class _G, class VVF>
    concept _Can_id_vvf_eval = adjacency_list<_G> && invocable<VVF, vertex_reference_t<_G>>;

    class _Cpo {
    private:
      enum class _St_id { _None, _Non_member, _Auto_eval };
      //enum class _St_id_fn { _None, _Non_member, _Auto_eval };

      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_id_ADL<_G>) {
          return {_St_id::_Non_member,
                  noexcept(_Fake_copy_init(neighbors(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
        } else if constexpr (_Can_id_eval<_G>) {
          return {_St_id::_Auto_eval,
                  noexcept(_Fake_copy_init(neighbors_view<_G, false, void>(
                        neighbor_iterator<_G, false, void>(declval<_G>(), declval<vertex_id_t<_G>>()),
                        end(edges(declval<_G>(), declval<vertex_id_t<_G>>())))))};
        } else {
          return {_St_id::_None};
        }
      }

      template <class _G>
      static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

      template <class _G, class VVF>
      [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id_vvf() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_id_vvf_ADL<_G, VVF>) {
          return {_St_id::_Non_member, noexcept(_Fake_copy_init(neighbors(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                                          declval<VVF>())))}; // intentional ADL
        } else if constexpr (_Can_id_vvf_eval<_G, VVF>) {
          return {_St_id::_Auto_eval,
                  noexcept(_Fake_copy_init(neighbors_view<_G, false, VVF>(
                        neighbor_iterator<_G, false, VVF>(declval<_G>(), declval<vertex_id_t<_G>>(), declval<VVF>()),
                        end(edges(declval<_G>(), declval<vertex_id_t<_G>>())))))};
        } else {
          return {_St_id::_None};
        }
      }

      template <class _G, class VVF>
      static constexpr _Choice_t<_St_id> _Choice_id_vvf = _Choose_id_vvf<_G, VVF>();

    public:
      /**
       * @brief Get the outgoing neighbors edges of a vertex id.
       * 
       * Complexity: O(n)
       * 
       * Default implementation: 
       *      neighbors_view<_G, false, void>(neighbor_iterator<_G, false, void>(__g, uid),
       *                                      end(edges(__g, uid)));
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @param uid Vertex id.
       * @return A range of the outgoing neighbors edges.
      */
      template <class _G>
      requires(_Choice_id<_G&>._Strategy != _St_id::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
            noexcept(_Choice_id<_G&>._No_throw) {
        constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;

        if constexpr (_Strat_id == _St_id::_Non_member) {
          return neighbors(__g, uid); // intentional ADL
        } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
          // default impl
          return neighbors_view<_G, false, void>(neighbor_iterator<_G, false, void>(__g, uid), end(edges(__g, uid)));
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "neighbors(g,uid) is not defined and the default implementation cannot be evaluated");
        }
      }

      /**
       * @brief Get the outgoing neighbors edges of a vertex id and include an edge value in the result.
       * 
       * Complexity: O(n)
       * 
       * Default implementation: 
       *      neighbors_view<_G, false, void>(neighbor_iterator<_G, false, void>(__g, uid),
       *                                      end(edges(__g, uid)));
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @param uid Vertex id.
       * @return A range of the outgoing neighbors edges.
      */
      template <class _G, class VVF>
      requires(_Choice_id_vvf<_G&, VVF>._Strategy != _St_id::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid, const VVF& vvf) const
            noexcept(_Choice_id_vvf<_G&, VVF>._No_throw) {
        constexpr _St_id _Strat_id = _Choice_id_vvf<_G&, VVF>._Strategy;

        if constexpr (_Strat_id == _St_id::_Non_member) {
          return neighbors(__g, uid); // intentional ADL
        } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
          // default impl
          return neighbors_view<_G, false, VVF>(neighbor_iterator<_G, false, VVF>(__g, uid, vvf), end(edges(__g, uid)));
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "neighbors(g,uid,vvf) is not defined and the default implementation cannot be evaluated");
        }
      }
    };
  } // namespace _Neighbors

  inline namespace _Cpos {
    inline constexpr _Neighbors::_Cpo neighbors;
  }

} // namespace views
} // namespace graph
