﻿#pragma once
#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"
#include <functional>
//
// vertexlist(g)       -> vertex_info<VId,V,VV> -> {id, vertex& [,value]}
// basic_vertexlist(g) -> vertex_info<VId> -> {id}
//
// given:    vvf = [&g](vertex_reference_t<G> u) -> decl_type(vertex_value(g)) { return vertex_value(g,u);}
//           (trailing return type is required if defined inline as vertexlist parameter)
//
//           vertex_iterator<G> first = ..., last = ...;
//
// examples: for(auto&& [uid, u]      : vertexlist(g))
//         : for(auto&& [uid, u, val] : vertexlist(g,vvf)
//
//         : for(auto&& [uid, u]      : vertexlist(g, vr))
//         : for(auto&& [uid, u, val] : vertexlist(g, vr, vvf)
//
// examples: for(auto&& [uid]      : basic_vertexlist(g))
//         : for(auto&& [uid]      : basic_vertexlist(g, vr))
//
namespace graph {

template <adjacency_list G, class VVF = void>
requires forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator;


template <adjacency_list G, class VVF>
requires forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator {
public:
  using graph_type            = G;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = vertex_iterator_t<graph_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<graph_type>;
  using vertex_value_func     = VVF;
  using vertex_value_type     = invoke_result_t<VVF, vertex_type&>;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_info<void, vertex_type, vertex_value_type>;
  using difference_type   = range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;

protected:
  // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_value_type =
        vertex_info<void, vertex_type, _detail::ref_to_ptr<vertex_value_type>>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value() : shadow_{} {}
    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  vertexlist_iterator(vertex_iterator_type iter, const VVF& value_fn)
        : iter_(iter), value_fn_(&value_fn) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    value_.shadow_.vertex = *iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.shadow_.value = invoke(*this->value_fn_, *iter_);
    return value_.value_;
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    // leave value_.vertex as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }

protected:
  mutable internal_value value_;
  vertex_iterator_type   iter_     = vertex_iterator_type();
  const VVF*             value_fn_ = nullptr;

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G>
requires forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator<G, void> {
public:
  using graph_type            = G;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = vertex_iterator_t<graph_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = vertex_reference_t<graph_type>;
  using vertex_value_func     = void;
  using vertex_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_info<void, vertex_type, void>;
  using difference_type   = range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // public & shadow vertions are identical when vertex_value_type is void
  using shadow_value_type  = vertex_info<void, vertex_type, void>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value() : shadow_{} {}
    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  vertexlist_iterator(graph_type& g) : iter_(begin(vertices(const_cast<graph_type&>(g)))) {}
  vertexlist_iterator(vertex_iterator_type iter) : iter_(iter) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    value_.shadow_.vertex = *iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.shadow_.value = this->value_fn_(*iter_);
    return value_.value_;
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    // leave value_.vertex as-is to avoid dereferencing iter_ to get value_.vertex when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }

protected:
  mutable internal_value value_;
  vertex_iterator_type   iter_ = vertex_iterator_type();

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, class VVF = void>
using vertexlist_view = subrange<vertexlist_iterator<G, VVF>, vertex_iterator_t<G>>;


namespace views {
  // vertexlist(g)       -> vertices[uid,u]
  // vertexlist(g,fn)    -> vertices[uid,u,value]
  // vertexlist(g,vr)    -> vertices[uid,u]
  // vertexlist(g,vr,fn) -> vertices[uid,u,value]

  namespace _Vertexlist {
#if defined(__clang__) || defined(__EDG__) || defined(__GNUC__) // TRANSITION, VSO-1681199
    void vertexlist() = delete;                                 // Block unqualified name lookup
#else                                                           // ^^^ no workaround / workaround vvv
    void vertexlist();
#endif                                                          // ^^^ workaround ^^^

    // all
    template <class _G>
    concept _Has_all_ADL = vertex_range<_G> && requires(_G&& __g) {
      { _Fake_copy_init(vertexlist(__g)) }; // intentional ADL
    };
    template <class _G>
    concept _Can_all_eval = vertex_range<_G>;

    template <class _G, class VVF>
    concept _Has_all_vvf_ADL =
          vertex_range<_G> && invocable<VVF, vertex_reference_t<_G>> && requires(_G&& __g, const VVF& vvf) {
            { _Fake_copy_init(vertexlist(__g, vvf)) }; // intentional ADL
          };

    template <class _G, class VVF>
    concept _Can_all_vvf_eval = vertex_range<_G> && invocable<VVF, vertex_reference_t<_G>>;

    // rng
    template <class _G, class Rng>
    concept _Has_rng_ADL = vertex_range<_G> && forward_range<Rng> && requires(_G&& __g, Rng&& vr) {
      { _Fake_copy_init(vertexlist(__g, vr)) }; // intentional ADL
    };
    template <class _G, class Rng>
    concept _Can_rng_eval = vertex_range<_G> && convertible_to<iterator_t<Rng>, vertex_iterator_t<_G>>;

    template <class _G, class Rng, class VVF>
    concept _Has_rng_vvf_ADL = forward_range<Rng> && invocable<VVF, vertex_reference_t<_G>> &&
                               requires(_G&& __g, vertex_range_t<_G>&& vr, const VVF& vvf) {
                                 { _Fake_copy_init(vertexlist(__g, vr, vvf)) }; // intentional ADL
                               };
    template <class _G, class Rng, class VVF>
    concept _Can_rng_vvf_eval = vertex_range<_G> && convertible_to<iterator_t<Rng>, vertex_iterator_t<_G>> &&
                                invocable<VVF, vertex_reference_t<_G>>;

    class _Cpo {
    private:
      enum class _St_all { _None, _Non_member, _Auto_eval };
      enum class _St_rng { _None, _Non_member, _Auto_eval };

      // all
      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_all> _Choose_all() noexcept {
        static_assert(is_lvalue_reference_v<_G>);

        if constexpr (_Has_all_ADL<_G>) {
          return {_St_all::_Non_member, noexcept(_Fake_copy_init(vertexlist(declval<_G>())))}; // intentional ADL
        } else if constexpr (_Can_all_eval<_G>) {
          using view_type = vertexlist_view<_G, void>;
          return {_St_all::_Auto_eval, noexcept(_Fake_copy_init(view_type(vertices(declval<_G>()))))};
        } else {
          return {_St_all::_None};
        }
      }

      template <class _G>
      static constexpr _Choice_t<_St_all> _Choice_all = _Choose_all<_G>();

      template <class _G, class VVF>
      [[nodiscard]] static consteval _Choice_t<_St_all> _Choose_vvf_all() noexcept {
        static_assert(is_lvalue_reference_v<_G>);

        if constexpr (_Has_all_vvf_ADL<_G, VVF>) {
          return {_St_all::_Non_member,
                  noexcept(_Fake_copy_init(vertexlist(declval<_G>(), declval<VVF>())))}; // intentional ADL
        } else if constexpr (_Can_all_vvf_eval<_G, VVF>) {
          using view_type     = vertexlist_view<_G, VVF>;
          using iterator_type = vertexlist_iterator<_G, VVF>;
          return {_St_all::_Auto_eval,
                  noexcept(_Fake_copy_init(view_type(declval<iterator_type>(), declval<vertex_iterator_t<_G>>())))};
        } else {
          return {_St_all::_None};
        }
      }

      template <class _G, class VVF>
      static constexpr _Choice_t<_St_all> _Choice_vvf_all = _Choose_vvf_all<_G, VVF>();

      // rng
      template <class _G, class Rng>
      [[nodiscard]] static consteval _Choice_t<_St_rng> _Choose_rng() noexcept {
        static_assert(is_lvalue_reference_v<_G>);

        if constexpr (_Has_rng_ADL<_G, Rng>) {
          return {_St_rng::_Non_member,
                  noexcept(_Fake_copy_init(vertexlist(declval<_G>(), declval<Rng>())))}; // intentional ADL
        } else if constexpr (_Can_rng_eval<_G, Rng>) {
          using view_type     = vertexlist_view<_G, void>;
          using iterator_type = vertexlist_iterator<_G>;
          return {_St_rng::_Auto_eval,
                  noexcept(_Fake_copy_init(view_type(declval<iterator_type>(), declval<vertex_iterator_t<_G>>())))};

        } else {
          return {_St_rng::_None};
        }
      }

      template <class _G, class Rng>
      static constexpr _Choice_t<_St_rng> _Choice_rng = _Choose_rng<_G, Rng>();

      template <class _G, class Rng, class VVF>
      [[nodiscard]] static consteval _Choice_t<_St_rng> _Choose_vvf_rng() noexcept {
        static_assert(is_lvalue_reference_v<_G>);

        if constexpr (_Has_rng_vvf_ADL<_G, Rng, VVF>) {
          return {_St_rng::_Non_member, noexcept(_Fake_copy_init(vertexlist(declval<_G>(), declval<Rng>(),
                                                                            declval<VVF>())))}; // intentional ADL
        } else if constexpr (_Can_rng_vvf_eval<_G, Rng, VVF>) {
          using view_type     = vertexlist_view<_G, VVF>;
          using iterator_type = vertexlist_iterator<_G, VVF>;
          return {_St_rng::_Auto_eval,
                  noexcept(_Fake_copy_init(view_type(declval<iterator_type>(), declval<vertex_iterator_t<_G>>())))};
        } else {
          return {_St_rng::_None};
        }
      }

      template <class _G, class Rng, class VVF>
      static constexpr _Choice_t<_St_rng> _Choice_vvf_rng = _Choose_vvf_rng<_G, Rng, VVF>();

    public:
      /**
       * @brief The number of outgoing edges of a vertex.
       * 
       * Complexity: O(1)
       * 
       * Default implementation: size(edges(g, u))
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @param u A vertex instance.
       * @return The number of outgoing edges of vertex u.
      */
      template <class _G>
      requires(_Choice_all<_G>._Strategy != _St_all::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_all<_G>._No_throw) {
        constexpr _St_all _Strat_all = _Choice_all<_G>._Strategy;

        if constexpr (_Strat_all == _St_all::_Non_member) {
          return vertexlist(__g); // intentional ADL
        } else if constexpr (_Strat_all == _St_all::_Auto_eval) {
          return vertexlist_view<_G, void>(vertices(forward<_G>(__g)));
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "vertexlist(g) is not defined and the default implementation cannot be evaluated");
        }
      }

      template <class _G, class VVF>
      requires(_Choice_vvf_all<_G, VVF>._Strategy != _St_all::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const VVF& vvf) const
            noexcept(_Choice_vvf_all<_G, VVF>._No_throw) {
        constexpr _St_all _Strat_all = _Choice_vvf_all<_G, VVF>._Strategy;

        if constexpr (_Strat_all == _St_all::_Non_member) {
          return vertexlist(__g, vvf); // intentional ADL
        } else if constexpr (_Strat_all == _St_all::_Auto_eval) {
          using view_type     = vertexlist_view<_G, VVF>;
          using view_iterator = vertexlist_iterator<_G, VVF>;
#if USE_VERTEX_DESCRIPTOR
          return view_type(view_iterator(begin(vertices(__g)), vvf), end(vertices(__g)));
#else
          auto first          = begin(vertices(forward<_G>(__g)));
          auto last           = end(vertices(forward<_G>(__g)));
          return view_type(view_iterator(forward<_G>(__g), vvf, first), last);
#endif
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "vertexlist(g,vvf) is not defined and the default implementation cannot be evaluated");
        }
      }

      /**
       * @brief A range of vertices for a graph.
       * 
       * Complexity: O(1)
       * 
       * Default implementation: 
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @param vr A range of vertices.
       * @return A range of vertices.
      */
      template <class _G, class Rng>
      requires(_Choice_rng<_G, Rng>._Strategy != _St_rng::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, Rng&& vr) const noexcept(_Choice_rng<_G, Rng>._No_throw) {
        constexpr _St_rng _Strat_rng = _Choice_rng<_G, Rng>._Strategy;

        if constexpr (_Strat_rng == _St_rng::_Non_member) {
          return vertexlist(forward<_G>(__g), forward<Rng>(vr)); // intentional ADL
        } else if constexpr (_Strat_rng == _St_rng::_Auto_eval) {
          using view_type     = vertexlist_view<_G, void>;
          using view_iterator = vertexlist_iterator<_G>;
#if USE_VERTEX_DESCRIPTOR
          return view_type(view_iterator(begin(vr)), end(vr));
#else
          auto first          = begin(vr);
          auto last           = end(vr);
          auto first_id       = static_cast<vertex_id_t<_G>>(first - begin(vertices(__g)));
          return view_type(view_iterator(first, first_id), last);
#endif
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "vertexlist(g,vr) is not defined and the default implementation cannot be evaluated");
        }
      }

      template <class _G, class Rng, class VVF>
      requires(_Choice_vvf_rng<_G, Rng, VVF>._Strategy != _St_rng::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, Rng&& vr, const VVF& vvf) const
            noexcept(_Choice_vvf_rng<_G, Rng, VVF>._No_throw) {
        constexpr _St_rng _Strat_rng = _Choice_vvf_rng<_G, Rng, VVF>._Strategy;

        if constexpr (_Strat_rng == _St_rng::_Non_member) {
          return vertexlist(forward<_G>(__g), forward<Rng>(vr), vvf); // intentional ADL
        } else if constexpr (_Strat_rng == _St_rng::_Auto_eval) {
          using view_type     = vertexlist_view<_G, VVF>;
          using view_iterator = vertexlist_iterator<_G, VVF>;
#if USE_VERTEX_DESCRIPTOR
          return view_type(view_iterator(begin(vr), vvf), end(vertices(vr)));
#else
          auto first          = begin(vr);
          auto last           = end(vr);
          auto first_id       = static_cast<vertex_id_t<_G>>(first - begin(vertices(__g)));
          return view_type(view_iterator(forward<_G>(__g), vvf, first, first_id), last);
#endif
        } else {
          static_assert(_AlwaysFalse<_G>,
                        "vertexlist(g,vr,vvf) is not defined and the default implementation cannot be evaluated");
        }
      }
    };
  } // namespace _Vertexlist

  inline namespace _Cpos {
    inline constexpr _Vertexlist::_Cpo vertexlist;
  }

} // namespace views

} // namespace graph
