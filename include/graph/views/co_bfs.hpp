#pragma once

#include "breadth_first_search.hpp"
#include "vertexlist.hpp"
#include <variant>
#include <coroutine>

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

enum class bfs_event : int {
  none              = 0, // useful?
  initialize_vertex = 0x0001,
  examine_vertex    = 0x0002,
  examine_edge      = 0x0004,
  discover_vertex   = 0x0008,
  edge_relaxed      = 0x0010,
  edge_not_relaxed  = 0x0020,
  finish_vertex     = 0x0040,

  vertex_default = discover_vertex, // useful?
  edge_default   = examine_edge     // useful?
};
//constexpr bfs_event& operator&=(bfs_event& lhs, bfs_event rhs) noexcept { return lhs &= rhs; }

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

template <class VId, class V, class VV, bool Sourced, class E, class EV>
using bfs_descriptor = std::variant<       //
      vertex_descriptor<VId, V, VV>,       //
      edge_descriptor<VId, Sourced, E, EV> //
      >;
// bfs_descriptor --> traverse_descriptor? (include neighbor_descriptor?)

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


#  if 0
template <index_adjacency_list G,
          bool                 IncludeVertex = false,
          bool                 Sourced       = false,
          bool                 IncludeEdge   = false,
          class Alloc                        = allocator<bool>>
class co_bfs {
public:
  using graph_type = remove_reference<G>;

  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_iterator       = vertex_iterator_t<graph_type>;
  using vertex_reference_type = conditional<IncludeVertex, vertex_reference_t<graph_type>, void>;

  using edge_type           = edge_t<G>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_reference_type = conditional<IncludeEdge, edge_reference_t<graph_type>, void>;

  using vertex_value_type = vertex_descriptor<vertex_id_type, vertex_reference_type, void>;
  using edge_value_type   = edge_descriptor<vertex_id_type, Sourced, edge_reference_type, void>;
  using value_type        = variant<vertex_value_type, edge_value_type>;

  using bfs_range_type = co_bfs<G, IncludeVertex, Sourced, IncludeEdge, Alloc>;

private:
  using graph_ref_type = reference_wrapper<graph_type>;
  using Queue          = queue<vertex_id_t<G>>;
  using queue_elem     = vertex_id_type;

  using parent_alloc = typename allocator_traits<typename Queue::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  co_bfs(G& g, vertex_id_type seed, bfs_event event_flags, const Alloc& alloc = Alloc())
        : graph_(&g), Q_(alloc), colors_(alloc), event_flags_(event_flags) {
    // Init colors_(ranges::size(vertices(g)), white, alloc); callback if initialize_vertex is set
    if (event_flags_ & bfs_event::initialize_vertex) {
      colors_.resize(ranges::size(vertices(g)), white);
      colors_.reserve(ranges::size(vertices(g)));
      for (auto&& [uid, u] : vertices(g)) {
        colors_.push_back(white);
        // Constructors cannot be coroutines
        // co_yield [bfs_event::initialize_vertex, vertex_value{uid,u}];
      }
    } else {
      colors_.resize(ranges::size(vertices(g)), white);
    }

    if (seed < ranges::size(vertices(*graph_)) && !ranges::empty(edges(*graph_, seed))) {
      uv_ = ranges::begin(edges(*graph_, seed));
      Q_.push(queue_elem{seed});
      colors_[seed] = grey;
    }
  }

  co_bfs()              = default;
  co_bfs(const co_bfs&) = delete; // can be expensive to copy
  co_bfs(co_bfs&&)      = default;
  ~co_bfs()             = default;

  co_bfs& operator=(const co_bfs&) = delete;
  co_bfs& operator=(co_bfs&&)      = default;

  // Properties
public:
  constexpr bool empty() const noexcept { return Q_.empty(); }

  constexpr auto size() const noexcept { return Q_.size(); }
  //constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept { return cancel_; }

  // Operations
protected:
  constexpr vertex_id_type real_target_id(edge_reference_t<G> uv, vertex_id_type) const
  requires ordered_edge<G, edge_type>
  {
    return target_id(*graph_, uv);
  }
  constexpr vertex_id_type real_target_id(edge_reference_t<G> uv, vertex_id_type src) const
  requires unordered_edge<G, edge_type>
  {
    if (target_id(*graph_, uv) != src)
      return target_id(*graph_, uv);
    else
      return source_id((*graph_), uv);
  }

  constexpr vertex_edge_iterator_t<G> find_unvisited(vertex_id_t<G> uid, vertex_edge_iterator_t<G> first) {
    return ranges::find_if(first, ranges::end(edges(*graph_, uid)), [this, uid](edge_reference_t<G> uv) -> bool {
      return colors_[real_target_id(uv, uid)] == white;
    });
  }

  void advance() {
    // current frontier vertex
    auto           u_id = Q_.front();
    vertex_id_type v_id = real_target_id(*uv_, u_id);

    switch (cancel_) {
    case cancel_search::continue_search:
      Q_.push(queue_elem{v_id});
      colors_[v_id] = grey; // visited v
      uv_           = find_unvisited(u_id, ++uv_);
      break;
    case cancel_search::cancel_branch:
      cancel_       = cancel_search::continue_search;
      colors_[v_id] = black;
      uv_           = find_unvisited(u_id, ++uv_);
      break; // u will be marked completed below
    case cancel_search::cancel_all:
      while (!Q_.empty())
        Q_.pop();
      return;
    }

    // visited all neighbors of u, or cancelled u
    if (uv_ == ranges::end(edges(*graph_, u_id))) {
      colors_[u_id] = black; // finished with u
      Q_.pop();
      while (!Q_.empty()) {
        u_id = Q_.front();
        uv_  = find_unvisited(u_id, ranges::begin(edges(*graph_, u_id)));
        if (uv_ != ranges::end(edges(*graph_, u_id))) {
          break;
        } else {
          Q_.pop();
          colors_[u_id] = black;
        }
      }
    }
  }

  // Iteration
public:
  class iterator;
  struct end_sentinel {
    bool operator==(const iterator& rhs) const noexcept { return rhs.the_range_->Q_.empty(); }
  };

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, void>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    // avoid difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = conditional<IncludeVertex, add_pointer<vertex_t<graph_type>>, void>;
    using shadow_edge_type   = conditional<IncludeEdge, add_pointer<edge_t<graph_type>>, void>;
    using shadow_value_type  = variant<shadow_vertex_type, shadow_edge_type>;

    union internal_value {
      value_type        value_;
      shadow_value_type shadow_;

      internal_value(vertex_id_type start_at) : shadow_{shadow_vertex_type{start_at}} {
        if constexpr (IncludeVertex)
          get<shadow_vertex_type>().vertex = nullptr;
      }
      internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
      internal_value() : shadow_{} {}
      ~internal_value() {}
      internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
    };

  public:
    iterator(const bfs_range_type& range) : the_range_(&const_cast<bfs_range_type&>(range)) {}
    iterator()                = default;
    iterator(const iterator&) = default;
    iterator(iterator&&)      = default;
    ~iterator()               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&)      = default;

    iterator& operator++() {
      the_range_->advance();
      return *this;
    }
    iterator operator++(int) const {
      iterator temp(*this);
      ++*this;
      return temp;
    }

    reference operator*() const noexcept {

      auto&& u_id = the_range_->Q_.front();
      auto&& uvi  = the_range_->uv_;
      if constexpr (Sourced) {
        value_.shadow_.source_id = u_id;
      }
      value_.shadow_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.shadow_.edge      = &*uvi;
      return value_.value_;
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->Q_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable internal_value value_;
    bfs_range_type*        the_range_ = nullptr;
    friend end_sentinel;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }

protected:
  graph_type*               graph_ = nullptr;
  Queue                     Q_;
  vertex_edge_iterator_t<G> uv_;
  vector<three_colors>      colors_;
  bfs_event                 event_flags_ = bfs_event::none;
  cancel_search             cancel_      = cancel_search::continue_search;
};
#  endif //0


} // namespace std::graph

#endif // GRAPH_CO_BFS_HPP
