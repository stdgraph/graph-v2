#pragma once

// (included from graph.hpp)
#include "tag_invoke.hpp"

#ifndef GRAPH_INVOKE_HPP
#  define GRAPH_INVOKE_HPP

namespace std::graph {

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
namespace tag_invoke {
  TAG_INVOKE_DEF(find_vertex);

  template <class G>
  concept _has_find_vertex_adl = requires(G&& g, vertex_id_t<G> uid) {
    { find_vertex(g, uid) };
  };
} // namespace tag_invoke

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
template <class G>
requires tag_invoke::_has_find_vertex_adl<G> || ranges::random_access_range<vertex_range_t<G>>
auto find_vertex(G&& g, vertex_id_t<G> uid) {
  if constexpr (tag_invoke::_has_find_vertex_adl<G>)
    return tag_invoke::find_vertex(g, uid);
  else if constexpr (ranges::random_access_range<vertex_range_t<G>>)
    return begin(vertices(g)) + static_cast<ranges::range_difference_t<vertex_range_t<G>>>(uid);
}

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
namespace tag_invoke {
  TAG_INVOKE_DEF(target_id);
}

/**
 * @brief Get the target vertex id of an edge.
 * 
 * Complexity: O(1)
 * 
 * Default implementation: n/a. This must be specialized for each graph type.
 * 
 * @tparam G The graph type.
 * @param g A graph instance.
 * @param uv An edge reference.
 * @return The target vertex id.
*/
template <class G>
auto target_id(G&& g, edge_reference_t<const G> uv) -> decltype(tag_invoke::target_id(g, uv)) {
  return tag_invoke::target_id(g, uv);
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

//
// degree(g,u) -> integral
//      default = size(edges(g,u)) if sized_range<vertex_edge_range_t<G>>
//
namespace tag_invoke {
  TAG_INVOKE_DEF(degree);

  template <class G>
  concept _has_degree_adl = requires(G&& g, vertex_reference_t<G> u) {
    { degree(g, u) };
  };
} // namespace tag_invoke

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
template <class G>
requires tag_invoke::_has_degree_adl<G> || ranges::sized_range<vertex_edge_range_t<G>>
auto degree(G&& g, vertex_reference_t<G> u) {
  if constexpr (tag_invoke::_has_degree_adl<G>)
    return tag_invoke::degree(g, u);
  else if constexpr (ranges::sized_range<vertex_edge_range_t<G>>) {
    return ranges::size(edges(g, u));
  }
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

template <class E>
auto edges(E&& el) -> decltype(tag_invoke::edges(el)) {
  return tag_invoke::edges(el);
}

template <class E>
using edgelist_range_t = decltype(std::graph::edgelist::edges(declval<E&&>()));

template <class E>
using edgelist_iterator_t = ranges::iterator_t<edgelist_range_t<E&&>>;

namespace tag_invoke {
  TAG_INVOKE_DEF(vertex_id_source);
}

template <class E>
auto vertex_id_source(E&& el, edgelist_iterator_t<E> e) {
    return tag_invoke::vertex_id_source(el, e);
}

template <class E>
using vertex_source_id_t = decltype(vertex_id_source(declval<E&&>(), declval<edgelist_iterator_t<E>>()));

namespace tag_invoke {
  TAG_INVOKE_DEF(vertex_id_target);
}

template <class E>
auto vertex_id_target(E&& el, edgelist_iterator_t<E> e) {
    return tag_invoke::vertex_id_target(el, e);
}

template <class E>
using vertex_target_id_t = decltype(vertex_id_target(declval<E&&>(), declval<edgelist_iterator_t<E>>()));


namespace tag_invoke {
  TAG_INVOKE_DEF(edge_value);
}

template <class E>
auto edge_value(E&& el, edgelist_iterator_t<E> e) {
    return tag_invoke::edge_value(el, e);
}

template <class E>
using edge_value_t = decltype(edge_value(declval<E&&>(), declval<edgelist_iterator_t<E>>()));
}

#  if 0
// bipartite idea
template <class EV     = tuple<int, double>,
          class VV     = tuple<int, double>,
          class GV     = void,
          integral VId = uint32_t,
          class Alloc  = allocator<uint32_t>>
class csr_partite_graph;

template<class G>
using partition_id_t = size_t;

template <class G>
partition_id_t<G> partition_id(G&& g, vertex_id_t<G> uid);

template <class G>
size_t partition_size(G&& g); // number of partitions in the graph

template<class G>
size_t partition_size(G&& g, partition_id_t<G> p); // number of vertices in the partition

template <class G>
vertex_range_t<G> vertices(G&& g, partition_id_t<G> p); // overloaded with vertices(g) (all)

template <class G, size_t Partition=0>
auto vertex_value(G&& g);

template <class G, size_t Partition=0>
auto edge_value(G&& g);

#  endif


} // namespace std::graph

#endif //GRAPH_INVOKE_HPP
