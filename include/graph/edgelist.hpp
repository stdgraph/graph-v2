#pragma once

#include "detail/graph_cpo.hpp"
#include "graph.hpp"
#include <ranges>
#include <type_traits>

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

// function support
// source_id(el)
// target_id(el)
// edge_value(el)
// num_edges
// has_edge
// contains_edge(el,uid,vid)
// edge_id
//
// type support
// vertex_id_t<EL>
// edge_t<EL>
// edge_value_t<EL>
// edge_range_t<EL>
//
// edge_descriptor<VId, true, void, void> : {source_id, target_id}
// edge_descriptor<VId, true, void, EV>   : {source_id, target_id, EV}
//
// is_edge_descriptor_v<E>
//

// Target concepts for edgelist
namespace std::graph::edgelist {
// move implementations for source_id(e), target_id(e), edge_value(e) into std::graph::edgelist?
// other functions to support: num_edges(el), contains_edge(uid,vid), edge_id(e)

// Support the use of std containers for adj list definitions
template <class _E>
concept _el_value = !ranges::forward_range<_E>; // avoid conflict with adjacency list

template <class _E>
concept _el_tuple_edge = _el_value<_E> && //
                         same_as<tuple_element_t<0, _E>, tuple_element_t<1, _E>>;

template <class _E>
concept _el_index_tuple_edge = _el_tuple_edge<_E> && //
                               integral<tuple_element_t<0, _E>>;

template <class _E>
concept _el_basic_sourced_edge_desc =
      same_as<typename _E::source_id_type, typename _E::target_id_type> && requires(_E elv) {
        { elv.source_id };
        { elv.target_id };
      };

template <class _E>
concept _el_basic_sourced_index_edge_desc = _el_basic_sourced_edge_desc<_E> && requires(_E elv) {
  { elv.source_id } -> integral;
  { elv.target_id } -> same_as<decltype(elv.source_id)>;
};

template <class _E>
concept _el_sourced_edge_desc = _el_basic_sourced_edge_desc<_E> && requires(_E elv) {
  { elv.value };
};

template <class _E>
concept _el_sourced_index_edge_desc = _el_basic_sourced_index_edge_desc<_E> && requires(_E elv) {
  { elv.value };
};


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
  concept _is_tuple_edge = _el_tuple_edge<_E>;

  template <class _E>
  concept _is_edge_desc = _el_basic_sourced_edge_desc<_E>;

  //template <class _E>
  //concept _has_value = _el_sourced_edge_desc<_E>;

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
      //static_assert(_Choice_edgl_ref<_E>._Strategy == _St_ref::_Tuple_id);
      //static_assert(same_as<tuple_element<0, _E>, int>);
      //static_assert(_Choice_edgl_ref<_E>._Strategy == _St_ref::_Tuple_id);

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
  concept _Has_edgl_ref_ADL = _Has_class_or_enum_type<_E>               //
                              && requires(_E&& __e) {
                                   { _Fake_copy_init(source_id(__e)) }; // intentional ADL
                                 };

  template <class _E>
  concept _is_tuple_edge = _el_tuple_edge<_E>;

  template <class _E>
  concept _is_edge_desc = _el_basic_sourced_edge_desc<_E>;

  //template <class _E>
  //concept _has_value = _el_sourced_edge_desc<_E>;

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
     * 
     * @tparam E The edgelist value_type.
     * @param e A edgelist edge instance.
     * @return The source_id of the edge.
    */
    template <class _E>
    requires(_Choice_edgl_ref<_E>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_E&& __e) const noexcept(_Choice_edgl_ref<_E>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_edgl_ref<_E>._Strategy;
      //static_assert(_Choice_edgl_ref<_E>._Strategy == _St_ref::_Tuple_id);
      //static_assert(same_as<tuple_element<1, _E>, int>);
      //static_assert(_Choice_edgl_ref<_E>._Strategy == _St_ref::_Tuple_id);

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __e.source_id();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return source_id(__e); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Tuple_id) {
        return get<1>(__e);    // first element of tuple/pair
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
// edgelist ranges
//
template <class E> // For exposition only
concept _source_target_id = requires(E e) {
  { source_id(e) };
  { target_id(e) } -> same_as<decltype(source_id(e))>;
};
template <class E> // For exposition only
concept _index_source_target_id = requires(E e) {
  { source_id(e) } -> integral;
  { target_id(e) } -> same_as<decltype(source_id(e))>;
};
template <class E> // For exposition only
concept has_edge_value = requires(E e) {
  { edge_value(e) };
};

template <class EL> // For exposition only
concept basic_sourced_edgelist = ranges::forward_range<EL> && _source_target_id<ranges::range_value_t<EL>>;

template <class EL> // For exposition only
concept basic_sourced_index_edgelist = ranges::forward_range<EL> && _index_source_target_id<ranges::range_value_t<EL>>;

// non-basic concepts imply edge reference which doesn't make much sense


//
// edgelist types (note that concepts don't really do anything except document expectations)
//
template <basic_sourced_edgelist EL> // For exposition only
using edge_iterator_t = ranges::iterator_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_t = ranges::range_value_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using edge_reference_t = ranges::range_reference_t<EL>;

template <basic_sourced_edgelist EL> // For exposition only
using vertex_id_t = decltype(source_id(declval<edge_t<EL>>()));

template <basic_sourced_edgelist EL> // For exposition only
using edge_value_t = decltype(edge_value(declval<edge_t<EL>>()));

template <basic_sourced_edgelist EL> // For exposition only
using edge_id_t = decltype(edge_id(declval<edge_t<EL>>()));

// template aliases can't be distinguished with concepts :(
//
//template <basic_adjacency_list G> // For exposition only
//using vid_t = decltype(vertex_id(declval<G>()));
//
//template <basic_sourced_edgelist EL> // For exposition only
//using vid_t = decltype(source_id(declval<edge_t<EL>>()));

} // namespace std::graph::edgelist

#endif
