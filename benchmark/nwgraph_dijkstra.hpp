#include <graph/graph.hpp>


// from nwgraph with minor refactoring to use graph-v2
template <adjacency_list Graph, class Weight, class Sources>
static auto nwgraph_dijkstra(
      Graph&& graph, const Sources& sources, Weight weight = [](auto& e) -> auto& {
        return std::get<1>(e);
      }) {
  using namespace std::graph;
  using vertex_id_type = vertex_id_t<Graph>;

  using distance_t = int64_t;
  std::vector<distance_t> dist(num_vertices(graph), std::numeric_limits<distance_t>::max() / 4);

  //using queue_t = std::priority_queue<vertex_id_type, std::vector<vertex_id_type>, std::greater<vertex_id_type>>;
  //queue_t mq;
  //auto mq = nw::graph::make_priority_queue<vertex_id_type>(
  //      [&dist](const vertex_id_type& a, vertex_id_type& b) { return dist[a] > dist[b]; });
  auto compare = [&dist](vertex_id_type& a, vertex_id_type& b) { return dist[a] > dist[b]; };
  using queue_t  = std::priority_queue<vertex_id_type, std::vector<vertex_id_type>, decltype(compare)>;
  queue_t mq(compare);

  for (auto&& source : sources) {
	mq.push(source);
	dist[source] = 0;
  }

#if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  size_t pop_cnt = 0, edge_cnt = 0;
#endif
  while (!mq.empty()) {
    auto u = mq.top();
    mq.pop();
#if defined(ENABLE_POP_COUNT)
    ++pop_cnt;
#endif
#if defined(ENABLE_EDGE_VISITED_COUNT)
    edge_cnt += size(edges(graph, u));
#endif
    for (auto&& elt : graph[u]) {
      auto&& v = target_id(graph, elt);
      auto&& w = weight(elt);

      auto tw = dist[u] + w;
      if (tw < dist[v]) {
        dist[v] = tw;
        mq.push(v);
      }
    }
  }

#if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  fmt::print("dijkstra_with_visitor: pop_cnt = {:L}, edge_cnt = {:L}\n", pop_cnt, edge_cnt);
#endif
  return dist;
}
