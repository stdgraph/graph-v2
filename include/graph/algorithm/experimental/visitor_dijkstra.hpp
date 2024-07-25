#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/detail/co_generator.hpp"
#include "graph/algorithm/experimental/co_cmn.hpp"
#include "graph/algorithm/experimental/bfs_cmn.hpp"

#include <variant>
#include <ranges>

namespace std::graph::experimental {

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

  // Construction, Destruction, Assignment
public:
  dijkstra_visitor_base(G& g) : g_(g) {}

  dijkstra_visitor_base()                             = delete;
  dijkstra_visitor_base(const dijkstra_visitor_base&) = default;
  dijkstra_visitor_base(dijkstra_visitor_base&&)      = default;
  ~dijkstra_visitor_base()                            = default;

  dijkstra_visitor_base& operator=(const dijkstra_visitor_base&) = default;
  dijkstra_visitor_base& operator=(dijkstra_visitor_base&&)      = default;

  // Property Functions
public:
  graph_type& graph() const noexcept { return g_; }

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

  // Data Members
private:
  reference_wrapper<graph_type> g_;
};

template <typename V>
concept _has_overridden_on_initialize_vertex = requires(V obj, typename V::vertex_desc_type vdesc) {
  { &V::on_initialize_vertex } -> same_as<void (V::*)()>;
};


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
 * @tparam Queue        The queue type. Defaults to a priority_queue with comparator of greater<vertex_id_t<G>>.
 */
template <index_adjacency_list G,
          class Visitor,
          ranges::input_range         Seeds,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF         = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Compare    = less<ranges::range_value_t<Distances>>,
          class Combine    = plus<ranges::range_value_t<Distances>>,
          _queueable Queue = priority_queue<vertex_id_t<G>, vector<vertex_id_t<G>>, greater<vertex_id_t<G>>>>
requires convertible_to<ranges::range_value_t<Seeds>, vertex_id_t<G>> && //
         is_arithmetic_v<ranges::range_value_t<Distances>> &&            //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G,
                                    WF,
                                    ranges::range_value_t<Distances>,
                                    Compare,
                                    Combine> //&& dijkstra_visitor<G, Visitor>
void dijkstra_with_visitor(
      G&            g_,
      Visitor&&     visitor,
      const Seeds&  seeds,
      Predecessors& predecessor,
      Distances&    distances,
      WF&           weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>(),
      Queue     queue   = Queue()) {
  using id_type       = vertex_id_t<G>;
  using DistanceValue = ranges::range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;

  auto relax_target = [&g_, &predecessor, &distances, &weight, &compare, &combine] //
        (edge_reference_t<G> e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    vertex_id_t<G>      vid = target_id(g_, e);
    const DistanceValue d_u = distances[uid];
    const DistanceValue d_v = distances[vid];
    //const auto          w_e = weight(e);

    // From BGL; This may no longer apply since the x87 is long gone:
    //
    // The seemingly redundant comparisons after the distance assignments are to
    // ensure that extra floating-point precision in x87 registers does not
    // lead to relax() returning true when the distance did not actually
    // change.
    if (compare(combine(d_u, w_e), d_v)) {
      distances[vid] = combine(d_u, w_e);
      if (compare(distances[vid], d_v)) {
        predecessor[vid] = uid;
        return true;
      }
    }
    return false;
  };

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_invalid_distance<DistanceValue>();

  const id_type N(static_cast<id_type>(num_vertices(g_)));

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  for (id_type uid = 0; uid < N; ++uid) {
    visitor.on_initialize_vertex({uid, *find_vertex(g_, uid)});
  }

  // Seed the queue with the initial vertice(s)
  for (auto&& seed : seeds) {
    if (seed >= N || seed < 0) {
      throw graph_error("dijkstra_with_visitor: seed vertex out of range");
    }
    queue.push(seed);
    distances[seed] = zero; // mark seed as discovered
    visitor.on_discover_vertex({seed, *find_vertex(g_, seed)});
  }

  // Main loop to process the queue
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
    visitor.on_examine_vertex({uid, *find_vertex(g_, uid)});

    for (auto&& [vid, uv, w] : views::incidence(g_, uid, weight)) {
      visitor.on_examine_edge({uid, vid, uv});

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (is_signed_v<weight_type>) {
		if (w < zero) {
		  throw graph_error("dijkstra_with_visitor: negative edge weight");
		}
	  }

      const bool is_neighbor_undiscovered = (distances[vid] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          visitor.on_edge_relaxed({uid, vid, uv});
          visitor.on_discover_vertex({vid, *find_vertex(g_, vid)});
          queue.push(vid);
        } else {
          throw graph_error("dijkstra_with_visitor: unexpected state where an edge to a new vertex was not relaxed");
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
    visitor.on_finish_vertex({uid, *find_vertex(g_, uid)});
  } // while(!queue.empty())
}

template <index_adjacency_list G,
          class Visitor,
          ranges::random_access_range Distances,
          ranges::random_access_range Predecessors,
          class WF         = std::function<ranges::range_value_t<Distances>(edge_reference_t<G>)>,
          class Compare    = less<ranges::range_value_t<Distances>>,
          class Combine    = plus<ranges::range_value_t<Distances>>,
          _queueable Queue = priority_queue<vertex_id_t<G>,
                                            vector<vertex_id_t<G>>,
                                            greater<vertex_id_t<G>>>>
requires is_arithmetic_v<ranges::range_value_t<Distances>> && //
         convertible_to<vertex_id_t<G>, ranges::range_value_t<Predecessors>> &&
         basic_edge_weight_function<G,
                                    WF,
                                    ranges::range_value_t<Distances>,
                                    Compare,
                                    Combine> //&& dijkstra_visitor<G, Visitor>
void dijkstra_with_visitor(
      G&             g_,
      Visitor&&      visitor,
      vertex_id_t<G> seed,
      Predecessors&  predecessor,
      Distances&     distances,
      WF&            weight =
            [](edge_reference_t<G> uv) { return ranges::range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Compare&& compare = less<ranges::range_value_t<Distances>>(),
      Combine&& combine = plus<ranges::range_value_t<Distances>>(),
      Queue     queue   = Queue()) {
  dijkstra_with_visitor(g_, forward<Visitor>(visitor), ranges::subrange(&seed, (&seed + 1)), predecessor, distances,
                        weight, forward<Compare>(compare), forward<Combine>(combine), queue);
}

} // namespace std::graph::experimental
