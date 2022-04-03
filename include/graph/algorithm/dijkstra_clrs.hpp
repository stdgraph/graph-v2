#pragma once

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <queue>

namespace std::graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && regular_invocable<F&, ranges::range_reference_t<vertex_edge_range_t<G>>>;


template <class T, class Allocator = std::allocator<T>>
class null_range : public std::vector<T, Allocator> {
public:
  constexpr null_range() noexcept(noexcept(Allocator())) {}
  constexpr explicit vector(const Allocator& alloc) noexcept {}
  constexpr vector(size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  constexpr explicit vector(size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  constexpr vector(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  constexpr vector(const vector& other) {}
  constexpr vector(const vector& other, const Allocator& alloc) {}
  constexpr vector(vector&& other) noexcept {}
  constexpr vector(vector&& other, const Allocator& alloc) {}
  constexpr vector(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
}


// The index into weight vector stored as the first property
template <incidence_graph G, ranges::random_access_range DM, ranges::random_access_range PM, class WF>
requires edge_weight_function<G, WF> &&
      is_arithmetic_v<invoke_result_t<WF, ranges::range_reference_t<vertex_edge_range_t<G>>>> &&
      ranges::random_access_range<vertex_range_t<G>>
void dijkstra_clrs(
      G&&            g,      //
      vertex_id_t<G> source, //
      DM&            distance,
      PM&            predecessor,
      WF             weight = [&g](edge_reference_t<G> uv) { return 1; }) {

  using id_type     = vertex_id_t<G>;
  using weight_type = decltype(weight(std::declval<edge_reference_t<G>>()));

  size_t N(size(vertices(g))); // Question(Andrew): Do we want a num_vertices(g) CPO?
  assert(source < N && source >= 0);

  std::ranges::fill(distance, numeric_limits<weight_type>::max());
  distance[source] = 0;

  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  // Remark(Andrew):  We may want to make this parameterizable as different types of heaps give different performance
  // But std::priority_queue is probably reasonable for now
  using q_compare = decltype([](const weighted_vertex& a, const weighted_vertex& b) { return a.weight > b.weight; })>;
  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, q_compare) Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto uid = Q.top().vertex_id;
    Q.pop();

      for (auto&& [vid, uv] : views::incidence(g, uid)) { // Remark(Andrew): +1
      weight_type w = weight(uv);
      if (distance[uid] + w < distance[vid]) {
        distance[vid] = distance[uid] + w;
        predecessor[vid] = uid;
        Q.push({vid, distance[vid]});
      }
    }
  }
}

} // namespace std::graph
