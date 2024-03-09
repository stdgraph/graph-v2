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


//enum class bfs_features : int {
//  edge_basic     = 0x0000,
//  edge_reference = 0x0001,
//
//  edge_unsourced = 0x0000,
//  edge_sourced   = 0x0002,
//
//  vertex_basic     = 0x0000,
//  vertex_reference = 0x0001
//};

// using bfs_desc = bfs_descriptor<...>;
//
// The following functions would be overloaded to access all possible values of the variant.
// source_id(bfs_desc);
// target_id(bfs_desc);
// edge(bfs_desc);
// edge_value(bfs_desc);
// vertex_id(bfs_desc);
// vertex(bfs_desc);
// vertex_value(bfs_desc);

// Additional values for bfs_descriptor? (e.g. distance, color, predecessor, etc.)
// An alternative could be a pointer to an internal structure that holds these values, and the functions would be overloaded to access them.
// That would be a much more versatile solution that goes beyond the existing descriptors.
// Structured bindings would no longer be necessary. (Could we retain them for our uses? Maybe retain them for existing views?)

/*
*/

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
    void                unhandled_exception() {
      exception_ = std::current_exception();
    } // saving
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


enum class bfs_event : int {
  none              = 0,      // useful?
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
constexpr bfs_event& operator&=(bfs_event& lhs, bfs_event rhs) noexcept {
  lhs = static_cast<bfs_event>(static_cast<int>(lhs) & static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_event  operator&(bfs_event lhs, bfs_event rhs) noexcept { return (lhs &= rhs); }
constexpr bfs_event& operator|=(bfs_event& lhs, bfs_event rhs) noexcept {
  lhs = static_cast<bfs_event>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}
constexpr bfs_event operator|(bfs_event lhs, bfs_event rhs) noexcept { return (lhs |= rhs); }

template <class VId, class V, bool Sourced, class E>
using bfs_descriptor = std::variant<         // --> traverse_descriptor? (include neighbor_descriptor?)
      vertex_descriptor<VId, V, void>,       //
      edge_descriptor<VId, Sourced, E, void> //
      >;

template <class G>
using bfs_vertex_value_t = vertex_descriptor<vertex_id_t<G>, vertex_t<G>, void>;
template <class G>
using bfs_edge_value_t = edge_descriptor<vertex_id_t<G>, true, edge_t<G>, void>;
template <class G>
using bfs_variant_value_t = variant<bfs_vertex_value_t<G>, bfs_edge_value_t<G>>;

template <class G>
using bfs_value_t = pair<bfs_event, bfs_variant_value_t<G>>;

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

inline static null_range_type null_predecessors;


template <class... Ts>
struct print_types_t;

template <class... Ts>
constexpr auto print_types(Ts...) {
  return print_types_t<Ts...>{};
}

#  define yield_vertex(event, uid, u)                                                                                  \
    if ((event & events) != bfs_event::none)                                                                           \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_vertex_type { uid, u }                                                                              \
      }

#  define yield_edge(event, uid, vid, uv)                                                                              \
    if ((event & events) != bfs_event::none)                                                                           \
      co_yield bfs_value_type {                                                                                        \
        event, bfs_edge_type { uid, vid, uv }                                                                          \
      }

template <index_adjacency_list G>
Generator<bfs_value_t<G>> co_bfs(G&&            g,    // graph
                                 vertex_id_t<G> seed, // starting vertex_id
                                 bfs_event      events)    // events to stop at
{
  using id_type         = vertex_id_t<G>;
  using bfs_vertex_type = bfs_vertex_value_t<G>;
  using bfs_edge_type   = bfs_edge_value_t<G>;
  using bfs_value_type  = bfs_value_t<G>;

  size_t N(size(vertices(g))); // Question(Andrew): Do we want a num_vertices(g) CPO?
  assert(seed < N && seed >= 0);

  vector<three_colors> color(N, three_colors::white);

  if ((events & bfs_event::initialize_vertex) != bfs_event::none) {
    for (id_type uid = 0; uid < num_vertices(g); ++uid) {
      yield_vertex(bfs_event::initialize_vertex, uid, *find_vertex(g, uid));
    }
  }
  color[seed] = three_colors::gray;
  yield_vertex(bfs_event::discover_vertex, seed, *find_vertex(g, seed));

  using q_compare = decltype([](const id_type& a, const id_type& b) { return a > b; });
  std::priority_queue<id_type, vector<id_type>, q_compare> Q;

  // Remark(Andrew):  CLRS puts all vertices in the queue to start but standard practice seems to be to enqueue source
  Q.push(seed);

  while (!Q.empty()) {
    const id_type uid = Q.top();
    Q.pop();
    yield_vertex(bfs_event::examine_vertex, uid, *find_vertex(g, uid));

    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      yield_edge(bfs_event::examine_edge, uid, vid, uv);

      if (color[vid] == three_colors::white) {
        color[vid] = three_colors::gray;
        yield_vertex(bfs_event::discover_vertex, vid, *find_vertex(g, vid));
        yield_edge(bfs_event::tree_edge, uid, vid, uv);
        Q.push(vid);
      } else {
        yield_edge(bfs_event::non_tree_edge, uid, vid, uv);
        if (color[vid] == three_colors::gray) {
          yield_vertex(bfs_event::gray_target, vid, *find_vertex(g, vid));
        } else {
          yield_vertex(bfs_event::black_target, vid, *find_vertex(g, vid));
        }
      }
    }
    color[uid] = three_colors::black;
    yield_vertex(bfs_event::finish_vertex, uid, *find_vertex(g, uid));
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
