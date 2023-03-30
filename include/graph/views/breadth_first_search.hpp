//
//	Author: J. Phillip Ratzloff
//
// inspired by bfs_range.hpp from: NWGraph
//
// breadth-first search graph views for vertices and edges.
// All functions have an allocator parameter (not shown) for internally defined containers.
//
// examples: for(auto&& [vid,v]     : vertices_breadth_first_search(g,seed))
//           for(auto&& [vid,v]     : vertices_breadth_first_search(g,seeds))
//           for(auto&& [vid,v,val] : vertices_breadth_first_search(g,seed,vvf))
//           for(auto&& [vid,v,val] : vertices_breadth_first_search(g,seeds,vvf))
//
//           for(auto&& [vid,uv]     : edges_breadth_first_search(g,seed))
//           for(auto&& [vid,uv]     : edges_breadth_first_search(g,seeds))
//           for(auto&& [vid,uv,val] : edges_breadth_first_search(g,seed,evf))
//           for(auto&& [vid,uv,val] : edges_breadth_first_search(g,seeds,evf))
//
//           for(auto&& [uid,vid,uv]     : sourced_edges_depth_first_search(g,seed))
//           for(auto&& [uid,vid,uv]     : sourced_edges_depth_first_search(g,seeds))
//           for(auto&& [uid,vid,uv,val] : sourced_edges_depth_first_search(g,seed,evf))
//           for(auto&& [uid,vid,uv,val] : sourced_edges_depth_first_search(g,seeds,evf))
//
// Given bfs is one of the breadth-first views above, the following functions are also available.
//
//  size(bfs) returns the size of the internal queue
//
//  bfs.cancel(cancel_search::cancel_branch) will stop searching from the current vertex
//  bfs.cancel(cancel_search::cancel_all) will stop searching and the iterator will be at the end()
//

#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"
#include <queue>
#include <vector>
#include <functional>

#if !defined(GRAPH_BFS_HPP)
#  define GRAPH_BFS_HPP

namespace std::graph {


/*template <adjacency_list G>
struct bfs_element {
  vertex_id_t<G> u_id;
};*/

template <adjacency_list G, class Queue, class Alloc>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_base : public ranges::view_base {
public:
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_type        = edge_t<G>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;

private:
  using graph_ref_type = reference_wrapper<graph_type>;
  //using queue_elem     = bfs_element<graph_type>;
  using queue_elem = vertex_id_type;

  using parent_alloc = typename allocator_traits<typename Queue::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  bfs_base(graph_type& g, vertex_id_type seed, const Alloc& alloc)
        : graph_(g), Q_(alloc), colors_(ranges::size(vertices(g)), white, alloc) {
    if (seed < ranges::size(vertices(graph_)) && !ranges::empty(edges(graph_, seed))) {
      uv_ = ranges::begin(edges(graph_, seed));
      Q_.push(queue_elem{seed});
      colors_[seed] = grey;
    }
  }

  template <class VKR>
  requires ranges::input_range<VKR> && convertible_to<ranges::range_value_t<VKR>, vertex_id_t<G>>
  bfs_base(graph_type& g, const VKR& seeds = 0) : graph_(g), colors_(ranges::size(vertices(g)), white) {
    for (auto&& [seed] : seeds) {
      if (seed < ranges::size(vertices(graph_)) && !ranges::empty(edges(graph_, seed))) {
        if (Q_.empty()) {
          uv_ = ranges::begin(edges(graph_, seed));
        }
        Q_.push(queue_elem{seed});
        colors_[seed] = grey;
      }
    }
    // advance uv_ to the first edge to be visited in case seeds adjacent to first seed
    while (!Q_.empty()) {
      auto          u_id = Q_.front();
      edge_iterator uvi  = find_unvisited(u_id, ranges::begin(edges(graph_, u_id)));
      if (uvi != ranges::end(edges(graph_, u_id))) {
        uv_ = uvi;
        break;
      } else {
        Q_.pop();
        colors_[u_id] = black;
      }
    }
  }

  bfs_base()                = default;
  bfs_base(const bfs_base&) = delete; // can be expensive to copy
  bfs_base(bfs_base&&)      = default;
  ~bfs_base()               = default;

  bfs_base& operator=(const bfs_base&) = delete;
  bfs_base& operator=(bfs_base&&)      = default;

  constexpr bool empty() const noexcept { return Q_.empty(); }

  constexpr auto size() const noexcept { return Q_.size(); }
  //constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept { return cancel_; }

protected:
  constexpr vertex_id_type real_target_id(edge_reference uv, vertex_id_type) const
  requires ordered_edge<G, edge_type>
  {
    return target_id(graph_, uv);
  }
  constexpr vertex_id_type real_target_id(edge_reference uv, vertex_id_type src) const
  requires unordered_edge<G, edge_type>
  {
    if (target_id(graph_, uv) != src)
      return target_id(graph_, uv);
    else
      return source_id((graph_), uv);
  }

  constexpr vertex_edge_iterator_t<G> find_unvisited(vertex_id_t<G> uid, vertex_edge_iterator_t<G> first) {
    return ranges::find_if(first, ranges::end(edges(graph_, uid)), [this, uid](edge_reference uv) -> bool {
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
    if (uv_ == ranges::end(edges(graph_, u_id))) {
      colors_[u_id] = black; // finished with u
      Q_.pop();
      while (!Q_.empty()) {
        u_id = Q_.front();
        uv_  = find_unvisited(u_id, ranges::begin(edges(graph_, u_id)));
        if (uv_ != ranges::end(edges(graph_, u_id))) {
          break;
        } else {
          Q_.pop();
          colors_[u_id] = black;
        }
      }
    }
  }

protected:
  _detail::ref_to_ptr<graph_type&> graph_;
  Queue                            Q_;
  vertex_edge_iterator_t<G>        uv_;
  vector<three_colors>             colors_;
  cancel_search                    cancel_ = cancel_search::continue_search;
};

/**
 * @brief Breadth-first search range for vertices, given a single seed vertex.
 * 
 * @tparam G     Graph type
 * @tparam VVF   Vertex Value Function type
 * @tparam Queue Queue type for internal use
 * @tparam Alloc Allocator type
*/
template <adjacency_list G, class VVF = void, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertices_breadth_first_search_view : public bfs_base<G, Queue, Alloc> {
public:
  using base_type        = bfs_base<G, Queue, Alloc>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_type        = edge_t<G>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using bfs_range_type   = vertices_breadth_first_search_view<graph_type, VVF, Queue, Alloc>;

  using vertex_value_func = VVF;
  using vertex_value_type = invoke_result_t<VVF, vertex_reference>;

public:
  vertices_breadth_first_search_view(graph_type&    g,
                                     vertex_id_type seed,
                                     const VVF&     value_fn,
                                     const Alloc&   alloc = Alloc())
        : base_type(g, seed, alloc), value_fn_(&value_fn) {}
  template <class VKR>
  requires ranges::input_range<VKR> && convertible_to<ranges::range_value_t<VKR>, vertex_id_t<G>>
  vertices_breadth_first_search_view(graph_type&  graph,
                                     const VKR&   seeds,
                                     const VVF&   value_fn,
                                     const Alloc& alloc = Alloc())
        : base_type(graph, seeds), value_fn_(&value_fn) {}

  vertices_breadth_first_search_view()                                          = default;
  vertices_breadth_first_search_view(const vertices_breadth_first_search_view&) = delete; // can be expensive to copy
  vertices_breadth_first_search_view(vertices_breadth_first_search_view&&)      = default;
  ~vertices_breadth_first_search_view()                                         = default;

  vertices_breadth_first_search_view& operator=(const vertices_breadth_first_search_view&) = delete;
  vertices_breadth_first_search_view& operator=(vertices_breadth_first_search_view&&)      = default;

public:
  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_descriptor<const vertex_id_type, vertex_type&, vertex_value_type>;
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
          vertex_descriptor<vertex_id_t<graph_type>, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

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
      auto&          g    = the_range_->graph_;
      auto&&         u_id = the_range_->Q_.front();
      auto&&         uvi  = the_range_->uv_;
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_              = {v_id, &v, invoke(*the_range_->value_fn_, v)};
      return reinterpret_cast<reference>(value_);
    }

    constexpr bool operator==(const end_sentinel&) const noexcept { return the_range_->Q_.empty(); }
    constexpr bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    bfs_range_type*           the_range_ = nullptr;
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


template <adjacency_list G, class Queue, class Alloc>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertices_breadth_first_search_view<G, void, Queue, Alloc> : public bfs_base<G, Queue, Alloc> {
public:
  using base_type        = bfs_base<G, Queue, Alloc>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_type        = edge_t<G>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using bfs_range_type   = vertices_breadth_first_search_view<graph_type, void, Queue, Alloc>;

public:
  vertices_breadth_first_search_view(graph_type& g, vertex_id_type seed, const Alloc& alloc = Alloc())
        : base_type(g, seed, alloc) {}
  template <class VKR>
  requires ranges::forward_range<VKR> && convertible_to<ranges::range_value_t<VKR>, vertex_id_t<G>>
  vertices_breadth_first_search_view(graph_type& g, const VKR& seeds, const Alloc& alloc = Alloc())
        : base_type(g, seeds, alloc) {}

  vertices_breadth_first_search_view()                                          = default;
  vertices_breadth_first_search_view(const vertices_breadth_first_search_view&) = delete; // can be expensive to copy
  vertices_breadth_first_search_view(vertices_breadth_first_search_view&&)      = default;
  ~vertices_breadth_first_search_view()                                         = default;

  vertices_breadth_first_search_view& operator=(const vertices_breadth_first_search_view&) = delete;
  vertices_breadth_first_search_view& operator=(vertices_breadth_first_search_view&&)      = default;

public:
  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = vertex_descriptor<const vertex_id_type, vertex_type&, void>;
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
    using shadow_value_type  = vertex_descriptor<vertex_id_t<graph_type>, shadow_vertex_type*, void>;

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
      auto&          g    = the_range_->graph_;
      auto&&         u_id = the_range_->Q_.front();
      auto&&         uvi  = the_range_->uv_;
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_              = {v_id, &v};
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->Q_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    bfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }
};


/**
 * @brief Breadth-first search range for edges, given a single seed vertex.
 * @tparam G       Graph type
 * @tparam EVF     Edge Value Function type
 * @tparam Sourced Does the graph support @c source_id()?
 * @tparam Queue   Queue type for internal use
 * @tparam Alloc   Allocator type
*/
template <adjacency_list G,
          class EVF    = void,
          bool Sourced = false,
          class Queue  = queue<vertex_id_t<G>>,
          class Alloc  = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class edges_breadth_first_search_view : public bfs_base<G, Queue, Alloc> {
public:
  using base_type           = bfs_base<G, Queue, Alloc>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using bfs_range_type      = edges_breadth_first_search_view<G, EVF, Sourced, Queue, Alloc>;

  using edge_value_func = EVF;
  using edge_value_type = invoke_result_t<EVF, edge_reference_type>;

public:
  edges_breadth_first_search_view(G& g, vertex_id_type seed, const EVF& value_fn, const Alloc& alloc = Alloc())
        : base_type(g, seed, alloc), value_fn_(&value_fn) {}
  template <class VKR>
  requires ranges::forward_range<VKR> && convertible_to<ranges::range_value_t<VKR>, vertex_id_t<G>>
  edges_breadth_first_search_view(G& graph, const VKR& seeds, const EVF& value_fn, const Alloc& alloc = Alloc())
        : base_type(graph, seeds, alloc), value_fn_(&value_fn) {}

  edges_breadth_first_search_view()                                       = default;
  edges_breadth_first_search_view(const edges_breadth_first_search_view&) = delete; // can be expensive to copy
  edges_breadth_first_search_view(edges_breadth_first_search_view&&)      = default;
  ~edges_breadth_first_search_view()                                      = default;

  edges_breadth_first_search_view& operator=(const edges_breadth_first_search_view&) = delete;
  edges_breadth_first_search_view& operator=(edges_breadth_first_search_view&&)      = default;

  struct end_sentinel {};

  class iterator {
  public:
    using iterator_category = input_iterator_tag;
    using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, edge_value_type>;
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
          edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

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
        value_.source_id = u_id;
      }
      value_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.edge      = &*uvi;
      value_.value     = invoke(*the_range_->value_fn_, *uvi);
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->Q_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    bfs_range_type*           the_range_ = nullptr;
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

template <adjacency_list G, bool Sourced, class Queue, class Alloc>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class edges_breadth_first_search_view<G, void, Sourced, Queue, Alloc> : public bfs_base<G, Queue, Alloc> {
public:
  using base_type           = bfs_base<G, Queue, Alloc>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using bfs_range_type      = edges_breadth_first_search_view<G, void, Sourced, Queue, Alloc>;

public:
  edges_breadth_first_search_view(G& g, vertex_id_type seed, const Alloc& alloc = Alloc())
        : base_type(g, seed, alloc) {}
  template <class VKR>
  requires ranges::forward_range<VKR> && convertible_to<ranges::range_value_t<VKR>, vertex_id_t<G>>
  edges_breadth_first_search_view(G& g, const VKR& seeds, const Alloc& alloc()) : base_type(g, seeds, alloc) {}

  edges_breadth_first_search_view()                                       = default;
  edges_breadth_first_search_view(const edges_breadth_first_search_view&) = delete; // can be expensive to copy
  edges_breadth_first_search_view(edges_breadth_first_search_view&&)      = default;
  ~edges_breadth_first_search_view()                                      = default;

  edges_breadth_first_search_view& operator=(const edges_breadth_first_search_view&) = delete;
  edges_breadth_first_search_view& operator=(edges_breadth_first_search_view&&)      = default;

  struct end_sentinel {};

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
    using shadow_edge_type  = remove_reference_t<edge_reference_type>;
    using shadow_value_type = edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, void>;

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
        value_.source_id = u_id;
      }
      value_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.edge      = &*uvi;
      return reinterpret_cast<reference>(value_);
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->Q_.empty(); }
    bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable shadow_value_type value_     = {};
    bfs_range_type*           the_range_ = nullptr;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }
};
} // namespace std::graph

namespace std::graph::tag_invoke {
// vertices_breadth_first_search CPO
TAG_INVOKE_DEF(vertices_breadth_first_search); // vertices_breadth_first_search(g,seed)    -> vertices[vid,v]
                                               // vertices_breadth_first_search(g,seed,fn) -> vertices[vid,v,value]

template <class G, class A>
concept _has_vtx_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const A& alloc) {
  { vertices_breadth_first_search(g, seed, alloc) };
};
template <class G, class VVF, class A>
concept _has_vtx_bfs_vvf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const VVF& vvf, const A& alloc) {
  { vertices_breadth_first_search(g, seed, vvf, alloc) };
};

// edges_breadth_first_search CPO
//  sourced_edges_breadth_first_search
TAG_INVOKE_DEF(edges_breadth_first_search);         // edges_breadth_first_search(g,seed)    -> edges[vid,v]
                                                    // edges_breadth_first_search(g,seed,fn) -> edges[vid,v,value]
TAG_INVOKE_DEF(sourced_edges_breadth_first_search); // sourced_edges_breadth_first_search(g,seed)    -> edges[uid,vid,v]
      // sourced_edges_breadth_first_search(g,seed,fn) -> edges[uid,vid,v,value]

template <class G, class A>
concept _has_edg_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const A& alloc) {
  { edges_breadth_first_search(g, seed, alloc) };
};
template <class G, class EVF, class A>
concept _has_edg_bfs_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf, const A& alloc) {
  { edges_breadth_first_search(g, seed, evf, alloc) };
};

template <class G, class A>
concept _has_src_edg_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const A& alloc) {
  { sourced_edges_breadth_first_search(g, seed, alloc) };
};
template <class G, class EVF, class A>
concept _has_src_edg_bfs_evf_adl =
      vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf, const A& alloc) {
        { sourced_edges_breadth_first_search(g, seed, evf, alloc) };
      };

} // namespace std::graph::tag_invoke


namespace std::graph::views {

//
// vertices_breadth_first_search(g,uid)
// vertices_breadth_first_search(g,uid,vvf)
//
template <adjacency_list G, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto vertices_breadth_first_search(G&& g, vertex_id_t<G> seed, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_vtx_bfs_adl<G, Alloc>)
    return tag_invoke::vertices_breadth_first_search(g, seed, alloc);
  else
    return vertices_breadth_first_search_view<G, void, Queue>(g, seed, alloc);
}

template <adjacency_list G, class VVF, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         is_invocable_v<VVF, vertex_reference_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto vertices_breadth_first_search(G&& g, vertex_id_t<G> seed, const VVF& vvf, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_vtx_bfs_vvf_adl<G, VVF, Alloc>)
    return tag_invoke::vertices_breadth_first_search(g, seed, vvf, alloc);
  else
    return vertices_breadth_first_search_view<G, VVF, Queue>(g, seed, vvf, alloc);
}

//
// edges_breadth_first_search(g,uid)
// edges_breadth_first_search(g,uid,evf)
//
template <adjacency_list G, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_edg_bfs_adl<G, Alloc>)
    return tag_invoke::edges_breadth_first_search(g, seed, alloc);
  else
    return edges_breadth_first_search_view<G, void, false, Queue>(g, seed, alloc);
}

template <adjacency_list G, class EVF, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         is_invocable_v<EVF, edge_reference_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_edg_bfs_evf_adl<G, EVF, Alloc>)
    return tag_invoke::edges_breadth_first_search(g, seed, evf, alloc);
  else
    return edges_breadth_first_search_view<G, EVF, false, Queue>(g, seed, evf, alloc);
}

//
// sourced_edges_breadth_first_search(g,uid)
// sourced_edges_breadth_first_search(g,uid,evf)
//
template <adjacency_list G, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto sourced_edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_src_edg_bfs_adl<G, Alloc>)
    return tag_invoke::sourced_edges_breadth_first_search(g, seed, alloc);
  else
    return edges_breadth_first_search_view<G, void, true, Queue>(g, seed, alloc);
}

template <adjacency_list G, class EVF, class Queue = queue<vertex_id_t<G>>, class Alloc = allocator<bool>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
         is_invocable_v<EVF, edge_reference_t<G>> && _detail::is_allocator_v<Alloc>
constexpr auto
sourced_edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf, const Alloc& alloc = Alloc()) {
  if constexpr (tag_invoke::_has_src_edg_bfs_evf_adl<G, EVF, Alloc>)
    return tag_invoke::sourced_edges_breadth_first_search(g, seed, evf, alloc);
  else
    return edges_breadth_first_search_view<G, EVF, true, Queue>(g, seed, evf, alloc);
}


} // namespace std::graph::views


#endif // GRAPH_BFS_HPP
