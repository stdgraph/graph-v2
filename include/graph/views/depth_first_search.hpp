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
enum cancel_search : int8_t { continue_search, cancel_branch, cancel_all };


template <class VId, class V, class VV, class Depth, bool Cancelable>
struct dfs_vertex_view : public views::vertex_view<VId, V, VV> {
  Depth         depth;
  cancel_search cancel;
  //VId         parent_id;
  //bool        is_path_end;
};


template <class VId, class E, class EV, class Depth, bool Cancelable>
struct dfs_edge_view : public views::edge_view<VId, false, E, EV> {
  Depth         depth;
  cancel_search cancel;
  //VId         parent_id;
  //bool        is_back_edge;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, typename Stack = std::stack<vertex_id_t<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_vertex_range {
public:
  using graph_type      = G;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<graph_type>;

public:
  dfs_vertex_range(graph_type& g, vertex_id_type seed = 0) : g_(g), colors_(g.end() - g.begin(), white) {
    S_.push(seed);
    colors_[seed] = grey;

    dfs_visit(g_.begin(), S_, colors_, seed);
  }

  dfs_vertex_range(const dfs_vertex_range&)  = delete;
  dfs_vertex_range(const dfs_vertex_range&&) = delete;

  constexpr bool empty() const noexcept { return S_.empty(); }

private:
  template <typename GraphRange>
  static void dfs_visit(const GraphRange& GR, Stack& S, std::vector<three_colors>& colors, vertex_id_type v) {
    auto u     = GR[v].begin();
    auto u_end = GR[v].end();

    while (u != u_end) {
      if (colors[std::get<0>(*u)] == white) {
        S.push(v);
        colors[v] = grey;

        v     = std::get<0>(*u);
        u     = GR[v].begin();
        u_end = GR[v].end();
      } else {
        ++u;
      }
      S.push(v);
      colors[v] = black;
    }
  }

public:
  class dfs_range_iterator {
  public:
    dfs_range_iterator(dfs_vertex_range<graph_type>& range) : the_range_(range), cursor_(0) {}

    dfs_range_iterator& operator++() {
      auto  GR     = the_range_.g_.begin();
      auto& S      = the_range_.S_;
      auto& colors = the_range_.colors_;

      while (!S.empty() && colors[S.top()] == black)
        S.pop();

      if (!S.empty()) {
        dfs_visit(GR, S, colors, S.top());
      } else {
        while (colors[++cursor_] != white && cursor_ != the_range_.g_.size())
          ;
        if (cursor_ != the_range_.g_.size()) {
          S.push(cursor_);
          colors[cursor_] = grey;
          dfs_visit(GR, S, colors, cursor_);
        }
      }

      return *this;
    }

    auto operator*() { return the_range_.S_.top(); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }

  private:
    dfs_vertex_range<graph_type>& the_range_;
    vertex_id_type                cursor_;
  };

  typedef dfs_range_iterator iterator;

  auto begin() { return dfs_range_iterator(*this); }
  auto end() { return typename dfs_range_iterator::end_sentinel_type(); }

private:
  graph_type&               g_;
  Stack                     S_;
  std::vector<three_colors> colors_;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for edges, given a single seed vertex.
///
template <incidence_graph G, typename A = allocator<char>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_edge_range {
public:
  dfs_edge_range(G& graph, vertex_id_t<G> seed, A alloc = A());

public:
  class const_iterator {};
  class iterator : public const_iterator {};

public:
  iterator       begin();
  const_iterator begin() const;
  const_iterator cbegin() const;

  iterator       end();
  const_iterator end() const;
  const_iterator cend() const;
};

} // namespace std::graph::views

#endif // GRAPH_DFS_HPP
