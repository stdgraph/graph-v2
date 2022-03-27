//
//	Author: J. Phillip Ratzloff
//
// inspired by new_dfs_range.hpp from: BGL17
//
// depth-first search graph algorithms for vertices and edges.
//

#include "../graph.hpp"
#include "graph/views/views_utility.hpp"
#include <stack>
#include <vector>

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

template <incidence_graph G, bool Cancelable, class Stack>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_base : public ranges::view_interface<dfs_base<G, Cancelable, Stack>> {
public:
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;

private:
  using stack_elem = dfs_elem<graph_type>;

  using parent_alloc = typename allocator_traits<typename Stack::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  dfs_base(graph_type& g, vertex_id_type seed = 0)
        : graph_(g)
        , colors_(ranges::size(vertices(g)), white)
  {
    if (seed < ranges::size(vertices(graph_)) && !ranges::empty(edges(graph_, seed))) {
      edge_iterator uv = ranges::begin(edges(graph_, seed));
      S_.push(stack_elem{seed, uv});
      colors_[seed] = grey;
    }
  }
  dfs_base(const dfs_base&) = delete; // can be expensive to copy
  dfs_base(dfs_base&&)      = default;
  ~dfs_base()               = default;

  dfs_base& operator=(const dfs_base&) = delete;
  dfs_base& operator=(dfs_base&&) = default;

  constexpr bool empty() const noexcept { return S_.empty(); }

  constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept requires Cancelable { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept requires Cancelable { return cancel_; }

protected : void advance() {
    auto& S      = S_;
    auto& g      = graph_;
    auto& colors = colors_;
    auto is_unvisited = [&g, &colors](edge_reference vw) -> bool { return colors[target_id(g, vw)] == white; };

    // next level in search
    auto [u_id, uvi]    = S.top();
    vertex_id_type v_id = target_id(g, *uvi);
    edge_iterator  vwi  = ranges::find_if(edges(g, v_id), is_unvisited); // find first unvisited edge of v

    // unvisited edge found for vertex v?
    if (vwi != ranges::end(edges(g, v_id))) {
      S.push(stack_elem{v_id, vwi});
      vertex_id_type w_id = target_id(g, *vwi);
      colors[w_id]        = grey; // visited w
    }
    // we've reached the end of a branch in the DFS tree; start unwinding the stack to find other unvisited branches
    else {
      colors[v_id] = black; // finished with v
      S.pop();
      while (!S.empty()) {
        auto [x_id, xyi] = S.top();
        S.pop();
        xyi = ranges::find_if(++xyi, ranges::end(edges(g, x_id)), is_unvisited);

        // unvisted edge found for vertex x?
        if (xyi != ranges::end(edges(g, x_id))) {
          S.push({x_id, xyi});
          vertex_id_type y_id = target_id(g, *xyi);
          colors[y_id]        = grey; // visited y
          break;
        } else {
          colors[x_id] = black; // finished with x
        }
      }
    }
  }

protected:
  graph_type&          graph_;
  Stack                S_;
  vector<three_colors> colors_;
  cancel_search cancel_ = cancel_search::continue_search;
}; // namespace std::graph::views


//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, bool Cancelable = false, class Stack = stack<dfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_vertex_range
      : public dfs_base<G, Cancelable, Stack>
      , public ranges::view_interface<dfs_vertex_range<G, Cancelable, Stack>> {
public:
  using base_type        = dfs_base<G, Cancelable, Stack>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;

public:
  dfs_vertex_range(graph_type& g, vertex_id_type seed = 0) : base_type(g, seed) {}
  dfs_vertex_range(const dfs_vertex_range&) = delete; // can be expensive to copy
  dfs_vertex_range(dfs_vertex_range&&)      = default;
  ~dfs_vertex_range()                       = default;

  dfs_vertex_range& operator=(const dfs_vertex_range&) = delete;
  dfs_vertex_range& operator=(dfs_vertex_range&&) = default;

public:
  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_view<const vertex_id_type, vertex_type&, void>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using different_type    = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = remove_reference_t<vertex_reference>;
    using shadow_value_type =
          vertex_view<vertex_id_t<graph_type>, shadow_vertex_type*, void>; //_detail::ref_to_ptr<vertex_value_type>

  public:
    iterator(dfs_vertex_range<graph_type>& range) : the_range_(range) {}

    iterator& operator++() {
      the_range_.advance();
      return *this;
    }
    iterator operator++(int) const {
      iterator temp(*this);
      ++*this;
      return temp;
    }

    reference operator*() noexcept {
      auto&& [u_id, uvi] = the_range_.S_.top();
      value_             = {target_id(the_range_.graph_, *uvi), &target(the_range_.graph_, *uvi)};
      return reinterpret_cast<reference>(value_);
    }

    struct end_sentinel_type {};

    bool operator==(const end_sentinel_type&) const noexcept {
      if constexpr (Cancelable)
        return the_range_.S_.empty() || (the_range_.cancel_ == cancel_search::cancel_all);
      else
        return the_range_.S_.empty();
    }
    bool operator!=(const end_sentinel_type& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type     value_ = {};
    dfs_vertex_range<graph_type>& the_range_;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return typename iterator::end_sentinel_type(); }
  auto end() const { return typename iterator::end_sentinel_type(); }
  auto cend() const { return typename iterator::end_sentinel_type(); }
}; // namespace std::graph::views


//---------------------------------------------------------------------------------------
/// depth-first search range for edges, given a single seed vertex.
///
template <incidence_graph G, bool Cancelable = false, class Stack = stack<dfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_edge_range
      : public dfs_base<G, Cancelable, Stack>
      , public ranges::view_interface<dfs_edge_range<G, Cancelable, Stack>> {
public:
  using base_type           = dfs_base<G, Cancelable, Stack>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;

public:
  dfs_edge_range(G& g, vertex_id_type seed = 0) : base_type(g, seed) {}

  dfs_edge_range(const dfs_edge_range&) = delete;
  dfs_edge_range(dfs_edge_range&&)      = default;
  ~dfs_edge_range()                     = default;

  dfs_edge_range& operator=(const dfs_edge_range&) = delete;
  dfs_edge_range& operator=(dfs_edge_range&&) = default;

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = edge_view<const vertex_id_type, false, edge_reference_type, void>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using different_type    = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    // avoid difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_edge_type  = remove_reference_t<edge_reference_type>;
    using shadow_value_type = edge_view<vertex_id_type, false, shadow_edge_type*, void>;

  public:
    iterator(dfs_edge_range<G, Cancelable, Stack>& range) : the_range_(range) {}

    iterator& operator++() {
      the_range_.advance();
      return *this;
    }
    iterator operator++(int) const {
      iterator temp(*this);
      ++*this;
      return temp;
    }

    reference operator*() noexcept {
      auto&& [u_id, uvi] = the_range_.S_.top();
      value_             = {target_id(the_range_.graph_, *uvi), &*uvi};
      return reinterpret_cast<reference>(value_);
    }

    struct end_sentinel_type {};

    bool operator==(const end_sentinel_type&) const noexcept {
      if constexpr (Cancelable)
        return the_range_.S_.empty() || (the_range_.cancel_ == cancel_search::cancel_all);
      else
        return the_range_.S_.empty();
    }
    bool operator!=(const end_sentinel_type& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type             value_ = {};
    dfs_edge_range<G, Cancelable, Stack>& the_range_;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return typename iterator::end_sentinel_type(); }
  auto end() const { return typename iterator::end_sentinel_type(); }
  auto cend() const { return typename iterator::end_sentinel_type(); }
};

} // namespace std::graph::views

#endif // GRAPH_DFS_HPP
