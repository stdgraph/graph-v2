#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_CPO_HPP
#  define GRAPH_CPO_HPP

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

// e.g. vector<vector<...>>
template <class _G>
concept _range_of_ranges = ranges::forward_range<_G> && ranges::forward_range<ranges::range_value_t<_G>>;

// Edge type for range-of-ranges
template <_range_of_ranges _G>
using _rr_edge_t = ranges::range_value_t<ranges::range_value_t<_G>>;

template <class _G>
concept _rr_simple_id = _range_of_ranges<_G> && integral<_rr_edge_t<_G>>;

template <class _G>
concept _rr_tuple_id = _range_of_ranges<_G> && integral<tuple_element_t<0, _rr_edge_t<_G>>>;


template <class _G>
struct _rr_vertex_id {
  using type = size_t; // default vertex_id is size_t
};

template <_rr_simple_id _G>
struct _rr_vertex_id<_G> {
  using type = _rr_edge_t<_G>; // The target id type is a single integral value, eg. vector<vector<int>>
};

template <_rr_tuple_id _G>
struct _rr_vertex_id<_G> {
  using type =
        tuple_element_t<0, _rr_edge_t<_G>>; // The target id is the first element of a single integral value of a tuple
};

template <class _G>
using _rr_vertex_id_t = typename _rr_vertex_id<_G>::type;


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
namespace _Vertices {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void vertices() = delete;                  // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void vertices();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = _Has_class_or_enum_type<_G> && //
                            requires(_G&& __g) {
                              { _Fake_copy_init(__g.vertices()) };
                            };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(vertices(__g)) }; // intentional ADL
                            };

  template <class _G>
  concept _Can_ref_eval = _Has_class_or_enum_type<_G> && ranges::random_access_range<_G>;

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().vertices()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(vertices(declval<_G>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval, noexcept(true)};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
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
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __g.vertices();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        //static_assert(is_reference_v<decltype(vertices(__g))>);
        return vertices(__g); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return std::forward<_G>(__g); // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "vertices(g) is not defined");
      }
    }
  };
} // namespace _Vertices

inline namespace _Cpos {
  inline constexpr _Vertices::_Cpo vertices;
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
using vertex_iterator_t = ranges::iterator_t<vertex_range_t<G>>;

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
namespace _Vertex_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void vertex_id() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void vertex_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_iterator_t<_G> ui) {
    { _Fake_copy_init(ui->vertex_id(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_iterator_t<_G> ui) {
                              { _Fake_copy_init(vertex_id(__g, ui)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = ranges::random_access_range<vertex_range_t<_G>>;

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member,
                noexcept(_Fake_copy_init(declval<vertex_iterator_t<_G>>()->vertex_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {
              _St_ref::_Non_member,
              noexcept(_Fake_copy_init(vertex_id(declval<_G>(), declval<vertex_iterator_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(declval<vertex_iterator_t<_G>>() -
                                         ranges::begin(vertices(declval<_G>()))))}; // intentional ADL
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

    // The vertex_id type is defined using the following criteria, with the first one found will be used
    //  1. return value of vertex_id(g,ui) if it is overridden for a graph
    //  2. for range-of-ranges patterns:
    //      a.  e.g. vector<vector<int>> uses int
    //      b.  e.g. vector<vector<tuple<int,...>>> uses int
    //  3. size_t for the final default to match with type used for index vector & deque
    template <class _G>
    using _vid_t = _rr_vertex_id_t<_G>;

  public:
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
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_iterator_t<_G> ui) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return ui->vertex_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return vertex_id(__g, ui); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return static_cast<_vid_t<_G>>(ui - ranges::begin(vertices(__g)));
      } else {
        static_assert(_Always_false<_G>, "vertices(g) is not defined or is not random-access");
      }
    }
  };
} // namespace _Vertex_id

inline namespace _Cpos {
  inline constexpr _Vertex_id::_Cpo vertex_id;
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

  template <class _G>
  concept _Has_member = requires(_G&& __g, const vertex_id_t<_G>& uid) {
    { _Fake_copy_init(__g.find_vertex(uid)) };
  };

  template <class _G>
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
      if constexpr (_Has_member<_G>) {
        return {_St::_Member, noexcept(_Fake_copy_init(declval<_G>().find_vertex(declval<vertex_id_t<_G>>())))};
      } else if constexpr (_Has_ADL<_G>) {
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
namespace _Partition_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void partition_id() = delete;              // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void partition_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(u.partition_id(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_reference_t<_G>& u) {
                              { _Fake_copy_init(partition_id(__g, u)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = _Has_class_or_enum_type<_G> && integral<vertex_id_t<_G>> //
                          && requires(_G&& __g, vertex_id_t<_G> uid) {
                               { _Fake_copy_init(vertex_id_t<_G>{0}) };
                             };

  template <class _G>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, const vertex_id_t<_G>& uid) {
                             { _Fake_copy_init(partition_id(__g, uid)) }; // intentional ADL
                           };
  template <class _G>
  concept _Can_id_eval = _Has_class_or_enum_type<_G> && integral<vertex_id_t<_G>> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(vertex_id_t<_G>{0}) };
                            };

  class _Cpo {
  private:
    enum class _St_id { _None, _Non_member, _Auto_eval };
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_id_ADL<_G>) {
        return {_St_id::_Non_member,
                noexcept(_Fake_copy_init(partition_id(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
        return {_St_id::_Auto_eval, noexcept(_Fake_copy_init(vertex_id_t<_G>(0)))}; // default impl
      } else {
        return {_St_id::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member,
                noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().partition_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(partition_id(
                                            declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(vertex_id_t<_G>(0)))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief The partition id of a vertex
     * 
     * Complexity: O(1)
     * 
     * Default implementation: vertex_id_t<_G>(0) if vertex_id_t<_G> is integral
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param u A vertex instance.
     * @return The partition id of u.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_reference_t<_G> u) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return u.partition_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return partition_id(__g, u); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return vertex_id_t<_G>{0}; // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "partition_id(g,u) is not defined and the default implementation cannot be evaluated");
      }
    }

    /**
     * @brief Get the outgoing partition_id of a vertex id.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: vertex_id_t<_G>(0) if vertex_id_t<_G> is integral
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uid Vertex id.
     * @return The partition id of uid.
    */
    template <class _G>
    requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
          noexcept(_Choice_id<_G&>._No_throw) {
      constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;

      if constexpr (_Strat_id == _St_id::_Non_member) {
        return partition_id(__g, uid); // intentional ADL
      } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
        return vertex_id_t<_G>{0}; // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "partition_id(g,uid) is not defined and the default implementation cannot be evaluated");
      }
    }
  };
} // namespace _Partition_id

inline namespace _Cpos {
  inline constexpr _Partition_id::_Cpo partition_id;
}

template <class G>
using partition_id_t = decltype(partition_id(declval<G>(), declval<vertex_reference_t<G>>()));


//
// edges(g,u)  -> vertex_edge_range_t<G>
// edges(g,uid) -> vertex_edge_range_t<G>
//      default = edges(g,*find_vertex(g,uid)) if the vertex is a range
//
// vertex_edge_range_t<G>    = edges(g,u)
// vertex_edge_iterator_t<G> = ranges::iterator_t<vertex_edge_range_t<G>>
// edge_t                    = ranges::range_value_t<vertex_edge_range_t<G>>
// edge_reference_t          = ranges::range_reference_t<vertex_edge_range_t<G>>
//
namespace _Edges {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void edges() = delete;                     // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void edges();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(u.edges(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_reference_t<_G>& u) {
                              { _Fake_copy_init(edges(__g, u)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = _Has_class_or_enum_type<_G> && ranges::forward_range<vertex_t<_G>>;

  template <class _G>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, const vertex_id_t<_G>& uid) {
                             { _Fake_copy_init(edges(__g, uid)) }; // intentional ADL
                           };
  template <class _G>
  concept _Can_id_eval = _Has_class_or_enum_type<_G> && ranges::forward_range<vertex_t<_G>> //
                         && requires(_G&& __g, vertex_id_t<_G> uid) {
                              { _Fake_copy_init(find_vertex(__g, uid)) };
                            };

  class _Cpo {
  private:
    enum class _St_id { _None, _Non_member, _Auto_eval };
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_id_ADL<_G>) {
        return {_St_id::_Non_member,
                noexcept(_Fake_copy_init(edges(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
        return {_St_id::_Auto_eval,
                noexcept(_Fake_copy_init(*find_vertex(declval<_G>(), declval<vertex_id_t<_G>>())))}; // default impl
      } else {
        return {_St_id::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().edges(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(edges(declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(*find_vertex(declval<_G>(), declval<vertex_id_t<_G>>())))};
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
     * Default implementation: vertex() if the vertex is a range; otherwise it must be overridden by the graph
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param u A vertex instance.
     * @return The number of outgoing edges of vertex u.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_reference_t<_G> u) const
          noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return u.edges(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return edges(__g, u); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return u; // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "edges(g,u) is not defined and the default implementation cannot be evaluated");
      }
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
    template <class _G>
    requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& uid) const
          noexcept(_Choice_id<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;

      if constexpr (_Strat_id == _St_id::_Non_member) {
        return edges(__g, uid); // intentional ADL
      } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
        return *find_vertex(__g, uid); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "edges(g,uid) is not defined and the default implementation cannot be evaluated");
      }
    }
  };
} // namespace _Edges

inline namespace _Cpos {
  inline constexpr _Edges::_Cpo edges;
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
// num_edges(g,)      -> integral   default = n=0; for (const auto& u : vertices(g)) n += distance(edges(g,u))
// num_edges(g,u,pid) -> integral   default = ?
//
namespace _NumEdges {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void num_edges() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void num_edges();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g) {
    { _Fake_copy_init(__g.num_edges()) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(num_edges(__g)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = requires(_G&& __g, vertex_t<_G> __u) {
    { vertices(__g) };
    { _Fake_copy_init(ranges::distance(edges(__g, __u))) };
  };

  // This is for edges(g, pid) which is not defined in the proposal.
  // The proposal only defines edges(g, u, pid) and edges(g, uid, pid)
  // We need edges(g, u) and edges(g, uid) which isn't implemented yet.
  //template <class _G>
  //concept _Has_id_ADL = _Has_class_or_enum_type<_G>                    //
  //                      && requires(_G&& __g, partition_id_t<_G> pid) {
  //                           { _Fake_copy_init(num_edges(__g, pid)) }; // intentional ADL
  //                         };
  //template <class _G>
  //concept _Can_id_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
  //                       && requires(_G&& __g, partition_id_t<_G> pid) {
  //                            { _Fake_copy_init(ranges::distance(edges(__g, pid))) };
  //                          };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };
    enum class _St_id { _None, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().num_edges()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(num_edges(declval<_G>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(ranges::distance(edges(declval<_G>(), declval<vertex_reference_t<_G>>()))))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

    //template <class _G>
    //[[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
    //  static_assert(is_lvalue_reference_v<_G>);
    //  if constexpr (_Has_id_ADL<_G>) {
    //    return {
    //          _St_id::_Non_member,
    //          noexcept(_Fake_copy_init(num_edges(declval<_G>(), declval<partition_id_t<_G>>())))}; // intentional ADL
    //  } else if constexpr (_Can_id_eval<_G>) {
    //    return {_St_id::_Auto_eval, noexcept(_Fake_copy_init(ranges::distance(
    //                                      edges(declval<_G>(), declval<partition_id_t<_G>>()))))}; // default impl
    //  } else {
    //    return {_St_id::_None};
    //  }
    //}

    //template <class _G>
    //static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

  public:
    /**
       * @brief The number of edges in a graph.
       * 
       * Complexity: 
       *    O(1) if overridden by the graph and it can support it
       *    O(|V|) for graphs with sized edges(g, u)
       *    O(|E|) for graphs without sized edges(g, u)
       * 
       * Default implementation:
       *    size_t n = 0;
       *    for (const auto& u : vertices(g))
       *        n += distance(edges(g, u));
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @return The number of edges in g.
      */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_id = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_id == _St_ref::_Member) {
        return __g.num_edges();
      } else if constexpr (_Strat_id == _St_ref::_Non_member) {
        return num_edges(__g); // intentional ADL
      } else if constexpr (_Strat_id == _St_ref::_Auto_eval) {
        using size_type = decltype(ranges::distance(edges(__g, declval<vertex_reference_t<_G>>())));
        size_type n     = 0; // default impl
        for (auto&& u : vertices(__g))
          n += ranges::distance(edges(__g, u));
        return n;
      } else {
        static_assert(_Always_false<_G>,
                      "num_edges(g) is not defined and the default implementation cannot be evaluated");
      }
    }

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
    //template <class _G>
    ////requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    //[[nodiscard]] constexpr auto operator()(_G&& __g, const partition_id_t<_G>& pid) const
    //      noexcept(_Choice_id<_G&>._No_throw) {
    //  constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;
    //  static_assert(_Strat_id == _St_id::_Auto_eval);

    //  if constexpr (_Strat_id == _St_id::_Non_member) {
    //    return num_edges(__g, pid);              // intentional ADL
    //  } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
    //    return ranges::size(vertices(__g, pid)); // default impl
    //  } else {
    //    static_assert(_Always_false<_G>,
    //                  "num_edges(g,pid) is not defined and the default implementation cannot be evaluated");
    //  }
    //}
  };
} // namespace _NumEdges

inline namespace _Cpos {
  inline constexpr _NumEdges::_Cpo num_edges;
}

//
// has_edge(g,)      -> bool        default = for(const auto& u : vertices(g)) if (!empty(edges(g,u))) return true; return false;
// has_edge(g,u,pid) -> bool        default = ?
//
namespace _HasEdge {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void has_edge() = delete;                  // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void has_edge();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g) {
    { _Fake_copy_init(__g.has_edge()) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(has_edge(__g)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = requires(_G&& __g, vertex_t<_G> __u) {
    { vertices(__g) };
    { _Fake_copy_init(ranges::empty(edges(__g, __u))) };
  };

  // This is for edges(g, pid) which is not defined in the proposal.
  // The proposal only defines edges(g, u, pid) and edges(g, uid, pid)
  // We need edges(g, u) and edges(g, uid) which isn't implemented yet.
  //template <class _G>
  //concept _Has_id_ADL = _Has_class_or_enum_type<_G>                    //
  //                      && requires(_G&& __g, partition_id_t<_G> pid) {
  //                           { _Fake_copy_init(has_edge(__g, pid)) }; // intentional ADL
  //                         };
  //template <class _G>
  //concept _Can_id_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
  //                       && requires(_G&& __g, partition_id_t<_G> pid) {
  //                            { _Fake_copy_init(ranges::distance(edges(__g, pid))) };
  //                          };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };
    enum class _St_id { _None, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().has_edge()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(has_edge(declval<_G>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(ranges::empty(edges(declval<_G>(), declval<vertex_reference_t<_G>>()))))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

    //template <class _G>
    //[[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
    //  static_assert(is_lvalue_reference_v<_G>);
    //  if constexpr (_Has_id_ADL<_G>) {
    //    return {
    //          _St_id::_Non_member,
    //          noexcept(_Fake_copy_init(has_edge(declval<_G>(), declval<partition_id_t<_G>>())))}; // intentional ADL
    //  } else if constexpr (_Can_id_eval<_G>) {
    //    return {_St_id::_Auto_eval, noexcept(_Fake_copy_init(ranges::distance(
    //                                      edges(declval<_G>(), declval<partition_id_t<_G>>()))))}; // default impl
    //  } else {
    //    return {_St_id::_None};
    //  }
    //}

    //template <class _G>
    //static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

  public:
    /**
       * @brief The number of edges in a graph.
       * 
       * Complexity: 
       *    O(1) if overridden by the graph and it can support it
       *    O(|V|) for default implementation
       * 
       * Default implementation:
       *    size_t n = 0;
       *    for (const auto& u : vertices(g))
       *        n += distance(edges(g, u));
       * 
       * @tparam G The graph type.
       * @param g A graph instance.
       * @return The number of edges in g.
      */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr bool operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_id = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_id == _St_ref::_Member) {
        return __g.has_edge();
      } else if constexpr (_Strat_id == _St_ref::_Non_member) {
        return has_edge(__g); // intentional ADL
      } else if constexpr (_Strat_id == _St_ref::_Auto_eval) {
        for (auto&& u : vertices(__g))
          if (ranges::empty(edges(__g, u)))
            return true;
        return false;
      } else {
        static_assert(_Always_false<_G>,
                      "has_edge(g) is not defined and the default implementation cannot be evaluated");
      }
    }

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
    //template <class _G>
    ////requires(_Choice_id<_G&>._Strategy != _St_id::_None)
    //[[nodiscard]] constexpr auto operator()(_G&& __g, const partition_id_t<_G>& pid) const
    //      noexcept(_Choice_id<_G&>._No_throw) {
    //  constexpr _St_id _Strat_id = _Choice_id<_G&>._Strategy;
    //  static_assert(_Strat_id == _St_id::_Auto_eval);

    //  if constexpr (_Strat_id == _St_id::_Non_member) {
    //    return has_edge(__g, pid);              // intentional ADL
    //  } else if constexpr (_Strat_id == _St_id::_Auto_eval) {
    //    return ranges::size(vertices(__g, pid)); // default impl
    //  } else {
    //    static_assert(_Always_false<_G>,
    //                  "has_edge(g,pid) is not defined and the default implementation cannot be evaluated");
    //  }
    //}
  };
} // namespace _HasEdge

inline namespace _Cpos {
  inline constexpr _HasEdge::_Cpo has_edge;
}

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

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(uv.target_id(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(target_id(__g, uv)) }; // intentional ADL
  };

  template <class _G>
  concept _Is_basic_id_adj = integral<_rr_edge_t<_G>>; // vertex<vertex<int>>

  template <class _G>
  concept _Is_tuple_id_adj = integral<tuple_element_t<0, _rr_edge_t<_G>>>; // vertex<vertex<tuple<int,...>>>

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Basic_id, _Tuple_id, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);

      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().target_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {
              _St_ref::_Non_member,
              noexcept(_Fake_copy_init(target_id(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Is_basic_id_adj<_G>) {
        return {_St_ref::_Basic_id,
                noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>()))}; // e.g. vector<list<int>>
      } else if constexpr (_Is_tuple_id_adj<_G>) {
        return {
              _St_ref::_Tuple_id,
              noexcept(_Fake_copy_init(get<0>(declval<edge_reference_t<_G>>())))}; // e.g. vector<list<tuple<int,...>>>
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
    //requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return uv.target_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return target_id(__g, uv); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Basic_id) {
        return uv;
      } else if constexpr (_Strat_ref == _St_ref::_Tuple_id) {
        return get<0>(uv);
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
//      default = *find_vertex(g,target_id(g,uv))
//
//      for random_access_range<vertices(g)> and integral<target_id(g,uv))
//      uv can be from edges(g,u) or vertices(g,u)
//
namespace _Target {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void target() = delete;                    // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void target();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                              { _Fake_copy_init(target(__g, uv)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = ranges::random_access_range<vertex_range_t<_G>> //
                          && requires(_G&& __g, edge_reference_t<_G> uv, vertex_id_t<_G> uid) {
                               { _Fake_copy_init(find_vertex(__g, uid)) };
                               { _Fake_copy_init(target_id(__g, uv)) } -> integral;
                             };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);

      if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(target(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(*find_vertex(
                      declval<_G>(), target_id(declval<_G>(), declval<edge_reference_t<_G>>()))))}; // intentional ADL
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
    [[nodiscard]] constexpr auto&& operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return target(__g, uv); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return *find_vertex(__g, target_id(__g, uv));
      } else {
        static_assert(_Always_false<_G>, "target(g,uv) or uv.target(g) or g.target_id(g,uv) is not defined");
      }
    }
  };
} // namespace _Target

inline namespace _Cpos {
  inline constexpr _Target::_Cpo target;
}


//
//
// source_id(g,uv) -> vertex_id_t<G> (optional; only when a source_id exists on an edge)
//
namespace _EL_Source_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void source_id() = delete;                 // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void source_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(uv.source_id(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                              { _Fake_copy_init(source_id(__g, uv)) }; // intentional ADL
                            };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().source_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {
              _St_ref::_Non_member,
              noexcept(_Fake_copy_init(source_id(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
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
        return uv.source_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return source_id(__g, uv); // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "source_id(g,uv) or g.source_id(uv) is not defined");
      }
    }
  };
} // namespace _EL_Source_id

inline namespace _Cpos {
  inline constexpr _EL_Source_id::_Cpo source_id;
}

//
// source(g,uv) -> vertex_reference_t<G>
//      default = *(begin(g,vertices(g)) + source_id(g,uv))
//
//      for random_access_range<vertices(g)> and integral<source_id(g,uv))
//      uv can be from edges(g,u) or vertices(g,u)
//
namespace _Source {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void source() = delete;                    // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void source();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                              { _Fake_copy_init(source(__g, uv)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = ranges::random_access_range<vertex_range_t<_G>> //
                          && requires(_G&& __g, edge_reference_t<_G> uv) {
                               { _Fake_copy_init(source_id(__g, uv)) } -> integral;
                             };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(source(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {
              _St_ref::_Auto_eval,
              noexcept(_Fake_copy_init(begin(vertices(declval<_G>())) +
                                       source_id(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief Get the source vertex of an edge.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: *(begin(vertices(g)) + source_id(g, uv)), 
     #         if @source_id(g,uv) is defined for G and random_access_range<vertex_range_t<G>>
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
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto&& operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return source(__g, uv); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return *(begin(vertices(__g)) + source_id(__g, uv));
      } else {
        static_assert(_Always_false<_G>, "source(g,uv) or g.source(uv) is not defined");
      }
    }
  };
} // namespace _Source

inline namespace _Cpos {
  inline constexpr _Source::_Cpo source;
}


//
// edge_id(g,uv) -> edge_id_t<G>
//      default = edge_id_t<G>(source_id(g,uv),target_id(g,uv))
//
// edge_id_t<G> = edge_descriptor<vertex_id_t<_G>, true, void, void>
//
template <class _G>
using edge_id_t = edge_descriptor<vertex_id_t<_G>, true, void, void>; // {source_id, target_id}

namespace _Edge_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void edge_id() = delete;                   // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void edge_id();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { uv.edge_id(__g) } -> convertible_to<edge_id_t<_G>>;
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                              { _Fake_copy_init(edge_id(__g, uv)) } -> convertible_to<edge_id_t<_G>>; // intentional ADL
                            };

  template <class _G>
  concept _Can_id_eval = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, edge_reference_t<_G> uv) {
                              { _Fake_copy_init(edge_id_t<_G>{source_id(__g, uv), target_id(__g, uv)}) };
                            };
  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().edge_id(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(edge_id(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(edge_id_t<_G>{source_id(declval<_G>(), declval<edge_reference_t<_G>>()),
                                                       target_id(declval<_G>(), declval<edge_reference_t<_G>>())}))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief The id of an edge, made from its source_id and target_id.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: 
     *      edge_descriptor<vertex_id_t<G>,true>{source_id(g,uv), target_id(g,uv)}
     *      given that source_id(g,uv) is defined.
     * 
     * @tparam G The graph type.
     * @param g  A graph instance.
     * @param uv An edge reference.
     * @return An edge_descriptor with the source_id and target_id.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return uv.edge_id(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return edge_id(__g, uv); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return edge_id_t<_G>{source_id(__g, uv), target_id(__g, uv)};
      } else {
        static_assert(_Always_false<_G>,
                      "edge_id(g,uv) is not defined, or target_id(g,uv) and source_id(g,uv) are not defined");
      }
    }
  };
} // namespace _Edge_id

inline namespace _Cpos {
  inline constexpr _Edge_id::_Cpo edge_id;
}


//
// find_vertex_edge(g,u,vid) -> vertex_edge_iterator<G>
//      default = find(edges(g,u), [](uv) {target_id(g,uv)==vid;}
//
// find_vertex_edge(g,uid,vid) -> vertex_edge_iterator<G>
//      default = find_vertex_edge(g,*find_vertex(g,uid),vid)
//
namespace _Find_vertex_edge {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void find_vertex_edge() = delete;          // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void find_vertex_edge();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u, const vertex_id_t<_G>& vid) {
    { _Fake_copy_init(u.find_vertex_edge(__g, vid)) };
  };

  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, vertex_reference_t<_G> u, const vertex_id_t<_G>& vid) {
                              { _Fake_copy_init(find_vertex_edge(__g, u, vid)) }; // intentional ADL
                            };

  template <class _G>
  concept _Can_ref_eval = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(edges(__g, u)) };
  };

  template <class _G>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, vertex_id_t<_G> uid, const vertex_id_t<_G>& vid) {
                             { _Fake_copy_init(find_vertex_edge(__g, uid, vid)) }; // intentional ADL
                           };

  template <class _G>
  concept _Can_id_eval = requires(_G&& __g, vertex_id_t<_G> uid) {
    { _Fake_copy_init(edges(__g, uid)) };
  };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };
    enum class _St_id { _None, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().find_vertex_edge(
                                        declval<_G>(), declval<vertex_id_t<_G>>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(find_vertex_edge(declval<_G>(), declval<vertex_reference_t<_G>>(),
                                                          declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        using fnc_find_t = decltype([](edge_reference_t<_G>) -> bool { return true; });
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(ranges::find_if(edges(declval<_G>(), declval<vertex_reference_t<_G>>()),
                                                         declval<fnc_find_t>())))}; // intentional ADL
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_id_ADL<_G>) {
        return {_St_id::_Non_member,
                noexcept(_Fake_copy_init(find_vertex_edge(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                          declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
        using fnc_find_t = decltype([](edge_reference_t<_G>) -> bool { return true; });
        return {_St_id::_Auto_eval,
                noexcept(_Fake_copy_init(ranges::find_if(edges(declval<_G>(), declval<vertex_id_t<_G>>()),
                                                         declval<fnc_find_t>())))}; // intentional ADL
      } else {
        return {_St_id::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_id> _Choice_id = _Choose_id<_G>();

  public:
    /**
     * @brief Find an edge given a source vertex reference and target vertex id.
     * 
     * Complexity: O(e), where e is the number of outgoing edges of vertex u
     * 
     * Default implementation: find_if(edges(g, u), [&g, &vid](auto&& uv) { return target_id(g, uv) == vid; })
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param u   Source vertex
     * @param vid Target vertex id.
     * @return An iterator to the edge if it exists, or end(edges(g,u)) if it doesn't exist.
    */
    template <class _G>
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_reference_t<_G> u, const vertex_id_t<_G>& vid) const
          noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat == _St_ref::_Member) {
        return u.find_vertex_edge(__g, vid);
      } else if constexpr (_Strat == _St_ref::_Non_member) {
        return find_vertex_edge(__g, u, vid); // intentional ADL
      } else if constexpr (_Strat == _St_ref::_Auto_eval) {
        return ranges::find_if(edges(__g, u), [&__g, &vid](auto&& uv) { return target_id(__g, uv) == vid; });
      } else {
        static_assert(_Always_false<_G>,
                      "find_vertex_edge(g,uid) has not been defined and the default implemenation cannot be evaluated");
      }
    }

    /**
     * @brief Find an edge given a source and target vertex ids.
     * 
     * Complexity: O(e), where e is the number of outgoing edges of vertex uid
     * 
     * Default implementation: find_if(edges(g, uid), [&g, &vid](auto&& uv) { return target_id(g, uv) == vid; })
     * 
     * @tparam G The graph type.
     * @param g   A graph instance.
     * @param u   Source vertex
     * @param vid Target vertex id.
     * @return An iterator to the edge if it exists, or end(edges(g,u)) if it doesn't exist.
    */
    template <class _G>
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_id_t<_G> uid, const vertex_id_t<_G>& vid) const
          noexcept(_Choice_id<_G&>._No_throw) {
      constexpr _St_id _Strat = _Choice_id<_G&>._Strategy;

      if constexpr (_Strat == _St_id::_Non_member) {
        return find_vertex_edge(__g, uid, uid); // intentional ADL
      } else if constexpr (_Strat == _St_id::_Auto_eval) {
        return ranges::find_if(edges(__g, uid), [&__g, &vid](auto&& uv) { return target_id(__g, uv) == vid; });
      } else {
        static_assert(_Always_false<_G>,
                      "find_vertex_edge(g,uid) has not been defined and the default implemenation cannot be evaluated");
      }
    }
  };
} // namespace _Find_vertex_edge

inline namespace _Cpos {
  inline constexpr _Find_vertex_edge::_Cpo find_vertex_edge;
}


//
// contains_edge(g,uid,vid) -> bool
//      default = uid < size(vertices(g)) && vid < size(vertices(g)), if adjacency_matrix<G>
//              = find_vertex_edge(g,uid) != ranges::end(edges(g,uid));
//
namespace _Contains_edge {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void contains_edge() = delete;             // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void contains_edge();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_id_t<_G>& uid, const vertex_id_t<_G>& vid) {
                              { _Fake_copy_init(contains_edge(__g, uid, vid)) }; // intentional ADL
                            };

  template <class _G>
  concept _Can_matrix_eval = _Has_class_or_enum_type<_G> && is_adjacency_matrix_v<_G> //
                             && ranges::sized_range<vertex_range_t<_G>>;

  template <class _G>
  concept _Can_id_eval = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, vertex_reference_t<_G> u, vertex_id_t<_G> uid, vertex_id_t<_G> vid) {
                              { _Fake_copy_init(find_vertex(__g, uid)) };
                              { _Fake_copy_init(edges(__g, u)) };
                              { _Fake_copy_init(find_vertex_edge(__g, u, vid)) };
                            };
  class _Cpo {
  private:
    enum class _St_ref { _None, _Non_member, _Matrix_eval, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(contains_edge(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                       declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_matrix_eval<_G>) {
        return {_St_ref::_Matrix_eval,
                noexcept(_Fake_copy_init(declval<vertex_id_t<_G>>() < ranges::size(vertices(declval<_G>()))))};
      } else if constexpr (_Can_id_eval<_G>) {
        return {_St_ref::_Auto_eval,
                noexcept(_Fake_copy_init(
                      find_vertex_edge(declval<_G>(), declval<vertex_reference_t<_G>>(), declval<vertex_id_t<_G>>()) !=
                      declval<vertex_iterator_t<_G>>()))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief Does an edge exist in the graph?
     * 
     * Complexity: O(1)
     * 
     * Default implementation: 
     *      uid < ranges::size(vertices(__g)) && vid < ranges::size(vertices(__g)), if is_adjacency_matrix_v<_G>
     *      find_vertex_edge(g, uid) != ranges::end(edges(g, uid)), otherwise
     * 
     * @tparam G The graph type.
     * @param g  A graph instance.
     * @param uv An edge reference.
     * @return An edge_descriptor with the source_id and target_id.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_id_t<_G>& uid, vertex_id_t<_G>& vid) const
          noexcept(_Choice_ref<_G&>._No_throw) -> bool {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return contains_edge(__g, uid, vid); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Matrix_eval) {
        return uid < ranges::size(vertices(__g)) && vid < ranges::size(vertices(__g));
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        auto ui = find_vertex(__g, uid);
        return find_vertex_edge(__g, *ui, vid) != ranges::end(edges(__g, *ui));
      } else {
        static_assert(_Always_false<_G>,
                      "contains_edge(g,uv) is not defined, or find_vertex_(g,uid) and source_id(g,uv) are not defined");
      }
    }
  };
} // namespace _Contains_edge

inline namespace _Cpos {
  inline constexpr _Contains_edge::_Cpo contains_edge;
}


//
// num_vertices(g,)    -> integral        default = size(vertices(g))
// num_vertices(g,pid) -> integral        default = size(vertices(g,pid))
//
namespace _NumVertices {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void num_vertices() = delete;              // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void num_vertices();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g) {
    { _Fake_copy_init(__g.num_vertices(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(num_vertices(__g)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = ranges::sized_range<vertex_range_t<_G>> //
                          && requires(_G&& __g) {
                               { _Fake_copy_init(ranges::size(vertices(__g))) };
                             };

  template <class _G>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, partition_id_t<_G> pid) {
                             { _Fake_copy_init(num_vertices(__g, pid)) }; // intentional ADL
                           };
  template <class _G>
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
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().num_vertices()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(num_vertices(declval<_G>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
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
      if constexpr (_Has_id_ADL<_G>) {
        return {
              _St_id::_Non_member,
              noexcept(_Fake_copy_init(num_vertices(declval<_G>(), declval<partition_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
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

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(u.degree(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, const vertex_reference_t<_G>& u) {
                              { _Fake_copy_init(degree(__g, u)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
                          && requires(_G&& __g, vertex_reference_t<_G> u) {
                               { _Fake_copy_init(edges(__g, u)) };
                             };

  template <class _G>
  concept _Has_id_ADL = _Has_class_or_enum_type<_G> //
                        && requires(_G&& __g, const vertex_id_t<_G>& uid) {
                             { _Fake_copy_init(degree(__g, uid)) }; // intentional ADL
                           };
  template <class _G>
  concept _Can_id_eval = ranges::sized_range<vertex_edge_range_t<_G>> //
                         && requires(_G&& __g, vertex_id_t<_G>& uid) {
                              { _Fake_copy_init(edges(__g, uid)) };
                            };

  class _Cpo {
  private:
    enum class _St_id { _None, _Non_member, _Auto_eval };
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_id> _Choose_id() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_id_ADL<_G>) {
        return {_St_id::_Non_member,
                noexcept(_Fake_copy_init(degree(declval<_G>(), declval<vertex_id_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_id_eval<_G>) {
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
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().degree(declval<_G>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member,
                noexcept(_Fake_copy_init(degree(declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
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
namespace _Vertex_value {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void vertex_value() = delete;              // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void vertex_value();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(u.vertex_value(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, vertex_reference_t<_G> u) {
                              { _Fake_copy_init(vertex_value(__g, u)) }; // intentional ADL
                            };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<vertex_reference_t<_G>>().vertex_value(
                                        declval<graph_reference_t<_G>>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(vertex_value(
                                            declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
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
          noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return u.vertex_value(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return vertex_value(__g, u); // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "vertex_value(g,u) must be defined for the graph");
      }
    }
  };
} // namespace _Vertex_value

inline namespace _Cpos {
  inline constexpr _Vertex_value::_Cpo vertex_value;
}

template <class G>
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_reference_t<G>>()));

//
// edge_value(g,uv) -> <<user-defined type>>
//
// edge_value_t<G> = decltype(edge_value(g,uv))
//
namespace _Edge_value {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void edge_value() = delete;                // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void edge_value();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
    { _Fake_copy_init(uv.edge_value(__g)) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g, edge_reference_t<_G> uv) {
                              { _Fake_copy_init(edge_value(__g, uv)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval =
        _Has_class_or_enum_type<_G> && ranges::forward_range<vertex_range_t<_G>> //
        && requires(edge_reference_t<_G> uv) { uv; }; // vertex is just a range, and edge type defined?

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(
                                        declval<edge_reference_t<_G>>().edge_value(declval<graph_reference_t<_G>>())))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {
              _St_ref::_Non_member,
              noexcept(_Fake_copy_init(edge_value(declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>()))}; // intentional ADL
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
     * Default implementation: uv (edge) if the vertex type is a range; otherwise it must be overridden by the graph
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv A vertex instance.
     * @return The number of outgoing edges of vertex uv.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
          noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return uv.edge_value(__g);
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return edge_value(__g, uv); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return uv; // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "edge_value(g,uv) must be defined for the graph");
      }
    }
  };
} // namespace _Edge_value

inline namespace _Cpos {
  inline constexpr _Edge_value::_Cpo edge_value;
}


// edge value types
template <class G>
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<edge_reference_t<G>>()));

//
// graph_value(g) -> <<user-defined type>>
//
// graph_value_t<G> = decltype(graph_value(g))
//
namespace _Graph_value {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void graph_value() = delete;               // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void graph_value();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g) {
    { _Fake_copy_init(__g.graph_value()) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(graph_value(__g)) }; // intentional ADL
                            };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<graph_reference_t<_G>>().graph_value()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(graph_value(declval<_G>())))}; // intentional ADL
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
    [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __g.graph_value();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return graph_value(__g); // intentional ADL
      } else {
        static_assert(_Always_false<_G>, "graph_value(g) must be defined for the graph");
      }
    }
  };
} // namespace _Graph_value

inline namespace _Cpos {
  inline constexpr _Graph_value::_Cpo graph_value;
}


namespace edgelist {
  namespace _Edges {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void edges() = delete;                   // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
    void edges();
#  endif                                     // ^^^ workaround ^^^

    template <class EL>
    concept _Has_ref_ADL = _Has_class_or_enum_type<EL> //
                           && ranges::forward_range<EL> && requires(EL&& el) {
                                { _Fake_copy_init(edges(el)) }; // intentional ADL
                              };

    class _Cpo {
    private:
      enum class _St_ref { _None, _Non_member };

      template <class EL>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        if constexpr (_Has_ref_ADL<EL>) {
          return {_St_ref::_Non_member, noexcept(_Fake_copy_init(edges(declval<EL>())))}; // intentional ADL
        } else {
          return {_St_ref::_None};
        }
      }

      template <class EL>
      static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<EL>();

    public:
      /**
     * @brief The number of outgoing edges of a vertex.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: none
     * 
     * @tparam G The graph type.
     * @param EL An edgelist instance.
     * @return The edgelist passed.
    */
      template <class EL>
      requires(_Choice_ref<EL&>._Strategy != _St_ref::_None)
      [[nodiscard]] constexpr auto&& operator()(EL&& el) const noexcept(_Choice_ref<EL&>._No_throw) {
        constexpr _St_ref _Strat_ref = _Choice_ref<EL&>._Strategy;

        if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return el; // intentional ADL
        } else {
          static_assert(_Always_false<EL>,
                        "edges(el) is not defined and the default implementation cannot be evaluated");
        }
      }
    };
  } // namespace _Edges

  inline namespace _Cpos {
    inline constexpr _Edges::_Cpo edges;
  }


  template <class EL>
  using edgelist_range_t = decltype(edges(declval<EL&&>()));

  template <class EL>
  using edgelist_iterator_t = ranges::iterator_t<edgelist_range_t<EL&&>>;

  template <class EL>
  using edge_t = ranges::range_value_t<edgelist_range_t<EL>>; // edge value type

  template <class EL>
  using edge_reference_t = ranges::range_reference_t<edgelist_range_t<EL>>; // edge reference type

  namespace _Source_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void source_id() = delete;               // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
    void source_id();
#  endif                                     // ^^^ workaround ^^^

    template <class _G>
    concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
      { _Fake_copy_init(uv.source_id(__g)) };
    };
    template <class _G>
    concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                           && requires(_G&& __g, const edge_reference_t<_G>& uv) {
                                { _Fake_copy_init(source_id(__g, uv)) }; // intentional ADL
                              };

    class _Cpo {
    private:
      enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_member<_G>) {
          return {_St_ref::_Member,
                  noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().source_id(declval<_G>())))};
        } else if constexpr (_Has_ref_ADL<_G>) {
          return {_St_ref::_Non_member, noexcept(_Fake_copy_init(source_id(
                                              declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
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
          return uv.source_id(__g);
        } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return source_id(__g, uv); // intentional ADL
        } else {
          static_assert(_Always_false<_G>, "source_id(g,uv) or g.source_id(uv) is not defined");
        }
      }
    };
  } // namespace _Source_id

  inline namespace _Cpos {
    inline constexpr _Source_id::_Cpo source_id;
  }

  template <class EL>
  using source_id_t = decltype(source_id(declval<EL&&>(), declval<edge_reference_t<EL>>()));


  namespace _Target_id {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void target_id() = delete;               // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
    void target_id();
#  endif                                     // ^^^ workaround ^^^

    template <class _G>
    concept _Has_ref_member = requires(_G&& __g, edge_reference_t<_G> uv) {
      { _Fake_copy_init(uv.target_id(__g)) };
    };
    template <class _G>
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
        if constexpr (_Has_ref_member<_G>) {
          return {_St_ref::_Member,
                  noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>().target_id(declval<_G>())))};
        } else if constexpr (_Has_ref_ADL<_G>) {
          return {_St_ref::_Non_member, noexcept(_Fake_copy_init(target_id(
                                              declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
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

  template <class EL>
  using target_id_t = decltype(target_id(declval<EL&&>(), declval<edge_reference_t<EL>>()));


  namespace _Edge_value {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void edge_value() = delete;              // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
    void edge_value();
#  endif                                     // ^^^ workaround ^^^

    template <class _G>
    concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                           && requires(_G&& __g, edge_reference_t<_G> uv) {
                                { _Fake_copy_init(edge_value(__g, uv)) }; // intentional ADL
                              };
    template <class _G>
    concept _Can_ref_eval = true;

    class _Cpo {
    private:
      enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_ADL<_G>) {
          return {_St_ref::_Non_member, noexcept(_Fake_copy_init(edge_value(
                                              declval<_G>(), declval<edge_reference_t<_G>>())))}; // intentional ADL
        } else if constexpr (_Can_ref_eval<_G>) {
          return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(declval<edge_reference_t<_G>>()))}; // intentional ADL
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
     * Default implementation: uv (edge) if the vertex type is a range; otherwise it must be overridden by the graph
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @param uv A vertex instance.
     * @return The number of outgoing edges of vertex uv.
    */
      template <class _G>
      requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, edge_reference_t<_G> uv) const
            noexcept(_Choice_ref<_G&>._No_throw) -> decltype(auto) {
        constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

        if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return edge_value(__g, uv); // intentional ADL
        } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
          return uv; // default
        } else {
          static_assert(_Always_false<_G>, "edge_value(g,uv) must be defined for the graph");
        }
      }
    };
  } // namespace _Edge_value

  inline namespace _Cpos {
    inline constexpr _Edge_value::_Cpo edge_value;
  }


  template <class EL>
  using edge_value_t = decltype(edge_value(declval<EL&&>(), declval<edge_reference_t<EL>>()));
} // namespace edgelist


// partition_count(g) -> ?   default = vertex_id_t<G>(1) when vertex_id_t<G> is integral, size_t(0) otherwise
//
namespace _Partition_count {
#  if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
  void partition_count() = delete;           // Block unqualified name lookup
#  else                                      // ^^^ no workaround / workaround vvv
  void partition_count();
#  endif                                     // ^^^ workaround ^^^

  template <class _G>
  concept _Has_ref_member = requires(_G&& __g, vertex_reference_t<_G> u) {
    { _Fake_copy_init(__g.partition_count()) };
  };
  template <class _G>
  concept _Has_ref_ADL = _Has_class_or_enum_type<_G> //
                         && requires(_G&& __g) {
                              { _Fake_copy_init(partition_count(__g)) }; // intentional ADL
                            };
  template <class _G>
  concept _Can_ref_eval = integral<vertex_id_t<_G>> //
                          && requires(_G&& __g) {
                               { _Fake_copy_init(vertex_id_t<_G>(1)) };
                             };

  class _Cpo {
  private:
    enum class _St_ref { _None, _Member, _Non_member, _Auto_eval };

    template <class _G>
    [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
      static_assert(is_lvalue_reference_v<_G>);
      if constexpr (_Has_ref_member<_G>) {
        return {_St_ref::_Member, noexcept(_Fake_copy_init(declval<_G>().partition_count()))};
      } else if constexpr (_Has_ref_ADL<_G>) {
        return {_St_ref::_Non_member, noexcept(_Fake_copy_init(partition_count(
                                            declval<_G>(), declval<vertex_reference_t<_G>>())))}; // intentional ADL
      } else if constexpr (_Can_ref_eval<_G>) {
        return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(vertex_id_t<_G>(1)))};
      } else {
        return {_St_ref::_None};
      }
    }

    template <class _G>
    static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G>();

  public:
    /**
     * @brief The number of partitions in a graph.
     * 
     * Complexity: O(1)
     * 
     * Default implementation: vertex_id_t<_G>(0)
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @return The number of partitions in the graph.
    */
    template <class _G>
    requires(_Choice_ref<_G&>._Strategy != _St_ref::_None)
    [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_ref<_G&>._No_throw) {
      constexpr _St_ref _Strat_ref = _Choice_ref<_G&>._Strategy;

      if constexpr (_Strat_ref == _St_ref::_Member) {
        return __g.partition_count();
      } else if constexpr (_Strat_ref == _St_ref::_Non_member) {
        return partition_count(__g); // intentional ADL
      } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
        return vertex_id_t<_G>(1); // default impl
      } else {
        static_assert(_Always_false<_G>,
                      "partition_count(g) is not defined and the default implementation cannot be evaluated");
      }
    }
  };
} // namespace _Partition_count

inline namespace _Cpos {
  inline constexpr _Partition_count::_Cpo partition_count;
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
#  if 0
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
#  endif

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

#endif //GRAPH_CPO_HPP
