#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <queue>
#include <algorithm>

namespace graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      std::copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;


//edge_weight_function<G, WF> &&
//strict_weak_order<Compare, range_value_t<Distance>, range_value_t<Distance>> &&
//assignable_from <range_reference_t<Distance>, invoke_result_t <Combine, invoke_result_t<WF, edge_t<G>>,

template <class G, class WF, class Distance, class Compare, class Combine>
concept basic_edge_weight_function2 = // e.g. weight(uv)
      is_arithmetic_v<range_value_t<Distance>> &&
      std::strict_weak_order<Compare, range_value_t<Distance>, range_value_t<Distance>> &&
      std::assignable_from<range_reference_t<Distance>, invoke_result_t<Combine, invoke_result_t<WF, edge_t<G>>>>;

template <class G, class WF, class Distance>
concept edge_weight_function2 = // e.g. weight(uv)
      is_arithmetic_v<invoke_result_t<WF, edge_t<G>>> &&
      basic_edge_weight_function2<G, WF, Distance, less<range_value_t<Distance>>, plus<range_value_t<Distance>>>;


class _null_range_type : public std::vector<size_t> {
  using T         = size_t;
  using Allocator = std::allocator<T>;
  using Base      = std::vector<T, Allocator>;

public:
  _null_range_type() noexcept(noexcept(Allocator())) = default;
  explicit _null_range_type(const Allocator& alloc) noexcept {}
  _null_range_type(Base::size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  explicit _null_range_type(Base::size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  _null_range_type(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  _null_range_type(const _null_range_type& other) : Base() {}
  _null_range_type(const _null_range_type& other, const Allocator& alloc) {}
  _null_range_type(_null_range_type&& other) noexcept {}
  _null_range_type(_null_range_type&& other, const Allocator& alloc) {}
  _null_range_type(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
};

inline static _null_range_type null_predecessors;


template <class... Ts>
struct print_types_t;

template <class... Ts>
constexpr auto print_types(Ts...) {
  return print_types_t<Ts...>{};
}


/**
 * @ingroup graph_algorithms
 * @brief Dijkstra's algorithm for finding the shortest path from a source vertex to all other vertices in a graph.
 * @tparam G The graph type.
 * @tparam WF The edge weight function type.
 * @tparam Distance The distance range type.
 * @tparam Predecessor The predecessor range type.
 *
 * @param graph The graph.
 * @param source The source vertex.
 * @param distance The distance vector.
 * @param predecessor The predecessor vector.
 * @param weight The edge weight function.
 *        The edge weight function must be a function object that returns the weight of an edge.
 *        The edge weight function must be copy constructible.
 *        The edge weight function must be invocable with an edge reference.
 *        The edge weight function must return a value that is arithmetic.
 *        The edge weight function must return a value that is convertible to the weight type.
 *        The edge weight function must not throw an exception.
 *        The edge weight function must not modify the graph, the edge, or the vertex (nor any of their associated data).
 */

#if 1 // not using Identifiers (original)

template <adjacency_list      G,
          random_access_range Distance,
          random_access_range Predecessor,
          class WF = function<range_value_t<Distance>(edge_reference_t<G>)>>
requires random_access_range<vertex_range_t<G>> &&                     //
         integral<vertex_id_t<G>> &&                                   //
         is_arithmetic_v<range_value_t<Distance>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessor>> && //
         edge_weight_function<G, WF>
void dijkstra_clrs(
      G&&            g,           // graph
      vertex_id_t<G> seed,        // starting vertex_id
      Distance&      distance,    // out: distance[uid] of uid from seed
      Predecessor&   predecessor, // out: predecessor[uid] of uid in path
      WF&& weight = [](edge_reference_t<G> uv) { return range_value_t<Distance>(1); }) // default weight(uv) -> 1
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  // Remark(Andrew): Do we want to allow null distance?  What about if both are null?  Still run algorithm at all?

  size_t N(num_vertices(g));
  assert(seed < N && seed >= 0);

  std::ranges::fill(distance, std::numeric_limits<weight_type>::max());
  distance[seed] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  // Remark(Andrew):  We may want to make this parameterizable as different types of heaps give different performance
  // But std::priority_queue is probably reasonable for now
  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push({seed, distance[seed]});

  while (!Q.empty()) {

    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) { // Remark(Andrew): +1
      //weight_type w = weight(uv);
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        if constexpr (!is_same_v<Predecessor, _null_range_type>)
          predecessor[vid] = uid;
        Q.push({vid, distance[vid]});
      }
    }
  }
}

#else  // using Identifiers

template <adjacency_list      G,
          random_access_range Distance,
          random_access_range Predecessor,
          class WF = function<range_value_t<Distance>(edge_identifier_t<G>)>>
requires random_access_range<vertex_range_t<G>> &&                     //
         integral<vertex_id_t<G>> &&                                   //
         is_arithmetic_v<range_value_t<Distance>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessor>> && //
         edge_weight_function<G, WF>
void dijkstra_clrs(
      G&&                    g,           // graph
      vertex_identifier_t<G> seed,        // starting vertex_id
      Distance&              distance,    // out: distance[uid] of uid from seed
      Predecessor&           predecessor, // out: predecessor[uid] of uid in path
      WF&& weight = [](edge_identifier_t<G> uv) { return range_value_t<Distance>(1); }) // default weight(uv) -> 1
{
  using id_type         = vertex_id_t<G>;
  using weight_type     = invoke_result_t<WF, edge_identifer_t<G>>;
  const id_type seed_id = vertex_id(g, seed);

  size_t N(num_vertices(g));
  assert(seed_id < N && seed_id >= 0);

  std::ranges::fill(distance, std::numeric_limits<weight_type>::max());
  distance[seed_id] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, q_compare> Q;

  Q.push({seed_id, distance[seed_id]});

  while (!Q.empty()) {
    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [v_identifier, uv_identifier, w] : views::incidence(g, find_vertex(g, uid), weight)) {
      id_type vid = vertex_id(v_identifier);
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        if constexpr (!is_same_v<Predecessor, _null_range_type>)
          predecessor[vid] = uid;
        Q.push({vid, distance[vid]});
      }
    }
  }
}

#endif // Identifiers

} // namespace graph
