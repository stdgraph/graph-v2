#pragma once

#include "vertexlist.hpp"
#include <variant>
#include <coroutine>

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include <queue>
#include <algorithm>


#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Woverloaded-virtual"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

//#include <boost/cobalt/generator.hpp>

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif


#ifndef GRAPH_CO_BFS_HPP
#  define GRAPH_CO_BFS_HPP

namespace std::graph {

template <class G, class F>
concept edge_weight_function = // e.g. weight(uv)
      copy_constructible<F> && is_arithmetic_v<invoke_result_t<F, edge_reference_t<G>>>;


// A simple generator from https://en.cppreference.com/w/cpp/language/coroutines.
template <typename T>
struct Generator {
  // The class name 'Generator' is our choice and it is not required for coroutine
  // magic. Compiler recognizes coroutine by the presence of 'co_yield' keyword.
  // You can use name 'MyGenerator' (or any other name) instead as long as you include
  // nested struct promise_type with 'MyGenerator get_return_object()' method.

  struct promise_type; // REQUIRED type name
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type // required
  {
    T                  value_;
    std::exception_ptr exception_;

    Generator           get_return_object() { return Generator(handle_type::from_promise(*this)); }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void                unhandled_exception() { exception_ = std::current_exception(); } // saving
    // exception

    template <std::convertible_to<T> From> // C++20 concept
    std::suspend_always yield_value(From&& from) {
      value_ = std::forward<From>(from); // caching the result in promise
      return {};
    }
    void return_void() {}
  };

  handle_type h_;

  Generator(handle_type h)
        : h_(h) //
  {}
  ~Generator() //
  {
    h_.destroy();
  }
  explicit operator bool() {
    fill(); // The only way to reliably find out whether or not we finished coroutine,
            // whether or not there is going to be a next value generated (co_yield)
            // in coroutine via C++ getter (operator () below) is to execute/resume
            // coroutine until the next co_yield point (or let it fall off end).
            // Then we store/cache result in promise to allow getter (operator() below
            // to grab it without executing coroutine).
    return !h_.done();
  }
  T operator()() {
    fill();
    full_ = false; // we are going to move out previously cached
                   // result to make promise empty again
    return std::move(h_.promise().value_);
  }

private:
  bool full_ = false;

  void fill() {
    if (!full_) {
      h_();
      if (h_.promise().exception_)
        std::rethrow_exception(h_.promise().exception_);
      // propagate coroutine exception in called context

      full_ = true;
    }
  }
};

#  if 0
Generator<std::uint64_t> fibonacci_sequence(unsigned n) {
  if (n == 0)
    co_return;

  if (n > 94)
    throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");

  co_yield 0;

  if (n == 1)
    co_return;

  co_yield 1;

  if (n == 2)
    co_return;

  std::uint64_t a = 0;
  std::uint64_t b = 1;

  for (unsigned i = 2; i < n; ++i) {
    std::uint64_t s = a + b;
    co_yield s;
    a = b;
    b = s;
  }
}

class fib_seq {
public:
  fib_seq(unsigned n) : n_(n) {}

  Generator<std::uint64_t> operator()() {
    if (n_ == 0)
      co_return;

    if (n_ > 94)
      throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");

    co_yield 0;

    if (n_ == 1)
      co_return;

    co_yield 1;

    if (n_ == 2)
      co_return;

    std::uint64_t a = 0;
    std::uint64_t b = 1;

    for (unsigned i = 2; i < n_; ++i) {
      std::uint64_t s = a + b;
      co_yield s;
      a = b;
      b = s;
    }
  }

private:
  unsigned n_ = 0;
};
#  endif //0


// These events duplicate boost::graph's BFSVisitorConcept
enum class bfs_events : int {
  none              = 0,
  initialize_vertex = 0x0001,
  discover_vertex   = 0x0002, // e.g. white_target
  examine_vertex    = 0x0004,
  examine_edge      = 0x0008,
  tree_edge         = 0x0010,
  non_tree_edge     = 0x0020,
  gray_target       = 0x0040,
  black_target      = 0x0080,
  finish_vertex     = 0x0100,
};
constexpr bfs_events& operator&=(bfs_events& lhs, bfs_events rhs) noexcept {
  lhs = static_cast<bfs_events>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_events  operator&(bfs_events lhs, bfs_events rhs) noexcept { return (lhs &= rhs); }
constexpr bfs_events& operator|=(bfs_events& lhs, bfs_events rhs) noexcept {
  lhs = static_cast<bfs_events>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_events operator|(bfs_events lhs, bfs_events rhs) noexcept { return (lhs |= rhs); }

// These types comprise the bfs value type, made up of bfs_events and variant<vertex_descriptor, edge_descriptor>.
// monostate is used to indicate that the value is not set and to make it default-constructible.
template <class G>
using bfs_vertex_value_t = vertex_descriptor<vertex_id_t<G>, reference_wrapper<vertex_t<G>>, void>;
template <class G>
using bfs_edge_value_t = edge_descriptor<vertex_id_t<G>, true, reference_wrapper<edge_t<G>>, void>;
template <class G>
using bfs_variant_value_t = variant<monostate, bfs_vertex_value_t<G>, bfs_edge_value_t<G>>;

template <class G>
using bfs_value_t = pair<bfs_events, bfs_variant_value_t<G>>;

// Helper macros to keep the visual clutter down in a coroutine. I'd like to investigate using CRTP to avoid them,
// but I'm not sure how it will play with coroutines.
#  define yield_vertex(event, uid)                                                                                     \
    if ((event & events) != bfs_events::none)                                                                          \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, *find_vertex(g, uid) }                                                           \
      }

#  define yield_edge(event, uid, vid, uv)                                                                              \
    if ((event & events) != bfs_events::none)                                                                          \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_edge_type { uid, vid, uv }                                                                          \
      }

// co_bfs is a coroutine that yields bfs_value_t<G> values as it traverses the graph, based on the
// events that are passed in. The generator is templated on the graph type, and the events are passed
// in as a bitmask.
//
// The implementation is based on boost::graph::breadth_first_visit.
//
// @param g      The graph to use
// @param seed   The starting vertex_id
// @param events A bitmap of the different events to stop at
//
template <index_adjacency_list G>
Generator<bfs_value_t<G>> co_bfs(G&&            g,    // graph
                                 vertex_id_t<G> seed, // starting vertex_id
                                 bfs_events     events)   // events to stop at
{
  using id_type         = vertex_id_t<G>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<G>;

  size_t N(num_vertices(g));
  assert(seed < N && seed >= 0);

  vector<three_colors> color(N, three_colors::white);

  if ((events & bfs_events::initialize_vertex) != bfs_events::none) {
    for (id_type uid = 0; uid < num_vertices(g); ++uid) {
      yield_vertex(bfs_events::initialize_vertex, uid);
    }
  }
  color[seed] = three_colors::gray;
  yield_vertex(bfs_events::discover_vertex, seed);

  using q_compare = decltype([](const id_type& a, const id_type& b) { return a > b; });
  std::priority_queue<id_type, vector<id_type>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push(seed);

  while (!Q.empty()) {
    const id_type uid = Q.top();
    Q.pop();
    yield_vertex(bfs_events::examine_vertex, uid);

    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      yield_edge(bfs_events::examine_edge, uid, vid, uv);

      if (color[vid] == three_colors::white) {
        color[vid] = three_colors::gray;
        yield_vertex(bfs_events::discover_vertex, vid);
        yield_edge(bfs_events::tree_edge, uid, vid, uv);
        Q.push(vid);
      } else {
        yield_edge(bfs_events::non_tree_edge, uid, vid, uv);
        if (color[vid] == three_colors::gray) {
          yield_vertex(bfs_events::gray_target, vid);
        } else {
          yield_vertex(bfs_events::black_target, vid);
        }
      }
    }
    color[uid] = three_colors::black;
    yield_vertex(bfs_events::finish_vertex, uid);
  }
}


enum class dijkstra_event : int {
  initialize_vertex,
  discover_vertex,
  examine_vertex,
  examine_edge,
  edge_relaxed,
  edge_not_relaxed,
  finish_vertex
};


} // namespace std::graph

#endif // GRAPH_CO_BFS_HPP
