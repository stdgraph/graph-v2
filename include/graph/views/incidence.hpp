#pragma once
#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"
#include <functional>

//
// incidence(g,uid)     -> edge_descriptor<VId,false,E,EV> -> {target_id, edge&}
// incidence(g,uid,evf) -> edge_descriptor<VId,false,E,EV> -> {target_id, edge&, value}
//
// basic_incidence(g,uid,evf) -> edge_descriptor<VId,false,void,void> -> {target_id}
//
// given:    auto evf = [&g](edge_reference_t<G> uv) { return edge_value(g,uv) };
//
// examples: for([vid, uv]        : incidence(g,uid))
//           for([vid, uv, value] : incidence(g,uid,evf))
//           for([vid]            : basic_incidence(g,uid))
//
// Since u is passed to incidence(), there's no need to include Sourced versions of
// incidence().
//
namespace graph {


template <adjacency_list G, bool Sourced = false, class EVF = void>
class incidence_iterator;

/**
 * @brief Iterator for an incidence range of edges for a vertex.
 *
 * @tparam G    Graph type
 * @tparam EVF  Edge Value Function type
*/
template <adjacency_list G, bool Sourced, class EVF>
class incidence_iterator : _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)> {
public:
  using base_type = _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)>;

  using graph_type      = remove_reference_t<G>;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  incidence_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter, const EVF& value_fn)
        : base_type(vertex_id(g, ui)), g_(&g), iter_(iter), value_fn_(&value_fn) {}
  incidence_iterator(graph_type& g, vertex_id_type uid, const EVF& value_fn)
        : base_type(uid), g_(&g), iter_(begin(edges(g, uid))), value_fn_(&value_fn) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&)      = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type = remove_reference_t<edge_reference_type>;
  using shadow_value_type =
        edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

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
    if constexpr (unordered_edge<G>) {
      if (target_id(*g_, *iter_) != this->source_vertex_id()) {
        value_.shadow_.source_id = source_id(*g_, *iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = target_id(*g_, *iter_);
        value_.shadow_.target_id = source_id(*g_, *iter_);
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.shadow_.source_id = source_id(*g_, *iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = this->source_vertex_id();
        value_.shadow_.target_id = target_id(*g_, *iter_);
      }
    } else {
      value_.shadow_.target_id = target_id(*g_, *iter_);
    }
    value_.shadow_.edge  = &*iter_;
    value_.shadow_.value = invoke(*value_fn_, *iter_);
    return value_.value_;
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  //mutable shadow_value_type        value_ = {};
  mutable internal_value value_;
  graph_type*            g_ = nullptr;
  edge_iterator          iter_;
  const EVF*             value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, bool Sourced>
class incidence_iterator<G, Sourced, void>
      : public _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)> {
public:
  using base_type = _detail::_source_vertex<G, ((Sourced && !sourced_adjacency_list<G>) || unordered_edge<G>)>;

  using graph_type      = remove_reference_t<G>;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type  = remove_reference_t<edge_reference_type>;
  using shadow_value_type = edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, edge_value_type>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    internal_value() : shadow_{} {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  incidence_iterator(graph_type& g, vertex_iterator ui, edge_iterator iter)
        : base_type(vertex_id(g, ui)), g_(&g), iter_(iter) {}
  incidence_iterator(graph_type& g, vertex_id_type uid) : base_type(uid), g_(&g), iter_(begin(edges(g, uid))) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    if constexpr (unordered_edge<G>) {
      static_assert(sourced_adjacency_list<G>);
      if (target_id(*g_, *iter_) != this->source_vertex_id()) {
        value_.shadow_.source_id = source_id(*g_.*iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = target_id(*g_.*iter_);
        value_.shadow_.target_id = source_id(*g_, *iter_);
      }
    } else if constexpr (Sourced) {
      if constexpr (sourced_adjacency_list<G>) {
        value_.shadow_.source_id = source_id(*g_, *iter_);
        value_.shadow_.target_id = target_id(*g_, *iter_);
      } else {
        value_.shadow_.source_id = this->source_vertex_id();
        value_.target_id         = target_id(*g_, *iter_);
      }
    } else {
      value_.shadow_.target_id = target_id(*g_, *iter_);
    }
    value_.shadow_.edge = &*iter_;
    return value_.value_;
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable internal_value value_;
  graph_type*            g_ = nullptr;
  edge_iterator          iter_;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};

template <class G, bool Sourced, class EVF>
using incidence_view = subrange<incidence_iterator<G, Sourced, EVF>, vertex_edge_iterator_t<G>>;

namespace views {
  //
  // incidence(g,uid)            -> edges[vid,v]
  // incidence(g,uid,fn)         -> edges[vid,v,value]
  //
  namespace _Incidence {
#if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void incidence() = delete;             // Block unqualified name lookup
#else                                      // ^^^ no workaround / workaround vvv
    void incidence();
#endif                                     // ^^^ workaround ^^^

    template <class _G>
    concept _Has_id_ADL = adjacency_list<_G> && requires(_G&& __g, const vertex_id_t<_G>& uid) {
      { _Fake_copy_init(incidence(__g, uid)) }; // intentional ADL
    };
    template <class _G>
    concept _Can_id_eval = adjacency_list<_G>;

    template <class _G, class EVF>
    concept _Has_id_evf_ADL = adjacency_list<_G> && invocable<EVF, edge_reference_t<_G>> &&
                              requires(_G&& __g, const vertex_id_t<_G>& uid, const EVF& evf) {
                                { _Fake_copy_init(incidence(__g, uid, evf)) }; // intentional ADL
                              };
    template <class _G, class EVF>
    concept _Can_id_evf_eval = adjacency_list<_G> && invocable<EVF, edge_reference_t<_G>>;

    class _Cpo {
    private:
      enum class _St_id { _None, _Non_member, _Auto_eval };
      //enum class _St_id_fn { _None, _Non_member, _Auto_eval };

      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_id_ADL<_G>) {
          return {_St_id::_Non_member,
                  noexcept(_Fake_copy_init(incidence(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
        } else if constexpr (_Can_id_eval<_G>) {
          return {_St_id::_Auto_eval,
                  noexcept(_Fake_copy_init(incidence_view<_G, false, void>(
                        incidence_iterator<_G, false, void>(declval<_G>(), declval<vertex_id_t<_G>>()),
                        end(edges(declval<_G>(), declval<vertex_id_t<_G>>())))))};
        } else {
          return {_St_id::_None};
        }
      }

      template <class _G>
      static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

      template <class _G, class EVF>
      [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id_evf() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_id_evf_ADL<_G, EVF>) {
          return {_St_id::_Non_member, noexcept(_Fake_copy_init(incidence(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                                          declval<EVF>())))}; // intentional ADL
        } else if constexpr (_Can_id_evf_eval<_G, EVF>) {
          return {_St_id::_Auto_eval,
                  noexcept(_Fake_copy_init(incidence_view<_G, false, EVF>(
                        incidence_iterator<_G, false, EVF>(declval<_G>(), declval<vertex_id_t<_G>>(), declval<EVF>()),
                        end(edges(declval<_G>(), declval<vertex_id_t<_G>>())))))};
        } else {
          return {_St_id::_None};
        }
      }

      template <class _G, class EVF>
      static constexpr _Choice_t<_St_id> _Choice_id_evf = _Choose_id_evf<_G, EVF>();

    public:
      /**
     * @brief Get the outgoing incidence edges of a vertex id.
     * 
     * Complexity: O(n)
     * 
     * Default implementation: 
     *      incidence_view<_G, false, void>(incidence_iterator<_G, false, void>(__g, uid),
     *                                      end(edges(__g, uid)));
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid Vertex id.
     * @return A range of the outgoing incidence edges.
    */
      template <class _G>
      requires(_Choice_id<_G&>._Strategy != _St_id::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
            noexcept(_Choice_id<_G&>._No_throw) {
        constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;

        if constexpr (_Strat_id == _St_id::_Non_member) {
          return incidence(__g, uid); // intentional ADL
        } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
          // default impl
          return incidence_view<_G, false, void>(incidence_iterator<_G, false, void>(__g, uid), end(edges(__g, uid)));
        } else {
          static_assert(_Always_false<_G>,
                        "incidence(g,uid) is not defined and the default implementation cannot be evaluated");
        }
      }

      /**
     * @brief Get the outgoing incidence edges of a vertex id and include an edge value in the result.
     * 
     * Complexity: O(n)
     * 
     * Default implementation: 
     *      incidence_view<_G, false, void>(incidence_iterator<_G, false, void>(__g, uid),
     *                                      end(edges(__g, uid)));
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid Vertex id.
     * @return A range of the outgoing incidence edges.
    */
      template <class _G, class EVF>
      requires(_Choice_id_evf<_G&, EVF>._Strategy != _St_id::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid, const EVF& evf) const
            noexcept(_Choice_id_evf<_G&, EVF>._No_throw) {
        constexpr _St_id _Strat_id = _Choice_id_evf<_G&, EVF>._Strategy;

        if constexpr (_Strat_id == _St_id::_Non_member) {
          return incidence(__g, uid); // intentional ADL
        } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
          // default impl
          return incidence_view<_G, false, EVF>(incidence_iterator<_G, false, EVF>(__g, uid, evf),
                                                end(edges(__g, uid)));
        } else {
          static_assert(_Always_false<_G>,
                        "incidence(g,uid,evf) is not defined and the default implementation cannot be evaluated");
        }
      }
    };
  } // namespace _Incidence

  inline namespace _Cpos {
    inline constexpr _Incidence::_Cpo incidence;
  }
} // namespace views
} // namespace graph
