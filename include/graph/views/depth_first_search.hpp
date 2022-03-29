//
//	Author: J. Phillip Ratzloff
//
// inspired by new_dfs_range.hpp from NWGraph
//
// depth-first search graph algorithms for vertices and edges.
//
// examples: for(auto&& [uid,u]     : vertices_depth_first_search(g,seed))
//           for(auto&& [uid,u,val] : vertices_depth_first_search(g,seed,vvf))
//
//           for(auto&& [vid,uv]     : edges_depth_first_search(g,seed))
//           for(auto&& [vid,uv,val] : edges_depth_first_search(g,seed,evf))
//
// (need to add examples for Sourced and Cancelable after they're implemented)
//

#include "../graph.hpp"
#include "graph/views/views_utility.hpp"
#include <stack>
#include <vector>
#include <functional>

#ifndef GRAPH_DFS_HPP
#  define GRAPH_DFS_HPP

namespace std::graph::views {

// features to consider:

enum three_colors : int8_t { black, white, grey }; // { finished, undiscovered, discovered }
enum struct cancel_search : int8_t { continue_search, cancel_branch, cancel_all };


template <incidence_graph G>
struct dfs_elem {
  vertex_id_t<G>            u_id;
  vertex_edge_iterator_t<G> uv;
};

//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, class Stack>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_base : public ranges::view_base {
public:
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;

private:
  using graph_ref_type = reference_wrapper<graph_type>;
  using stack_elem     = dfs_elem<graph_type>;

  using parent_alloc = typename allocator_traits<typename Stack::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  dfs_base(graph_type& g, vertex_id_type seed = 0) : graph_(&g), colors_(ranges::size(vertices(g)), white) {
    if (seed < ranges::size(vertices(*graph_)) && !ranges::empty(edges(*graph_, seed))) {
      edge_iterator uv = ranges::begin(edges(*graph_, seed));
      S_.push(stack_elem{seed, uv});
      colors_[seed] = grey;
    }
  }
  dfs_base()                = default;
  dfs_base(const dfs_base&) = delete; // can be expensive to copy
  dfs_base(dfs_base&&)      = default;
  ~dfs_base()               = default;

  dfs_base& operator=(const dfs_base&) = delete;
  dfs_base& operator=(dfs_base&&) = default;

  constexpr bool empty() const noexcept { return S_.empty(); }

  constexpr auto size() const noexcept { return S_.size(); }
  constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept { return cancel_; }

protected:
  void advance() requires directed_incidence_graph<graph_type> {
    auto is_unvisited = [this](edge_reference vw) -> bool { return colors_[target_id(*graph_, vw)] == white; };

    // next level in search
    auto [u_id, uvi]    = S_.top();
    vertex_id_type v_id = target_id(*graph_, *uvi);

    edge_iterator vwi = ranges::end(edges(*graph_, v_id));
    switch (cancel_) {
    case cancel_search::continue_search:
      // find first unvisited edge of v
      vwi = ranges::find_if(edges(*graph_, v_id), is_unvisited);
      break;
    case cancel_search::cancel_branch: {
      cancel_       = cancel_search::continue_search;
      colors_[v_id] = black; // finished with v

      // Continue with sibling?
      uvi = ranges::find_if(++uvi, ranges::end(edges(*graph_, u_id)), is_unvisited);
      if (uvi != ranges::end(edges(*graph_, u_id))) {
        S_.top().uv = uvi;
        return;
      }
      // drop thru to unwind the stack to the parent
    } break;
    case cancel_search::cancel_all:
      while (!S_.empty())
        S_.pop();
      return;
    }

    // unvisited edge found for vertex v?
    if (vwi != ranges::end(edges(*graph_, v_id))) {
      S_.push(stack_elem{v_id, vwi});
      vertex_id_type w_id = target_id(*graph_, *vwi);
      colors_[w_id]       = grey; // visited w
    }
    // we've reached the end of a branch in the DFS tree; start unwinding the stack to find other unvisited branches
    else {
      colors_[v_id] = black; // finished with v
      S_.pop();
      while (!S_.empty()) {
        auto [x_id, xyi] = S_.top();
        S_.pop();
        xyi = ranges::find_if(++xyi, ranges::end(edges(*graph_, x_id)), is_unvisited);

        // unvisted edge found for vertex x?
        if (xyi != ranges::end(edges(*graph_, x_id))) {
          S_.push({x_id, xyi});
          vertex_id_type y_id = target_id(*graph_, *xyi);
          colors_[y_id]       = grey; // visited y
          break;
        } else {
          colors_[x_id] = black; // finished with x
        }
      }
    }
  }

  static vertex_id_type undir_target_id(const graph_type& g,
                                        edge_reference    e,
                                        vertex_id_type    src) requires undirected_incidence_graph<graph_type> {
    if (target_id(g, e) != src)
      return target_id(g, e);
    else
      return source_id(g, e);
  }

  void advance() requires undirected_incidence_graph<graph_type> {
    // next level in search
    auto [u_id, uvi]    = S_.top();
    vertex_id_type v_id = target_id(*graph_, *uvi);

    edge_iterator vwi = ranges::end(edges(*graph_, v_id));
    switch (cancel_) {
    case cancel_search::continue_search:
      // find first unvisited edge of v
      vwi = ranges::find_if(edges(*graph_, v_id), [this, v_id](edge_reference vw) -> bool {
        return colors_[undir_target_id(*graph_, vw, v_id)] == white;
      });
      break;
    case cancel_search::cancel_branch: {
      cancel_       = cancel_search::continue_search;
      colors_[v_id] = black; // finished with v

      // Continue with sibling?
      uvi = ranges::find_if(++uvi, [this, u_id](edge_reference uv) -> bool {
        return colors_[undir_target_id(*graph_, uv, u_id)] == white;
                });
      if (uvi != ranges::end(edges(*graph_, u_id))) {
        S_.top().uv = uvi;
        return;
      }
      // drop thru to unwind the stack to the parent
    } break;
    case cancel_search::cancel_all:
      while (!S_.empty())
        S_.pop();
      return;
    }

    // unvisited edge found for vertex v?
    if (vwi != ranges::end(edges(*graph_, v_id))) {
      S_.push(stack_elem{v_id, vwi});
      vertex_id_type w_id = undir_target_id(*graph_, *vwi, v_id);
      colors_[w_id]       = grey; // visited w
    }
    // we've reached the end of a branch in the DFS tree; start unwinding the stack to find other unvisited branches
    else {
      colors_[v_id] = black; // finished with v
      S_.pop();
      while (!S_.empty()) {
        auto [x_id, xyi] = S_.top();
        S_.pop();
        xyi = ranges::find_if(++xyi, ranges::end(edges(*graph_, x_id)), [this, x_id](edge_reference xy) -> bool {
          return colors_[undir_target_id(*graph_, xy, x_id)] == white;
        });

        // unvisted edge found for vertex x?
        if (xyi != ranges::end(edges(*graph_, x_id))) {
          S_.push({x_id, xyi});
          vertex_id_type y_id = undir_target_id(*graph_, *xyi, x_id);
          colors_[y_id]       = grey; // visited y
          break;
        } else {
          colors_[x_id] = black; // finished with x
        }
      }
    }
  }

protected : graph_type* graph_;
  Stack                 S_;
  vector<three_colors>  colors_;
  cancel_search         cancel_ = cancel_search::continue_search;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, class VVF = void, class Stack = stack<dfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_vertex_range : public dfs_base<G, Stack> {
public:
  using base_type        = dfs_base<G, Stack>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = dfs_vertex_range<graph_type, VVF, Stack>;

  using vertex_value_func = VVF;
  using vertex_value_type = invoke_result_t<VVF, vertex_reference>;

public:
  dfs_vertex_range(graph_type& g, vertex_id_type seed, const VVF& value_fn)
        : base_type(g, seed), value_fn_(&value_fn) {}
  dfs_vertex_range()                        = default;
  dfs_vertex_range(const dfs_vertex_range&) = delete; // can be expensive to copy
  dfs_vertex_range(dfs_vertex_range&&)      = default;
  ~dfs_vertex_range()                       = default;

  dfs_vertex_range& operator=(const dfs_vertex_range&) = delete;
  dfs_vertex_range& operator=(dfs_vertex_range&&) = default;

public:
  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_view<const vertex_id_type, vertex_type&, vertex_value_type>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = remove_reference_t<vertex_reference>;
    using shadow_value_type =
          vertex_view<vertex_id_t<graph_type>, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
    iterator()                = default;
    iterator(const iterator&) = default;
    iterator(iterator&&)      = default;
    ~iterator()               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&) = default;

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
      auto& g             = *the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = 0;
      if constexpr (directed_incidence_graph<graph_type>) {
        v_id = target_id(g, *uvi);
      } else {
        v_id = undir_target_id(g, *uvi, u_id);
      }
      auto& v = *find_vertex(g, v_id);
      value_  = {v_id, &v, invoke(*the_range_->value_fn_, v)};
      return reinterpret_cast<reference>(value_);
    }

    constexpr bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    constexpr bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    dfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }

private:
  const VVF* value_fn_ = nullptr;
};


template <incidence_graph G, class Stack>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_vertex_range<G, void, Stack> : public dfs_base<G, Stack> {
public:
  using base_type        = dfs_base<G, Stack>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = dfs_vertex_range<graph_type, void, Stack>;

public:
  dfs_vertex_range(graph_type& g, vertex_id_type seed) : base_type(g, seed) {}
  dfs_vertex_range()                        = default;
  dfs_vertex_range(const dfs_vertex_range&) = delete; // can be expensive to copy
  dfs_vertex_range(dfs_vertex_range&&)      = default;
  ~dfs_vertex_range()                       = default;

  dfs_vertex_range& operator=(const dfs_vertex_range&) = delete;
  dfs_vertex_range& operator=(dfs_vertex_range&&) = default;

public:
  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_view<const vertex_id_type, vertex_type&, void>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = remove_reference_t<vertex_reference>;
    using shadow_value_type  = vertex_view<vertex_id_t<graph_type>, shadow_vertex_type*, void>;

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
    iterator()                = default;
    iterator(const iterator&) = default;
    iterator(iterator&&)      = default;
    ~iterator()               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&) = default;

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
      auto& g             = *the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = 0;
      if constexpr (directed_incidence_graph<graph_type>) {
        v_id = target_id(g, *uvi);
      } else {
        v_id = undir_target_id(g, *uvi, u_id);
      }
      auto& v = *find_vertex(g, v_id);
      value_  = {v_id, &v};
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    dfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }
};


//---------------------------------------------------------------------------------------
/// depth-first search range for edges, given a single seed vertex.
///
template <incidence_graph G, class EVF = void, bool Sourced = false, class Stack = stack<dfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_edge_range : public dfs_base<G, Stack> {
public:
  using base_type           = dfs_base<G, Stack>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = dfs_edge_range<G, EVF, Sourced, Stack>;

  using edge_value_func = EVF;
  using edge_value_type = invoke_result_t<EVF, edge_reference_type>;

public:
  dfs_edge_range(G& g, vertex_id_type seed, const EVF& value_fn) : base_type(g, seed), value_fn_(&value_fn) {}

  dfs_edge_range()                      = default;
  dfs_edge_range(const dfs_edge_range&) = delete; // can be expensive to copy
  dfs_edge_range(dfs_edge_range&&)      = default;
  ~dfs_edge_range()                     = default;

  dfs_edge_range& operator=(const dfs_edge_range&) = delete;
  dfs_edge_range& operator=(dfs_edge_range&&) = default;

  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = edge_view<const vertex_id_type, Sourced, edge_reference_type, edge_value_type>;
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
    using shadow_edge_type = remove_reference_t<edge_reference_type>;
    using shadow_value_type =
          edge_view<vertex_id_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
    iterator()                = default;
    iterator(const iterator&) = default;
    iterator(iterator&&)      = default;
    ~iterator()               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&) = default;

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
      auto&& [u_id, uvi] = the_range_->S_.top();
      if constexpr (Sourced) {
        value_.source_id = u_id;
      }
      if constexpr (directed_incidence_graph<graph_type>) {
        value_.target_id = target_id(*the_range_->graph_, *uvi);
      } else {
        value_.target_id = undir_target_id(*the_range_->graph_, *uvi, u_id);
      }
      value_.edge  = &*uvi;
      value_.value = invoke(*the_range_->value_fn_, *uvi);
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    dfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }

private:
  const EVF* value_fn_ = nullptr;
};

template <incidence_graph G, bool Sourced, class Stack>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_edge_range<G, void, Sourced, Stack> : public dfs_base<G, Stack> {
public:
  using base_type           = dfs_base<G, Stack>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = dfs_edge_range<G, void, Sourced, Stack>;

public:
  dfs_edge_range(G& g, vertex_id_type seed) : base_type(g, seed) {}

  dfs_edge_range()                      = default;
  dfs_edge_range(const dfs_edge_range&) = delete; // can be expensive to copy
  dfs_edge_range(dfs_edge_range&&)      = default;
  ~dfs_edge_range()                     = default;

  dfs_edge_range& operator=(const dfs_edge_range&) = delete;
  dfs_edge_range& operator=(dfs_edge_range&&) = default;

  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = edge_view<const vertex_id_type, Sourced, edge_reference_type, void>;
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
    using shadow_edge_type  = remove_reference_t<edge_reference_type>;
    using shadow_value_type = edge_view<vertex_id_type, Sourced, shadow_edge_type*, void>;

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
    iterator()                = default;
    iterator(const iterator&) = default;
    iterator(iterator&&)      = default;
    ~iterator()               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&) = default;

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
      auto&& [u_id, uvi] = the_range_->S_.top();
      if constexpr (Sourced) {
        value_.source_id = u_id;
      }
      if constexpr (directed_incidence_graph<graph_type>) {
        value_.target_id = target_id(*the_range_->graph_, *uvi);
      } else {
        value_.target_id = undir_target_id(*the_range_->graph_, *uvi, u_id);
      }
      value_.edge = &*uvi;
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    dfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }
};

} // namespace std::graph::views

#endif // GRAPH_DFS_HPP
