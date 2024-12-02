
#ifndef NWGRAPH_BFS_EDGE_RANGE_HPP
#define NWGRAPH_BFS_EDGE_RANGE_HPP

//#include "graph_concepts.hpp"
#include "graph/graph.hpp"

#include <cassert>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>

enum three_colors { black, white, grey };

template <typename Graph, typename Queue = std::queue<graph::vertex_id_t<Graph>>>
requires graph::adjacency_list<Graph>
class bfs_edge_range {
private:
  using vertex_id_type = graph::vertex_id_t<Graph>;

public:
  explicit bfs_edge_range(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), visited_(graph.size(), false) {
    Q_.push(seed);
    visited_[seed] = true;
  }

  bfs_edge_range(const bfs_edge_range&)  = delete;
  bfs_edge_range(const bfs_edge_range&&) = delete;

  bool empty() {
    bool b = Q_.empty();
    return b;
  }

  class bfs_edge_range_iterator {
  private:
    bfs_edge_range<Graph, Queue>& the_range_;
    typename Graph::iterator      G;
    vertex_id_type                v_;
    //typename inner_range_t<Graph>::iterator u_begin, u_end;
    graph::vertex_iterator_t<Graph> u_begin, u_end;

  public:
    bfs_edge_range_iterator(bfs_edge_range<Graph, Queue>& range)
          : the_range_(range)
          , G(the_range_.the_graph_.begin())
          , v_(the_range_.Q_.front())
          , u_begin(G[v_].begin())
          , u_end(G[v_].end()) {}

    bfs_edge_range_iterator(const bfs_edge_range_iterator& ite)
          : the_range_(ite.the_range_), G(ite.G), v_(ite.v_), u_begin(u_begin), u_end(u_end) {}

    bfs_edge_range_iterator& operator++() {
      auto& Q       = the_range_.Q_;
      auto& visited = the_range_.visited_;

      visited[target(the_range_.the_graph_, *u_begin)] = true;
      Q.push(target(the_range_.the_graph_, (*u_begin)));

      ++u_begin;
      while ((u_begin != u_end) && (visited[target(the_range_.the_graph_, *u_begin)] != false)) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        Q.pop();
        if (Q.empty())
          break;

        v_ = Q.front();

        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && visited[target(the_range_.the_graph_, *u_begin)] != false) {
          ++u_begin;
        }
      }

      return *this;
    }

    //    auto operator*() { return std::make_tuple(v_, target(the_range_.the_graph_, *u_begin)); }
    auto operator*() { return std::tuple_cat(std::make_tuple(v_), untarget(the_range_.the_graph_, *u_begin)); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };

  typedef bfs_edge_range_iterator iterator;

  auto begin() { return bfs_edge_range_iterator(*this); }
  auto end() { return typename bfs_edge_range_iterator::end_sentinel_type(); }

private:
  Graph&            the_graph_;
  Queue             Q_;
  std::vector<bool> visited_;
};

template <typename Graph, typename PriorityQueue>
requires graph::adjacency_list<Graph>
class bfs_edge_range_2 {
private:
  using vertex_id_type = typename Graph::vertex_id_type;

public:
  bfs_edge_range_2(Graph& graph, PriorityQueue& Q, std::tuple<size_t, size_t> seed = {0, 0})
        : the_graph_(graph), Q_(Q), colors_(graph.end() - graph.begin(), white) {
    Q_.push(seed);
    colors_[std::get<0>(seed)] = grey;
  }

  bfs_edge_range_2(const bfs_edge_range_2&)  = delete;
  bfs_edge_range_2(const bfs_edge_range_2&&) = delete;

  bool empty() { return Q_.empty(); }

  class bfs_edge_range_2_iterator {
  private:
    bfs_edge_range_2<Graph, PriorityQueue>& the_range_;
    typename Graph::outer_iterator          G;
    vertex_id_type                          v_;
    typename Graph::inner_iterator          u_begin, u_end;

    // Graph -> v, u, w
    // Q -> v, d

  public:
    bfs_edge_range_2_iterator(bfs_edge_range_2<Graph, PriorityQueue>& range)
          : the_range_(range)
          , G(the_range_.the_graph_.begin())
          , v_(std::get<0>(the_range_.Q_.top()))
          , u_begin(G[v_].begin())
          , u_end(G[v_].end()) {}

    bfs_edge_range_2_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      Q.push({std::get<0>(*u_begin), size_t(0xffffffffffffffffULL)});
      colors[std::get<0>(*u_begin)] = grey;

      ++u_begin;
      while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        colors[v_] = black;

        while (colors[std::get<0>(Q.top())] == black && !Q.empty())
          Q.pop();

        if (Q.empty())
          break;

        v_      = std::get<0>(Q.top());
        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
          ++u_begin;
        }
      }

      return *this;
    }

    auto operator*() {
      return std::tuple<vertex_id_type, vertex_id_type, size_t>(v_, std::get<0>(*u_begin), std::get<1>(*u_begin));
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };

  typedef bfs_edge_range_2_iterator iterator;

  auto begin() { return bfs_edge_range_2_iterator(*this); }
  auto end() { return typename bfs_edge_range_2_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  PriorityQueue&            Q_;
  std::vector<three_colors> colors_;
};

#endif // NWGRAPH_BFS_EDGE_RANGE_HPP
