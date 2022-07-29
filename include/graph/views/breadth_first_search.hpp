//
//	Author: J. Phillip Ratzloff
//
// inspired by bfs_range.hpp from: NWGraph
//

#include "../graph.hpp"
#include <queue>
#include <vector>

#if !defined(GRAPH_BFS_HPP)
#  define GRAPH_BFS_HPP

namespace std::graph::views {


enum three_colors : int8_t { black, white, grey }; // { finished, undiscovered, discovered }
enum struct cancel_search : int8_t { continue_search, cancel_branch, cancel_all };

template <incidence_graph G>
struct bfs_elem {
  vertex_id_t<G>            u_id;
};

template <incidence_graph G, class Queue>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_base : public ranges::view_base {
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
  using queue_elem     = bfs_elem<graph_type>;

  using parent_alloc = typename allocator_traits<typename Queue::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  bfs_base(graph_type& g, vertex_id_type seed = 0) : graph_(&g), colors_(ranges::size(vertices(g)), white) {
    if (seed < ranges::size(vertices(*graph_)) && !ranges::empty(edges(*graph_, seed))) {
      uv_ = ranges::begin(edges(*graph_, seed));
      Q_.push(queue_elem{seed});
      colors_[seed] = grey;
    }
  }

  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  bfs_base(graph_type& g, const VKR& seeds = 0) : graph_(&g), colors_(ranges::size(vertices(g)), white) {
    for (auto&& [seed] : seeds ) {
      if (seed < ranges::size(vertices(*graph_)) && !ranges::empty(edges(*graph_, seed))) {
	if ( Q_.empty() ) {
	  uv_ = ranges::begin(edges(*graph_, seed));
	}
	Q_.push(queue_elem{seed});
	colors_[seed] = grey;
      }
    }
    // advance uv_ to the first edge to be visited in case seeds adjacent to first seed
    while (!Q_.empty()) {
        auto u_id = Q_.front();
	edge_iterator uvi = ranges::end(edges(*graph_, u_id));
        uvi = ranges::find_if(ranges::begin(edges(*graph_, u_id)), ranges::end(edges(*graph_, u_id)), is_unvisited);
	if (uvi != ranges::end(edges(*graph_, u_id))) {
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

  bfs_base& operator=(const dfs_base&) = delete;
  bfs_base& operator=(dfs_base&&) = default;

  constexpr bool empty() const noexcept { return Q_.empty(); }

  constexpr auto size() const noexcept { return Q_.size(); }
  //constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept { return cancel_; }

protected:
   constexpr vertex_id_type real_target_id(edge_reference uv, vertex_id_type) const requires directed_incidence_graph<graph_type> {
    return target_id(graph_, uv);
  }
  constexpr vertex_id_type real_target_id(edge_reference uv,
                                vertex_id_type src) const requires undirected_incidence_graph<graph_type> {
    if (target_id(graph_, uv) != src)
      return target_id(graph_, uv);
    else
      return source_id((graph_), uv);
  }

  void advance() {
    // current frontier vertex
    auto u_id    = Q_.front();

    switch (cancel_) {
    case cancel_search::continue_search:
      vertex_id_type v_id = real_target_id(*uv_, u_id);
      Q_.push(queue_elem{v_id});
      colors_[v_id]       = grey; // visited v
      uv_ = ranges::find_if(++uv_, ranges::end(edges(*graph_, u_id)), [this, u_id](edge_reference uv) -> bool {
	return colors_[real_target_id(uv, u_id)] == white;
      });
      break;
    case cancel_search::cancel_branch:
      cancel_       = cancel_search::continue_search;
      uv_ = ranges::end(edges(*graph_, u_id));
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
	uv_ = ranges::end(edges(*graph_, u_id));
        uv_ = ranges::find_if(ranges::begin(edges(*graph_, u_id)), ranges::end(edges(*graph_, u_id)), [this, u_id](edge_reference uv) -> bool {
	  return colors_[real_target_id(uv, u_id)] == white;
	});
	if (uv_ != ranges::end(edges(*graph_, u_id))) {
	  break;
	} else {
	  Q_.pop();
	  colors_[u_id] = black;
	}
      }
    } 
  }

protected : graph_type* graph_;
  Queue                 Q_;
  vertex_edge_iterator_t<G> uv_;
  vector<three_colors>  colors_;
  cancel_search         cancel_ = cancel_search::continue_search;
};

//---------------------------------------------------------------------------------------
/// breadth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, class VVF = void, class Queue = queue<dfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_vertex_range : public bfs_base<G, Queue> {
public:
  using base_type        = dfs_base<G, Queue>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = dfs_vertex_range<graph_type, VVF, Queue>;

  using vertex_value_func = VVF;
  using vertex_value_type = invoke_result_t<VVF, vertex_reference>;

public:
  bfs_vertex_range(graph_type& g, vertex_id_type seed, const VVF& value_fn)
        : base_type(g, seed), value_fn_(&value_fn) {}
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  bfs_vertex_range(G& graph, const VKR& seeds, const VVF& value_fn)
    : base_type(g, seeds) value_fn_(&value_fn) {}
  
  bfs_vertex_range()                        = default;
  bfs_vertex_range(const bfs_vertex_range&) = delete; // can be expensive to copy
  bfs_vertex_range(bfs_vertex_range&&)      = default;
  ~bfs_vertex_range()                       = default;

  bfs_vertex_range& operator=(const bfs_vertex_range&) = delete;
  bfs_vertex_range& operator=(bfs_vertex_range&&) = default;

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
    iterator(const bfs_range_type& range) : the_range_(&const_cast<bfs_range_type&>(range)) {}
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
      auto&& u_id  = the_range_->Q_.front();
      auto&& uvi = the_range_->uv_;
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


template <incidence_graph G, class Queue>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_vertex_range<G, void, Queue> : public bfs_base<G, Queue> {
public:
  using base_type        = bfs_base<G, Queue>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using bfs_range_type   = bfs_vertex_range<graph_type, void, Queue>;

public:
  bfs_vertex_range(graph_type& g, vertex_id_type seed) : base_type(g, seed) {}
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  bfs_vertex_range(G& graph, const VKR& seeds) : base_type(g, seeds) {}
  
  bfs_vertex_range()                        = default;
  bfs_vertex_range(const bfs_vertex_range&) = delete; // can be expensive to copy
  bfs_vertex_range(bfs_vertex_range&&)      = default;
  ~bfs_vertex_range()                       = default;

  bfs_vertex_range& operator=(const bfs_vertex_range&) = delete;
  bfs_vertex_range& operator=(bfs_vertex_range&&) = default;

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
    iterator(const bfs_range_type& range) : the_range_(&const_cast<bfs_range_type&>(range)) {}
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
      auto&& u_id  = the_range_->Q_.top();
      auto&& uvi   = the_range->uv_;
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


//---------------------------------------------------------------------------------------
/// breadth-first search range for edges, given a single seed vertex.
///
template <incidence_graph G, class EVF = void, bool Sourced = false, class Queue = queue<bfs_elem<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_edge_range : public bfs_base<G, Queue> {
public:
  using base_type           = bfs_base<G, Queue>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using bfs_range_type      = bfs_edge_range<G, EVF, Sourced, Queue>;

  using edge_value_func = EVF;
  using edge_value_type = invoke_result_t<EVF, edge_reference_type>;

public:
  bfs_edge_range(G& g, vertex_id_type seed, const EVF& value_fn) : base_type(g, seed), value_fn_(&value_fn) {}
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  bfs_edge_range(G& graph, const VKR& seeds, const EVF& value_fn) : base_type(g, seeds), value_fn_(&value_fn) {}
  
  
  bfs_edge_range()                      = default;
  bfs_edge_range(const bfs_edge_range&) = delete; // can be expensive to copy
  bfs_edge_range(bfs_edge_range&&)      = default;
  ~bfs_edge_range()                     = default;

  bfs_edge_range& operator=(const bfs_edge_range&) = delete;
  bfs_edge_range& operator=(bfs_edge_range&&) = default;

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
    iterator(const bfs_range_type& range) : the_range_(&const_cast<bfs_range_type&>(range)) {}
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
      auto&& u_id = the_range_->Q_.top();
      auto&& uvi = the_range_->uv_;
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

template <incidence_graph G, bool Sourced, class Queue>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class bfs_edge_range<G, void, Sourced, Queue> : public bfs_base<G, Queue> {
public:
  using base_type           = bfs_base<G, Queue>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using bfs_range_type      = bfs_edge_range<G, void, Sourced, Queue>;

public:
  bfs_edge_range(G& g, vertex_id_type seed) : base_type(g, seed) {}
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  bfs_edge_range(G& graph, const VKR& seeds) : base_type(g, seeds) {}
  
  bfs_edge_range()                      = default;
  bfs_edge_range(const bfs_edge_range&) = delete; // can be expensive to copy
  bfs_edge_range(bfs_edge_range&&)      = default;
  ~bfs_edge_range()                     = default;

  bfs_edge_range& operator=(const bfs_edge_range&) = delete;
  bfs_edge_range& operator=(bfs_edge_range&&) = default;

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
    iterator(const bfs_range_type& range) : the_range_(&const_cast<bfs_range_type&>(range)) {}
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
      auto&& u_id = the_range_->Q_.top();
      auto&& uvi = the_range_->uv_;
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

namespace tag_invoke {
  // vertices_breadth_first_search CPO
  TAG_INVOKE_DEF(vertices_breadth_first_search); // vertices_breadth_first_search(g,seed)    -> vertices[vid,v]
                                               // vertices_breadth_first_search(g,seed,fn) -> vertices[vid,v,value]

  template <class G>
  concept _has_vtx_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {vertices_breadth_first_search(g, seed)};
  };
  template <class G, class VVF>
  concept _has_vtx_bfs_vvf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const VVF& vvf) {
    {vertices_breadth_first_search(g, seed, vvf)};
  };

  // edges_breadth_first_search CPO
  //  sourced_edges_breadth_first_search
  TAG_INVOKE_DEF(edges_breadth_first_search);         // edges_breadth_first_search(g,seed)    -> edges[vid,v]
                                                    // edges_breadth_first_search(g,seed,fn) -> edges[vid,v,value]
  TAG_INVOKE_DEF(sourced_edges_breadth_first_search); // sourced_edges_breadth_first_search(g,seed)    -> edges[uid,vid,v]
        // sourced_edges_breadth_first_search(g,seed,fn) -> edges[uid,vid,v,value]

  template <class G>
  concept _has_edg_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {edges_breadth_first_search(g, seed)};
  };
  template <class G, class EVF>
  concept _has_edg_bfs_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf) {
    {edges_breadth_first_search(g, seed, evf)};
  };

  template <class G>
  concept _has_src_edg_bfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {sourced_edges_breadth_first_search(g, seed)};
  };
  template <class G, class EVF>
  concept _has_src_edg_bfs_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf) {
    {sourced_edges_breadth_first_search(g, seed, evf)};
  };

} // namespace tag_invoke


//
// vertices_breadth_first_search(g,uid)
// vertices_breadth_first_search(g,uid,vvf)
//
template <incidence_graph G, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto vertices_breadth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_vtx_bfs_adl<G>)
    return tag_invoke::vertices_breadth_first_search(g, seed);
  else
    return vertices_breadth_first_search_view<G, void, Queue>(g, seed);
}

template <incidence_graph G, class VVF, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<VVF, vertex_reference_t<G>>
constexpr auto vertices_breadth_first_search(G&& g, vertex_id_t<G> seed, const VVF& vvf) {
  if constexpr (tag_invoke::_has_vtx_bfs_vvf_adl<G, VVF>)
    return tag_invoke::vertices_breadth_first_search(g, seed, vvf);
  else
    return vertices_breadth_first_search_view<G, VVF, Queue>(g, seed, vvf);
}

//
// edges_breadth_first_search(g,uid)
// edges_breadth_first_search(g,uid,evf)
//
template <incidence_graph G, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto edges_breadth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_edg_bfs_adl<G>)
    return tag_invoke::edges_breadth_first_search(g, seed);
  else
    return edges_breadth_first_search_view<G, void, false, Queue>(g, seed);
}

template <incidence_graph G, class EVF, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<EVF, edge_reference_t<G>>
constexpr auto edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf) {
  if constexpr (tag_invoke::_has_edg_bfs_evf_adl<G, EVF>)
    return tag_invoke::edges_breadth_first_search(g, seed, evf);
  else
    return edges_breadth_first_search_view<G, EVF, false, Queue>(g, seed, evf);
}

//
// sourced_edges_breadth_first_search(g,uid)
// sourced_edges_breadth_first_search(g,uid,evf)
//
template <incidence_graph G, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto sourced_edges_breadth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_src_edg_bfs_adl<G>)
    return tag_invoke::sourced_edges_breadth_first_search(g, seed);
  else
    return edges_breadth_first_search_view<G, void, true, Queue>(g, seed);
}

template <incidence_graph G, class EVF, class Queue = queue<bfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<EVF, edge_reference_t<G>>
constexpr auto sourced_edges_breadth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf) {
  if constexpr (tag_invoke::_has_src_edg_bfs_evf_adl<G, EVF>)
    return tag_invoke::sourced_edges_breadth_first_search(g, seed, evf);
  else
    return edges_breadth_first_search_view<G, EVF, true, Queue>(g, seed, evf);
}

  
#if 0
// options: breadth_limit, {cancel, cancel_branch}

template <class G>
struct bfs_vertex_view {
  vertex_reference_t<G> vertex;
  vertex_reference_t<G> parent;
  vertex_id_t<G>        parent_id;
  vertex_id_t<G>        seed        = 0;
  bool                  is_path_end = false;
  size_t                breadth       = 0;
};

template <class G>
struct bfs_edge_view {
  edge_reference_t<G>   edge;
  vertex_reference_t<G> parent;
  vertex_id_t<G>        parent_id;
  vertex_id_t<G>        seed         = 0;
  bool                  is_back_edge = false;
  size_t                breadth        = 0;
};

//----------------------------------------------------------------------------------------
/// breadth-first search range for vertices, given a single seed vertex.
///
template <incidence_graph G, typename A = allocator<char>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class breadth_first_search_vertex_range {
public:
  // single-source BFS
  breadth_first_search_vertex_range(G& graph, vertex_id_t<G> seed, A alloc = A());

  // multi-source BFS
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  breadth_first_search_vertex_range(G& graph, const VKR& seeds, A alloc = A());

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


//----------------------------------------------------------------------------------------
/// breadth-first search range for edges, given a single seed vertex.
///
/// requires bi-directional edges to get last edge on a vertex
///
template <incidence_graph G, typename A = allocator<char>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class breadth_first_search_edge_range {
public:
  // single-source BFS
  breadth_first_search_edge_range(G& graph, vertex_id_t<G> seed, A alloc = A());

  // multi-source BFS
  template <class VKR>
  requires ranges::is_range_v<VKR> && convertible_to_v<ranges::ranges_value_t<VKR>, vertex_id_t<G>>
  breadth_first_search_edge_range(G& graph, const VKR& seeds, A alloc = A());

  class const_iterator {};
  class iterator : public const_iterator {

  public:
    iterator       begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator       end();
    const_iterator end() const;
    const_iterator cend() const;
  };
#endif 

} // namespace std::graph

#endif // GRAPH_BFS_HPP
