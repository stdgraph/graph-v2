#pragma once

#include <concepts>
#include <vector>
#include <forward_list>
#include <list>
#include "graph/graph.hpp"
#include "container_utility.hpp"

// load_vertices(vrng, vvalue_fnc) -> [uid,vval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid],       [uid,vval]
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid, eval], [uid,vval]
//
// load_edges(initializer_list<[uid,vid]>
// load_edges(initializer_list<[uid,vid,eval]>
//
// [uid,vval]     <-- copyable_vertex<VId,VV>
// [uid,vid]      <-- copyable_edge<VId,void>
// [uid,vid,eval] <-- copyable_edge<VId,EV>
//

namespace std::graph::container {

//--------------------------------------------------------------------------------------------------
// dynamic_graph traits forward references
//

template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false>
struct vofl_graph_traits;

template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false>
struct vol_graph_traits;

template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false>
struct vov_graph_traits;


//--------------------------------------------------------------------------------------------------
// dynamic_graph forward references
//

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex;

template <class EV     = void,
          class VV     = void,
          class GV     = void,
          class VId    = uint32_t,
          bool Sourced = false,
          class Traits = vofl_graph_traits<EV, VV, GV, VId, Sourced>>
class dynamic_graph;

//--------------------------------------------------------------------------------------------------
// dynamic_graph traits declarations
//

/**
 * @ingroup graph_containers
 * @brief A dynamic graph traits definition for vector of forward-lists.
 * 
 * Vertices are stored in a @c vector<V>. Edges are stored in a @c forward_list<E>.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructible.
 * @tparam GV      @showinitializer =void The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
*/
template <class EV, class VV, class GV, class VId, bool Sourced>
struct vofl_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, vofl_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, vofl_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, vofl_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = forward_list<edge_type>;
};

/**
 * @ingroup graph_containers
 * @brief A dynamic graph traits definition for vector of lists.
 * 
 * Vertices are stored in a @c vector<V>. Edges are stored in a @c list<E>.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructible.
 * @tparam GV      @showinitializer =void The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
*/
template <class EV, class VV, class GV, class VId, bool Sourced>
struct vol_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, vol_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, vol_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, vol_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = list<edge_type>;
};

/**
 * @ingroup graph_containers
 * @brief A dynamic graph traits definition for vector of vectors.
 * 
 * Vertices are stored in a @c vector<V>. Edges are stored in a @c vector<E>.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructible.
 * @tparam GV      @showinitializer =void The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
*/
template <class EV, class VV, class GV, class VId, bool Sourced>
struct vov_graph_traits {
  using edge_value_type                      = EV;
  using vertex_value_type                    = VV;
  using graph_value_type                     = GV;
  using vertex_id_type                       = VId;
  constexpr inline const static bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, vov_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, vov_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, vov_graph_traits>;

  using vertices_type = vector<vertex_type>;
  using edges_type    = vector<edge_type>;
};

/**
 * @ingroup graph_containers
 * @brief A templated type alias to simplify definition of a dynamic_graph.
 * 
 * The @c Traits type has all the types and values required for dynamic_graph
 * template arguments, allowing a simpler definition that only takes the @c Traits
 * struct.
 * 
 * @tparam Traits The traits struct/class. (See examples above.)
*/
template <class Traits>
using dynamic_adjacency_graph = dynamic_graph<typename Traits::edge_value_type,
                                              typename Traits::vertex_value_type,
                                              typename Traits::graph_value_type,
                                              typename Traits::vertex_id_type,
                                              Traits::sourced,
                                              Traits>;

//--------------------------------------------------------------------------------------------------
// dynamic_edge
//

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c target_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_source for the source id and dynamic_edge_value for an edge value.
 * 
 * Unlike the other composable edge property classes, this class doesn't have an open for not
 * existing. The target id always exists. It could easily be merged into the dynamic_edge class,
 * but was kept separate for design symmatry with the other property classes.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge_target {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;

public:
  constexpr dynamic_edge_target(vertex_id_type targ_id) : target_id_(targ_id) {}

  constexpr dynamic_edge_target()                           = default;
  constexpr dynamic_edge_target(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target(dynamic_edge_target&&)      = default;
  constexpr ~dynamic_edge_target()                          = default;

  constexpr dynamic_edge_target& operator=(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target& operator=(dynamic_edge_target&&)      = default;

public:
  //constexpr vertex_id_type target_id() const { return target_id_; }
  //constexpr vertex_id_type source_id() const { return source_id_; }

private:
  vertex_id_type target_id_ = vertex_id_type();

private:
  // target_id(g,uv), target(g,uv)
  friend constexpr vertex_id_type target_id(const graph_type& g, const edge_type& uv) noexcept { return uv.target_id_; }

  friend constexpr vertex_type& target(graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_id_];
  }
  friend constexpr const vertex_type& target(const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.target_id_];
  }
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c source_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_value for an edge value.
 * 
 * A specialization for @c Sourced=false exists as an empty class so any call to @c source_id(g,uv) will
 * generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge_source {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;

public:
  constexpr dynamic_edge_source(vertex_id_type source_id) : source_id_(source_id) {}

  constexpr dynamic_edge_source()                           = default;
  constexpr dynamic_edge_source(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source(dynamic_edge_source&&)      = default;
  constexpr ~dynamic_edge_source()                          = default;

  constexpr dynamic_edge_source& operator=(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source& operator=(dynamic_edge_source&&)      = default;

public:
  //constexpr vertex_id_type source_id() const { return source_id_; }
  //constexpr vertex_id_type source_id() const { return source_id_; }

private:
  vertex_id_type source_id_ = vertex_id_type();

private:
  // source_id(g,uv), source(g)
  friend constexpr vertex_id_type source_id(const graph_type& g, const edge_type& uv) noexcept { return uv.source_id_; }

  friend constexpr vertex_type& source(graph_type& g, edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_id_];
  }
  friend constexpr const vertex_type& source_fn(const graph_type& g, const edge_type& uv) noexcept {
    return begin(vertices(g))[uv.source_id_];
  }
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c source_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_value for an edge value.
 * 
 * This specialization for @c Sourced=false is a simple placeholder that doesn't define anything.
 * If the user attempts to call to @c source_id(g,uv) or @c source(g,uv) will generate a compile 
 * error so they can resolve the incorrect usage.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 source(g,uv) will generate an error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, class Traits>
class dynamic_edge_source<EV, VV, GV, VId, false, Traits> {};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c edge_value(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_source for source id.
 * 
 * A specialization for @c EV=void exists as an empty class so any call to @c edge_value(g,uv) will
 * generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge_value {
public:
  using value_type  = EV;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type   = dynamic_edge_value<EV, VV, GV, VId, Sourced, Traits>;

public:
  constexpr dynamic_edge_value(const value_type& value) : value_(value) {}
  constexpr dynamic_edge_value(value_type&& value) : value_(std::move(value)) {}

  constexpr dynamic_edge_value()                          = default;
  constexpr dynamic_edge_value(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value(dynamic_edge_value&&)      = default;
  constexpr ~dynamic_edge_value()                         = default;

  constexpr dynamic_edge_value& operator=(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value& operator=(dynamic_edge_value&&)      = default;

public:
  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type&       edge_value(graph_type& g, edge_type& uv) noexcept { return uv.value_; }
  friend constexpr const value_type& edge_value(const graph_type& g, const edge_type& uv) noexcept { return uv.value_; }
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c edge_value(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_source for source id.
 * 
 * This specialization for @c EV=void is a simple placeholder that doesn't define anything.
 * Any call to @c edge_value(g,uv) will generate a compile error so the user can resolve the
 * incorrect usage.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge_value<void, VV, GV, VId, Sourced, Traits> {};


/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * The following combinations of EV and Sourced are supported in @c dynamic_edge specializations.
 * The only difference between them are the values taken by the constructors to match the 
 * inclusion/exclusion of the source Id and/or value properties.
 *   *  < @c EV not void, @c Sourced=true >  (this implementation)
 *   *  < @c EV not void, @c Sourced=false > 
 *   *  < @c EV is  void, @c Sourced=true >
 *   *  < @c EV is  void, @c Sourced=false >
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge
      : public dynamic_edge_target<EV, VV, GV, VId, Sourced, Traits>
      , public dynamic_edge_source<EV, VV, GV, VId, Sourced, Traits>
      , public dynamic_edge_value<EV, VV, GV, VId, Sourced, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, VId, Sourced, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, VId, Sourced, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, VId, Sourced, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id)
        : base_target_type(target_id), base_source_type(source_id) {}
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id, const value_type& val)
        : base_target_type(target_id), base_source_type(source_id), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id, value_type&& val)
        : base_target_type(target_id), base_source_type(source_id), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV=void and @c Sourced=true.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [true] Source id is stored on the edge. Use of @c source_id(g,uv) and
 *                 @c source(g,uv) will return a value without error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, class Traits>
class dynamic_edge<void, VV, GV, VId, true, Traits>
      : public dynamic_edge_target<void, VV, GV, VId, true, Traits>
      , public dynamic_edge_source<void, VV, GV, VId, true, Traits>
      , public dynamic_edge_value<void, VV, GV, VId, true, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, VId, true, Traits>;
  using base_source_type = dynamic_edge_source<void, VV, GV, VId, true, Traits>;
  using base_value_type  = dynamic_edge_value<void, VV, GV, VId, true, Traits>;

  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<void, VV, GV, VId, true, Traits>;
  using vertex_type    = dynamic_vertex<void, VV, GV, VId, true, Traits>;
  using edge_type      = dynamic_edge<void, VV, GV, VId, true, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type source_id, vertex_id_type target_id)
        : base_target_type(target_id), base_source_type(source_id) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};


/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV!=void and @c Sourced=false.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, class Traits>
class dynamic_edge<EV, VV, GV, VId, false, Traits>
      : public dynamic_edge_target<EV, VV, GV, VId, false, Traits>
      , public dynamic_edge_source<EV, VV, GV, VId, false, Traits>
      , public dynamic_edge_value<EV, VV, GV, VId, false, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, VId, false, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, VId, false, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, VId, false, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, false, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, false, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, false, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type targ_id) : base_target_type(targ_id) {}
  constexpr dynamic_edge(vertex_id_type targ_id, const value_type& val)
        : base_target_type(targ_id), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type targ_id, value_type&& val)
        : base_target_type(targ_id), base_value_type(std::move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV=void and @c Sourced=false.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, class Traits>
class dynamic_edge<void, VV, GV, VId, false, Traits>
      : public dynamic_edge_target<void, VV, GV, VId, false, Traits>
      , public dynamic_edge_source<void, VV, GV, VId, false, Traits>
      , public dynamic_edge_value<void, VV, GV, VId, false, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, VId, false, Traits>;
  using vertex_id_type   = VId;
  using value_type       = void;
  using graph_type       = dynamic_graph<void, VV, GV, VId, false, Traits>;
  using vertex_type      = dynamic_vertex<void, VV, GV, VId, false, Traits>;
  using edge_type        = dynamic_edge<void, VV, GV, VId, false, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type target_id) : base_target_type(target_id) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;
};

//--------------------------------------------------------------------------------------------------
// dynamic_vertex
//

/**
 * @ingroup graph_containers
 * @brief Base implementation of a vertex that provides access to outgoing edges on the vertex.
 * 
 * Edges are stored in a container on the vertex.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex_base {
public:
  using vertex_id_type = VId;
  using value_type     = VV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex_base()                           = default;
  constexpr dynamic_vertex_base(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base(dynamic_vertex_base&&)      = default;
  constexpr ~dynamic_vertex_base()                          = default;

  constexpr dynamic_vertex_base& operator=(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base& operator=(dynamic_vertex_base&&)      = default;

  constexpr dynamic_vertex_base(allocator_type alloc) : edges_(alloc) {}

public:
  constexpr edges_type&       edges() noexcept { return edges_; }
  constexpr const edges_type& edges() const noexcept { return edges_; }

  constexpr auto begin() noexcept { return edges_.begin(); }
  constexpr auto begin() const noexcept { return edges_.begin(); }
  constexpr auto cbegin() const noexcept { return edges_.begin(); }

  constexpr auto end() noexcept { return edges_.end(); }
  constexpr auto end() const noexcept { return edges_.end(); }
  constexpr auto cend() const noexcept { return edges_.end(); }

private:
  edges_type edges_;

private: // tag_invoke properties
  friend constexpr edges_type& tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, vertex_type& u) {
    return u.edges_;
  }
  friend constexpr const edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u.edges_;
  }

  friend constexpr typename edges_type::iterator
  find_vertex_edge(graph_type& g, vertex_id_type uid, vertex_id_type vid) {
    return ranges::find(g[uid].edges_, [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  }
  friend constexpr typename edges_type::const_iterator
  find_vertex_edge(const graph_type& g, vertex_id_type uid, vertex_id_type vid) {
    return ranges::find(g[uid].edges_, [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  }
};


/**
 * @ingroup graph_containers
 * @brief Implementation of a vertex class of dynamic_graph that provides access to outgoing edges 
 * and the vertex value.
 * 
 * A specialization of this class exists for VV=void that excludes the vertex value. When that is used,
 * Any attempt to use @c vertex_value(g,u) will generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex : public dynamic_vertex_base<EV, VV, GV, VId, Sourced, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_id_type = VId;
  using value_type     = remove_cvref_t<VV>;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex(const value_type& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(value) {}
  constexpr dynamic_vertex(value_type&& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(move(value)) {}
  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}

  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

public:
  using base_type::edges;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // tag_invoke properties
  friend constexpr value_type&       vertex_value(graph_type& g, vertex_type& u) { return u.value_; }
  friend constexpr const value_type& vertex_value(const graph_type& g, const vertex_type& u) { return u.value_; }
};


/**
 * @ingroup graph_containers
 * @brief Implementation of a vertex class of dynamic_graph that provides access to outgoing edges.
 * 
 * This is a specialization for @c VV=void that excludes the @c vertex_value(g,u) definition. When it is used,
 * a compile error will be generated so the user can resolve the incorrect usage.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      [void] No user value is stored on the vertex and calling @c vertex_value(g,u)
 *                 will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex<EV, void, GV, VId, Sourced, Traits>
      : public dynamic_vertex_base<EV, void, GV, VId, Sourced, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, void, GV, VId, Sourced, Traits>;
  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<EV, void, GV, VId, Sourced, Traits>;
  using vertex_type    = dynamic_vertex<EV, void, GV, VId, Sourced, Traits>;
  using edge_type      = dynamic_edge<EV, void, GV, VId, Sourced, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}
};

/**-------------------------------------------------------------------------------------------------
 * @ingroup graph_containers
 * @brief dynamic_graph_base defines the core implementation for a graph with a variety 
 * characteristics including edge, vertex and graph value types, whether edges are sourced or not, 
 * the vertex id type, and selection of the containers used for vertices and edges.
 * 
 * This class includes the vertices and functions to access them, and the constructors and 
 * functions to load the vertices and edges into the graph.
 * 
 * dynamic_graph provides the interface that includes or excludes (GV=void) a graph value.
 * dynamic_graph_base has the core implementation for the graph.
 * 
 * No additional space is used if the edge value type (EV) or vertex value type (VV) is void.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     The type used for the vertex id.
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph_base {
public: // types
  using graph_type   = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using graph_traits = Traits;

  using vertex_id_type        = VId;
  using vertex_type           = dynamic_vertex<EV, VV, GV, VId, Sourced, Traits>;
  using vertices_type         = typename Traits::vertices_type;
  using vertex_allocator_type = typename vertices_type::allocator_type;
  using size_type             = typename vertices_type::size_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;

public: // Construction/Destruction/Assignment
  constexpr dynamic_graph_base()                          = default;
  constexpr dynamic_graph_base(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base(dynamic_graph_base&&)      = default;
  constexpr ~dynamic_graph_base()                         = default;

  constexpr dynamic_graph_base& operator=(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base& operator=(dynamic_graph_base&&)      = default;

  /**
   * @brief Construct an empty graph using the allocator passed.
   * 
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph_base(vertex_allocator_type alloc) : vertices_(alloc) {}

  /**
   * @brief Construct the graph using edge and vertex ranges.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * The value_type of @c vrng must be converted to @c copyable_vertex_t<G> before it can be
   * added to the graph, which is done using the @c vproj function. If the value_type of
   * @c vrng is already copyable_vertex_t<G>, @c std::identity can be used instead. @c copyable_vertex_t<G>
   * will have a vertex value member if a vertex value type has been defined for the graph (e.g.
   * @c VV is not @c void).
   * 
   * @tparam ERng  The edge data range.
   * @tparam VRng  The vertex data range.
   * @tparam EProj A function type to convert the ERng value_type to copyable_edge_t<G>.
   *               If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * @tparam VProj A projection function type to convert the VRng value_type to copyable_vertex_t<G>
   *               If VRng value_type is already copyable_vertex_t<G>, identify can be used.
   * 
   * @param erng  The edge values.
   * @param vrng  The vertex values.
   * @param eproj The projection function that converts the ERng value_type to copyable_edge_t<G>, 
   *              or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param vproj The projection function that converts the ERng value_type to copyable_vertex_t<G>, or 
   *              identity() if VRng value_type is already copyable_vertex_t<G>.
   * @param alloc The allocator used for vertices and edges containers.
  */
  template <class ERng, class VRng, class EProj = identity, class VProj>
  dynamic_graph_base(ERng&& erng, VRng&& vrng, EProj eproj, VProj vproj, vertex_allocator_type alloc)
        : vertices_(alloc) {
    load_vertices(vrng, vproj);
    load_edges(vertices_.size(), 0, erng, eproj);
  }

  /**
   * @brief Construct the graph using an edge range.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * Edges are scanned to determine the largest vertex id needed.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @tparam ERng  The edge data range.
   * @tparam EProj A projection function type to convert the ERng value_type to copyable_edge_t<G>.
   *               If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * 
   * @param erng  The edge values.
   * @param eproj The projection function that converts the ERng value_type to copyable_edge_t<G>, 
   *              or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param alloc The allocator used for vertices and edges containers.
  */
  template <class ERng, class EProj>
  dynamic_graph_base(ERng&& erng, EProj eproj, vertex_allocator_type alloc) : vertices_(alloc) {

    load_edges(move(erng), eproj);
  }

  /**
   * @brief Construct the graph using a vertex count and an edge range.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * Edges are NOT scanned to determine the largest vertex id needed.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @tparam ERng  The edge data range.
   * @tparam EProj A function type to convert the ERng value_type to copyable_edge_t<G>.
   *               If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * 
   * @param vertex_count The number of vertices to create.
   * @param erng         The edge values.
   * @param eproj        A function that converts the ERng value_type to copyable_edge_t<G>, 
   *                     or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param alloc        The allocator used for vertices and edges containers.
  */
  template <class ERng, class EProj>
  dynamic_graph_base(size_type vertex_count, ERng&& erng, EProj eproj, vertex_allocator_type alloc) : vertices_(alloc) {

    load_edges(vertex_count, 0, move(erng), eproj);
  }

  /**
   * @brief Construct the graph using a intializer list of copyable edges.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph_base(const initializer_list<copyable_edge_t<VId, EV>>& il,
                     edge_allocator_type                               alloc = edge_allocator_type())
        : vertices_(alloc) {
    size_t last_id = 0;
    for (auto&& e : il)
      last_id = max(last_id, static_cast<size_t>(max(e.source_id, e.target_id)));
    resize_vertices(last_id + 1);
    load_edges(il);
  }

public: // Load operations
  /**
   * @brief Load vertices and copy vertex values into the graph.
   * 
   * The id assigned in the returned @c copyable_vertex_t<VId,VV> by @vproj from @vproj is used to look up 
   * the vertex in the internal vertices container and the vertex value is copied to the vertex's value. This 
   * implies the following consequences/behavior:
   *  * Vertex values must be default-constructable
   *  * @c vrng entries don't need to be assigned consecutive vertex id's.
   *  * If the same id is used for different entries of @c vrng (or subsequent calls to @c load_vertices())
   *    then there is a conflict as to which one should be used. In this case, the last one assigned is used.
   * 
   * @tparam VRng        Type of the range of vertices to be loaded.
   * @tparam VProj       Type of projection function to convert a @c VRng value type to a @c copyable_vertex_t.
   *                     If the @c VRng value type is already a copyable_vertex_t then std::identity can be used.
   * @param vrng         The range of vertices to be loaded.
   * @param vproj        The projection function to convert the @c vrng value type to a @c copyable_vertex_t.
   *                     if the @c vrng value type is already a copyable_vertex_t then std::identity() can be used.
   * @param vertex_count The number of vertices to resize the internal vertices container to, if available. This
   *                     can be used to pre-extend the number of vertices beyond those supplied in @c vrng.
  */
  template <class VRng, class VProj = identity>
  void load_vertices(const VRng& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (ranges::sized_range<VRng> && resizable<vertices_type>) {
      vertex_count = max(vertex_count, ranges::size(vertices_));
      resize_vertices(max(vertex_count, ranges::size(vrng)));
    }
    for (auto&& v : vrng) {
      auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
      size_t k           = static_cast<size_t>(id);
      if constexpr (ranges::random_access_range<vertices_type>)
        assert(k < vertices_.size());
      vertices_[k].value() = value;
    }
  }

  /**
   * @brief Load vertices and move vertex values into the graph.
   * 
   * The id assigned in the returned @c copyable_vertex_t<VId,VV> by @vproj from @vproj is used to look up 
   * the vertex in the internal vertices container and the vertex value is moved to the vertex's value. This 
   * implies the following consequences/behavior:
   *  * Vertex values must be default-constructable
   *  * @c vrng entries don't need to be assigned consecutive vertex id's.
   *  * If the same id is used for different entries of @c vrng (or subsequent calls to @c load_vertices())
   *    then there is a conflict as to which one should be used. In this case, the last one assigned is used.
   * 
   * TODO This is the wrong design for move vs. copy because we're not moving the container, only some of the
   * contents of the container. It would be better to have a policy parameter (type, run-time or both) or to
   * have functions with different names to make it clear. Examples might be @c copy_vertex_values & 
   * @c move_vertex_values.
   * 
   * @tparam VRng        Type of the range of vertices to be loaded.
   * @tparam VProj       Type of projection function to convert a @c VRng value type to a @c copyable_vertex_t.
   *                     If the @c VRng value type is already a copyable_vertex_t then std::identity can be used.
   * @param vrng         The range of vertices to be loaded.
   * @param vproj        The projection function to convert the @c vrng value type to a @c copyable_vertex_t.
   *                     if the @c vrng value type is already a copyable_vertex_t then std::identity() can be used.
   * @param vertex_count The number of vertices to resize the internal vertices container to, if available. This
   *                     can be used to pre-extend the number of vertices beyond those supplied in @c vrng.
  */
  template <class VRng, class VProj = identity>
  void load_vertices(VRng&& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (ranges::sized_range<VRng> && resizable<vertices_type>) {
      vertex_count = max(vertex_count, ranges::size(vertices_));
      resize_vertices(max(vertex_count, ranges::size(vrng)));
    }
    for (auto&& v : vrng) {
      auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
      size_t k           = static_cast<size_t>(id);
      if constexpr (ranges::random_access_range<vertices_type>)
        assert(k < vertices_.size());
      vertices_[k].value() = std::move(value);
    }
  }


  /**
   * @brief Load edges and copy edge values into the graph.
   * 
   * Edge values are appended to the container for a vertex. No check is made for duplicate entries.
   * 
   * If edge values have been defined for the graph they will be copied into the graph's edge value.
   * 
   * If the @c source_id or @c target_id of the returned @c copyable_edge_t<VId,EV> by @c eproj is
   * larger than the number of vertices an exception will be thrown.
   * 
   * TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   *
   * @tparam ERng  Range type of source data for edges
   * @tparam EProj Projection function type that converts @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already a @c copyable_edge_t<VId,EV> then @c std::identity can 
   *               be used instead.
   *
   * @param erng            The source range of edge data.
   * @param eproj           The projection function to convert an @c erng value type to a @c copyable_edge_t<VId,EV>.
   *                        If the @c erng value type is already a @c copyable_edge_t<VId,EV> then std::identity()
   *                        can be used instead.
   * @param vertex_count    If larger than the existing number of vertices then the number of vertices will be grown 
   *                        to match @c vertex_count, if applicable. Vertex values need to be movable.
   * @param edge_count_hint (not used)
  */
  template <class ERng, class EProj = identity>
  void load_edges(const ERng& erng, EProj eproj = {}, size_type vertex_count = 0, size_type edge_count_hint = 0) {
    if constexpr (resizable<vertices_type>) {
      if (vertices_.size() < vertex_count)
        vertices_.resize(vertex_count, vertex_type(vertices_.get_allocator()));
    }

    // add edges
    for (auto&& edge_data : erng) {
      auto&& e = eproj(edge_data); //views::copyable_edge_t<VId, EV>

      if (static_cast<size_t>(e.source_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("source id exceeds the number of vertices in load_edges");
      }
      if (static_cast<size_t>(e.target_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("target id exceeds the number of vertices in load_edges");
      }

      auto&& edge_adder = push_or_insert(vertices_[e.source_id].edges());
      if constexpr (Sourced) {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(std::move(e.source_id), std::move(e.target_id)));
        } else {
          edge_adder(edge_type(std::move(e.source_id), std::move(e.target_id), std::move(e.value)));
        }
      } else {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(std::move(e.target_id)));
        } else {
          edge_adder(edge_type(std::move(e.target_id), std::move(e.value)));
        }
      }
    }
  }

  /**
   * @brief Load edges and move edge values into the graph.
   * 
   * Edge values are appended to the container for a vertex. No check is made for duplicate entries.
   * 
   * If edge values have been defined for the graph they will be copied into the graph's edge value.
   * 
   * If the @c source_id or @c target_id of the returned @c copyable_edge_t<VId,EV> by @c eproj is
   * larger than the number of vertices an exception will be thrown.
   * 
   * TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   * 
   * TODO: This is the wrong design because we're not moving the @c erng into the graph, we're just
   *       moving edge values. Alternative designs include using a policy parameter (type and/or run-time)
   *       or different function names, such as append_edges_copy_values and append_edges_move_values, or
   *       simple copy_edge_values and move_edge_values.
   *
   * @tparam ERng  Range type of source data for edges
   * @tparam EProj Projection function type that converts @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already a @c copyable_edge_t<VId,EV> then @c std::identity can 
   *               be used instead.
   *
   * @param erng            The source range of edge data.
   * @param eproj           The projection function to convert an @c erng value type to a @c copyable_edge_t<VId,EV>.
   *                        If the @c erng value type is already a @c copyable_edge_t<VId,EV> then std::identity()
   *                        can be used instead.
   * @param vertex_count    If larger than the existing number of vertices then the number of vertices will be grown 
   *                        to match @c vertex_count, if applicable. Vertex values need to be movable.
   * @param edge_count_hint (not used)
  */
  template <class ERng, class EProj = identity>
  void load_edges(ERng&& erng, EProj eproj = {}, size_type vertex_count = 0, size_type edge_count_hint = 0) {
    if constexpr (resizable<vertices_type>) {
      if (vertices_.size() < vertex_count)
        vertices_.resize(vertex_count, vertex_type(vertices_.get_allocator()));
    }

    // add edges
    for (auto&& edge_data : erng) {
      auto&& e = eproj(edge_data); //views::copyable_edge_t<VId, EV>

      if (static_cast<size_t>(e.source_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("source id exceeds the number of vertices in load_edges");
      }
      if (static_cast<size_t>(e.target_id) >= vertices_.size()) {
        assert(false);
        throw runtime_error("target id exceeds the number of vertices in load_edges");
      }

      auto&& edge_adder = push_or_insert(vertices_[e.source_id].edges());
      if constexpr (Sourced) {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(std::move(e.source_id), std::move(e.target_id)));
        } else {
          edge_adder(edge_type(std::move(e.source_id), std::move(e.target_id), std::move(e.value)));
        }
      } else {
        if constexpr (is_void_v<EV>) {
          edge_adder(edge_type(std::move(e.target_id)));
        } else {
          edge_adder(edge_type(std::move(e.target_id), std::move(e.value)));
        }
      }
    }
  }

#if 0
  /// @todo ERng not a forward_range because CSV reader doesn't conform to be a forward_range
  template <class ERng, class EProj = identity>
  void load_edges(ERng&& erng, EProj eproj = {}) {
    // Nothing to do?
    if (erng.begin() == erng.end())
      return;

    // Evaluate max vertex id needed
    size_type       erng_size   = 0;
    vertex_id_type max_row_idx = 0;
    for (auto& edge_data : erng) {
      auto&& e    = eproj(edge_data);
      max_row_idx = max(max_row_idx, max(e.source_id, e.target_id));
      ++erng_size;
    }

    load_edges(static_cast<size_type>(max_row_idx + 1), erng_size, erng, eproj);
  }
#endif

public: // Properties
  constexpr auto begin() noexcept { return vertices_.begin(); }
  constexpr auto begin() const noexcept { return vertices_.begin(); }
  constexpr auto cbegin() const noexcept { return vertices_.begin(); }

  constexpr auto end() noexcept { return vertices_.end(); }
  constexpr auto end() const noexcept { return vertices_.end(); }
  constexpr auto cend() const noexcept { return vertices_.end(); }

  constexpr auto size() const noexcept { return vertices_.size(); }

  constexpr typename vertices_type::value_type&       operator[](size_type i) noexcept { return vertices_[i]; }
  constexpr const typename vertices_type::value_type& operator[](size_type i) const noexcept { return vertices_[i]; }

public: // Operations
  void reserve_vertices(size_type count) {
    if constexpr (reservable<vertices_type>) // reserve if we can; otherwise ignored
      vertices_.reserve(count);
  }
  void reserve_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

  void resize_vertices(size_type count) {
    if constexpr (resizable<vertices_type>) // resize if we can; otherwise ignored
      vertices_.resize(count);
  }
  void resize_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

private: // Member Variables
  vertices_type vertices_;

private: // tag_invoke properties
  friend constexpr vertices_type& tag_invoke(::std::graph::tag_invoke::vertices_fn_t, dynamic_graph_base& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_type& tag_invoke(::std::graph::tag_invoke::vertices_fn_t,
                                                   const dynamic_graph_base& g) {
    return g.vertices_;
  }

  friend vertex_id_type vertex_id(const dynamic_graph_base& g, typename vertices_type::const_iterator ui) {
    return static_cast<vertex_id_type>(ui - g.vertices_.begin());
  }

  friend constexpr edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid].edges();
  }
  friend constexpr const edges_type&
  tag_invoke(::std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid].edges();
  }
};

/**
 * @ingroup graph_containers
 * @brief dynamic_graph defines a graph with a variety characteristics including edge, vertex 
 * and graph value types, whether edges are sourced or not, the vertex id type, and selection 
 * of the containers used for vertices and edges.
 * 
 * dynamic_graph provides the interface that includes or excludes (GV=void) a graph value.
 * dynamic_graph_base has the core implementation for the graph.
 * 
 * No additional space is used if the edge value type (EV), vertex value type (VV) or graph
 * value type (GV) is void.
 * 
 * Only random access vertex container types (e.g. @c vector<V> & @c deque<V>) are supported at the 
 * moment for vertex containers in @c Traits. It is the goal to support bidirectional container types 
 * (e.g. map<VId,V> & unordered_map<VId,V>) in the future.
 * 
 * Forward list containers are supported for edge containers. @c vector<E>, @c deque<E>, @c list<E>
 * and @c forward_list<E> have been tested. Additional work is required to support set<E> and map<VId,E>.
 * 
 * Only integral VId types are supported at the moment. It is the goal to support any type that can be
 * used as a key to find a vertex in the vertices container.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructible.
 * @tparam GV      @showinitializer =void The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
 * @tparam Traits  @showinitializer =vofl_graph_traits<EV,VV,GV,Sourced,VId> Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph : public dynamic_graph_base<EV, VV, GV, VId, Sourced, Traits> {

public: // Types & Constants
  using base_type      = dynamic_graph_base<EV, VV, GV, VId, Sourced, Traits>;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = GV;
  using allocator_type = typename Traits::vertices_type::allocator_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, VId, Sourced, Traits>;

  constexpr inline const static bool sourced = Sourced;

public: // Construction/Destruction/Assignment
  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  // gv&,  alloc
  // gv&&, alloc
  //       alloc

  /**
   * @brief Construct a dynamic_graph with an empty collection of vertices.
   * 
   * The graph value is initialized using the default constructor.
   * 
   * @param alloc Construct the vertices container with this allocator. Edges will use this allocator also.
  */
  dynamic_graph(allocator_type alloc) : base_type(alloc) {}
  /**
   * @brief Construct a dynamic_graph with an empty collection of vertices that .
   * 
   * @param gv    Initialize the graph value with a copy of this value.
   * @param alloc Construct the vertices container with this allocator. Edges will use this allocator also.
  */
  dynamic_graph(const GV& gv, allocator_type alloc = allocator_type()) : base_type(alloc), value_(gv) {}
  /**
   * @brief Construct a dynamic_graph with an empty collection of vertices that .
   * 
   * @param gv    Initialize the graph value by moving this value to it.
   * @param alloc Construct the vertices container with this allocator. Edges will use this allocator also.
  */
  dynamic_graph(GV&& gv, allocator_type alloc = allocator_type()) : base_type(alloc), value_(move(gv)) {}


  //       erng, eproj, vproj, alloc
  // gv&,  erng, eproj, vproj, alloc
  // gv&&, erng, eproj, vproj, alloc

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The graph value is default-constructed.
   * 
   * The vertices container is pre-extended to accomodate the largest vertex id, if applicable. The
   * @c vrng size is used to determine the number of vertices and then also is used as a container of values
   * to assign to the internal vertices. Given that, @c vrng elements don't have to be ordered sequentially
   * by vertex id; the vertex id returned by @c vproj is used to find an existing vertex before assigning
   * the value to it.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam VRng  The range type of vertex data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * @tparam VProj The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   *               If the @c VRng value type is already @c copyable_vertex_t<VId,VV> then @c identity can be used instead.
   * 
   * @param erng  The edge data range used to create new edges.
   * @param vrng  The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param vproj The vertex projection function used to convert the @c vrng value type to @c copyable_vertex_t<VId,VV>.
   *              If the @c vrng value type is already @c copyable_vertex_t<VId,VV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  dynamic_graph(
        const ERng& erng, const VRng& vrng, EProj eproj = {}, VProj vproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc) {}

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The graph value is intitialized by copying from the gv parameter.
   * 
   * The vertices container is pre-extended to accomodate the largest vertex id, if applicable. The
   * @c vrng size is used to determine the number of vertices and then also is used as a container of values
   * to assign to the internal vertices. Given that, @c vrng elements don't have to be ordered sequentially
   * by vertex id; the vertex id returned by @c vproj is used to find an existing vertex before assigning
   * the value to it.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam VRng  The range type of vertex data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * @tparam VProj The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   *               If the @c VRng value type is already @c copyable_vertex_t<VId,VV> then @c identity can be used instead.
   * 
   * @param gv    The value to copy to the internal graph value.
   * @param erng  The edge data range used to create new edges.
   * @param vrng  The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param vproj The vertex projection function used to convert the @c vrng value type to @c copyable_vertex_t<VId,VV>.
   *              If the @c vrng value type is already @c copyable_vertex_t<VId,VV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  dynamic_graph(const GV&      gv,
                const ERng&    erng,
                const VRng&    vrng,
                EProj          eproj = {},
                VProj          vproj = {},
                allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc), value_(gv) {}

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The graph value is intitialized by moving from the gv parameter.
   * 
   * The vertices container is pre-extended to accomodate the largest vertex id, if applicable. The
   * @c vrng size is used to determine the number of vertices and then also is used as a container of values
   * to assign to the internal vertices. Given that, @c vrng elements don't have to be ordered sequentially
   * by vertex id; the vertex id returned by @c vproj is used to find an existing vertex before assigning
   * the value to it.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam VRng  The range type of vertex data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * @tparam VProj The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   *               If the @c VRng value type is already @c copyable_vertex_t<VId,VV> then @c identity can be used instead.
   * 
   * @param gv    The value to move to the internal graph value.
   * @param erng  The edge data range used to create new edges.
   * @param vrng  The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param vproj The vertex projection function used to convert the @c vrng value type to @c copyable_vertex_t<VId,VV>.
   *              If the @c vrng value type is already @c copyable_vertex_t<VId,VV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj, class VRng, class VProj = identity>
  dynamic_graph(
        const ERng& erng, const VRng& vrng, EProj eproj, VProj vproj, GV&& gv, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc), value_(move(gv)) {}


  //       max_vertex_id, erng, eproj, alloc
  // gv&,  max_vertex_id, erng, eproj, alloc
  // gv&&, max_vertex_id, erng, eproj, alloc

  /**
   * @brief Construct the graph given the maximum vertex id and edge data range.
   * 
   * The vertices container is pre-extended to accomodate the number of vertices referenced by edges and avoids
   * the need to scan the edges to determine the maximum vertex id.
   * 
   * The graph value is default-constructed.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param max_vertex_id The maximum vertex id used by edges. The vertices container is pre-extended to store the number
   *                      of elements, if applicable.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc) {}

  /**
   * @brief Construct the graph given the graph value, maximum vertex id and edge data range.
   * 
   * The vertices container is pre-extended to accomodate the number of vertices referenced by edges and avoids
   * the need to scan the edges to determine the maximum vertex id.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param gv            The value to copy to the internal graph value.
   * @param max_vertex_id The maximum vertex id used by edges. The vertices container is pre-extended to store the number
   *                      of elements, if applicable.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(const GV&      gv,
                vertex_id_type max_vertex_id,
                ERng&          erng,
                EProj          eproj = {},
                allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc), value_(gv) {}

  /**
   * @brief Construct the graph given the graph value, maximum vertex id and edge data range.
   * 
   * The vertices container is pre-extended to accomodate the number of vertices referenced by edges and avoids
   * the need to scan the edges to determine the maximum vertex id.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param gv            The value to move to the internal graph value.
   * @param max_vertex_id The maximum vertex id used by edges. The vertices container is pre-extended to store the number
   *                      of elements, if applicable.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(
        GV&& gv, vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc), value_(move(gv)) {}

  // erng, eproj,       alloc
  // erng, eproj, gv&,  alloc
  // erng, eproj, gv&&, alloc

  /**
   * @brief Construct the graph given a edge data range.
   * 
   * The edge data range is scanned to determine the largest vertex id needed. Once determined, the internal vertices 
   * container is pre-extended to include all vertex ids needed.
   * 
   * The graph value is default-constructed.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc) {}

  /**
   * @brief Construct the graph given a graph value and edge data range.
   * 
   * The edge data range is scanned to determine the largest vertex id needed. Once determined, the internal vertices 
   * container is pre-extended to include all vertex ids needed.
   * 
   * The graph value is initialized by copying from the @c gv parameter.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param gv    The value to copy to the internal graph value.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(const GV& gv, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc), value_(gv) {}

  /**
   * @brief Construct the graph given a graph value and edge data range.
   * 
   * The edge data range is scanned to determine the largest vertex id needed. Once determined, the internal vertices 
   * container is pre-extended to include all vertex ids needed.
   * 
   * The graph value is initialized by moving from the @c gv parameter.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param gv    The value to move to the internal graph value.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(GV&& gv, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc), value_(move(gv)) {}


  /**
   * @brief Construct the graph using a intializer list of copyable edges.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * The graph value is default-constructed.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(const initializer_list<copyable_edge_t<VId, EV>>& il, edge_allocator_type alloc = edge_allocator_type())
        : base_type(il, alloc) {}

public:
  constexpr value_type&       value() { return value_; }
  constexpr const value_type& value() const { return value_; }

private:
  value_type value_; ///< Graph value

private: // tag_invoke properties
  friend constexpr value_type&       graph_value(graph_type& g) { return g.value_; }
  friend constexpr const value_type& graph_value(const graph_type& g) { return g.value_; }
};

/**
 * @ingroup graph_containers
 * @brief dynamic_graph defines a graph with a variety characteristics including edge, vertex 
 * and graph value types, whether edges are sourced or not, the vertex id type, and selection 
 * of the containers used for vertices and edges.
 * 
 * This is a specialization of dynamic_graph where @c GV=void. Calling @c graph_value(g)
 * will generate a compiler error so the user can resolve the incorrect usage.
 * 
 * See the dynamic_graph description for more information about this class.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructible.
 * @tparam GV      [void] No graph value is stored on the graph. Use of @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
 * @tparam Traits  @showinitializer =vofl_graph_traits<EV,VV,GV,Sourced,VId> Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class VId, bool Sourced, class Traits>
class dynamic_graph<EV, VV, void, VId, Sourced, Traits>
      : public dynamic_graph_base<EV, VV, void, VId, Sourced, Traits> {

public: // Types & Constants
  using base_type      = dynamic_graph_base<EV, VV, void, VId, Sourced, Traits>;
  using graph_type     = dynamic_graph<EV, VV, void, VId, Sourced, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = void;
  using allocator_type = typename Traits::vertices_type::allocator_type;
  using typename base_type::vertices_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, void, VId, Sourced, Traits>;

  constexpr inline const static bool sourced = Sourced;

public: // Construction/Destruction/Assignment
  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  /**
   * @brief Construct the graph given a edge data range.
   * 
   * The edge data range is scanned to determine the largest vertex id needed. Once determined, the internal vertices 
   * container is pre-extended to include all vertex ids needed.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class EProj = identity>
  dynamic_graph(ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, eproj, alloc) {}

  /**
   * @brief Construct the graph given the maximum vertex id and edge data range.
   * 
   * The vertices container is pre-extended to accomodate the number of vertices referenced by edges and avoids
   * the need to scan the edges to determine the maximum vertex id.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * 
   * @param max_vertex_id The maximum vertex id used by edges. The vertices container is pre-extended to store the number
   *                      of elements, if applicable.
   * @param erng  The edge data range used to create new edges.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, typename EProj = identity>
  dynamic_graph(vertex_id_type max_vertex_id, ERng& erng, EProj eproj = {}, allocator_type alloc = allocator_type())
        : base_type(max_vertex_id, erng, eproj, alloc) {}

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The vertices container is pre-extended to accomodate the largest vertex id, if applicable. The
   * @c vrng size is used to determine the number of vertices and then also is used as a container of values
   * to assign to the internal vertices. Given that, @c vrng elements don't have to be ordered sequentially
   * by vertex id; the vertex id returned by @c vproj is used to find an existing vertex before assigning
   * the value to it.
   * 
   * @tparam ERng  The range type of edge data to load into the graph.
   * @tparam VRng  The range type of vertex data to load into the graph.
   * @tparam EProj The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already @c copyable_edge_t<VId,EV> then @c identity can be used instead.
   * @tparam VProj The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   *               If the @c VRng value type is already @c copyable_vertex_t<VId,VV> then @c identity can be used instead.
   * 
   * @param erng  The edge data range used to create new edges.
   * @param vrng  The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj The edge projection function used to convert the @c erng value type to @c copyable_edge_t<VId,EV>.
   *              If the @c erng value type is already @c copyable_edge_t<VId,EV> then @c identity() can be used instead.
   * @param vproj The vertex projection function used to convert the @c vrng value type to @c copyable_vertex_t<VId,VV>.
   *              If the @c vrng value type is already @c copyable_vertex_t<VId,VV> then @c identity() can be used instead.
   * @param alloc The allocator used for the vertices and edges containers.
  */
  template <class ERng, class VRng, class EProj = identity, class VProj = identity>
  dynamic_graph(ERng& erng, VRng& vrng, EProj eproj = {}, VProj vproj = {}, allocator_type alloc = allocator_type())
        : base_type(erng, vrng, eproj, vproj, alloc) {}

  /**
   * @brief Construct the graph using a intializer list of copyable edges.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(const initializer_list<copyable_edge_t<VId, EV>>& il, edge_allocator_type alloc = edge_allocator_type())
        : base_type(il, alloc) {}
};


} // namespace std::graph::container
