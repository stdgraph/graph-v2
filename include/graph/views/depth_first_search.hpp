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


/// <summary>
/// Defines the value returned by a dfs_vertex_range iterator.
/// </summary>
/// <typeparam name="VId">Vertex Id</typeparam>
/// <typeparam name="V">Vertex type</typeparam>
/// <typeparam name="VV">Vertex Value type</typeparam>
/// <typeparam name="Depth">Depth type</typeparam>
/// <typeparam name="Cancelable">Can the caller cancel iteration?</typeparam>
template <class VId, class V, class VV, class Depth, bool Cancelable>
struct dfs_vertex_view : public vertex_view<VId, V, VV> {
  Depth         depth;
  cancel_search cancel;
  //VId         parent_id;
  //bool        is_path_end;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, class Stack = stack<vertex_id_t<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_vertex_range : public ranges::view_interface<dfs_vertex_range<G, Stack>> {
public:
  using graph_type      = G;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

public:
  dfs_vertex_range(graph_type& g, vertex_id_type seed = 0) : g_(g), colors_(ranges::size(vertices(g)), white) {
    S_.push(seed);
    colors_[seed] = grey;

    dfs_visit(seed);
  }
  dfs_vertex_range(const dfs_vertex_range&) = delete;
  dfs_vertex_range(dfs_vertex_range&&)      = default;
  ~dfs_vertex_range()                       = default;

  dfs_vertex_range& operator=(const dfs_vertex_range&) = delete;
  dfs_vertex_range& operator=(dfs_vertex_range&&) = default;

  constexpr bool empty() const noexcept { return S_.empty(); }

private:
  void dfs_visit(vertex_id_type uid) {
    auto uv     = ranges::begin(edges(g_, uid));
    auto uv_end = ranges::end(edges(g_, uid));

    while (uv != uv_end) {
      if (colors_[target_id(g_, *uv)] == white) {
        S_.push(uid);
        colors_[uid] = grey;

        uid    = target_id(g_, *uv);
        uv     = ranges::begin(edges(g_, uid));
        uv_end = ranges::end(edges(g_, uid));
      } else {
        ++uv;
      }
      S_.push(uid);
      colors_[uid] = black;
    }
  }

public:
  class dfs_range_iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_id_type;
    using reference         = value_type&;
    using pointer           = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using different_type    = ranges::range_difference_t<vertex_range_t<graph_type>>;

  public:
    dfs_range_iterator(dfs_vertex_range<graph_type>& range) : the_range_(range), cursor_(0) {}

    dfs_range_iterator& operator++() {
      auto& S      = the_range_.S_;
      auto& colors = the_range_.colors_;

      while (!S.empty() && colors[S.top()] == black)
        S.pop();

      if (!S.empty()) {
        the_range_.dfs_visit(S.top());
      } else {
        while (++cursor_ < ranges::size(vertices(the_range_.g_)) && colors[cursor_] != white)
          ;
        if (cursor_ < ranges::size(vertices(the_range_.g_))) {
          S.push(cursor_);
          colors[cursor_] = grey;
          the_range_.dfs_visit(cursor_);
        }
      }

      return *this;
    }
    dfs_range_iterator operator++(int) const {
      dfs_range_iterator temp(*this);
      ++*this;
      return temp;
    }

    reference operator*() noexcept { return the_range_.S_.top(); }

    struct end_sentinel_type {};

    bool operator==(const end_sentinel_type&) const noexcept { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const noexcept { return !the_range_.empty(); }

  private:
    dfs_vertex_range<graph_type>& the_range_;
    vertex_id_type                cursor_;
  };

  using iterator = dfs_range_iterator;

  auto begin() { return dfs_range_iterator(*this); }
  auto begin() const { return dfs_range_iterator(*this); }
  auto cbegin() const { return dfs_range_iterator(*this); }

  auto end() { return typename dfs_range_iterator::end_sentinel_type(); }
  auto end() const { return typename dfs_range_iterator::end_sentinel_type(); }
  auto cend() const { return typename dfs_range_iterator::end_sentinel_type(); }

private:
  graph_type&          g_;
  Stack                S_;
  vector<three_colors> colors_;
};


//---------------------------------------------------------------------------------------
/// <summary>
/// Defines the value returned by a dfs_edge_range iterator.
/// </summary>
/// <typeparam name="VId">Vertex Id</typeparam>
/// <typeparam name="E">Edge type</typeparam>
/// <typeparam name="EV">Edge Value type</typeparam>
/// <typeparam name="Depth">Depth type</typeparam>
/// <typeparam name="Cancelable">Can the caller cancel iteration?</typeparam>
template <class VId, bool Sourced, class E, class EV, class Depth>
struct dfs_edge_view {
  VId   source_id;
  VId   target_id;
  E     edge;
  EV    value;
  Depth depth;
};

template <class VId, class E>
struct dfs_edge_view<VId, true, E, void, void> {
  VId source_id;
  VId target_id;
  E   edge;
};
template <class VId>
struct dfs_edge_view<VId, true, void, void, void> {
  VId source_id;
  VId target_id;
};
template <class VId, class EV>
struct dfs_edge_view<VId, true, void, EV, void> {
  VId source_id;
  VId target_id;
  EV  value;
};

template <class VId, class E, class EV>
struct dfs_edge_view<VId, false, E, EV, void> {
  VId target_id;
  E   edge;
  EV  value;
};
template <class VId, class E>
struct dfs_edge_view<VId, false, E, void, void> {
  VId target_id;
  E   edge;
};

template <class VId, class EV>
struct dfs_edge_view<VId, false, void, EV, void> {
  VId target_id;
  EV  value;
};
template <class VId>
struct dfs_edge_view<VId, false, void, void, void> {
  VId target_id;
};


template <class VId, class E, class Depth>
struct dfs_edge_view<VId, true, E, void, Depth> {
  VId   source_id;
  VId   target_id;
  E     edge;
  Depth depth;
};
template <class VId, class Depth>
struct dfs_edge_view<VId, true, void, void, Depth> {
  VId   source_id;
  VId   target_id;
  Depth depth;
};
template <class VId, class EV, class Depth>
struct dfs_edge_view<VId, true, void, EV, Depth> {
  VId   source_id;
  VId   target_id;
  EV    value;
  Depth depth;
};

template <class VId, class E, class EV, class Depth>
struct dfs_edge_view<VId, false, E, EV, Depth> {
  VId   target_id;
  E     edge;
  EV    value;
  Depth depth;
};
template <class VId, class E, class Depth>
struct dfs_edge_view<VId, false, E, void, Depth> {
  VId   target_id;
  E     edge;
  Depth depth;
};

template <class VId, class EV, class Depth>
struct dfs_edge_view<VId, false, void, EV, Depth> {
  VId   target_id;
  EV    value;
  Depth depth;
};
template <class VId, class Depth>
struct dfs_edge_view<VId, false, void, void, Depth> {
  VId   target_id;
  Depth depth;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for edges, given a single seed vertex.
///
template <incidence_graph G, class Depth = size_t, bool Cancelable = false, class Stack = stack<vertex_id_t<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      (is_void_v<Depth> || integral<Depth>) //
      class dfs_edge_range {
public:
  using graph_type      = G;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;
  using edge_type       = edge_t<G>;

public:
  dfs_edge_range(G& g, vertex_id_type seed = 0) : g_(g), colors_(ranges::size(vertices(g)), white) {
    S_.push(seed);
    colors_[seed] = grey;
  }

  dfs_edge_range(const dfs_edge_range&) = delete;
  dfs_edge_range(dfs_edge_range&&)      = default;
  ~dfs_edge_range()                     = default;

  dfs_edge_range& operator=(const dfs_edge_range&) = delete;
  dfs_edge_range& operator=(dfs_edge_range&&) = default;

  constexpr bool empty() const noexcept { return S_.empty(); }

  class dfs_edge_range_iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = dfs_edge_view<vertex_id_type, false, edge_type&, void, Depth>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using size_type         = ranges::range_size_t<vertex_range_t<graph_type>>;
    using different_type    = ranges::range_difference_t<vertex_range_t<graph_type>>;

  private:
    using shadow_value_type = dfs_edge_view<vertex_id_type, false, edge_type*, void, Depth>;

  public:
    dfs_edge_range_iterator(dfs_edge_range<G, Depth, Cancelable, Stack>& range)
          : the_range_(range)
          , uid_(the_range_.S_.top())
          , uv_begin(ranges::begin(edges(the_range_.g_, uid_)))
          , uv_end(ranges::end(edges(the_range_.g_, uid_))) {}

    dfs_edge_range_iterator& operator++() {
      auto& S      = the_range_.S_;
      auto& colors = the_range_.colors_;

      S.push(uid_);
      colors[uid_] = grey;

      uid_     = target_id(the_range_.g_, *uv_begin);
      uv_begin = ranges::begin(edges(the_range_.g_, uid_));
      uv_end   = ranges::end(edges(the_range_.g_, uid_));

      // ++uv_begin;
      while (uv_begin != uv_end && colors[target_id(the_range_.g_, *uv_begin)] != white) {
        ++uv_begin;
      }

      while (uv_begin == uv_end) {
        colors[uid_] = black;
        uid_         = S.top();
        S.pop();
        if (S.empty())
          break;

        assert(colors[uid_] == grey);
        uv_begin = ranges::begin(edges(the_range_.g_, uid_));
        uv_end   = ranges::end(edges(the_range_.g_, uid_));

        while (uv_begin != uv_end && colors[target_id(the_range_.g_, *uv_begin)] != white) {
          ++uv_begin;
        }
      }

      return *this;
    }

    reference operator*() const {
      //value_ = {uid_, &*uv_begin};
      value_.target_id = uid_;
      value_.edge      = &*uv_begin;
      if constexpr (!is_void_v<Depth>)
        value_.depth = static_cast<Depth>(ranges::size(the_range_.S_));
      return reinterpret_cast<reference>(value_);
      //return std::tuple_cat(std::make_tuple(uid_), *uv_begin);
    }

    struct end_sentinel_type {};

    bool operator==(const end_sentinel_type&) const noexcept {
      if constexpr (Cancelable)
        return the_range_.empty() || (cancel_ == cancel_search::cancel_all);
      else
        return the_range_.empty();
    }
    bool operator!=(const end_sentinel_type& rhs) const noexcept { return !operator==(rhs); }

    void cancel(cancel_search cancel_type) noexcept requires Cancelable { cancel_ = cancel_type; }

  private : mutable shadow_value_type            value_;
    dfs_edge_range<G, Depth, Cancelable, Stack>& the_range_;
    vertex_id_type                               uid_;
    vertex_edge_iterator_t<G>                    uv_begin, uv_end;
  };

  using iterator = dfs_edge_range_iterator;

  auto begin() { return dfs_edge_range_iterator(*this); }
  auto begin() const { return dfs_edge_range_iterator(*this); }
  auto cbegin() const { return dfs_edge_range_iterator(*this); }

  auto end() { return typename dfs_edge_range_iterator::end_sentinel_type(); }
  auto end() const { return typename dfs_edge_range_iterator::end_sentinel_type(); }
  auto cend() const { return typename dfs_edge_range_iterator::end_sentinel_type(); }

private:
  G&                   g_;
  Stack                S_;
  vector<three_colors> colors_;
  cancel_search        cancel_ = cancel_search::continue_search;
};

} // namespace std::graph::views

#endif // GRAPH_DFS_HPP
