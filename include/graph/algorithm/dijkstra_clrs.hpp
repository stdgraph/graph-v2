#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <queue>

namespace std::graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;

class null_range_type : public std::vector<size_t> {
  using T         = size_t;
  using Allocator = std::allocator<T>;
  using Base      = std::vector<T, Allocator>;

public:
  null_range_type() noexcept(noexcept(Allocator())) = default;
  explicit null_range_type(const Allocator& alloc) noexcept {}
  null_range_type(Base::size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  explicit null_range_type(Base::size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  null_range_type(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  null_range_type(const null_range_type& other) : Base() {}
  null_range_type(const null_range_type& other, const Allocator& alloc) {}
  null_range_type(null_range_type&& other) noexcept {}
  null_range_type(null_range_type&& other, const Allocator& alloc) {}
  null_range_type(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
};

inline static null_range_type null_range;


template <class... Ts>
struct print_types_t;

template <class... Ts>
constexpr auto print_types(Ts...) {
  return print_types_t<Ts...>{};
}


/**
 * @brief Dijkstra's algorithm for finding the shortest path from a source vertex to all other vertices in a graph.
 * @tparam G The graph type.
 * @tparam WF The edge weight function type.
 * @tparam Distance The distance range type.
 * @tparam Predecessor The predecessor range type.
 *
 * @param graph The graph.
 * @param source The source vertex.
*  @param distance The distance vector.
*  @param predecessor The predecessor vector.
 * @param weight The edge weight function.
 *        The edge weight function must be a function object that returns the weight of an edge.
 *        The edge weight function must be copy constructible.
 *        The edge weight function must be invocable with an edge reference.
 *        The edge weight function must return a value that is arithmetic.
 *        The edge weight function must return a value that is convertible to the weight type.
 *        The edge weight function must not throw an exception.
 *        The edge weight function must not modify the graph, the edge, or the vertex (nor any of their associated data).
 */
template <adjacency_list              G,
          ranges::random_access_range Distance,
          ranges::random_access_range Predecessor,
          class WF = std::function<ranges::range_value_t<Distance>(edge_reference_t<G>)>>
requires ranges::random_access_range<vertex_range_t<G>> &&                  //
      integral<vertex_id_t<G>> &&                                           //
      is_arithmetic_v<ranges::range_value_t<Distance>> &&                   //
      convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessor>> && //
      edge_weight_function<G, WF>
void dijkstra_clrs(
      G&&            g,           // graph
      vertex_id_t<G> seed,        // starting vertex_id
      Distance&      distance,    // out: distance[uid] of uid from seed
      Predecessor&   predecessor, // out: predecessor[uid] of uid in path
      WF weight = [](edge_reference_t<G> uv) { return ranges::range_value_t<Distance>(1); }) // default weight(uv) -> 1
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_reference_t<G>>;

  // Remark(Andrew): Do we want to allow null distance?  What about if both are null?  Still run algorithm at all?

  size_t N(size(vertices(g))); // Question(Andrew): Do we want a num_vertices(g) CPO?
  assert(seed < N && seed >= 0);

  std::ranges::fill(distance, numeric_limits<weight_type>::max());
  distance[seed] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  // Remark(Andrew):  We may want to make this parameterizable as different types of heaps give different performance
  // But std::priority_queue is probably reasonable for now
  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; });
  std::priority_queue<weighted_vertex, vector<weighted_vertex>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push({seed, distance[seed]});

  while (!Q.empty()) {

    auto uid = Q.top().vertex_id;
    Q.pop();

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) { // Remark(Andrew): +1
      //weight_type w = weight(uv);
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        if constexpr (!is_same_v<Predecessor, null_range_type>)
          predecessor[vid] = uid;
        Q.push({vid, distance[vid]});
      }
    }
  }
}

} // namespace std::graph
