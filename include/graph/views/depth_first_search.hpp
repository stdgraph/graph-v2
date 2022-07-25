//
//	Author: J. Phillip Ratzloff
//
// inspired by new_dfs_range.hpp from NWGraph
//
// depth-first search graph views for vertices and edges.
//
// examples: for(auto&& [uid,u]     : vertices_depth_first_search(g,seed))
//           for(auto&& [uid,u,val] : vertices_depth_first_search(g,seed,vvf))
//
//           for(auto&& [vid,uv]     : edges_depth_first_search(g,seed))
//           for(auto&& [vid,uv,val] : edges_depth_first_search(g,seed,evf))
//
//           for(auto&& [uid,vid,uv]     : sourced_edges_depth_first_search(g,seed))
//           for(auto&& [vid,vid,uv,val] : sourced_edges_depth_first_search(g,seed,evf))
//
// Given dfs is one of the depth-first views above, the following functions are also available.
//
//  size(dfs) returns the depth of the current search (the size of the internal stack)
//
//  dfs.cancel(cancel_search::cancel_branch) will stop searching from the current vertex
//  dfs.cancel(cancel_search::cancel_all) will stop searching and the iterator will be at the end()
//

#include "../graph.hpp"
#include "graph/views/views_utility.hpp"
#include <stack>
#include <vector>
#include <functional>

#ifndef GRAPH_DFS_HPP
#  define GRAPH_DFS_HPP

namespace std::graph::views {

/// <summary>
/// The element in a depth-first search stack.
/// </summary>
template <incidence_graph G>
struct dfs_element {
  vertex_id_t<G>            u_id;
  vertex_edge_iterator_t<G> uv;
};

//---------------------------------------------------------------------------------------
/// depth-first search view for vertices, given a single seed vertex.
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
  using stack_elem     = dfs_element<graph_type>;

  using parent_alloc = typename allocator_traits<typename Stack::container_type::allocator_type>::template rebind_alloc<
        vertex_id_type>;

public:
  dfs_base(graph_type& g, vertex_id_type seed = 0) : graph_(g), colors_(ranges::size(vertices(g)), white) {
    if (seed < ranges::size(vertices(graph_)) && !ranges::empty(edges(graph_, seed))) {
      edge_iterator uv = ranges::begin(edges(graph_, seed));
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

  constexpr vertex_edge_iterator_t<G> find_unvisited(vertex_id_t<G> uid, vertex_edge_iterator_t<G> first) {
    return ranges::find_if(first, ranges::end(edges(graph_, uid)), [this, uid](edge_reference uv) -> bool {
      return colors_[real_target_id(uv, uid)] == white;
    });
  }

  constexpr void advance() {
    // next level in search
    auto [u_id, uvi]    = S_.top();
    vertex_id_type v_id = real_target_id(*uvi, u_id);

    edge_iterator vwi = ranges::end(edges(graph_, v_id));
    switch (cancel_) {
    case cancel_search::continue_search:
      // find first unvisited edge of v
      vwi = find_unvisited(v_id, ranges::begin(edges(graph_, v_id)));
      break;
    case cancel_search::cancel_branch: {
      cancel_       = cancel_search::continue_search;
      colors_[v_id] = black; // finished with v

      // Continue with sibling?
      uvi = find_unvisited(u_id, ++uvi);
      if (uvi != ranges::end(edges(graph_, u_id))) {
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
    if (vwi != ranges::end(edges(graph_, v_id))) {
      S_.push(stack_elem{v_id, vwi});
      vertex_id_type w_id = real_target_id(*vwi, v_id);
      colors_[w_id]       = grey; // visited w
    }
    // we've reached the end of a branch in the DFS tree; start unwinding the stack to find other unvisited branches
    else {
      colors_[v_id] = black; // finished with v
      S_.pop();
      while (!S_.empty()) {
        auto [x_id, xyi] = S_.top();
        S_.pop();
        xyi = find_unvisited(x_id, ++xyi);

        // unvisted edge found for vertex x?
        if (xyi != ranges::end(edges(graph_, x_id))) {
          S_.push({x_id, xyi});
          vertex_id_type y_id = real_target_id(*xyi, x_id);
          colors_[y_id]       = grey; // visited y
          break;
        } else {
          colors_[x_id] = black; // finished with x
        }
      }
    }
  }

protected:
  _detail::ref_to_ptr<graph_type&> graph_;
  Stack                            S_;
  vector<three_colors>             colors_;
  cancel_search                    cancel_ = cancel_search::continue_search;
};


//---------------------------------------------------------------------------------------
/// depth-first search range for vertices, given a single seed vertex.
///

template <incidence_graph G, class VVF = void, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertices_depth_first_search_view : public dfs_base<G, Stack> {
public:
  using base_type        = dfs_base<G, Stack>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = vertices_depth_first_search_view<graph_type, VVF, Stack>;

  using vertex_value_func = VVF;
  using vertex_value_type = invoke_result_t<VVF, vertex_reference>;

public:
  vertices_depth_first_search_view(graph_type& g, vertex_id_type seed, const VVF& value_fn)
        : base_type(g, seed), value_fn_(&value_fn) {}
  vertices_depth_first_search_view()                                        = default;
  vertices_depth_first_search_view(const vertices_depth_first_search_view&) = delete; // can be expensive to copy
  vertices_depth_first_search_view(vertices_depth_first_search_view&&)      = default;
  ~vertices_depth_first_search_view()                                       = default;

  vertices_depth_first_search_view& operator=(const vertices_depth_first_search_view&) = delete;
  vertices_depth_first_search_view& operator=(vertices_depth_first_search_view&&) = default;

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
      auto& g             = the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_              = {v_id, &v, invoke(*the_range_->value_fn_, v)};
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
class vertices_depth_first_search_view<G, void, Stack> : public dfs_base<G, Stack> {
public:
  using base_type        = dfs_base<G, Stack>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = vertices_depth_first_search_view<graph_type, void, Stack>;

public:
  vertices_depth_first_search_view(graph_type& g, vertex_id_type seed) : base_type(g, seed) {}
  vertices_depth_first_search_view()                                        = default;
  vertices_depth_first_search_view(const vertices_depth_first_search_view&) = delete; // can be expensive to copy
  vertices_depth_first_search_view(vertices_depth_first_search_view&&)      = default;
  ~vertices_depth_first_search_view()                                       = default;

  vertices_depth_first_search_view& operator=(const vertices_depth_first_search_view&) = delete;
  vertices_depth_first_search_view& operator=(vertices_depth_first_search_view&&) = default;

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
      auto& g             = the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_              = {v_id, &v};
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
/// depth-first search view for edges, given a single seed vertex.
///
template <incidence_graph G, class EVF = void, bool Sourced = false, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class edges_depth_first_search_view : public dfs_base<G, Stack> {
public:
  using base_type           = dfs_base<G, Stack>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = edges_depth_first_search_view<G, EVF, Sourced, Stack>;

  using edge_value_func = EVF;
  using edge_value_type = invoke_result_t<EVF, edge_reference_type>;

public:
  edges_depth_first_search_view(G& g, vertex_id_type seed, const EVF& value_fn)
        : base_type(g, seed), value_fn_(&value_fn) {}

  edges_depth_first_search_view()                                     = default;
  edges_depth_first_search_view(const edges_depth_first_search_view&) = delete; // can be expensive to copy
  edges_depth_first_search_view(edges_depth_first_search_view&&)      = default;
  ~edges_depth_first_search_view()                                    = default;

  edges_depth_first_search_view& operator=(const edges_depth_first_search_view&) = delete;
  edges_depth_first_search_view& operator=(edges_depth_first_search_view&&) = default;

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
      value_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.edge      = &*uvi;
      value_.value     = invoke(*the_range_->value_fn_, *uvi);
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
class edges_depth_first_search_view<G, void, Sourced, Stack> : public dfs_base<G, Stack> {
public:
  using base_type           = dfs_base<G, Stack>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = edges_depth_first_search_view<G, void, Sourced, Stack>;

public:
  edges_depth_first_search_view(G& g, vertex_id_type seed) : base_type(g, seed) {}

  edges_depth_first_search_view()                                     = default;
  edges_depth_first_search_view(const edges_depth_first_search_view&) = delete; // can be expensive to copy
  edges_depth_first_search_view(edges_depth_first_search_view&&)      = default;
  ~edges_depth_first_search_view()                                    = default;

  edges_depth_first_search_view& operator=(const edges_depth_first_search_view&) = delete;
  edges_depth_first_search_view& operator=(edges_depth_first_search_view&&) = default;

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
      value_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.edge      = &*uvi;
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

namespace tag_invoke {
  // vertices_depth_first_search CPO
  TAG_INVOKE_DEF(vertices_depth_first_search); // vertices_depth_first_search(g,seed)    -> vertices[vid,v]
                                               // vertices_depth_first_search(g,seed,fn) -> vertices[vid,v,value]

  template <class G>
  concept _has_vtx_dfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {vertices_depth_first_search(g, seed)};
  };
  template <class G, class VVF>
  concept _has_vtx_dfs_vvf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const VVF& vvf) {
    {vertices_depth_first_search(g, seed, vvf)};
  };

  // edges_depth_first_search CPO
  //  sourced_edges_depth_first_search
  TAG_INVOKE_DEF(edges_depth_first_search);         // edges_depth_first_search(g,seed)    -> edges[vid,v]
                                                    // edges_depth_first_search(g,seed,fn) -> edges[vid,v,value]
  TAG_INVOKE_DEF(sourced_edges_depth_first_search); // sourced_edges_depth_first_search(g,seed)    -> edges[uid,vid,v]
        // sourced_edges_depth_first_search(g,seed,fn) -> edges[uid,vid,v,value]

  template <class G>
  concept _has_edg_dfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {edges_depth_first_search(g, seed)};
  };
  template <class G, class EVF>
  concept _has_edg_dfs_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf) {
    {edges_depth_first_search(g, seed, evf)};
  };

  template <class G>
  concept _has_src_edg_dfs_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed) {
    {sourced_edges_depth_first_search(g, seed)};
  };
  template <class G, class EVF>
  concept _has_src_edg_dfs_evf_adl = vertex_range<G> && requires(G&& g, vertex_id_t<G> seed, const EVF& evf) {
    {sourced_edges_depth_first_search(g, seed, evf)};
  };

} // namespace tag_invoke


//
// vertices_depth_first_search(g,uid)
// vertices_depth_first_search(g,uid,vvf)
//
template <incidence_graph G, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto vertices_depth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_vtx_dfs_adl<G>)
    return tag_invoke::vertices_depth_first_search(g, seed);
  else
    return vertices_depth_first_search_view<G, void, Stack>(g, seed);
}

template <incidence_graph G, class VVF, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<VVF, vertex_reference_t<G>>
constexpr auto vertices_depth_first_search(G&& g, vertex_id_t<G> seed, const VVF& vvf) {
  if constexpr (tag_invoke::_has_vtx_dfs_vvf_adl<G, VVF>)
    return tag_invoke::vertices_depth_first_search(g, seed, vvf);
  else
    return vertices_depth_first_search_view<G, VVF, Stack>(g, seed, vvf);
}

//
// edges_depth_first_search(g,uid)
// edges_depth_first_search(g,uid,evf)
//
template <incidence_graph G, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto edges_depth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_edg_dfs_adl<G>)
    return tag_invoke::edges_depth_first_search(g, seed);
  else
    return edges_depth_first_search_view<G, void, false, Stack>(g, seed);
}

template <incidence_graph G, class EVF, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<EVF, edge_reference_t<G>>
constexpr auto edges_depth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf) {
  if constexpr (tag_invoke::_has_edg_dfs_evf_adl<G, EVF>)
    return tag_invoke::edges_depth_first_search(g, seed, evf);
  else
    return edges_depth_first_search_view<G, EVF, false, Stack>(g, seed, evf);
}

//
// sourced_edges_depth_first_search(g,uid)
// sourced_edges_depth_first_search(g,uid,evf)
//
template <incidence_graph G, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
constexpr auto sourced_edges_depth_first_search(G&& g, vertex_id_t<G> seed) {
  if constexpr (tag_invoke::_has_src_edg_dfs_adl<G>)
    return tag_invoke::sourced_edges_depth_first_search(g, seed);
  else
    return edges_depth_first_search_view<G, void, true, Stack>(g, seed);
}

template <incidence_graph G, class EVF, class Stack = stack<dfs_element<G>>>
requires ranges::random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>> &&
      is_invocable_v<EVF, edge_reference_t<G>>
constexpr auto sourced_edges_depth_first_search(G&& g, vertex_id_t<G> seed, const EVF& evf) {
  if constexpr (tag_invoke::_has_src_edg_dfs_evf_adl<G, EVF>)
    return tag_invoke::sourced_edges_depth_first_search(g, seed, evf);
  else
    return edges_depth_first_search_view<G, EVF, true, Stack>(g, seed, evf);
}


} // namespace std::graph::views

#endif // GRAPH_DFS_HPP
