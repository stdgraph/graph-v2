#pragma once

/**
 * @defgroup general_concepts   Concepts
 * 
 * @defgroup graph_algorithms   Algorithms
 * @defgroup graph_views        Views / Adaptors
 * @defgroup graph_containers   Containers
 * @defgroup graph_construction Graph Construction
 * @defgroup graph_io           Graph I/O
 * @defgroup graph_utilities    Utilities
 * 
 * @defgroup graph_concepts Graph Concepts
 * @ingroup general_concepts
 * 
 * @defgroup graph_properties Graph Property Concepts
 * @ingroup general_concepts
 */

//#define ENABLE_EDGELIST_RANGE

#include <ranges>
#include <concepts>
#include <type_traits>
#include <stdexcept>

#include "graph_descriptors.hpp"
#include "detail/graph_cpo.hpp"


// Naming Conventions
//
// Template
// Parameter Variables      Description
// --------- -------------- ----------------------------------------------------
// G         g              Graph
// GV                       Graph Value (user-defined or void)
//
// V                        Vertex type
//           u,v,x,y        Vertex reference
// VId       uid,vid,seed   Vertex Id
// VV                       Vertex Value (user-defined or void)
// VR                       Vertex Range
// VI        ui,vi          Vertex Iterator
// VVF                      Vertex Value Function: vvf(u) -> value
//
// E                        Edge type
//           uv,vw          Edge reference
// EV                       Edge Value (user-defined or void)
// ER                       Edge Range
// EI        uvi,vwi        Edge iterator
// EVF       evf            Edge Value Function: evf(uv) -> value
//
// ELR       elr            Edge List Range; an arbitrary range where its values can be projected to be an edge_descriptor.
// Proj      proj           Projection function: proj(y) -> edge_descriptor<...>, where y is the value type of an ELR

#ifndef GRAPH_HPP
#  define GRAPH_HPP

namespace std::graph {

//
// graph concepts
//


/**
 * @ingroup graph_concepts
 * @brief Concept for a targeted edge.
 * 
 * A basic targeted edge has only the @c target_id(g,uv) function defined for it.
 * 
 * @tparam G The graph type.
 * @tparam E The edge type.
 */
template <class G, class E> // For exposition only
concept basic_targeted_edge = requires(G&& g, edge_reference_t<G> uv) { target_id(g, uv); };

template <class G, class E> // For exposition only
concept basic_sourced_edge = requires(G&& g, edge_reference_t<G> uv) { source_id(g, uv); };

template <class G, class E>                                        // For exposition only
concept basic_sourced_targeted_edge = basic_targeted_edge<G, E> && //
                                      basic_sourced_edge<G, E> &&  //
                                      requires(G&& g, edge_reference_t<G> uv) { edge_id(g, uv); };


/**
 * @ingroup graph_concepts
 * @brief Concept for a targeted edge.
 * 
 * A normal targeted edge has both the @c target_id(g,uv) and @c target(g,uv) function defined for it.
 * 
 * @tparam G The graph type.
 * @tparam E The edge type.
 */
template <class G, class E>                          // For exposition only
concept targeted_edge = basic_targeted_edge<G, E> && //
                        requires(G&& g, edge_reference_t<G> uv) { target(g, uv); };

template <class G, class E>                        // For exposition only
concept sourced_edge = basic_sourced_edge<G, E> && //
                       requires(G&& g, edge_reference_t<G> uv) { source(g, uv); };

template <class G, class E>                            // For exposition only
concept sourced_targeted_edge = targeted_edge<G, E> && //
                                sourced_edge<G, E> &&  //
                                requires(G&& g, edge_reference_t<G> uv) { edge_id(g, uv); };


/**
 * @ingroup graph_concepts
 * @brief Concept for a basic range of vertices.
 * 
 * A vertex range must be a sized range, at a minimum.
 *
 * Required functions that must also be defined include
 *  * @c vertices(g) that returns a range of vertices of a graph (via vertex_range_t<G>)
 *  * @c vertex_id(g,ui) that returns a vertex id for a graph and vertex iterator of the graph.
 * 
 * @tparam G The graph type.
 */
template <class G>                                                       // (exposition only)
concept _common_vertex_range = ranges::sized_range<vertex_range_t<G>> && //
                               requires(G&& g, vertex_iterator_t<G> ui) { vertex_id(g, ui); };

template <class G>                                                // For exposition only
concept vertex_range = _common_vertex_range<vertex_range_t<G>> && //
                       ranges::forward_range<vertex_range_t<G>>;

template <class G>                                                             // For exposition only
concept index_vertex_range = _common_vertex_range<vertex_range_t<G>> &&        //
                             ranges::random_access_range<vertex_range_t<G>> && //
                             integral<vertex_id_t<G>>;

// Will something like this be needed when vertices are in a map? TBD
//template <class G>
//concept key_vertex_range = _common_vertex_range<vertex_range_t<G>> && //
//                           ranges::bidirectional_range<vertex_range_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for a target edge range
*/
template <class G> // For exposition only
concept basic_targeted_edge_range = requires(G&& g, vertex_id_t<G> uid) {
  { edges(g, uid) } -> ranges::forward_range;
};

template <class G>                                            // For exposition only
concept targeted_edge_range = basic_targeted_edge_range<G> && //
                              requires(G&& g, vertex_reference_t<G> u) {
                                { edges(g, u) } -> ranges::forward_range;
                              };

//--------------------------------------------------------------------------------------------

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and without a vertex object.
 * 
 * @tparam G The graph type.
*/
template <class G>                                             // For exposition only
concept basic_adjacency_list = vertex_range<G> &&              //
                               basic_targeted_edge_range<G> && //
                               targeted_edge<G, edge_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and without a vertex object. It also requires that vertices are in a random-access
 * range and the vertex_id is integral.
 * 
 * @tparam G The graph type.
*/
template <class G>                                                   // For exposition only
concept basic_index_adjacency_list = index_vertex_range<G> &&        //
                                     basic_targeted_edge_range<G> && //
                                     basic_targeted_edge<G, edge_t<G>>;
/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and without a vertex object. The edge type has a source_id.
 * 
 * @tparam G The graph type.
*/
template <class G>                                                     // For exposition only
concept basic_sourced_adjacency_list = vertex_range<G> &&              //
                                       basic_targeted_edge_range<G> && //
                                       basic_sourced_targeted_edge<G, edge_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and without a vertex object. It also requires that vertices are in a random-access
 * range, the vertex_id is integral and the edge type has a source_id.
 * 
 * @tparam G The graph type.
*/
template <class G>                                                           // For exposition only
concept basic_sourced_index_adjacency_list = index_vertex_range<G> &&        //
                                             basic_targeted_edge_range<G> && //
                                             basic_sourced_targeted_edge<G, edge_t<G>>;

//--------------------------------------------------------------------------------------------

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and *with* a vertex object.
 * 
 * @tparam G The graph type.
*/
template <class G>                                 // For exposition only
concept adjacency_list = vertex_range<G> &&        //
                         targeted_edge_range<G> && //
                         targeted_edge<G, edge_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and *with* a vertex object. It also requires that vertices are in a random-access
 * range, the vertex_id is integral and the edge type has a source_id.
 * 
 * @tparam G The graph type.
*/
template <class G>                                       // For exposition only
concept index_adjacency_list = index_vertex_range<G> &&  //
                               targeted_edge_range<G> && //
                               targeted_edge<G, edge_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and *with* a vertex object. The edge type has a source_id.
 * 
 * @tparam G The graph type.
*/
template <class G>                                         // For exposition only
concept sourced_adjacency_list = vertex_range<G> &&        //
                                 targeted_edge_range<G> && //
                                 sourced_targeted_edge<G, edge_t<G>>;

/**
 * @ingroup graph_concepts
 * @brief Concept for an adjacency list graph.
 * 
 * A basic_adjacency_list list defines the minimal adjacency list concept with a vertex_id
 * and *with* a vertex object. It also requires that vertices are in a random-access
 * range, the vertex_id is integral and the edge type has a source_id.
 * 
 * @tparam G The graph type.
*/
template <class G>                                               // For exposition only
concept sourced_index_adjacency_list = index_vertex_range<G> &&  //
                                       targeted_edge_range<G> && //
                                       sourced_targeted_edge<G, edge_t<G>>;

//--------------------------------------------------------------------------------------------

#  ifdef ENABLE_EDGELIST_RANGE
template <class ELR>
concept basic_edgelist_range = ranges::forward_range<ELR> && negation_v<basic_adjacency_list<ELR>>;
template <class ELR>
concept edgelist_range = ranges::forward_range<ELR> && negation_v<adjacency_list<ELR>>;
#  endif

//
// property concepts
//

/**
 * @brief Type trait to determine if an edge is sourced.
 * 
 * Use @c is_sourced_edge<G,E>::value to deterine if the edge is sourced.
 * 
 * @tparam G The graph type.
 * @tparam E The edge type.
*/
template <class G, class E>
struct is_sourced_edge : public integral_constant<bool, sourced_edge<G, E>> {};

/**
 * @brief Type trait helper to determine if an edge is sourced.
 * 
 * Use @c is_sourced_edge_v<G,E> to deterine if the edge is sourced.
 * 
 * @tparam G The graph type.
 * @tparam E The edge type.
*/
template <class G, class E>
inline constexpr bool is_sourced_edge_v = is_sourced_edge<G, E>::value;

/**
 * @ingroup graph_properties
 * @brief Concept for the existance of degree function for graph G.
 * 
 * Returns true if degree(g) exists for graph G.
 * 
 * @tparam G The graph type
*/
template <class G> // For exposition only
concept has_degree = requires(G&& g, vertex_reference_t<G> u) {
  { degree(g, u) };
};

//
// find/contains concepts
//

/**
 * @ingroup graph_properties
 * @brief Concept for the existance of the find_vertex(g,uid) function for graph G.
 * 
 * Returns true if find_vertex(g,uid) exists for graph G.
 * 
 * @tparam G The graph type
*/
template <class G> // For exposition only
concept has_find_vertex = requires(G&& g, vertex_id_t<G> uid) {
  { find_vertex(g, uid) } -> forward_iterator;
};

/**
 * @ingroup graph_properties
 * @brief Concept for the existance of the find_vertex_edge(g,uid,vid) function for graph G.
 * 
 * Returns true if find_vertex_edge(g,u,vid) and find_vertex_edge(g,uid,vid) exists for graph G.
 * 
 * @tparam G The graph type
*/
template <class G> // For exposition only
concept has_find_vertex_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid, vertex_reference_t<G> u) {
  { find_vertex_edge(g, u, vid) } -> forward_iterator;
  { find_vertex_edge(g, uid, vid) } -> forward_iterator;
};

/**
 * @ingroup graph_properties
 * @brief Concept for the existance of the has_contains_edge(g,uid,vid) function for graph G.
 * 
 * Returns true if has_contains_edge(g,uid,vid) exists for graph G.
 * 
 * @tparam G The graph type
*/
template <class G> // For exposition only
concept has_contains_edge = requires(G&& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
  { contains_edge(g, uid, vid) } -> convertible_to<bool>;
};


/**
 * @ingroup graph_properties
 * @brief Override for an edge type where source and target are unordered
 * 
 * For instance, given:
 *  @code
 *      vertex_iterator_t<G> ui = ...;
 *      for(auto&& uv : edges(g,*ui)) ...
 *  @endcode
 *  if(source_id(g,u) != vertex_id(ui)) then target_id(g,u) == vertex_id(ui)
 *
 * Example:
 *  @code
 *  namespace my_namespace {
 *      template<class X>
 *      class my_graph { ... };
 *  }
 *  namespace std::graph {
 *     template<class X>
 *     inline constexpr bool is_undirected_edge_v<edge_t<my_namespace::my_graph<X>>> = true;
 *  }
 *  @endcode
 * 
 * @tparam name E The edge type with unordered source and target
 */
template <class E>
inline constexpr bool is_undirected_edge_v = false;

/**
 * @ingroup graph_properties
 * @ brief Specializable to define that a graph type has unordered edges.
 * 
 * Override for a graph type where source_id and target_id are unordered
 * on an edge so views and algorithms know to choose the correct target
 * based on where they came from.
 *
 * An unordered edge implies sourced_edge<G> is true so that an algorithm can
 * decide if source_id(g,uv) or target_id(g,uv) is the true target, based on
 * where the algorithm came from.
 *
 * If a graph container implementation has a run-time property of ordered or
 * unordered (e.g. it can't be determined at compile-time) then unordered_edge<G,E>
 * should be true_type. The only consequence is that an additional if is done to
 * check whether source_id or target_id is used for the target in this library.
 * The container implementation can still preserve its implementation of order,
 * assuming it always includes a source_id on the edge.
 *
 * Example:
 *  namespace my_namespace {
 *      template <class T>
 *      class my_graph { ... };
 *      template class< T>
 *      class my_edge { int src_id; int tgt_id; ... };
 *  }
 *  namespace std::graph {
 *     template<class T>
 *     struct define_unordered_edge<my_namespace::my_graph<T>, my_namespace::my_edge<T>> : public true_type {};
 *  }
 *
 * @tparam G Graph type
 */

template <class E>
struct define_unordered_edge : public false_type {}; // specialized for graph container edge

template <class G, class E>
struct is_unordered_edge : public conjunction<define_unordered_edge<E>, is_sourced_edge<G, E>> {};

template <class G, class E>
inline constexpr bool is_unordered_edge_v = is_unordered_edge<G, E>::value;

template <class G, class E> // For exposition only
concept unordered_edge = is_unordered_edge_v<G, E>;

//
// is_ordered_edge, ordered_edge
//
template <class G, class E>
struct is_ordered_edge : public negation<is_unordered_edge<G, E>> {};

template <class G, class E>
inline constexpr bool is_ordered_edge_v = is_ordered_edge<G, E>::value;

template <class G, class E> // For exposition only
concept ordered_edge = is_ordered_edge_v<G, E>;


//
// graph_error
//
class graph_error : public runtime_error {
public:
  explicit graph_error(const string& what_arg) : runtime_error(what_arg) {}
  explicit graph_error(const char* what_arg) : runtime_error(what_arg) {}
};


} // namespace std::graph

#endif //GRAPH_HPP
