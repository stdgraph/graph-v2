#pragma once

#include "graph/container/container_utility.hpp"

// references for to_tuple
// https://www.reddit.com/r/cpp/comments/4yp7fv/c17_structured_bindings_convert_struct_to_a_tuple/
// https://gist.github.com/utilForever/1a058050b8af3ef46b58bcfa01d5375d

template <class T, class... TArgs>
decltype(void(T{std::declval<TArgs>()...}), std::true_type{}) test_is_braces_constructible(int);

template <class, class...>
std::false_type test_is_braces_constructible(...);

template <class T, class... TArgs>
using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

struct any_type {
  template <class T>
  constexpr operator T(); // non explicit
};

template <class T>
auto to_tuple(T&& object) noexcept {
  using type = std::decay_t<T>;
  if constexpr (is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3, p4] = object;
    return std::make_tuple(p1, p2, p3, p4);
  } else if constexpr (is_braces_constructible<type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3] = object;
    return std::make_tuple(p1, p2, p3);
  } else if constexpr (is_braces_constructible<type, any_type, any_type>{}) {
    auto&& [p1, p2] = object;
    return std::make_tuple(p1, p2);
  } else if constexpr (is_braces_constructible<type, any_type>{}) {
    auto&& [p1] = object;
    return std::make_tuple(p1);
  } else {
    return std::make_tuple();
  }
}

template <class T>
using to_tuple_t = decltype(to_tuple(std::declval<T>()));

//template <class C>
//concept has_push_back = requires(C& container, std::ranges::range_reference_t<C> val) {
//                          { container.push_back(val) };
//                        };
//template <class C>
//concept has_push_front = requires(C& container, std::ranges::range_reference_t<C> val) {
//                           { container.push_front(val) };
//                         };

template <class Outer>
concept range_of_ranges =
      std::ranges::random_access_range<Outer> &&                       // outer range is an random_access_range
      std::ranges::forward_range<std::ranges::range_value_t<Outer>> && // inner range is forward_range
      std::integral<std::tuple_element_t<0,
                                         to_tuple_t<std::ranges::range_value_t<std::ranges::range_value_t<
                                               Outer>>>>>; // first element of edge is integral (target_id)


template <range_of_ranges Outer, std::ranges::random_access_range VVR>
requires std::ranges::contiguous_range<Outer> && std::ranges::random_access_range<VVR>
// contiguous so we can calculate vertex_id, given vertex_value(g,u), where u is a vertex reference
class rr_adaptor {
public:
  using graph_type        = rr_adaptor<Outer, VVR>;
  using vertices_range    = Outer;
  using vertex_type       = std::ranges::range_value_t<vertices_range>;
  using edges_range       = std::ranges::range_value_t<vertices_range>; // Inner range
  using edge_type         = std::ranges::range_value_t<edges_range>;    //
  using vertex_id_type    = std::remove_cv_t<std::tuple_element_t<0, to_tuple_t<edge_type>>>;
  using edge_value_type   = std::conditional_t<(std::tuple_size_v<to_tuple_t<edge_type>> <= 1),
                                             void,
                                             std::tuple_element_t<1, to_tuple_t<edge_type>>>;
  using vertex_value_type = std::ranges::range_value_t<VVR>;

public:
  template <std::ranges::forward_range ERng, class EProj = std::identity>
  rr_adaptor(VVR&         vertex_values,       //
             const ERng&  erng,                //
             const EProj& eproj     = EProj(), // EProj(ERng::value_type&) -> edge_view<VId,true[,val]>
             bool         dup_edges = false)
        : vertex_values_(vertex_values) {
    vertex_id_type max_vid = max_vertex_id(erng, eproj);

    // assure vertices & vertex_values are big enough for the elements & sized the same
    size_t vcnt = std::max(static_cast<size_t>(max_vid + 1), vertex_values_.size());
    vertices_.resize(vcnt);
    vertex_values_.resize(vcnt);

    // Add edges
    for (auto&& e : erng) {
      if constexpr (std::is_void_v<edge_value_type>) {
        push(vertices_[static_cast<size_t>(e.source_id)], e.target_id);
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], e.source_id);
      } else {
        push(vertices_[static_cast<size_t>(e.source_id)], {e.target_id, e.value});
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], {e.source_id, e.value});
      }
    }
  }

private:
  // scan for the max vertex id used in input edges
  template <std::ranges::forward_range ERng, class EProj>
  vertex_id_type max_vertex_id(const ERng& erng, const EProj& eproj) const {
    vertex_id_type max_vid = 0;
    for (auto&& e : erng)
      max_vid = std::max(max_vid, std::max(e.source_id, e.target_id));
    return max_vid;
  }

  void push(edges_range& edges, const edge_type& val) {
    if constexpr (std::graph::container::has_push_back<edges_range>)
      edges.push_back(val);
    else if constexpr (std::graph::container::has_push_front<edges_range>)
      edges.push_front(val);
  }

private: // tag_invoke definitions
  friend constexpr vertices_range& tag_invoke(std::graph::tag_invoke::vertices_fn_t, graph_type& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_range& tag_invoke(std::graph::tag_invoke::vertices_fn_t, const graph_type& g) {
    return g.vertices_;
  }

  friend vertex_id_type
  tag_invoke(std::graph::tag_invoke::vertex_id_fn_t, const graph_type& g, std::ranges::iterator_t<vertices_range> ui) {
    return static_cast<vertex_id_type>(ui -
                                       std::ranges::begin(g.vertices_)); // overriden to assure correct type returned
  }

  friend constexpr edges_range& tag_invoke(std::graph::tag_invoke::edges_fn_t, graph_type& g, vertex_type& u) {
    return u;
  }
  friend constexpr const edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return u;
  }

  friend constexpr edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid];
  }
  friend constexpr const edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_id_type uid) {
    return g.vertices_[uid];
  }

  friend constexpr vertex_id_type
  tag_invoke(std::graph::tag_invoke::target_id_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return get<0>(to_tuple(uv));
  }

  friend constexpr vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    size_t uidx = static_cast<size_t>(&u - g.vertices_.data());
    return g.vertex_values_[uidx];
  }
  friend constexpr const vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    size_t uidx = static_cast<size_t>(&u - g.vertices_.data());
    return g.vertex_values_[uidx];
  }

  // edge_value(g,uv)
  friend constexpr edge_value_type& tag_invoke(std::graph::tag_invoke::edge_value_fn_t, graph_type& g, edge_type& uv) {
    auto t = to_tuple(uv);
    return get<1>(t);
  }
  friend constexpr const edge_value_type&
  tag_invoke(std::graph::tag_invoke::edge_value_fn_t, const graph_type& g, const edge_type& uv) {
    auto t = to_tuple(uv);
    return get<1>(t);
  }

private:
  vertices_range vertices_;
  VVR&           vertex_values_;
};

// rr_adaptor<RR> has recognizes the following patterns
//
//      edge     = VId | pair<Vid,T> | struct {Vid} | struct {Vid,T} | tuple<VId,U> | tuple<VId>
//      edges    = Inner<edge>
//      vertex   = edges | pair<edges,VV> | tuple<edges,VV> | struct{edges,VV}; VV can be tuple<...>
//      vertices = Outer<vertex>
//

template <class E>
struct rr_has_edge_value : public std::integral_constant<bool, (std::tuple_size_v<to_tuple_t<E>> > 1)> {};
template <class E>
inline constexpr bool rr_has_edge_value_v = rr_has_edge_value<E>::value;

template <class E>
struct rr_vertex_id_type {
  using type = std::tuple_element_t<0, to_tuple_t<E>>; // type of scaler or first element of pair, tuple, struct, class
};
template <class V>
inline constexpr bool rr_vertex_id_type_t = rr_vertex_id_type<V>::type;

template <class E>
struct rr_edge_value_type {
  using type = std::conditional<rr_has_edge_value_v<E>,
                                std::tuple_element_t<1, to_tuple_t<E>>,
                                void>; // Doesn't handle tuple<VId,...>
};
template <class V>
using rr_edge_value_type_t = typename rr_edge_value_type<V>::type;

template <class V>
struct rr_has_vertex_value : public std::integral_constant<bool, (std::tuple_size_v<to_tuple_t<V>> > 1)> {};
template <class V>
inline constexpr bool rr_has_vertex_value_v = rr_has_vertex_value<V>::value;

template <class V>
struct rr_vertex_edges_type {
  using type = std::tuple_element_t<0, to_tuple_t<V>>; // type of scaler or first element of pair, tuple, struct, class
};
template <class V>
using rr_vertex_edges_type_t = typename rr_vertex_edges_type<V>::type;

template <class V>
struct rr_vertex_value_type {
  using type = std::conditional<rr_has_vertex_value_v<V>,
                                std::tuple_element_t<1, to_tuple_t<V>>,
                                void>; // Doesn't handle tuple<T,...>
};
template <class V>
using rr_vertex_value_type_t = typename rr_vertex_value_type<V>::type;

template <class Outer>
concept range_of_ranges2 =
      std::ranges::random_access_range<Outer> &&                       // outer range is an random_access_range
      std::ranges::forward_range<std::ranges::range_value_t<Outer>> && // inner range is forward_range
      std::integral<std::tuple_element_t<0,
                                         to_tuple_t<std::ranges::range_value_t<std::ranges::range_value_t<
                                               Outer>>>>>; // first element of edge is integral (target_id)


template <class Outer>
//requires range_of_ranges<Outer>
class rr_adaptor2 {
public:
  using graph_type        = rr_adaptor2<Outer>;                         // this type
  using vertices_range    = Outer;                                      // Outer range
  using vertex_type       = std::ranges::range_value_t<vertices_range>; //
  using edges_range       = rr_vertex_edges_type<vertex_type>;          // Inner range
  using edge_type         = std::ranges::range_value_t<edges_range>;    //
  using vertex_id_type    = rr_vertex_id_type<edge_type>;               //
  using edge_value_type   = rr_edge_value_type_t<edge_type>;            //
  using vertex_value_type = rr_vertex_value_type_t<vertex_type>;        //

public:
  template <class InputEdges, class EdgeIdFnc>
  rr_adaptor2(const InputEdges& inputEdges, const EdgeIdFnc& edge_id_fn, bool dup_edges = false) {
    size_t max_vid = max_vertex_id(inputEdges, edge_id_fn);

    // assure vertices have enough elements
    size_t vcnt = max_vid + 1;
    vertices_.resize(vcnt);

    // Add edges
    for (auto&& e : inputEdges) {
      auto         edge_id = edge_id_fn(e); // pair<VId,VId>
      edges_range& er      = edges(*this, edge_id.first);
      push(er, edge_id.second);
      if (dup_edges) {
        edges_range& er2 = edges(*this, edge_id.second);
        push(er2, edge_id.first);
      }
    }
  }

  template <class InputEdges, class EdgeIdFnc, class EdgeValFnc>
  rr_adaptor2(const InputEdges& inputEdges,
              const EdgeIdFnc&  edge_id_fn,
              const EdgeValFnc& edge_val_fn,
              bool              dup_edges = false) {
    vertex_id_type max_vid = max_vertex_id(inputEdges, edge_id_fn);

    // assure vertices have enough elements
    size_t vcnt = static_cast<size_t>(max_vid + 1);
    vertices_.resize(vcnt);

    // Add edges
    for (auto&& e : inputEdges) {
      auto         edge_id = edge_id_fn(e); // pair<VId,VId>
      edges_range& er      = edges(*this, edge_id.first);
      push(er, {static_cast<vertex_id_type>(edge_id.second), edge_val_fn(e)});
      if (dup_edges) {
        edges_range& er2 = edges(*this, edge_id.second);
        push(er2, {static_cast<vertex_id_type>(edge_id.first), edge_val_fn(e)});
      }
    }
  }

  template <class InputEdges,
            class EdgeIdFnc,
            class EdgeValFnc,
            class InputVertices,
            std::ranges::sized_range VertexIdFnc,
            class VertexValFnc>
  rr_adaptor2(const InputEdges&    inputEdges,
              const EdgeIdFnc&     edge_id_fn,
              const EdgeValFnc&    edge_val_fn,
              const InputVertices& inputVertices,
              const VertexIdFnc&   vertex_id_fn,
              const VertexValFnc&  vertex_val_fn,
              bool                 dup_edges = false) {
    vertex_id_type max_vid = max_vertex_id(inputEdges, edge_id_fn);

    // assure vertices have enough elements
    size_t vcnt = std::max(static_cast<size_t>(max_vid + 1), size(inputVertices));
    vertices_.resize(vcnt);

    // Add vertices
    for (auto&& uu : inputVertices) {
      vertex_id_type uid     = vertex_id_fn(uu);
      auto&          u       = vertices_[uid];
      vertex_value(*this, u) = vertex_val_fn(uu);
    }

    // Add edges
    for (auto&& e : inputEdges) {
      auto         edge_id = edge_id_fn(e); // pair<VId,VId>
      edges_range& er      = edges(*this, edge_id.first);
      push(er, {static_cast<vertex_id_type>(edge_id.second), edge_val_fn(e)});
      if (dup_edges) {
        edges_range& er2 = edges(*this, edge_id.second);
        push(er2, {static_cast<vertex_id_type>(edge_id.first), edge_val_fn(e)});
      }
    }
  }

private:
  // scan for the max vertex id used in input edges
  template <class InputEdges, class EdgeIdFnc>
  vertex_id_type max_vertex_id(const InputEdges& input, const EdgeIdFnc& edge_id_fn) const noexcept {
    vertex_id_type max_vid = 0;
    for (auto&& e : input) {
      auto edge_id = edge_id_fn(e); // pair<VId,VId>
      max_vid      = std::max(max_vid, std::max(edge_id.first, edge_id.second));
    }
    return max_vid;
  }

  void push(edges_range& edges, const edge_type& val) {
    if constexpr (std::graph::container::has_push_back<edges_range>)
      edges.push_back(val);
    else if constexpr (std::graph::container::has_push_front<edges_range>)
      edges.push_front(val);
  }

private:
  friend constexpr vertices_range& tag_invoke(std::graph::tag_invoke::vertices_fn_t, graph_type& g) {
    return g.vertices_;
  }
  friend constexpr const vertices_range& tag_invoke(std::graph::tag_invoke::vertices_fn_t, const graph_type& g) {
    return g.vertices_;
  }

  friend vertex_id_type
  tag_invoke(std::graph::tag_invoke::vertex_id_fn_t, const graph_type& g, std::ranges::iterator_t<vertices_range> ui) {
    return static_cast<vertex_id_type>(ui -
                                       std::ranges::begin(g.vertices_)); // overriden to assure correct type returned
  }

  friend constexpr edges_range& tag_invoke(std::graph::tag_invoke::edges_fn_t, graph_type& g, vertex_type& u) {
    return get<0>(to_tuple(u));
  }
  friend constexpr const edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_type& u) {
    return get<0>(to_tuple(u));
  }

  friend constexpr edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, graph_type& g, const vertex_id_type uid) {
    return get<0>(to_tuple(g.vertices_[uid]));
  }
  friend constexpr const edges_range&
  tag_invoke(std::graph::tag_invoke::edges_fn_t, const graph_type& g, const vertex_id_type uid) {
    return get<0>(to_tuple(g.vertices_[uid]));
  }

  friend constexpr vertex_id_type
  tag_invoke(std::graph::tag_invoke::target_id_fn_t, const graph_type& g, const edge_type& uv) noexcept {
    return get<0>(to_tuple(uv));
  }

  friend constexpr vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, graph_type& g, vertex_type& u) {
    return get<1>(to_tuple(u));
  }
  friend constexpr const vertex_value_type&
  tag_invoke(std::graph::tag_invoke::vertex_value_fn_t, const graph_type& g, const vertex_type& u) {
    return get<1>(to_tuple(u));
  }

  // edge_value(g,uv)
  friend constexpr edge_value_type& tag_invoke(std::graph::tag_invoke::edge_value_fn_t, graph_type& g, edge_type& uv) {
    return get<1>(to_tuple(uv));
  }
  friend constexpr const edge_value_type&
  tag_invoke(std::graph::tag_invoke::edge_value_fn_t, const graph_type& g, const edge_type& uv) {
    return get<1>(to_tuple(uv));
  }

private:
  vertices_range vertices_;
};
