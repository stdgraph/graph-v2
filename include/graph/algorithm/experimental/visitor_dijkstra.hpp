#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/experimental/co_cmn.hpp"
#include "graph/algorithm/experimental/bfs_cmn.hpp"

#include <variant>
#include <ranges>
#include <fmt/format.h>

namespace std::graph::experimental {

/**
 * @ingroup graph_algorithms
 * @brief A concept that describes a queueable container. It reflects the capabilities of
 * std::queue and std::priority_queue.
 * 
 * Use of this defines the required capabilities, including those of containers in std and
 * the caller's domain.
*/
template <class Q>
concept queueable = requires(Q&& q, typename Q::value_type value) {
  typename Q::value_type;
  typename Q::size_type;
  typename Q::reference;

  { q.top() };
  { q.push(value) };
  { q.pop() };
  { q.empty() };
  { q.size() };
};

//queueable Q = priority_queue < weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>,
//          vector<weighted_vertex<G, invoke_result_t<EVF, edge_reference_t<G>>>>, greater < weighted_vertex < G,
//          invoke_result_t < EVF,
//          edge_reference_t < G >>>>>>


// Notes
//  shortest paths vs. shortest distances
//  throw on error

// Design Considerations:
// 1. The visitor should be a template parameter to the algorithm.

template <class G, class Visitor>
concept dijkstra_visitor = //is_arithmetic<typename Visitor::distance_type> &&
      requires(Visitor v, typename Visitor::vertex_desc_type vdesc, typename Visitor::edge_desc_type edesc) {
        //typename Visitor::distance_type;

        { v.on_initialize_vertex(vdesc) };
        { v.on_discover_vertex(vdesc) };
        { v.on_examine_vertex(vdesc) };
        { v.on_finish_vertex(vdesc) };

        { v.on_examine_edge(edesc) };
        { v.on_edge_relaxed(edesc) };
        { v.on_edge_not_relaxed(edesc) };
      };

template <adjacency_list G>
class dijkstra_visitor_base {
  // Types
public:
  using graph_type             = G;
  using vertex_desc_type       = vertex_descriptor<vertex_id_t<G>, vertex_reference_t<G>, void>;
  using sourced_edge_desc_type = edge_descriptor<vertex_id_t<G>, true, edge_reference_t<G>, void>;

  // Visitor Functions
public:
  // vertex visitor functions
  constexpr void on_initialize_vertex(const vertex_desc_type& vdesc) {}
  constexpr void on_discover_vertex(const vertex_desc_type& vdesc) {}
  constexpr void on_examine_vertex(const vertex_desc_type& vdesc) {}
  constexpr void on_finish_vertex(const vertex_desc_type& vdesc) {}

  // edge visitor functions
  constexpr void on_examine_edge(const sourced_edge_desc_type& edesc) {}
  constexpr void on_edge_relaxed(const sourced_edge_desc_type& edesc) {}
  constexpr void on_edge_not_relaxed(const sourced_edge_desc_type& edesc) {}
};

template <typename V>
concept _has_overridden_on_initialize_vertex = requires(V obj, typename V::vertex_desc_type vdesc) {
  { &V::on_initialize_vertex } -> same_as<void (V::*)()>;
};

template <adjacency_list G, ranges::random_access_range Distances>
class _dijkstra_distance_compare {
  const Distances& distances_;

public:
  _dijkstra_distance_compare(const G&, const Distances& distances) : distances_(distances) {}
  _dijkstra_distance_compare(const Distances& distances) : distances_(distances) {}

  constexpr bool operator()(const vertex_id_t<G>& a, const vertex_id_t<G>& b) const {
    return distances_[static_cast<size_t>(a)] > distances_[static_cast<size_t>(b)];
  }
};

template <class G, class Distances>
using _dijkstra_queue =
      priority_queue<vertex_id_t<G>, vector<vertex_id_t<G>>, _dijkstra_distance_compare<G, Distances>>;

using DD = vector<double>;
DD dd;

using GG = vector<vector<tuple<int, double>>>;
GG gg;

using CC  = _dijkstra_distance_compare<GG, DD>;
using PQV = vector<vertex_id_t<GG>>;

priority_queue<vertex_id_t<GG>, vector<vertex_id_t<GG>>, _dijkstra_distance_compare<GG, DD>>
      pq(CC(gg, dd), vector<vertex_id_t<GG>>());


_dijkstra_queue<GG, DD> pq2{CC(gg, dd), vector<vertex_id_t<GG>>()};

/**
 * @brief dijkstra shortest paths
 * 
 * This is an experimental implementation of Dijkstra's shortest paths.
 * 
 * The implementation was taken from boost::graph (BGL) dijkstra_shortes_paths_no_init.
 * 
 * Exposing the Queue type exposes the internals of the algorithm and requires that the
 * caller support the same semantics if they want to provide their own queue.
 * 
 * @tparam G            The graph type,
 * @tparam Distances    The distance vector.
 * @tparam Predecessors The predecessor vector.
 * @tparam WF           Edge weight function. Defaults to a function that returns 1.
 * @tparam Compare      Comparison function for Distance values. Defaults to less<DistanceValue>.
 * @tparam Combine      Combine function for Distance values. Defaults to plus<DistanctValue>.
 * @tparam Que          The queue type. Defaults to a priority_queue with comparator of _dijkstra_distance_compare. (demonstration only)
 */
template <index_adjacency_list G,
          class Visitor,
          ranges::input_range         Sources,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>
          //queueable Que = _dijkstra_queue<G, Distances> // not used
          >
requires convertible_to<ranges::range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<ranges::range_value_t<Distances>> &&              //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G,
                                    WF,
                                    ranges::range_value_t<Distances>,
                                    Compare,
                                    Combine> //&& dijkstra_visitor<G, Visitor>
void dijkstra_with_visitor(
      G&             g,
      const Sources& sources,
      Predecessors&  predecessor,
      Distances&     distances,
      WF&            weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = dijkstra_visitor_base<G>(),
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using DistanceValue = ranges::range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;

#if ENABLE_INLINE_RELAX_TARGET == 0
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    vertex_id_t<G>      vid = target_id(g, e);
    const DistanceValue d_u = distances[static_cast<size_t>(uid)];
    const DistanceValue d_v = distances[static_cast<size_t>(vid)];
    //const auto          w_e = weight(e);

    if (compare(combine(d_u, w_e), d_v)) {
      distances[static_cast<size_t>(vid)] = combine(d_u, w_e);
#  ifdef ENABLE_PREDECESSORS
      predecessor[static_cast<size_t>(vid)] = uid;
#  endif
      return true;
    }
    return false;
  };
#endif

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_invalid_distance<DistanceValue>();

  const id_type N(static_cast<id_type>(num_vertices(g)));

  // This demonstrates the use of a template parameter for the queue type. However,
  // it has problems becuase the comparator requies the graph and distances, which
  // are not available at the time of the queue's construction. The existing implementation
  // below should be sufficient for the proposal. If the caller wants to specify their own
  // queue, they can copy the implementation and use their own queue type.
  //    Allowing the caller to defined the queue also exposes the internals of the algorithm
  // and requires that the caller support the same semantics, making it more complex.
  //
  //Que que = _dijkstra_queue<G, Distances>(_dijkstra_distance_compare(g, distances), vector<vertex_id_t<G>>());

  auto qcompare = [&distances](id_type a, id_type b) {
    return distances[static_cast<size_t>(a)] > distances[static_cast<size_t>(b)];
  };
  using Queue = std::priority_queue<vertex_id_t<G>, vector<vertex_id_t<G>>, decltype(qcompare)>;
  Queue queue(qcompare);

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  for (id_type uid = 0; uid < N; ++uid) {
    visitor.on_initialize_vertex({uid, *find_vertex(g, uid)});
  }

  // Seed the queue with the initial vertice(s)
  for (auto&& source : sources) {
    if (source >= N || source < 0) {
      throw out_of_range("dijkstra_with_visitor: source vertex out of range");
    }
    queue.push(source);
    distances[static_cast<size_t>(source)] = zero; // mark source as discovered
    visitor.on_discover_vertex({source, *find_vertex(g, source)});
  }

  // Main loop to process the queue
#if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  size_t pop_cnt = 0, edge_cnt = 0;
#endif
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
#if defined(ENABLE_POP_COUNT)
    ++pop_cnt;
#endif
#if defined(ENABLE_EDGE_VISITED_COUNT)
    edge_cnt += size(edges(g, uid));
#endif
    visitor.on_examine_vertex({uid, *find_vertex(g, uid)});

    for (auto&& [vid, uv, w] : views::incidence(g, uid, weight)) {
      visitor.on_examine_edge({uid, vid, uv});

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (is_signed_v<weight_type>) {
        if (w < zero) {
          throw graph_error("dijkstra_with_visitor: negative edge weight");
        }
      }

#if ENABLE_INLINE_RELAX_TARGET
      const DistanceValue d_u                      = distances[uid];
      DistanceValue&      d_v                      = distances[vid];
      const bool          is_neighbor_undiscovered = (d_v == infinite);
      bool                was_edge_relaxed         = false;

      const DistanceValue d_v_new = combine(d_u, w);
      if (compare(d_v_new, d_v)) {
        d_v = d_v_new;
#  ifdef ENABLE_PREDECESSORS
        predecessor[vid] = uid;
#  endif
        was_edge_relaxed = true;
      }
#else
      const bool is_neighbor_undiscovered = (distances[static_cast<size_t>(vid)] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);
#endif

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          visitor.on_edge_relaxed({uid, vid, uv});
          visitor.on_discover_vertex({vid, *find_vertex(g, vid)});
          queue.push(vid);
        } else {
          // This is an indicator of a bug in the algorithm and should be investigated.
          throw logic_error("dijkstra_with_visitor: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          visitor.on_edge_relaxed({uid, vid, uv});
          queue.push(vid); // re-enqueue vid to re-evaluate its neighbors with a shorter path
        } else {
          visitor.on_edge_not_relaxed({uid, vid, uv});
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be call subsequently on the same vertex.
    visitor.on_finish_vertex({uid, *find_vertex(g, uid)});
  } // while(!queue.empty())

#if defined(ENABLE_POP_COUNT) || defined(ENABLE_EDGE_VISITED_COUNT)
  fmt::print("dijkstra_with_visitor: pop_cnt = {:L}, edge_cnt = {:L}\n", pop_cnt, edge_cnt);
#endif
}

template <index_adjacency_list G,
          class Visitor,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF      = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Compare = less<ranges::range_value_t<Distances>>,
          class Combine = plus<ranges::range_value_t<Distances>>,
          queueable Que = _dijkstra_queue<G, Distances>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G,
                                    WF,
                                    ranges::range_value_t<Distances>,
                                    Compare,
                                    Combine> //&& dijkstra_visitor<G, Visitor>
void dijkstra_with_visitor(
      G&             g,
      Visitor&&      visitor,
      vertex_id_t<G> source,
      Predecessors&  predecessor,
      Distances&     distances,
      WF&            weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>()) {
  dijkstra_with_visitor(g, forward<Visitor>(visitor), ranges::subrange(&source, (&source + 1)), predecessor, distances,
                        weight, forward<Compare>(compare), forward<Combine>(combine));
}

} // namespace std::graph::experimental
