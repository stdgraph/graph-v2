#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

#  ifndef _MSC_VER
// Taken from gcc11 definition of __decay_copy().
// Very similar (identical?) to std::_Fake_copy_init<T> in msvc.
struct _Decay_copy final {
  template <typename _Tp>
  constexpr decay_t<_Tp> operator()(_Tp&& __t) const noexcept(is_nothrow_convertible_v<_Tp, decay_t<_Tp>>) {
    return std::forward<_Tp>(__t);
  }
} inline constexpr _Fake_copy_init{};

template <class _Ty>
concept _Has_class_or_enum_type =
      __is_class(remove_reference_t<_Ty>) || __is_enum(remove_reference_t<_Ty>) || __is_union(remove_reference_t<_Ty>);

template <class>
// false value attached to a dependent name (for static_assert)
inline constexpr bool _Always_false = false;

template <class _Ty>
inline constexpr bool _Is_nonbool_integral = is_integral_v<_Ty> && !is_same_v<remove_cv_t<_Ty>, bool>;

template <class _Ty>
inline constexpr bool _Integer_class = requires {
  typename _Ty::_Signed_type;
  typename _Ty::_Unsigned_type;
};

template <class _Ty>
concept _Integer_like = _Is_nonbool_integral<remove_cv_t<_Ty>> || _Integer_class<_Ty>;

template <class _Ty>
concept _Signed_integer_like = _Integer_like<_Ty> && static_cast<_Ty>(-1) < static_cast<_Ty>(0);

template <class _Ty>
struct _Choice_t {
  _Ty  _Strategy = _Ty{};
  bool _No_throw = false;
};
#  endif

// Tags are defined in tag_invoke namespace to avoid conflicts with function names
// in std::graph, allowing customization for default behavior.
//
// graphs must use tags like std::graph::tag_invoke::vertex_id_fn_t when defining
// CPO specialization.
//
// Minimal requirements for a graph with random_access vertices(g)
//      vertices(g), edges(g,u), target_id(g,u)
// To have vertex_id_t<G> be something other than size_t
//      vertex_id(g,ui)
// Properties, as supported by the graph:
//      edge_value(g,uv), vertex_value(g,uv), graph_value(g)
//

// Additional functions to consider for future
//  reserve_vertices(g,n) - noop if n/a
//  reserve_edges(g,n)    - noop if n/a
//
//  load_graph(g,erng,vrng,eproj,vproj)
//
// Graph reference of graph type G.*</ summary>*<typeparam name = "G"> Graph</ typeparam>


template <class G>
using graph_reference_t = add_lvalue_reference<G>;


/** 
 * @brief Tag a graph type as an adjacency matrix.
 * 
 * Specialize for a graph type where edges are defined densely in a matrix to allow for
 * optimized algorithms can take advantage of the memory layout.
 *
 * Example:
 * @code
 *  namespace my_namespace {
 *      template<class X>
 *      class my_graph { ... };
 *  }
 *  namespace std::graph {
 *     template<>
 *     struct is_adjacency_matrix<my_namespace::my_graph<X>> : true_type;
 *  }
 * @endcode
 * 
 * @tparam G The graph type
 */
template <class G>
struct define_adjacency_matrix : public false_type {}; // specialized for graph container

template <class G>
struct is_adjacency_matrix : public define_adjacency_matrix<G> {};

template <class G>
inline constexpr bool is_adjacency_matrix_v = is_adjacency_matrix<G>::value;

template <class G>
concept adjacency_matrix = is_adjacency_matrix_v<G>;

//
// vertices(g) -> vertex_range_t<G>
//
// vertex_range_t<G>     = decltype(vertices(g))
// vertex_iterator_t<G>  = ranges::iterator_t<vertex_range_t<G>>
// vertex_t<G>           = ranges::range_value_t<vertex_range_t<G>>
// vertex_reference_t<G> = ranges::range_reference_t<vertex_range_t<G>>
//
namespace tag_invoke {
  TAG_INVOKE_DEF(vertices); // vertices(g) -> [graph vertices]
}

/**
 * @brief Returns the vertices range for a graph G.
 * 
 * Default implementation: n/a.
 * 
 * Complexity: O(1)
 * 
 * This is a customization point function that is required to be overridden for each
 * graph type.
 * 
 * @tparam G The graph type
 * @param g A graph instance
*/
template <class G>
auto vertices(G&& g) -> decltype(tag_invoke::vertices(g)) {
  return tag_invoke::vertices(g);
}

/**
 * @brief The vertex range type for a graph G.
 * @tparam G The graph type.
 */
template <class G>
using vertex_range_t = decltype(std::graph::vertices(declval<G&&>()));

/**
 * @brief The vertex iterator type for a graph G.
 * @tparam G The graph type.
 */
template <class G>
using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G&&>>;

/**
 * @brief The vertex type for a graph G.
 * @tparam G The graph type.
 */
template <class G>
using vertex_t = ranges::range_value_t<vertex_range_t<G>>;

/**
 * @brief The vertex reference type for a graph G.
 * @tparam G The graph type.
*/
template <class G>
using vertex_reference_t = ranges::range_reference_t<vertex_range_t<G>>;


//
// vertex_id(g,ui) -> vertex_id_t<G>
//      default = ui - begin(vertices(g)), if random_access_iterator<ui>
//
namespace tag_invoke {
  TAG_INVOKE_DEF(vertex_id);

  template <class G>
  concept _has_vertex_id_adl = requires(G&& g, vertex_iterator_t<G> ui) {
    { vertex_id(g, ui) };
  };
} // namespace tag_invoke

/**
 * @brief Get's the id of a vertex.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: ui - begin(g)
 * 
 * This is a customization point function that may be overriden for a graph type.
 * The main reason to do so is to change the return type to be something different
 * than range_difference_t<vertex_range_t<G>>. For 64-bit systems, that's typically
 * int64_t. The return type is used to define the type vertex_id_t<G> which is used
 * for vertex id in other functions.
 * 
 * Why does this function take a vertex iterator instead of a vertex reference?
 * The vertex id is often calculated rather than stored. Given an iterator, the id is easily
 * calculated by id = (ui - begin(vertices(g))). If a vertex reference v is passed instead
 * it is also easily calculated for vertices stored in contiguous memory like std::vector.
 * However, if it's a random access container like a deque, then the reference won't work
 * and an iterator is the only option.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param ui A vertex iterator for a vertext in graph G.
 * @return The vertex id of a vertex.
*/
template <class G>
requires tag_invoke::_has_vertex_id_adl<G> || random_access_iterator<vertex_iterator_t<G>>
auto vertex_id(G&& g, vertex_iterator_t<G> ui) {
  if constexpr (tag_invoke::_has_vertex_id_adl<G>)
    return tag_invoke::vertex_id(g, ui);
  else if constexpr (random_access_iterator<vertex_iterator_t<G>>)
    return ui - ranges::begin(vertices(g));
}

/**
 * @brief Defines the type of the vertex id.
 * 
 * Complexity: O(1)
 * 
 * The vertex id type for graph type G.
 * 
 * @tparam G The graph type.
*/
template <class G>
using vertex_id_t = decltype(vertex_id(declval<G&&>(), declval<vertex_iterator_t<G>>()));


//
// find_vertex(g,uid) -> vertex_iterator_t<G>
//
// default = begin(vertices(g)) + uid, if random_access_range<vertex_range_t<G>>
//
namespace _Find_vertex {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void find_vertex() = delete;               // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void find_vertex();
#  endif                                     // ^^^ workaround ^^^

  template <class _G, class _UnCV>
  concept _Has_member = requires(_G&& __g, const vertex_id_t<_G>& uid) {
    { _Fake_copy_init(__g.find_vertex(uid)) };
  };

  template <class _G, class _UnCV>
  concept _Has_ADL = _Has_class_or_enum_type<_G> //
                     && requires(_G&& __g, const vertex_id_t<_G>& uid) {
                          { _Fake_copy_init(find_vertex(__g, uid)) }; // intentional ADL
                        };

  class _Cpo {
  private:
    enum class _St { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St> _Choose() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_member<_G, _UnCV>) {
        return {_St::_Member, noexcept(_Fake_copy_init(declval<_G>().find_vertex(declval<vertex_id_t<_G>>())))};
      } else if constexpr (_Has_ADL<_G, _UnCV>) {
        return {_St::_Non_member,
                noexcept(_Fake_copy_init(find_vertex(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else {
        return {_St::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St> _Choice = _Choose<_G>();

  public:
    /**
     * @brief Find a vertex given a vertex id.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: begin(vertices(g)) + uid, if random_access_range<vertex_range_t<G>>
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid Vertex id.
     * @return An iterator to the vertex if the vertex exists, or end(vertices(g)) if it doesn't exist.
    */
    template <class _G>
    [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
          noexcept(_Choice<_G&>._No_throw) {
      constexpr _St _Strat = _Choice<_G&>._Strategy;

      if constexpr (_Strat == _St::_Member) {
        return __g.find_vertex(uid);
      } else if constexpr (_Strat == _St::_Non_member) {
        return find_vertex(__g, uid); // intentional ADL
      } else if constexpr (random_access_iterator<vertex_iterator_t<_G>>) {
        auto uid_diff = static_cast<ranges::range_difference_t<vertex_range_t<_G>>>(uid);
        if (uid_diff < ssize(vertices(__g)))
          return begin(vertices(__g)) + uid_diff;
        else
          return end(vertices(__g));
      } else {
        static_assert(_Always_false<_G>,
                      "find_vertex(g,uid) has not been defined and the default implemenation cannot be evaluated");
      }
    }
  };
} // namespace _Find_vertex

inline namespace _Cpos {
  inline constexpr _Find_vertex::_Cpo find_vertex;
}

// partition_id(g,uid) -> ?   default = vertex_id_t<G>() if not overridden; must be overridden for bipartite or multipartite graphs
// partition_id(g,u)          default = partition_id(g,vertex_id(u))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(partition_id);

  template <class G>
  concept _has_partition_id_uid_adl = requires(G&& g, vertex_id_t<G> uid) {
    { partition_id(g, uid) };
  };

  template <class G>
  concept _has_partition_id_uref_adl = requires(G&& g, vertex_reference_t<G> u) {
    { partition_id(g, u) };
  };
} // namespace tag_invoke

/**
 * @brief Get's the parition_id of a vertex.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: partition_id(g,vertex_id(g,u))
 * 
 * This is a customization point function that may be overriden if graph G supports bi-partite
 * or multi-partite graphs. If it doesn't then a value of 0 is returned.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid A vertex id for a vertex in graph G.
 * @return The partition id of a vertex. 0 if G doesn't support partitioning.
*/
template <class G>
//requires tag_invoke::_has_partition_id_uref_adl<G>
auto partition_id(G&& g, vertex_reference_t<G> u) {
  if constexpr (tag_invoke::_has_partition_id_uref_adl<G>)
    return tag_invoke::partition_id(g, u);
  else if constexpr (is_integral_v<vertex_id_t<G>>) {
    return vertex_id_t<G>();
  } else
    return size_t(0);
}

/**
 * @brief Get's the parition_id of a vertex_id.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: 0; graph container must define when supported
 * 
 * This is a customization point function that may be overriden if graph G supports bi-partite
 * or multi-partite graphs. If it doesn't then a value of 0 is returned.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid A vertex id for a vertex in graph G.
 * @return The partition id of a vertex. 0 if G doesn't support partitioning.
*/
template <class G>
//requires tag_invoke::_has_partition_id_uid_adl<G>
auto partition_id(G&& g, vertex_id_t<G> uid) {
  if constexpr (tag_invoke::_has_partition_id_uid_adl<G>)
    return tag_invoke::partition_id(g, uid);
  else
    return partition_id(g, *find_vertex(g, uid));
}


template <class G>
using partition_id_t = decltype(partition_id(declval<G>(), declval<vertex_reference_t<G>>()));


//
// edges(g,u)  -> vertex_edge_range_t<G>
// edges(g,uid) -> vertex_edge_range_t<G>
//      default = edges(g,*find_vertex(g,uid))
//
// vertex_edge_range_t<G>    = edges(g,u)
// vertex_edge_iterator_t<G> = ranges::iterator_t<vertex_edge_range_t<G>>
// edge_t                    = ranges::range_value_t<vertex_edge_range_t<G>>
// edge_reference_t          = ranges::range_reference_t<vertex_edge_range_t<G>>
//
namespace tag_invoke {
  TAG_INVOKE_DEF(edges);

  template <class G>
  concept _has_edges_vtxref_adl = requires(G&& g, vertex_reference_t<G> u) {
    { edges(g, u) };
  };

  template <class G>
  concept _has_edges_vtxid_adl = requires(G&& g, vertex_id_t<G> uid) {
    { edges(g, uid) };
  };
} // namespace tag_invoke

/**
 * @brief Get the outgoing edges of a vertex.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for each graph type.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param u Vertex reference.
 * @return A range of the outgoing edges.
*/
template <class G>
requires tag_invoke::_has_edges_vtxref_adl<G>
auto edges(G&& g, vertex_reference_t<G> u) -> decltype(tag_invoke::edges(g, u)) {
  return tag_invoke::edges(g, u); // graph author must define
}

/**
 * @brief Get the outgoing edges of a vertex id.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: edges(g, *find_vertex(g, uid))
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid Vertex id.
 * @return A range of the outgoing edges.
*/
template <class G>
auto edges(G&& g, vertex_id_t<G> uid) -> decltype(tag_invoke::edges(g, uid)) {
  if constexpr (tag_invoke::_has_edges_vtxid_adl<G>)
    return tag_invoke::edges(g, uid);
  else
    return edges(g, *find_vertex(g, uid));
}

/**
 * @brief The outgoing edge range type of a vertex for graph G.
 * @tparam G The graph type.
*/
template <class G>
using vertex_edge_range_t = decltype(edges(declval<G&&>(), declval<vertex_reference_t<G>>()));

/**
 * @brief The outgoing edge iterator type of a vertex for graph G.
 * @tparam G The graph type.
*/
template <class G>
using vertex_edge_iterator_t = ranges::iterator_t<vertex_edge_range_t<G>>;

/**
 * @brief The edge type for graph G.
 * @tparam G The graph type.
*/
template <class G>
using edge_t = ranges::range_value_t<vertex_edge_range_t<G>>;

/**
 * @brief The edge reference type for graph G.
 * @tparam G The graph type.
*/
template <class G>
using edge_reference_t = ranges::range_reference_t<vertex_edge_range_t<G>>;


//
// target_id(g,uv) -> vertex_id_t<G>
//
// Graph data structure must define
//
namespace _Target_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void target_id() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void target_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G, class _UnCV>
  concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(uv.target_id(__g)) };
  };
  template <class _G, class _UnCV>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                              { _Fake_copy_init(target_id(__g, uv)) }; // intentional ADL
                            };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_ref_member<_G, _UnCV>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().target_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G, _UnCV>) {
        return {
              _St_ref::_Non_member,
              noexcept(_Fake_copy_init(target_id(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief The number of outgoing edges of a vertex.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: size(edges(g, uv))
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv A vertex instance.
     * @return The number of outgoing edges of vertex uv.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return uv.target_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return target_id(__g, uv); // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "target_id(g,uv) or g.target_id(uv) is not defined");
      }
    }
  };
} // namespace _Target_id

inline namespace _Cpos {
  inline constexpr _Target_id::_Cpo target_id;
}

//
// target(g,uv) -> vertex_reference_t<G>
//      default = *(begin(g,vertices(g)) + target_id(g,uv))
//
//      for random_access_range<vertices(g)> and integral<target_id(g,uv))
//      uv can be from edges(g,u) or vertices(g,u)
//
namespace tag_invoke {
  TAG_INVOKE_DEF(target);

  template <class G, class ER>
  concept _has_target_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    { target(g, uv) };
  };
} // namespace tag_invoke
template <class G, class ER>
concept _can_eval_target =
      ranges::random_access_range<vertex_range_t<G>> && requires(G&& g, ranges::range_reference_t<ER> uv) {
        { target_id(g, uv) } -> integral;
      };

/**
 * @brief Get the target vertex of an edge.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: *(begin(vertices(g)) + target_id(g, uv))
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv An edge reference.
 * @return The target vertex reference.
*/
template <class G>
requires tag_invoke::_has_target_adl<G, vertex_edge_range_t<G>> || _can_eval_target<G, vertex_edge_range_t<G>>
auto&& target(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_target_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::target(g, uv);
  else if constexpr (_can_eval_target<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + target_id(g, uv));
}

//
//
// source_id(g,uv) -> vertex_id_t<G> (optional; only when a source_id exists on an edge)
//
namespace tag_invoke {
  TAG_INVOKE_DEF(source_id);
}

/**
 * @brief Get the source vertex id of an edge.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for each graph type, if available.
 * 
 * Not all graphs support a source id on an edge. The existance of @c source_id(g,uv) function 
 * for a graph type G determines if it is considered a "sourced" edge or not.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv An edge reference.
 * @return The source vertex id.
*/
template <class G>
auto source_id(G&& g, edge_reference_t<G> uv) -> decltype(tag_invoke::source_id(g, uv)) {
  return tag_invoke::source_id(g, uv);
}

//
// source(g,uv) -> vertex_reference_t<G>
//      default = *(begin(g,vertices(g)) + source_id(g,uv))
//
//      for random_access_range<vertices(g)> and integral<source_id(g,uv))
//      uv can be from edges(g,u) or vertices(g,u)
//
namespace tag_invoke {
  TAG_INVOKE_DEF(source);

  template <class G, class ER>
  concept _has_source_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    { source(g, uv) };
  };
} // namespace tag_invoke

template <class G, class ER>
concept _can_eval_source_id = ranges::random_access_range<ER> && requires(G&& g, ranges::range_reference_t<ER> uv) {
  { source_id(g, uv) } -> integral;
};

/**
 * @brief Get the source vertex of an edge.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: *(begin(vertices(g)) + source_id(g, uv)), if @source_id(g,uv) is defined for G
 * 
 * Not all graphs support a source on an edge. The existance of @c source_id(g,uv) function 
 * for a graph type G determines if it is considered a "sourced" edge or not. If it is, 
 * @c source(g,uv) will also exist.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv An edge reference.
 * @return The source vertex reference.
*/
template <class G>
requires tag_invoke::_has_source_adl<G, vertex_edge_range_t<G>> || _can_eval_source_id<G, vertex_edge_range_t<G>>
auto&& source(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_source_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::source(g, uv);
  else if constexpr (_can_eval_source_id<G, vertex_edge_range_t<G>>)
    return *(begin(vertices(g)) + source_id(g, uv));
}

//
// edge_id(g,uv) -> pair<vertex_id_t<G>,vertex_id_t<G>>
//      default = pair(source_id(g,uv),target_id(g,uv))
//
// edge_id_t<G> = decltype(edge_id(g,uv))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(edge_id);

  template <class G, class ER>
  concept _has_edge_id_adl = requires(G&& g, ranges::range_reference_t<ER> uv) {
    { edge_id(g, uv) };
  };
} // namespace tag_invoke

template <class G, class ER>
concept _can_eval_edge_id = requires(G&& g, ranges::range_reference_t<ER> uv) {
  { target_id(g, uv) };
  { source_id(g, uv) };
};

/**
 * @brief Get the edge id of an edge.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: pair(source_id(g, uv), target_id(g, uv))
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv An edge reference.
 * @return The edge id as a pair of vertex id's.
*/
template <class G>
requires tag_invoke::_has_edge_id_adl<G, vertex_edge_range_t<G>> || _can_eval_edge_id<G, vertex_edge_range_t<G>>
auto edge_id(G&& g, edge_reference_t<G> uv) {
  if constexpr (tag_invoke::_has_edge_id_adl<G, vertex_edge_range_t<G>>)
    return tag_invoke::edge_id(g, uv);
  else if constexpr (_can_eval_edge_id<G, vertex_edge_range_t<G>>)
    return pair(source_id(g, uv), target_id(g, uv));
}

template <class G>
using edge_id_t = decltype(edge_id(declval<G&&>(),
                                   declval<edge_reference_t<G>>())); // e.g. pair<vertex_id_t<G>,vertex_id_t<G>>


//
// find_vertex_edge(g,u,vid) -> vertex_edge_iterator<G>
//      default = find(edges(g,u), [](uv) {target_id(g,uv)==vid;}
//
// find_vertex_edge(g,uid,vid) -> vertex_edge_iterator<G>
//      default = find_vertex_edge(g,*find_vertex(g,uid),vid)
//
namespace tag_invoke {
  TAG_INVOKE_DEF(find_vertex_edge);

  template <class G>
  concept _has_find_vertex_edge_adl = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
    { find_vertex_edge(g, u, vid) };
  };
  template <class G>
  concept _has_find_vertex_id_edge_adl =
        requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
          { find_vertex_edge(g, uid, vid) };
        };
} // namespace tag_invoke

/**
 * @brief Find an edge of a vertex.
 * 
 * Complexity: O(E), where |E| is the number of outgoing edges of vertex u
 * 
 * Default implementation: find_if(edges(g, u), [&g, &vid](auto&& uv) { return target_id(g, uv) == vid; })
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param u A vertex instance.
 * @param vid A target vertex id.
 * @return An edge iterator of an outgoing edge of u with a target id of vid. end(edges(g,u)) will
 *         be returned if an edge doesn't exist.
*/
template <class G>
auto find_vertex_edge(G&& g, vertex_reference_t<G> u, vertex_id_t<G> vid) {
  if constexpr (tag_invoke::_has_find_vertex_edge_adl<G>)
    return tag_invoke::find_vertex_edge(g, u, vid);
  else
    return ranges::find_if(edges(g, u), [&g, &vid](auto&& uv) { return target_id(g, uv) == vid; });
}

/**
 * @brief Find an edge in the graph.
 * 
 * Complexity: O(E), where |E| is the number of outgoing edges of vertex u
 * 
 * Default implementation: find_vertex_edge(g, *(begin(vertices(g)) + uid), vid)
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid The vertex source id of the edge.
 * @param vid The vertex target id of the edge.
 * @return An edge iterator of an outgoing edge of u with a target id of vid. end(edges(g,uid)) will
 *         be returned if an edge doesn't exist.
*/
template <class G>
requires tag_invoke::_has_find_vertex_id_edge_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex_edge(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  if constexpr (tag_invoke::_has_find_vertex_id_edge_adl<G>)
    return tag_invoke::find_vertex_edge(g, uid, vid);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    find_vertex_edge(g, *(begin(vertices(g)) + uid), vid);
}

//
// contains_edge(g,uid,vid) -> bool
//      default = uid < size(vertices(g)) && vid < size(vertices(g)), if adjacency_matrix<G>
//              = find_vertex_edge(g,uid) != ranges::end(edges(g,uid));
//
namespace tag_invoke {
  TAG_INVOKE_DEF(contains_edge);

  template <class G>
  concept _has_contains_edge_adl = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
    { contains_edge(g, uid, vid) };
  };
} // namespace tag_invoke

/**
 * @brief Does an edge exist in the graph?
 * 
 * Complexity: O(E), where |E| is the number of outgoing edges of vertex u
 * 
 * Default implementation: find_vertex_edge(g, *ui) != end(edges(g, *ui));
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid The vertex source id of the edge.
 * @param vid The vertex target id of the edge.
 * @return true if the edge exists, or false otherwise.
*/
template <class G>
auto contains_edge(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  if constexpr (tag_invoke::_has_contains_edge_adl<G>)
    return tag_invoke::contains_edge(g, uid, vid);
  else if constexpr (is_adjacency_matrix_v<G>) {
    return uid < ranges::size(vertices(g)) && vid < ranges::size(vertices(g));
  } else {
    auto ui = find_vertex(g, uid);
    return find_vertex_edge(g, *ui) != ranges::end(edges(g, *ui));
  }
}


#  if 1
//
// num_vertices(g,)    -> integral        default = size(vertices(g))
// num_vertices(g,pid) -> integral        default = size(vertices(g,pid))
//
namespace _NumVertices {
#    if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void num_vertices() = delete;                // Block unqualified name lookup
#    else                                      // ^^^ no workaround / workaround vvv
  void num_vertices();
#    endif                                     // ^^^ workaround ^^^

  template <class _G, class _UnCV>
  concept _Has_ref_member = requires(_G&& __g) {
    { _Fake_copy_init(__g.num_vertices(__g)) };
  };
  template <class _G, class _UnCV>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(num_vertices(__g)) }; // intentional ADL
                            };
  template <class _G, class _UnCV>
  concept _Can_ref_eval = ranges::sized_range<vertex_range_t<_G>> //
                          && requires(_G&& __g) {
                               { _Fake_copy_init(ranges::size(vertices(__g))) };
                             };

  template <class _G, class _UnCV>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, partition_id_t<_G> pid) {
                             { _Fake_copy_init(num_vertices(__g, pid)) }; // intentional ADL
                           };
  template <class _G, class _UnCV>
  concept _Can_id_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
                         && requires(_G&& __g, partition_id_t<_G> pid) {
                              { _Fake_copy_init(ranges::size(vertices(__g, pid))) };
                            };

  class _Cpo {
  private:
    enum class _St_id { _None, _Non_member, _Auto_eval };
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_ref_member<_G, _UnCV>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().num_vertices()))};
      } else if constexpr (_Has_ref_ADL<_G, _UnCV>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(num_vertices(declval<_G>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G, _UnCV>) {
        return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(ranges::size(vertices(declval<_G>()))))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_id_ADL<_G, _UnCV>) {
        return {
              _St_id::_Non_member,
              noexcept(_Fake_copy_init(num_vertices(declval<_G>(), declval<partition_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G, _UnCV>) {
        return {_St_id::_Auto_eval, noexcept(_Fake_copy_init(ranges::size(
                                          vertices(declval<_G>(), declval<partition_id_t<_G>>()))))}; // default impl
      } else {
        return {_St_id::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

  public:
    /**
     * @brief Get number of vertices in a partition of a graph.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: size(vertices(g,pid))
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param pid Vertex id.
     * @return The number of vertices in partition pid of graph g.
    */
    template <class _G>
    //requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, const partition_id_t<_G>& pid) const
          noexcept(_Choice_id<_G&>._No_throw) {
      constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;
      static_assert(_Strat_id == _St_id::_Auto_eval);

      if constexpr (_Strat_id == _St_id::_Non_member) {
        return num_vertices(__g, pid); // intentional ADL
      } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
        return ranges::size(vertices(__g, pid)); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "num_vertices(g,pid) is not defined and the default implementation cannot be evaluated");
      }
    }

    /**
     * @brief The number of vertices in a graph.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: size(vertices(g))
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @return The number of vertices in g.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_id = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_id == _St_ref::_Member) {
        return __g.num_vertices();
      } else if constexpr (_Strat_id == _St_ref::_Non_member) {
        return num_vertices(__g); // intentional ADL
      } else if constexpr (_Strat_id == _St_ref::_Auto_eval) {
        return ranges::size(vertices(__g)); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "num_vertices(g) is not defined and the default implementation cannot be evaluated");
      }
    }
  };
} // namespace _NumVertices

inline namespace _Cpos {
  inline constexpr _NumVertices::_Cpo num_vertices;
}
#  else
//
// num_vertices(g) -> integral
//      default = size(vertices(g))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(num_vertices);

  template <class G>
  concept _has_num_vertices_adl = requires(G&& g, vertex_reference_t<G> u) {
    { num_vertices(g) };
  };
} // namespace tag_invoke

/**
 * @brief The number of outgoing edges of a vertex.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: size(vertices(g))
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @return The number of vertices in a graph.
*/
template <class G>
requires tag_invoke::_has_num_vertices_adl<G>
auto num_vertices(G&& g) {
  if constexpr (tag_invoke::_has_num_vertices_adl<G>)
    return tag_invoke::num_vertices(g);
  else {
    return ranges::size(vertices(g));
  }
}
#  endif

//
// degree(g,u  ) -> integral        default = size(edges(g,u))   if sized_range<vertex_edge_range_t<G>>
// degree(g,uid) -> integral        default = size(edges(g,uid)) if sized_range<vertex_edge_range_t<G>>
//
namespace _Degree {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void degree() = delete;                    // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void degree();
#  endif                                     // ^^^ workaround ^^^

  template <class _G, class _UnCV>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(u.degree(__g)) };
  };
  template <class _G, class _UnCV>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_reference_t<_G>& u) {
                              { _Fake_copy_init(degree(__g, u)) }; // intentional ADL
                            };
  template <class _G, class _UnCV>
  concept _Can_ref_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
                          && requires(_G&& __g, vertex_reference_t<_G> u) {
                               { _Fake_copy_init(edges(__g, u)) };
                             };

  template <class _G, class _UnCV>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, const vertex_id_t<_G>& uid) {
                             { _Fake_copy_init(degree(__g, uid)) }; // intentional ADL
                           };
  template <class _G, class _UnCV>
  concept _Can_id_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
                         && requires(_G&& __g, vertex_id_t<_G> uid) {
                              { _Fake_copy_init(edges(__g, uid)) };
                            };

  class _Cpo {
  private:
    enum class _St_id { _None, _Non_member, _Auto_eval };
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_id_ADL<_G, _UnCV>) {
        return {_St_id::_Non_member,
                noexcept(_Fake_copy_init(degree(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G, _UnCV>) {
        return {_St_id::_Auto_eval, noexcept(_Fake_copy_init(ranges::size(
                                          edges(declval<_G>(), declval<vertex_id_t<_G>>()))))}; // default impl
      } else {
        return {_St_id::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_ref_member<_G, _UnCV>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().degree(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G, _UnCV>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(degree(declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G, _UnCV>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(ranges::size(edges(declval<_G>(), declval<vertex_reference_t<_G>>()))))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

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
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_reference_t<_G> u) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return u.degree(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return degree(__g, u); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return ranges::size(edges(__g, u)); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "degree(g,u) is not defined and the default implementation cannot be evaluated");
      }
    }

    /**
     * @brief Get the outgoing degree of a vertex id.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: degree(g, *find_vertex(g, uid))
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid Vertex id.
     * @return A range of the outgoing degree.
    */
    template <class _G>
    requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
          noexcept(_Choice_id<_G&>._No_throw) {
      constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;

      if constexpr (_Strat_id == _St_id::_Non_member) {
        return degree(__g, uid); // intentional ADL
      } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
        return ranges::size(edges(__g, uid)); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "degree(g,uid) is not defined and the default implementation cannot be evaluated");
      }
    }
  };
} // namespace _Degree

inline namespace _Cpos {
  inline constexpr _Degree::_Cpo degree;
}


//
// vertex_value(g,u) -> <<user-defined type>>
//
// vertex_value_t<G> = decltype(vertex_value(g,u))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(vertex_value); // vertex_value(g,u) -> ?
}

/**
 * @brief The user-defined value for a vertex, if it exists.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for the vertex type of each graph type, if available.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param u A vertex instance.
 * @return The value associated with vertex u.
*/
template <class G>
auto vertex_value(G&& g, vertex_reference_t<G> u) -> decltype(tag_invoke::vertex_value(g, u)) {
  return tag_invoke::vertex_value(g, u);
}
template <class G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

//
// edge_value(g,uv) -> <<user-defined type>>
//
// edge_value_t<G> = decltype(edge_value(g,uv))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(edge_value);
}

/**
 * @brief The user-defined value for a edge, if it exists.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for the edge type of each graph type, if available.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv A edge instance.
 * @return The value associated with edge uv.
*/
template <class G>
auto edge_value(G&& g, edge_reference_t<G> uv) -> decltype(tag_invoke::edge_value(g, uv)) {
  return tag_invoke::edge_value(g, uv);
}

// edge value types
template <class G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));

//
// graph_value(g) -> <<user-defined type>>
//
// graph_value_t<G> = decltype(graph_value(g))
//
namespace tag_invoke {
  TAG_INVOKE_DEF(graph_value); // graph_value(g) -> GV&
}

/**
 * @brief The user-defined value for a graph, if it exists.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for each graph type, if available.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @return The value associated with graph g.
*/
template <class G>
auto&& graph_value(G&& g) {
  return tag_invoke::graph_value(g);
}
template <class G>
using graph_value_t = decltype(graph_value(declval<G&&>()));

namespace edgelist {
  namespace tag_invoke {
    TAG_INVOKE_DEF(edges); // edges(e) -> [edge list vertices]
  }

  template <class EL>
  auto edges(EL&& el) {
    return el;
  }

  template <class EL>
  using edgelist_range_t = decltype(edges(declval<EL&&>()));

  template <class EL>
  using edgelist_iterator_t = ranges::iterator_t<edgelist_range_t<EL&&>>;

  template <class EL>
  using edge_t = ranges::range_value_t<edgelist_range_t<EL>>; // edge value type

  template <class EL>
  using edge_reference_t = ranges::range_reference_t<edgelist_range_t<EL>>; // edge reference type

  namespace tag_invoke {
    TAG_INVOKE_DEF(source_id);
  }

  template <class EL>
  auto source_id(EL&& el, edge_reference_t<EL> uv) {
    return tag_invoke::source_id(el, uv);
  }

  template <class EL>
  using source_id_t = decltype(source_id(declval<EL&&>(), declval<edge_reference_t<EL>>()));

  namespace tag_invoke {
    TAG_INVOKE_DEF(target_id);
  }

  template <class EL>
  auto target_id(EL&& el, edge_reference_t<EL> uv) {
    return tag_invoke::target_id(el, uv);
  }

  template <class EL>
  using target_id_t = decltype(target_id(declval<EL&&>(), declval<edge_reference_t<EL>>()));


  namespace tag_invoke {
    TAG_INVOKE_DEF(edge_value);
  }

  template <class EL>
  auto edge_value(EL&& el, edge_reference_t<EL> uv) {
    return tag_invoke::edge_value(el, uv);
  }

  template <class EL>
  using edge_value_t = decltype(edge_value(declval<EL&&>(), declval<edge_reference_t<EL>>()));
} // namespace edgelist


// partition_count(g) -> ?   default = vertex_id_t<G>(1) when vertex_id_t<G> is integral, size_t(0) otherwise
//
namespace tag_invoke {
  TAG_INVOKE_DEF(partition_count);

  template <class G>
  concept _has_partition_count_adl = requires(G&& g) {
    { partition_count(g) };
  };
} // namespace tag_invoke

/**
 * @brief Get's the number of partitions in a graph.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: 0; graph container must override if it supports bi-partite
 * or multipartite graphs.
 * 
 * This is a customization point function that may be overriden if graph G supports bi-partite
 * or multi-partite graphs. If it doesn't then a value of 0 is returned.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @return The number of partitions in a graph. 0 if G doesn't support partitioning.
*/
template <class G>
requires tag_invoke::_has_partition_count_adl<G>
auto partition_count(G&& g) {
  if constexpr (tag_invoke::_has_partition_count_adl<G>)
    return tag_invoke::partition_count(g);
  else if constexpr (is_integral_v<vertex_id_t<G>>)
    return vertex_id_t<G>(1);
  else
    return size_t(1);
}

// vertices(g,pid) -> range of vertices; graph container must override if it supports bi-partite or
// multi-partite graph.
//
namespace tag_invoke {
  //TAG_INVOKE_DEF(vertices); // vertices(g) -> [graph vertices] (already defined for vertices(g))

  template <class G>
  concept _has_vertices_pid_adl = requires(G&& g, partition_id_t<G> pid) {
    { vertices(g, pid) };
  };
} // namespace tag_invoke

/**
 * @brief Get's the range of vertices for a partition in a graph.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: empty range of vertices; the type returned may not be the same
 * as vertex_range_t<G>. The graph container must override if it supports bi-partite
 * or multipartite graphs.
 * 
 * This is a customization point function that may be overriden if graph G supports bi-partite
 * or multi-partite graphs. If it doesn't then an empty range is returned.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @return The number of partitions in a graph. 0 if G doesn't support partitioning.
*/
template <class G>
requires tag_invoke::_has_vertices_pid_adl<G>
auto vertices(G&& g, partition_id_t<G> pid) {
  if constexpr (tag_invoke::_has_vertices_pid_adl<G>)
    return tag_invoke::vertices(g, pid);
  else
    return vertices(g);
}

template <class G>
using partition_vertex_range_t = decltype(vertices(declval<G>(), declval<partition_id_t<G>>()));


template <class G>
struct _partition_vertex_id {
  partition_id_t<G> partition_id; //
  vertex_id_t<G>    vertex_id;    // vertex id within the partition_id
};

template <class G>
using partition_vertex_id_t = _partition_vertex_id<G>;


//
// partition_vertex_id(g,uid) -> partition_vertex_id_t<_G>
//      default = partition_vertex_id(g,vertex_id(g,uid))
//
// partition_vertex_id(g,ui) -> partition_vertex_id_t<_G>
//      default = partition_vertex_id(g,vertex_id(g,ui))
//
namespace _Partition_vertex_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void partition_vertex_id() = delete;       // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void partition_vertex_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_UId_member = requires(_G&& __g, vertex_id_t<_G> uid) {
    { _Fake_copy_init(__g.partition_vertex_id(uid)) };
  };

  template <class _G>
  concept _Has_UId_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, vertex_id_t<_G> uid) {
                              { _Fake_copy_init(partition_vertex_id(__g, uid)) }; // intentional ADL
                            };

  template <class _G>
  concept _Has_UIter_member = requires(_G&& __g, vertex_iterator_t<_G> ui) {
    { _Fake_copy_init(__g.partition_vertex_id(ui)) };
  };

  template <class _G>
  concept _Has_UIter_ADL = _Has_class_or_enum_type<_G> //
                           && requires(_G&& __g, vertex_iterator_t<_G> ui) {
                                { _Fake_copy_init(partition_vertex_id(__g, ui)) }; // intentional ADL
                              };

  class _Cpo {
  private:
    enum class _StId { _None, _Member, _Non_member };
    enum class _StIter { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_StId> _ChooseId() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      //using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_UId_member<_G>) {
        return {_StId::_Member,
                noexcept(_Fake_copy_init(declval<_G>().partition_vertex_id(declval<vertex_id_t<_G>>())))};
      } else if constexpr (_Has_UId_ADL<_G>) {
        return {_StId::_Non_member, noexcept(_Fake_copy_init(partition_vertex_id(
                                          declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else {
        return {_StId::_None};
      }
    }

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_StIter> _ChooseIter() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      //using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_UIter_member<_G>) {
        return {_StIter::_Member,
                noexcept(_Fake_copy_init(declval<_G>().partition_vertex_id(declval<vertex_iterator_t<_G>>())))};
      } else if constexpr (_Has_UIter_ADL<_G>) {
        return {_StIter::_Non_member, noexcept(_Fake_copy_init(partition_vertex_id(
                                            declval<_G>(), declval<vertex_iterator_t<_G>>())))}; // intentional ADL
      } else {
        return {_StIter::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_StId> _ChoiceId = _ChooseId<_G>();
    template <class _G>
    static constexpr _Choice_t<_StIter> _ChoiceIter = _ChooseIter<_G>();

  public:
    /**
     * @brief Get's the partition_id and relative vertex_id given a vertex_id_t<G>.
     *
     * Complexity: O(1)
     *
     * Default implementation: a single partition is assumed and {0, uid} is returned for the
     * partition_id and relative vertex_id respectfully.
     *
     * This is a customization point function that must be overriden if graph G supports bi-partite
     * or multi-partite graphs. 
     *
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid The vertex_id.
     * 
     * @return partition_vertex_id_t<G> with the partition_id and relative vertex_id in the partition 
     *         for the vertex_id passed.
    */
    template <class _G>
    requires(_ChoiceId<_G&>._Strategy != _StId::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_id_t<_G> uid) const noexcept(_ChoiceId<_G&>._No_throw) {
      constexpr _StId _Strat = _ChoiceId<_G&>._Strategy;

      if constexpr (_Strat == _StId::_Member) {
        return __g.partition_vertex_id(uid);
      } else if constexpr (_Strat == _StId::_Non_member) {
        return partition_vertex_id(__g, uid); // intentional ADL
      } else {
        return partition_vertex_id_t<_G>{0, uid};
      }
    }

    /**
     * @brief Get's the partition_id and relative vertex_id given a vertex_iterator_t<G>.
     *
     * Complexity: O(1)
     *
     * Default implementation: a single partition is assumed and {0, vertex_id(g,ui)} is returned 
     * for the partition_id and relative vertex_id respectfully.
     *
     * This is a customization point function that must be overriden if graph G supports bi-partite
     * or multi-partite graphs. 
     *
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param ui The vertex_iterator.
     * 
     * @return partition_vertex_id_t<G> with the partition_id and relative vertex_id in the partition 
     *         for the vertex_id passed.
    */
    template <class _G>
    requires(_ChoiceIter<_G&>._Strategy != _StIter::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_iterator_t<_G> ui) const
          noexcept(_ChoiceIter<_G&>._No_throw) {
      constexpr _StIter _Strat = _ChoiceIter<_G&>._Strategy;

      if constexpr (_Strat == _StIter::_Member) {
        return __g.partition_vertex_id(ui);
      } else if constexpr (_Strat == _StIter::_Non_member) {
        return partition_vertex_id(__g, ui); // intentional ADL
      } else {
        return (*this)(__g, vertex_id(__g, ui)); // use partition_vertex_id(g, vertex_id(g,ui))
      }
    }
  };
} // namespace _Partition_vertex_id

inline namespace _Cpos {
  inline constexpr _Partition_vertex_id::_Cpo partition_vertex_id;
}


//
// find_partition_vertex(g,puid) -> vertex_t<_G>
//      default = find_vertex(g, puid.vertex_id)
//
namespace _Find_partition_vertex {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void find_partition_vertex() = delete;     // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void find_partition_vertex();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_UId_member = requires(_G&& __g, partition_vertex_id_t<_G> puid) {
    { _Fake_copy_init(__g.find_partition_vertex(puid)) };
  };

  template <class _G>
  concept _Has_UId_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, partition_vertex_id_t<_G> puid) {
                              { _Fake_copy_init(find_partition_vertex(__g, puid)) }; // intentional ADL
                            };

  class _Cpo {
  private:
    enum class _StId { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_StId> _ChooseId() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      //using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_UId_member<_G>) {
        return {_StId::_Member,
                noexcept(_Fake_copy_init(declval<_G>().find_partition_vertex(declval<partition_vertex_id_t<_G>>())))};
      } else if constexpr (_Has_UId_ADL<_G>) {
        return {_StId::_Non_member, noexcept(_Fake_copy_init(find_partition_vertex(
                                          declval<_G>(), declval<partition_vertex_id_t<_G>>())))}; // intentional ADL
      } else {
        return {_StId::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_StId> _ChoiceId = _ChooseId<_G>();

  public:
    /**
     * @brief Find a vertex given a partition_vertex_id_t<G>
     *
     * Complexity: O(1)
     *
     * Default implementation: a single partition is assumed find_vertex(g,puid) is used to find the
     * vertex.
     *
     * This is a customization point function that must be overriden if graph G supports bi-partite
     * or multi-partite graphs. 
     *
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param puid The partition_vertex_id to find.
     * 
     * @return a vertex_iterator_t<G> for the partition_vertex_id_t<G> passed. If it doesn't
     *         exist end(vertices(g)) will be returned.
    */
    template <class _G>
    requires(_ChoiceId<_G&>._Strategy != _StId::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, partition_vertex_id_t<_G> puid) const
          noexcept(_ChoiceId<_G&>._No_throw) {
      constexpr _StId _Strat = _ChoiceId<_G&>._Strategy;

      if constexpr (_Strat == _StId::_Member) {
        return __g.find_partition_vertex(puid);
      } else if constexpr (_Strat == _StId::_Non_member) {
        return find_partition_vertex(__g, puid); // intentional ADL
      } else {
        return find_vertex(__g, puid.vertex_id); // assume 1 partition with all vertices
      }
    }
  };
} // namespace _Find_partition_vertex

inline namespace _Cpos {
  inline constexpr _Find_partition_vertex::_Cpo find_partition_vertex;
}


template <class G>
using partition_edge_range_t = vertex_edge_range_t<G>;


//
// edges(g,u)  -> vertex_edge_range_t<G>
// edges(g,uid) -> vertex_edge_range_t<G>
//      default = edges(g,*find_vertex(g,uid))
//
// vertex_edge_range_t<G>    = edges(g,u)
// vertex_edge_iterator_t<G> = ranges::iterator_t<vertex_edge_range_t<G>>
// edge_t                    = ranges::range_value_t<vertex_edge_range_t<G>>
// edge_reference_t          = ranges::range_reference_t<vertex_edge_range_t<G>>
//
namespace tag_invoke {
  //TAG_INVOKE_DEF(edges);

  template <class G>
  concept _has_edges_vtxref_part_adl = requires(G&& g, vertex_reference_t<G> u, partition_id_t<G> p) {
    { edges(g, u, p) };
  };

  template <class G>
  concept _has_edges_vtxid_part_adl = requires(G&& g, vertex_id_t<G> uid, partition_id_t<G> p) {
    { edges(g, uid, p) };
  };
} // namespace tag_invoke

/**
 * @brief Get the outgoing edges of a vertex.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: edges(g,u)
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param u Vertex reference.
 * @return A range of the outgoing edges.
*/
template <class G>
requires tag_invoke::_has_edges_vtxref_part_adl<G>
auto edges(G&& g, vertex_reference_t<G> u, partition_id_t<G> p) -> decltype(tag_invoke::edges(g, u, p)) {
  if constexpr (tag_invoke::_has_edges_vtxref_part_adl<G>)
    return tag_invoke::edges(g, u, p); // graph author must define
  else
    return edges(g, u);
}

/**
 * @brief Get the outgoing edges of a vertex id.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: edges(g, *find_vertex(g, uid), p)
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uid Vertex id.
 * @return A range of the outgoing edges.
*/
template <class G>
requires tag_invoke::_has_edges_vtxid_part_adl<G>
auto edges(G&& g, vertex_id_t<G> uid, partition_id_t<G> p) -> decltype(tag_invoke::edges(g, uid, p)) {
  if constexpr (tag_invoke::_has_edges_vtxid_part_adl<G>)
    return tag_invoke::edges(g, uid, p);
  else
    return edges(g, *find_vertex(g, uid), p);
}


//
// partition_target_id(g,puid) -> partition_vertex_id_t<_G>
//      default = partition_vertex_id_t<G>{0, target_id(__g, uv)}
//
namespace _Partition_target_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void partition_target_id() = delete;       // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void partition_target_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_UVRef__member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(__g.partition_target_id(uv)) };
  };

  template <class _G>
  concept _Has_UVRef__ADL = _Has_class_or_enum_type<_G> //
                            && requires(_G&& __g, edge_reference_t<_G> uv) {
                                 { _Fake_copy_init(partition_target_id(__g, uv)) }; // intentional ADL
                               };

  class _Cpo {
  private:
    enum class _StRef { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_StRef> _ChooseRef() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      //using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_UVRef__member<_G>) {
        return {_StRef::_Member,
                noexcept(_Fake_copy_init(declval<_G>().partition_target_id(declval<edge_reference_t<_G>>())))};
      } else if constexpr (_Has_UVRef__ADL<_G>) {
        return {_StRef::_Non_member, noexcept(_Fake_copy_init(partition_target_id(
                                           declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else {
        return {_StRef::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_StRef> _ChoiceId = _ChooseRef<_G>();

  public:
    /**
     * @brief Find a vertex given a edge_reference_t<G>
     *
     * Complexity: O(1)
     *
     * Default implementation: a single partition is assumed find_vertex(g,uv) is used to find the
     * vertex.
     *
     * This is a customization point function that must be overriden if graph G supports bi-partite
     * or multi-partite graphs. 
     *
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv The partition_vertex_id to find.
     * 
     * @return a vertex_iterator_t<G> for the edge_reference_t<G> passed. If it doesn't
     *         exist end(vertices(g)) will be returned.
    */
    template <class _G>
    requires(_ChoiceId<_G&>._Strategy != _StRef::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_ChoiceId<_G&>._No_throw) {
      constexpr _StRef _Strat = _ChoiceId<_G&>._Strategy;

      if constexpr (_Strat == _StRef::_Member) {
        return __g.partition_target_id(uv);
      } else if constexpr (_Strat == _StRef::_Non_member) {
        return partition_target_id(__g, uv); // intentional ADL
      } else {
        return partition_vertex_id_t<_G>{0, target_id(__g, uv)}; // assume 1 partition with all vertices
      }
    }
  };
} // namespace _Partition_target_id

inline namespace _Cpos {
  inline constexpr _Partition_target_id::_Cpo partition_target_id;
}


//
// partition_source_id(g,puid) -> partition_vertex_id_t<_G>
//      default = partition_vertex_id_t<G>{0, source_id(__g, uv)}
//
namespace _Partition_source_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void partition_source_id() = delete;       // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void partition_source_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_UVRef__member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(__g.partition_source_id(uv)) };
  };

  template <class _G>
  concept _Has_UVRef__ADL = _Has_class_or_enum_type<_G> //
                            && requires(_G&& __g, edge_reference_t<_G> uv) {
                                 { _Fake_copy_init(partition_source_id(__g, uv)) }; // intentional ADL
                               };

  class _Cpo {
  private:
    enum class _StRef { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_StRef> _ChooseRef() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      //using _UnCV = remove_cvref_t<_G>;

      if constexpr (_Has_UVRef__member<_G>) {
        return {_StRef::_Member,
                noexcept(_Fake_copy_init(declval<_G>().partition_source_id(declval<edge_reference_t<_G>>())))};
      } else if constexpr (_Has_UVRef__ADL<_G>) {
        return {_StRef::_Non_member, noexcept(_Fake_copy_init(partition_source_id(
                                           declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else {
        return {_StRef::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_StRef> _ChoiceId = _ChooseRef<_G>();

  public:
    /**
     * @brief Find a vertex given a edge_reference_t<G>
     *
     * Complexity: O(1)
     *
     * Default implementation: a single partition is assumed find_vertex(g,uv) is used to find the
     * vertex.
     *
     * This is a customization point function that must be overriden if graph G supports bi-partite
     * or multi-partite graphs. 
     *
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv The partition_vertex_id to find.
     * 
     * @return a vertex_iterator_t<G> for the edge_reference_t<G> passed. If it doesn't
     *         exist end(vertices(g)) will be returned.
    */
    template <class _G>
    requires(_ChoiceId<_G&>._Strategy != _StRef::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_ChoiceId<_G&>._No_throw) {
      constexpr _StRef _Strat = _ChoiceId<_G&>._Strategy;

      if constexpr (_Strat == _StRef::_Member) {
        return __g.partition_source_id(uv);
      } else if constexpr (_Strat == _StRef::_Non_member) {
        return partition_source_id(__g, uv); // intentional ADL
      } else {
        return partition_vertex_id_t<_G>{0, source_id(__g, uv)}; // assume 1 partition with all vertices
      }
    }
  };
} // namespace _Partition_source_id

inline namespace _Cpos {
  inline constexpr _Partition_source_id::_Cpo partition_source_id;
}


} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
