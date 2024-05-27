#pragma once

#include "graph.hpp"
#include <ranges>
#include <type_traits>

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

// This file implements the interface for an edgelist (el).
//
// An edgelist is a range of edges where source_id(e) and target_id(e) are property
// functions that can be called on an edge (value type of the range).
//
// An optional edge_value(e) property can also be used if a value is defined for
// the edgelist. Use the has_edge_value<EL> concept to determine if it defined.
//
// The concepts, types and property functions mirror definitions for edges and
// and edge for an adjacency list.
//

// edgelist concepts
// -----------------------------------------------------------------------------
// basic_sourced_edgelist<EL>
// basic_sourced_index_edgelist<EL>
// has_edge_value<EL>
//
// Type aliases
// -----------------------------------------------------------------------------
// edge_range_t<EL>     = EL
// edge_iterator_t<EL>  = range_iterator_t<EL>
// edge_t<EL>           = range_value_t<EL>
// edge_reference_t<EL> = range_value_t<EL>
// edge_value_t<EL>     = decltype(edge_value(e)) (optional)
// vertex_id_t<EL>      = decltype(source_id(e))
//
// edgelist functions
// -----------------------------------------------------------------------------
// num_edges(el)  (todo)
// has_edge(el)  (todo)
// contains_edge(el,uid,vid)  (todo)
//
// edge functions
// -----------------------------------------------------------------------------
// source_id(e)
// target_id(e)
// edge_value(e)
// edge_id(e) (todo)
//
// Edge definitions supported without overrides
// -----------------------------------------------------------------------------
// The standard implementation supports two edge types with support for
// source_id(e) and target_id(e) to return their respective values, and an
// optional edge_value(e) if the edge has a value (shown following). The
// functions can be overridden for user-defined edge types.
//
//  pair<T,T>
//  tuple<T,T>
//  tuple<T,T,EV,...>
//
//  edge_descriptor<VId, true, void, void> : {source_id, target_id}
//  edge_descriptor<VId, true, void, EV>   : {source_id, target_id, EV}
//
//  edge_descriptor<VId, true, E&, void>   : {source_id, target_id, edge}
//  edge_descriptor<VId, true, E&, EV>     : {source_id, target_id, edge, EV}
//
// Naming conventions
// -----------------------------------------------------------------------------
// Type     Variable    Description
// -------- ----------- --------------------------------------------------------
// EL       el          EdgeList
// E        e           Edge on an edgelist
// EV       val         Edge Value
//

// merge implementation into std::graph with single namespace?
// Issues:
//  1.  name conflict with edgelist view? No: basic_sourced_edgelist vs. views::edgelist.
//  2.  template aliases can't be distinguished by concepts
//  3.  vertex_id_t definition for adjlist and edgelist have be done in separate locations

namespace std::graph::edgelist {

namespace _detail {
  //
  // An edge type cannot be a range, which distinguishes it from an adjacency list
  // that is a range-of-ranges.
  //
  template <class _E> // For exposition only
  concept _el_edge = !ranges::range<_E>;

  //
  // Support the use of std containers for edgelist edge definitions
  //
  template <class _E>                      // For exposition only
  concept _el_tuple_edge = _el_edge<_E> && //
                           same_as<tuple_element_t<0, _E>, tuple_element_t<1, _E>>;

  template <class _E>                                  // For exposition only
  concept _el_index_tuple_edge = _el_tuple_edge<_E> && //
                                 integral<tuple_element_t<0, _E>>;

  //
  // Suport the use of edge_descriptor for edgelist edge definitions
  // (Only types and values needed from edge_descriptor are used and there is no
  // explicit use of edge_descriptor. This is deemed more flexible and no
  // functionality is compromised for it.)
  //
  template <class _E>                                   // For exposition only
  concept _el_basic_sourced_edge_desc = _el_edge<_E> && //
                                        same_as<typename _E::source_id_type, decltype(declval<_E>().source_id)> &&
                                        same_as<typename _E::target_id_type, decltype(declval<_E>().target_id)> &&
                                        same_as<typename _E::source_id_type, typename _E::target_id_type>;

  template <class _E> // For exposition only
  concept _el_basic_sourced_index_edge_desc =
        _el_basic_sourced_edge_desc<_E> && integral<typename _E::source_id_type> &&
        integral<typename _E::target_id_type>;
} // namespace _detail

//
// target_id(g,uv) -> vertex_id_t<E>
//
// For E=tuple<VId,VId>,                returns get<0>(e)
// For E=edge_descriptor<VId,true,...>, returns e.target_id
// Caller may override
//
namespace _Target_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void target_id() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void target_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _E>
  concept _Has_edgl_ref_member = requires(_E&& e) {
    { _Fake_copy_init(e.target_id()) };
  };
  template <class _E>
  concept _Has_edgl_ref_ADL = requires(_E&& __e) {
    { _Fake_copy_init(target_id(__e)) }; // intentional ADL
  };

  template <class _E>
  concept _is_tuple_edge = _detail::_el_tuple_edge<_E>;

  template <class _E>
  concept _is_edge_desc = _detail::_el_basic_sourced_edge_desc<_E>;

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Tuple_id, _EDesc_id };

    template <class _E>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_edgl_ref() noexcept {
      //static_assert(is_lvalue_reference_v<_E>);

      if constexpr (_Has_edgl_ref_member<_E>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_E&>().target_id()))};
      } else if constexpr (_Has_edgl_ref_ADL<_E>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(target_id(declval<_E>())))}; // intentional ADL
      } else if constexpr (_is_tuple_edge<_E>) {
        return {_St_ref::_Tuple_id,
                noexcept(_Fake_copy_init(declval<tuple_element_t<0, _E>>()))}; // first element of tuple/pair
      } else if constexpr (_is_edge_desc<_E>) {
        return {_St_ref::_EDesc_id,
                noexcept(_Fake_copy_init(declval<typename _E::target_id_type>()))}; // target_id of edge_descriptor
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _E>
    static constexpr _Choice_t<_St_ref> _Choice_edgl_ref = _Choose_edgl_ref<remove_reference_t<_E>>();

  public:
    /**
     * @brief The target_id of an edgelist edge.
     * 
     * Complexity: O(1)
     * 
     * Default implementation:
     *  get<0>(e)   for tuple<T,T,...> or pair<T,T>
     *  e.source_id for edge_descriptor<VId,true,_,_>
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv An edge instance.
     * @return The target_id on an edge for an ajacency_list
    */
    template <class _E>
    requires(_Choice_edgl_ref<_E>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_E&& __e) const noexcept(_Choice_edgl_ref<_E>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_edgl_ref<_E>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __e.target_id();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return target_id(__e); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Tuple_id) {
        //static_assert(same_as<tuple_element<0, _E>, int>);
        return get<0>(__e); // first element of tuple/pair
      } else if constexpr (_Strat_ref == _St_ref::_EDesc_id) {
        return __e.target_id;
      } else {
        static_assert(_Always_false<_E>, "target_id(e) or e.target_id() is not defined");
      }
    }
  };
} // namespace _Target_id

inline namespace _Cpos {
  inline constexpr _Target_id::_Cpo target_id;
}

//
// source_id(e) -> vertex_id_t<E>
//
// For E=tuple<VId,VId>,                returns get<1>(e)
// For E=edge_descriptor<VId,true,...>, returns e.source_id
// Caller may override
//
namespace _Source_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void source_id() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void source_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _E>
  concept _Has_edgl_ref_member = requires(_E&& __e) {
    { _Fake_copy_init(__e.source_id()) };
  };
  template <class _E>
  concept _Has_edgl_ref_ADL = _Has_class_or_enum_type<_E> //
                              && requires(_E&& __e) {
                                   { _Fake_copy_init(source_id(__e)) }; // intentional ADL
                                 };

  template <class _E>
  concept _is_tuple_edge = _detail::_el_tuple_edge<_E>;

  template <class _E>
  concept _is_edge_desc = _detail::_el_basic_sourced_edge_desc<_E>;

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Tuple_id, _EDesc_id };

    template <class _E>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_edgl_ref() noexcept {
      //static_assert(is_lvalue_reference_v<_E>);
      if constexpr (_Has_edgl_ref_member<_E>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_E>().source_id()))};
      } else if constexpr (_Has_edgl_ref_ADL<_E>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(source_id(declval<_E>())))}; // intentional ADL
      } else if constexpr (_is_tuple_edge<_E>) {
        return {_St_ref::_Tuple_id,
                noexcept(_Fake_copy_init(declval<tuple_element_t<1, _E>>()))}; // first element of tuple/pair
      } else if constexpr (_is_edge_desc<_E>) {
        return {_St_ref::_EDesc_id,
                noexcept(_Fake_copy_init(declval<typename _E::source_id_type>()))}; // source_id of edge_descriptor
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _E>
    static constexpr _Choice_t<_St_ref> _Choice_edgl_ref = _Choose_edgl_ref<remove_reference_t<_E>>();

  public:
    /**
     * @brief The source_id of an edgelist edge
     * 
     * Complexity: O(1)
     * 
     * Default implementation: 
     *  get<1>(e)   for tuple<T,T,...> or pair<T,T>
     *  e.target_id for edge_descriptor<VId,true,_,_>
     * 
     * @tparam E The edgelist value_type.
     * @param e A edgelist edge instance.
     * @return The source_id of the edge.
    */
    template <class _E>
    requires(_Choice_edgl_ref<_E>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_E&& __e) const noexcept(_Choice_edgl_ref<_E>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_edgl_ref<_E>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __e.source_id();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return source_id(__e); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Tuple_id) {
        return get<1>(__e); // first element of tuple/pair
      } else if constexpr (_Strat_ref == _St_ref::_EDesc_id) {
        return __e.source_id;
      } else {
        static_assert(_Always_false<_E>, "source_id(e) or e.source_id() is not defined");
      }
    }
  };
} // namespace _Source_id

inline namespace _Cpos {
  inline constexpr _Source_id::_Cpo source_id;
}


//
// edge_value(e) -> vertex_id_t<E>
//
// For E=tuple<VId,VId,Vid>,             returns get<2>(e)
// For E=edge_descriptor<VId,true,_,EV>, returns e.value
// Caller may override
//
namespace _Edge_value {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void edge_value() = delete;                // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void edge_value();
#  endif                                     // ^^^ workaround ^^^

  template <class _E>
  concept _Has_edgl_ref_member = requires(_E&& __e) {
    { _Fake_copy_init(__e.edge_value()) };
  };
  template <class _E>
  concept _Has_edgl_ref_ADL = _Has_class_or_enum_type<_E> //
                              && requires(_E&& __e) {
                                   { _Fake_copy_init(edge_value(__e)) }; // intentional ADL
                                 };

  template <class _E>
  concept _is_tuple_edge = _detail::_el_tuple_edge<_E> && (tuple_size_v<_E> >= 3);

  template <class _E>
  concept _is_edge_desc = _detail::_el_basic_sourced_edge_desc<_E> && requires(_E e) {
    { e.value }; //->same_as<typename _E::value_type>;
  };

  class _Cpo {
  public:
    enum class _St_ref { _None, _Member, _Non_member, _Tuple_id, _EDesc_id };

    template <class _E>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_edgl_ref() noexcept {
      //static_assert(is_lvalue_reference_v<_E>);
      if constexpr (_Has_edgl_ref_member<_E>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_E>().edge_value()))};
      } else if constexpr (_Has_edgl_ref_ADL<_E>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(edge_value(declval<_E>())))}; // intentional ADL
      } else if constexpr (_is_tuple_edge<_E>) {
        return {_St_ref::_Tuple_id,
                noexcept(_Fake_copy_init(declval<tuple_element_t<2, _E>>()))}; // first element of tuple/pair
      } else if constexpr (_is_edge_desc<_E>) {
        return {_St_ref::_EDesc_id,
                noexcept(_Fake_copy_init(declval<typename _E::value_type>()))}; // edge_value of edge_descriptor
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _E>
    static constexpr _Choice_t<_St_ref> _Choice_edgl_ref = _Choose_edgl_ref<remove_reference_t<_E>>();

  public:
    /**
     * @brief The edge_value of an edgelist edge
     * 
     * Complexity: O(1)
     * 
     * Default implementation: 
     *  get<2>(e)   for tuple<T,T,V,...>
     *  e.value     for edge_descriptor<VId,true,_,V>
     * 
     * @tparam E The edgelist value_type.
     * @param e A edgelist edge instance.
     * @return The edge_value of the edge, when supported.
    */
    template <class _E>
    requires(_Choice_edgl_ref<_E>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_E&& __e) const noexcept(_Choice_edgl_ref<_E>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_edgl_ref<_E>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __e.edge_value();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return edge_value(__e); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Tuple_id) {
        return get<2>(__e); // first element of tuple/pair
      } else if constexpr (_Strat_ref == _St_ref::_EDesc_id) {
        return __e.value;
      } else {
        static_assert(_Always_false<_E>, "edge_value(e) or e.edge_value() is not defined");
      }
    }
  };
} // namespace _Edge_value

inline namespace _Cpos {
  inline constexpr _Edge_value::_Cpo edge_value;
}


//
// edgelist concepts
//
template <class EL>                                                           // For exposition only
concept basic_sourced_edgelist = ranges::forward_range<EL> &&                 //
                                 !ranges::range<ranges::range_value_t<EL>> && // distinguish from adjacency list
                                 requires(ranges::range_value_t<EL> e) {
                                   { source_id(e) };
                                   { target_id(e) } -> same_as<decltype(source_id(e))>;
                                 };

template <class EL>                                                  // For exposition only
concept basic_sourced_index_edgelist = basic_sourced_edgelist<EL> && //
                                       requires(ranges::range_value_t<EL> e) {
                                         { source_id(e) } -> integral;
                                         { target_id(e) } -> integral; // this is redundant, but makes it clear
                                       };

template <class EL>                                    // For exposition only
concept has_edge_value = basic_sourced_edgelist<EL> && //
                         requires(ranges::range_value_t<EL> e) {
                           { edge_value(e) };
                         };

//template<class EL>
//struct has_directed_edge = ...;

// (non-basic concepts imply inclusion of an edge reference which doesn't make much sense)


//
// edgelist types (note that concepts don't really do anything except document expectations)
//
template <basic_sourced_edgelist EL> // For exposition only
using edge_range_t = EL;

template <basic_sourced_edgelist EL> // For exposition only
using edge_iterator_t = ranges::iterator_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_t = ranges::range_value_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_reference_t = ranges::range_reference_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_value_t = decltype(edge_value(declval<edge_t<edge_range_t<EL>>>()));

template <basic_sourced_edgelist EL> // For exposition only
using edge_id_t = decltype(edge_id(declval<edge_t<edge_range_t<EL>>>()));

template <basic_sourced_edgelist EL> // For exposition only
using vertex_id_t = decltype(source_id(declval<edge_t<edge_range_t<EL>>>()));


// template aliases can't be distinguished with concepts :(
//
//template <basic_adjacency_list G> // For exposition only
//using vid_t = decltype(vertex_id(declval<G>()));
//
//template <basic_sourced_edgelist EL> // For exposition only
//using vid_t = decltype(source_id(declval<edge_t<EL>>()));

} // namespace std::graph::edgelist

#endif
