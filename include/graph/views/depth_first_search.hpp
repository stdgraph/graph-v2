//
//	Author: J. Phillip Ratzloff
//
// inspired by new_dfs_range.hpp from NWGraph
//
// depth-first search graph views for vertices and edges.
// All functions have an allocator parameter (not shown) for internally defined containers.
//
// examples: for(auto&& [vid,v]     : vertices_depth_first_search(g,seed))
//           for(auto&& [vid,v,val] : vertices_depth_first_search(g,seed,vvf))
//
//           for(auto&& [vid,uv]     : edges_depth_first_search(g,seed))
//           for(auto&& [vid,uv,val] : edges_depth_first_search(g,seed,evf))
//
//           for(auto&& [uid,vid,uv]     : sourced_edges_depth_first_search(g,seed))
//           for(auto&& [uid,vid,uv,val] : sourced_edges_depth_first_search(g,seed,evf))
//
// Given dfs is one of the depth-first views above, the following functions are also available.
//
//  size(dfs) returns the depth of the current search (the size of the internal stack)
//
//  dfs.cancel(cancel_search::cancel_branch) will stop searching from the current vertex
//  dfs.cancel(cancel_search::cancel_all) will stop searching and the iterator will be at the end()
//

#include "graph/graph.hpp"
#include "graph/graph_utility.hpp"
#include <stack>
#include <vector>
#include <functional>

#ifndef GRAPH_DFS_HPP
#  define GRAPH_DFS_HPP

namespace graph {

/**
 * @brief Base class for depth-first search view for vertices and edges, given a single seed vertex.
 * 
 * @tparam G     Graph type
 * @tparam Alloc Allocator type
*/
template <adjacency_list G, class Alloc>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class dfs_base : public std::ranges::view_base {
public:
  using graph_type       = remove_reference_t<G>;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_type        = edge_t<G>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;

private:
  struct stack_elem {
    vertex_id_t<G>            u_id;
    vertex_edge_iterator_t<G> uv;
  };
  using Stack = std::stack<stack_elem>;

  using graph_ref_type = reference_wrapper<graph_type>;

  using parent_alloc = typename std::allocator_traits<
        typename Stack::container_type::allocator_type>::template rebind_alloc<vertex_id_type>;

public:
  dfs_base(graph_type& g, vertex_id_type seed, const Alloc& alloc)
        : graph_(&g), S_(alloc), colors_(std::ranges::size(vertices(g)), white, alloc) {
    if (seed < static_cast<vertex_id_type>(std::ranges::size(vertices(*graph_))) &&
        !std::ranges::empty(edges(*graph_, seed))) {
      edge_iterator uvi = begin(edges(*graph_, seed));
      S_.push(stack_elem{seed, uvi});
      colors_[seed] = gray;

      // Mark initial vertex as visited
      if (uvi != end(edges(*graph_, seed))) {
        vertex_id_type v_id = real_target_id(*uvi, seed);
        colors_[v_id]       = gray;
      }
    }
  }
  dfs_base()                = default;
  dfs_base(const dfs_base&) = delete; // can be expensive to copy
  dfs_base(dfs_base&&)      = default;
  ~dfs_base()               = default;

  dfs_base& operator=(const dfs_base&) = delete;
  dfs_base& operator=(dfs_base&&)      = default;

  constexpr bool empty() const noexcept { return S_.empty(); }

  constexpr auto size() const noexcept { return S_.size(); }
  constexpr auto depth() const noexcept { return S_.size(); }

  constexpr void          cancel(cancel_search cancel_type) noexcept { cancel_ = cancel_type; }
  constexpr cancel_search canceled() noexcept { return cancel_; }

protected:
  constexpr vertex_id_type real_target_id(edge_reference uv, vertex_id_type) const
  requires ordered_edge<G>
  {
    return target_id(*graph_, uv);
  }
  constexpr vertex_id_type real_target_id(edge_reference uv, vertex_id_type src) const
  requires unordered_edge<G>
  {
    if (target_id(*graph_, uv) != src)
      return target_id(*graph_, uv);
    else
      return source_id((*graph_), uv);
  }

  constexpr vertex_edge_iterator_t<G> find_unvisited(vertex_id_t<G> uid, vertex_edge_iterator_t<G> first) {
    return std::ranges::find_if(first, end(edges(*graph_, uid)), [this, uid](edge_reference uv) -> bool {
      return colors_[real_target_id(uv, uid)] == white;
    });
  }

  constexpr void advance() {
    // next level in search
    auto [u_id, uvi]    = S_.top();
    vertex_id_type v_id = real_target_id(*uvi, u_id);

    edge_iterator vwi = end(edges(*graph_, v_id));
    switch (cancel_) {
    case cancel_search::continue_search:
      // find first unvisited edge of v
      vwi = find_unvisited(v_id, begin(edges(*graph_, v_id)));
      break;
    case cancel_search::cancel_branch: {
      cancel_       = cancel_search::continue_search;
      colors_[v_id] = black; // finished with v

      // Continue with sibling?
      uvi = find_unvisited(u_id, ++uvi);
      if (uvi != end(edges(*graph_, u_id))) {
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
    if (vwi != end(edges(*graph_, v_id))) {
      S_.push(stack_elem{v_id, vwi});
      vertex_id_type w_id = real_target_id(*vwi, v_id);
      colors_[w_id]       = gray; // visited w
    }
    // we've reached the end of a branch in the DFS tree; start unwinding the stack to find other unvisited branches
    else {
      colors_[v_id] = black; // finished with v
      while (!S_.empty()) {
        auto [x_id, xyi] = S_.top();
        S_.pop();
        xyi = find_unvisited(x_id, ++xyi);

        // unvisted edge found for vertex x?
        if (xyi != end(edges(*graph_, x_id))) {
          S_.push({x_id, xyi});
          vertex_id_type y_id = real_target_id(*xyi, x_id);
          colors_[y_id]       = gray; // visited y
          break;
        } else {
          colors_[x_id] = black; // finished with x
        }
      }
    }
  }

protected:
  graph_type*               graph_ = nullptr;
  Stack                     S_;
  std::vector<three_colors> colors_;
  cancel_search             cancel_ = cancel_search::continue_search;
};


/**
 * @brief Depth-first search range for vertices, given a single seed vertex.
 * 
 * @tparam G     Graph type
 * @tparam VVF   Vertex Value Function type
 * @tparam Alloc Allocator type
*/
template <adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertices_depth_first_search_view : public dfs_base<G, Alloc> {
public:
  using base_type        = dfs_base<G, Alloc>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = vertices_depth_first_search_view<graph_type, VVF, Alloc>;

  using vertex_value_func = remove_reference_t<VVF>;
  using vertex_value_type = invoke_result_t<VVF, vertex_reference>;

public:
  vertices_depth_first_search_view(graph_type&    g,
                                   vertex_id_type seed,
                                   const VVF&     value_fn,
                                   const Alloc&   alloc = Alloc())
        : base_type(g, seed, alloc), value_fn_(&value_fn) {}
  vertices_depth_first_search_view()                                        = default;
  vertices_depth_first_search_view(const vertices_depth_first_search_view&) = delete; // can be expensive to copy
  vertices_depth_first_search_view(vertices_depth_first_search_view&&)      = default;
  ~vertices_depth_first_search_view()                                       = default;

  vertices_depth_first_search_view& operator=(const vertices_depth_first_search_view&) = delete;
  vertices_depth_first_search_view& operator=(vertices_depth_first_search_view&&)      = default;

public:
  class iterator;
  struct end_sentinel {
    constexpr bool operator==(const iterator& rhs) const noexcept { return rhs.the_range_->S_.empty(); }
    //constexpr bool operator!=(const iterator& rhs) const noexcept { return !operator==(rhs); }
  };

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = vertex_descriptor<const vertex_id_type, vertex_type&, vertex_value_type>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = range_difference_t<vertex_range_t<graph_type>>;

  private:
    // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = remove_reference_t<vertex_reference>;
    using shadow_value_type =
          vertex_descriptor<vertex_id_t<graph_type>, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

    union internal_value {
      value_type        value_;
      shadow_value_type shadow_;

      internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
      internal_value() : shadow_{} {}
      ~internal_value() {}
      internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
    };

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
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
      auto& g             = *the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_.shadow_      = {v_id, &v, invoke(*the_range_->value_fn_, v)};
      return value_.value_;
    }

    constexpr bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    //constexpr bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable internal_value value_;
    dfs_range_type*        the_range_ = nullptr;
    friend end_sentinel;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }

private:
  const vertex_value_func* value_fn_ = nullptr;
};


template <adjacency_list G, class Alloc>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertices_depth_first_search_view<G, void, Alloc> : public dfs_base<G, Alloc> {
public:
  using base_type        = dfs_base<G, Alloc>;
  using graph_type       = G;
  using vertex_type      = vertex_t<G>;
  using vertex_id_type   = vertex_id_t<graph_type>;
  using vertex_reference = vertex_reference_t<graph_type>;
  using vertex_iterator  = vertex_iterator_t<graph_type>;
  using edge_reference   = edge_reference_t<G>;
  using edge_iterator    = vertex_edge_iterator_t<graph_type>;
  using dfs_range_type   = vertices_depth_first_search_view<graph_type, void, Alloc>;

public:
  vertices_depth_first_search_view(graph_type& g, vertex_id_type seed, const Alloc& alloc = Alloc())
        : base_type(g, seed, alloc) {}
  vertices_depth_first_search_view()                                        = default;
  vertices_depth_first_search_view(const vertices_depth_first_search_view&) = delete; // can be expensive to copy
  vertices_depth_first_search_view(vertices_depth_first_search_view&&)      = default;
  ~vertices_depth_first_search_view()                                       = default;

  vertices_depth_first_search_view& operator=(const vertices_depth_first_search_view&) = delete;
  vertices_depth_first_search_view& operator=(vertices_depth_first_search_view&&)      = default;

public:
  struct end_sentinel {
    bool operator==(const end_sentinel& rhs) const noexcept { return rhs.the_range_->S_.empty(); }
  };

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = vertex_descriptor<const vertex_id_type, vertex_type&, void>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = range_difference_t<vertex_range_t<graph_type>>;

  private:
    // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_vertex_type = remove_reference_t<vertex_reference>;
    using shadow_value_type  = vertex_descriptor<vertex_id_t<graph_type>, shadow_vertex_type*, void>;

    union internal_value {
      value_type        value_;
      shadow_value_type shadow_;

      internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
      internal_value() : shadow_{} {}
      ~internal_value() {}
      internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
    };

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
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
      auto& g             = *the_range_->graph_;
      auto&& [u_id, uvi]  = the_range_->S_.top();
      vertex_id_type v_id = the_range_->real_target_id(*uvi, u_id);
      auto&          v    = *find_vertex(g, v_id);
      value_.shadow_      = {v_id, &v};
      return value_.value_;
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    //bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable internal_value value_;
    dfs_range_type*        the_range_ = nullptr;
    friend end_sentinel;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel{}; }
  auto end() const { return end_sentinel{}; }
  auto cend() const { return end_sentinel{}; }
};


/**
 * @brief Depth-first search view for edges, given a single seed vertex.
 * 
 * @tparam G       Graph type
 * @tparam EVF     Edge Value Function type
 * @tparam Sourced Does graph G support @c source_id()?
 * @tparam Alloc   Allocator type
*/
template <adjacency_list G, class EVF = void, bool Sourced = false, class Alloc = std::allocator<bool>>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class edges_depth_first_search_view : public dfs_base<G, Alloc> {
public:
  using base_type           = dfs_base<G, Alloc>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = edges_depth_first_search_view<G, EVF, Sourced, Alloc>;

  using edge_value_func = remove_reference_t<EVF>;
  using edge_value_type = invoke_result_t<EVF, edge_reference_type>;

public:
  edges_depth_first_search_view(G& g, vertex_id_type seed, const EVF& value_fn, const Alloc& alloc = Alloc())
        : base_type(g, seed, alloc), value_fn_(&value_fn) {}

  edges_depth_first_search_view()                                     = default;
  edges_depth_first_search_view(const edges_depth_first_search_view&) = delete; // can be expensive to copy
  edges_depth_first_search_view(edges_depth_first_search_view&&)      = default;
  ~edges_depth_first_search_view()                                    = default;

  edges_depth_first_search_view& operator=(const edges_depth_first_search_view&) = delete;
  edges_depth_first_search_view& operator=(edges_depth_first_search_view&&)      = default;

  class iterator;
  struct end_sentinel {
    constexpr bool operator==(const iterator& rhs) const noexcept { return rhs.the_range_->S_.empty(); }
    //constexpr bool operator!=(const iterator& rhs) const noexcept { return !operator==(rhs); }
  };

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, edge_value_type>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = range_difference_t<vertex_range_t<graph_type>>;

  private:
    // avoid difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_edge_type = remove_reference_t<edge_reference_type>;
    using shadow_value_type =
          edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

    union internal_value {
      value_type        value_;
      shadow_value_type shadow_;

      internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
      internal_value() : shadow_{} {}
      ~internal_value() {}
      internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
    };

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
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
      auto&& [u_id, uvi] = the_range_->S_.top();
      if constexpr (Sourced) {
        value_.shadow_.source_id = u_id;
      }
      value_.shadow_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.shadow_.edge      = &*uvi;
      value_.shadow_.value     = invoke(*the_range_->value_fn_, *uvi);
      return value_.value_;
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    //bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable internal_value value_;
    dfs_range_type*        the_range_ = nullptr;
    friend end_sentinel;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }

private:
  const edge_value_func* value_fn_ = nullptr;
};

template <adjacency_list G, bool Sourced, class Alloc>
requires random_access_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class edges_depth_first_search_view<G, void, Sourced, Alloc> : public dfs_base<G, Alloc> {
public:
  using base_type           = dfs_base<G, Alloc>;
  using graph_type          = G;
  using vertex_id_type      = vertex_id_t<graph_type>;
  using vertex_iterator     = vertex_iterator_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using dfs_range_type      = edges_depth_first_search_view<G, void, Sourced, Alloc>;

public:
  edges_depth_first_search_view(G& g, vertex_id_type seed, const Alloc& alloc = Alloc()) : base_type(g, seed, alloc) {}

  edges_depth_first_search_view()                                     = default;
  edges_depth_first_search_view(const edges_depth_first_search_view&) = delete; // can be expensive to copy
  edges_depth_first_search_view(edges_depth_first_search_view&&)      = default;
  ~edges_depth_first_search_view()                                    = default;

  edges_depth_first_search_view& operator=(const edges_depth_first_search_view&) = delete;
  edges_depth_first_search_view& operator=(edges_depth_first_search_view&&)      = default;

  class iterator;
  struct end_sentinel {
    bool operator==(const end_sentinel& rhs) const noexcept { return rhs.the_range_->S_.empty(); }
    //bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }
  };

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = edge_descriptor<const vertex_id_type, Sourced, edge_reference_type, void>;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using rvalue_reference  = value_type&&;
    using pointer           = value_type*;
    using const_pointer     = value_type*;
    using size_type         = range_size_t<vertex_range_t<graph_type>>;
    using difference_type   = range_difference_t<vertex_range_t<graph_type>>;

  private:
    // avoid difficulty in undefined vertex reference value in value_type
    // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
    using shadow_edge_type  = remove_reference_t<edge_reference_type>;
    using shadow_value_type = edge_descriptor<vertex_id_type, Sourced, shadow_edge_type*, void>;

    union internal_value {
      value_type        value_;
      shadow_value_type shadow_;

      internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
      internal_value() : shadow_{} {}
      ~internal_value() {}
      internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
    };

  public:
    iterator(const dfs_range_type& range) : the_range_(&const_cast<dfs_range_type&>(range)) {}
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
      auto&& [u_id, uvi] = the_range_->S_.top();
      if constexpr (Sourced) {
        value_.shadow_.source_id = u_id;
      }
      value_.shadow_.target_id = the_range_->real_target_id(*uvi, u_id);
      value_.shadow_.edge      = &*uvi;
      return value_.value_;
    }

    bool operator==(const end_sentinel&) const noexcept { return the_range_->S_.empty(); }
    //bool operator!=(const end_sentinel& rhs) const noexcept { return !operator==(rhs); }

  private:
    mutable internal_value value_;
    dfs_range_type*        the_range_ = nullptr;
    friend end_sentinel;
  };

  auto begin() { return iterator(*this); }
  auto begin() const { return iterator(*this); }
  auto cbegin() const { return iterator(*this); }

  auto end() { return end_sentinel(); }
  auto end() const { return end_sentinel(); }
  auto cend() const { return end_sentinel(); }
};

namespace views {
  //
  // vertices_depth_first_search(g,seed)     -> vertex_descriptor[vid,v]
  // vertices_depth_first_search(g,seed,vvf) -> vertex_descriptor[vid,v,value]
  //
  namespace _Vertices_DFS {
#  if defined(__clang__) || defined(__EDG__)     // TRANSITION, VSO-1681199
    void vertices_depth_first_search() = delete; // Block unqualified name lookup
#  else                                          // ^^^ no workaround / workaround vvv
    void vertices_depth_first_search();
#  endif                                         // ^^^ workaround ^^^

    template <class _G, class _Alloc>
    concept _Has_ref_ADL = _HasClassOrEnumType<_G> //
                           && requires(_G&& __g, const vertex_id_t<_G>& uid, _Alloc alloc) {
                                { _Fake_copy_init(vertices_depth_first_search(__g, uid, alloc)) }; // intentional ADL
                              };
    template <class _G, class _Alloc>
    concept _Can_ref_eval = index_adjacency_list<_G> //
                            && requires(_G&& __g, vertex_id_t<_G> uid, _Alloc alloc) {
                                 { _Fake_copy_init(vertices_depth_first_search_view<_G, void>(__g, uid, alloc)) };
                               };

    template <class _G, class _VVF, class _Alloc>
    concept _Has_ref_vvf_ADL = _HasClassOrEnumType<_G>                //
                               && invocable<_VVF, vertex_reference_t<_G>> //
                               && requires(_G&& __g, const vertex_id_t<_G>& uid, _VVF vvf, _Alloc alloc) {
                                    {
                                      _Fake_copy_init(vertices_depth_first_search(__g, uid, vvf, alloc))
                                    }; // intentional ADL
                                  };
    template <class _G, class _VVF, class _Alloc>
    concept _Can_ref_vvf_eval = index_adjacency_list<_G>                   //
                                && invocable<_VVF, vertex_reference_t<_G>> //
                                && requires(_G&& __g, vertex_id_t<_G> uid, _VVF vvf, _Alloc alloc) {
                                     {
                                       _Fake_copy_init(vertices_depth_first_search_view<_G, _VVF>(__g, uid, vvf, alloc))
                                     };
                                   };

    class _Cpo {
    private:
      enum class _St_ref { _None, _Non_member, _Auto_eval };
      enum class _St_ref_vvf { _None, _Non_member, _Auto_eval };

      template <class _G, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_ADL<_G, _Alloc>) {
          return {_St_ref::_Non_member,
                  noexcept(_Fake_copy_init(vertices_depth_first_search(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                                       declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_eval<_G, _Alloc>) {
          return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(vertices_depth_first_search_view<_G, void>(
                                             declval<_G>(), declval<vertex_id_t<_G>>(), declval<_Alloc>())))};
        } else {
          return {_St_ref::_None};
        }
      }

      template <class _G, class _Alloc>
      static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G, _Alloc>();

      template <class _G, class _VVF, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref_vvf> _Choose_ref_vvf() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_vvf_ADL<_G, _VVF, _Alloc>) {
          return {_St_ref_vvf::_Non_member, noexcept(_Fake_copy_init(vertices_depth_first_search(
                                                  declval<_G>(), declval<vertex_id_t<_G>>(), declval<_VVF>(),
                                                  declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_vvf_eval<_G, _VVF, _Alloc>) {
          return {_St_ref_vvf::_Auto_eval,
                  noexcept(_Fake_copy_init(vertices_depth_first_search_view<_G, _VVF>(
                        declval<_G>(), declval<vertex_id_t<_G>>(), declval<_VVF>(), declval<_Alloc>())))};
        } else {
          return {_St_ref_vvf::_None};
        }
      }

      template <class _G, class _VVF, class _Alloc>
      static constexpr _Choice_t<_St_ref_vvf> _Choice_ref_vvf = _Choose_ref_vvf<_G, _VVF, _Alloc>();

    public:
      /**
       * @brief Single Source, Breadth First Search for vertices
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam Alloc The allocator type.
       * @param g      A graph instance.
       * @param seed   The vertex id to start the search.
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref<_G&, _Alloc>._Strategy != _St_ref::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& seed, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref<_G&, _Alloc>._No_throw) {
        constexpr _St_ref _Strat_ref = _Choice_ref<_G&, _Alloc>._Strategy;

        if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return vertices_depth_first_search(__g, seed, alloc); // intentional ADL
        } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
          return vertices_depth_first_search_view<_G, void>(__g, seed, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "vertices_depth_first_search(g,seed,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }

      /**
       * @brief Single Source, Breadth First Search for vertices with VVF
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam VVF   The vertex value function type.
       * @tparam Alloc The allocator type.
       * 
       * @param g      A graph instance.
       * @param vvf    The vertex value function.
       * @param seed   The vertex id to start the search.
       * 
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _VVF, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref_vvf<_G&, _VVF, _Alloc>._Strategy != _St_ref_vvf::_None)
      [[nodiscard]] constexpr auto
      operator()(_G&& __g, const vertex_id_t<_G>& seed, _VVF&& vvf, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref_vvf<_G&, _VVF, _Alloc>._No_throw) {
        constexpr _St_ref_vvf _Strat_ref_vvf = _Choice_ref_vvf<_G&, _VVF, _Alloc>._Strategy;

        if constexpr (_Strat_ref_vvf == _St_ref_vvf::_Non_member) {
          return vertices_depth_first_search(__g, seed, vvf, alloc); // intentional ADL
        } else if constexpr (_Strat_ref_vvf == _St_ref_vvf::_Auto_eval) {
          return vertices_depth_first_search_view<_G, _VVF>(__g, seed, vvf, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "vertices_depth_first_search(g,seed,vvf,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }
    };
  } // namespace _Vertices_DFS

  inline namespace _Cpos {
    inline constexpr _Vertices_DFS::_Cpo vertices_depth_first_search;
  }


  //
  // edges_depth_first_search(g,seed)     -> edge_descriptor[vid,uv]
  // edges_depth_first_search(g,seed,evf) -> edge_descriptor[vid,uv,value]
  //
  namespace _Edges_DFS {
#  if defined(__clang__) || defined(__EDG__)  // TRANSITION, VSO-1681199
    void edges_depth_first_search() = delete; // Block unqualified name lookup
#  else                                       // ^^^ no workaround / workaround vvv
    void edges_depth_first_search();
#  endif                                      // ^^^ workaround ^^^

    template <class _G, class _Alloc>
    concept _Has_ref_ADL = _HasClassOrEnumType<_G> //
                           && requires(_G&& __g, const vertex_id_t<_G>& uid, _Alloc alloc) {
                                { _Fake_copy_init(edges_depth_first_search(__g, uid, alloc)) }; // intentional ADL
                              };
    template <class _G, class _Alloc>
    concept _Can_ref_eval = index_adjacency_list<_G> //
                            && requires(_G&& __g, vertex_id_t<_G> uid, _Alloc alloc) {
                                 { _Fake_copy_init(edges_depth_first_search_view<_G, void, false>(__g, uid, alloc)) };
                               };

    template <class _G, class _EVF, class _Alloc>
    concept _Has_ref_evf_ADL = _HasClassOrEnumType<_G>              //
                               && invocable<_EVF, edge_reference_t<_G>> //
                               && requires(_G&& __g, const vertex_id_t<_G>& uid, _EVF evf, _Alloc alloc) {
                                    {
                                      _Fake_copy_init(edges_depth_first_search(__g, uid, evf, alloc))
                                    }; // intentional ADL
                                  };
    template <class _G, class _EVF, class _Alloc>
    concept _Can_ref_evf_eval =
          index_adjacency_list<_G>                 //
          && invocable<_EVF, edge_reference_t<_G>> //
          && requires(_G&& __g, vertex_id_t<_G> uid, _EVF evf, _Alloc alloc) {
               { _Fake_copy_init(edges_depth_first_search_view<_G, _EVF, false>(__g, uid, evf, alloc)) };
             };

    class _Cpo {
    private:
      enum class _St_ref { _None, _Non_member, _Auto_eval };
      enum class _St_ref_evf { _None, _Non_member, _Auto_eval };

      template <class _G, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_ADL<_G, _Alloc>) {
          return {_St_ref::_Non_member,
                  noexcept(_Fake_copy_init(edges_depth_first_search(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                                    declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_eval<_G, _Alloc>) {
          return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(edges_depth_first_search_view<_G, void, false>(
                                             declval<_G>(), declval<vertex_id_t<_G>>(), declval<_Alloc>())))};
        } else {
          return {_St_ref::_None};
        }
      }

      template <class _G, class _Alloc>
      static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G, _Alloc>();

      template <class _G, class _EVF, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref_evf> _Choose_ref_evf() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_evf_ADL<_G, _EVF, _Alloc>) {
          return {_St_ref_evf::_Non_member, noexcept(_Fake_copy_init(edges_depth_first_search(
                                                  declval<_G>(), declval<vertex_id_t<_G>>(), declval<_EVF>(),
                                                  declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_evf_eval<_G, _EVF, _Alloc>) {
          return {_St_ref_evf::_Auto_eval,
                  noexcept(_Fake_copy_init(edges_depth_first_search_view<_G, _EVF, false>(
                        declval<_G>(), declval<vertex_id_t<_G>>(), declval<_EVF>(), declval<_Alloc>())))};
        } else {
          return {_St_ref_evf::_None};
        }
      }

      template <class _G, class _EVF, class _Alloc>
      static constexpr _Choice_t<_St_ref_evf> _Choice_ref_evf = _Choose_ref_evf<_G, _EVF, _Alloc>();

    public:
      /**
       * @brief Single Source, Breadth First Search for edges
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam Alloc The allocator type.
       * 
       * @param g      A graph instance.
       * @param seed   The vertex id to start the search.
       * 
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref<_G&, _Alloc>._Strategy != _St_ref::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& seed, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref<_G&, _Alloc>._No_throw) {
        constexpr _St_ref _Strat_ref = _Choice_ref<_G&, _Alloc>._Strategy;

        if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return edges_depth_first_search(__g, seed, alloc); // intentional ADL
        } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
          return edges_depth_first_search_view<_G, void, false>(__g, seed, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "edges_depth_first_search(g,seed,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }

      /**
       * @brief Single Source, Breadth First Search for edges with EVF
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam EVF   The vertex value function type.
       * @tparam Alloc The allocator type.
       * 
       * @param g      A graph instance.
       * @param evf    The vertex value function.
       * @param seed   The vertex id to start the search.
       * 
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _EVF, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref_evf<_G&, _EVF, _Alloc>._Strategy != _St_ref_evf::_None)
      [[nodiscard]] constexpr auto
      operator()(_G&& __g, const vertex_id_t<_G>& seed, _EVF&& evf, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref_evf<_G&, _EVF, _Alloc>._No_throw) {
        constexpr _St_ref_evf _Strat_ref_evf = _Choice_ref_evf<_G&, _EVF, _Alloc>._Strategy;

        if constexpr (_Strat_ref_evf == _St_ref_evf::_Non_member) {
          return edges_depth_first_search(__g, seed, alloc); // intentional ADL
        } else if constexpr (_Strat_ref_evf == _St_ref_evf::_Auto_eval) {
          return edges_depth_first_search_view<_G, _EVF, false>(__g, seed, evf, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "edges_depth_first_search(g,seed,evf,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }
    };
  } // namespace _Edges_DFS

  inline namespace _Cpos {
    inline constexpr _Edges_DFS::_Cpo edges_depth_first_search;
  }


  //
  // sourced_edges_depth_first_search(g,seed)     -> edge_descriptor[uid,vid,uv]
  // sourced_edges_depth_first_search(g,seed,evf) -> edge_descriptor[uid,vid,uv,value]
  //
  namespace _Sourced_Edges_DFS {
#  if defined(__clang__) || defined(__EDG__)          // TRANSITION, VSO-1681199
    void sourced_edges_depth_first_search() = delete; // Block unqualified name lookup
#  else                                               // ^^^ no workaround / workaround vvv
    void sourced_edges_depth_first_search();
#  endif                                              // ^^^ workaround ^^^

    template <class _G, class _Alloc>
    concept _Has_ref_ADL = _HasClassOrEnumType<_G> //
                           && requires(_G&& __g, const vertex_id_t<_G>& uid, _Alloc alloc) {
                                {
                                  _Fake_copy_init(sourced_edges_depth_first_search(__g, uid, alloc))
                                }; // intentional ADL
                              };
    template <class _G, class _Alloc>
    concept _Can_ref_eval = index_adjacency_list<_G> //
                            && requires(_G&& __g, vertex_id_t<_G> uid, _Alloc alloc) {
                                 { _Fake_copy_init(edges_depth_first_search_view<_G, void, true>(__g, uid, alloc)) };
                               };

    template <class _G, class _EVF, class _Alloc>
    concept _Has_ref_evf_ADL = _HasClassOrEnumType<_G>              //
                               && invocable<_EVF, edge_reference_t<_G>> //
                               && requires(_G&& __g, const vertex_id_t<_G>& uid, _EVF evf, _Alloc alloc) {
                                    {
                                      _Fake_copy_init(sourced_edges_depth_first_search(__g, uid, evf, alloc))
                                    }; // intentional ADL
                                  };
    template <class _G, class _EVF, class _Alloc>
    concept _Can_ref_evf_eval =
          index_adjacency_list<_G>                 //
          && invocable<_EVF, edge_reference_t<_G>> //
          && requires(_G&& __g, vertex_id_t<_G> uid, _EVF evf, _Alloc alloc) {
               { _Fake_copy_init(edges_depth_first_search_view<_G, _EVF, true>(__g, uid, evf, alloc)) };
             };

    class _Cpo {
    private:
      enum class _St_ref { _None, _Non_member, _Auto_eval };
      enum class _St_ref_evf { _None, _Non_member, _Auto_eval };

      template <class _G, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref> _Choose_ref() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_ADL<_G, _Alloc>) {
          return {_St_ref::_Non_member,
                  noexcept(_Fake_copy_init(sourced_edges_depth_first_search(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                                            declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_eval<_G, _Alloc>) {
          return {_St_ref::_Auto_eval, noexcept(_Fake_copy_init(edges_depth_first_search_view<_G, void, true>(
                                             declval<_G>(), declval<vertex_id_t<_G>>(), declval<_Alloc>())))};
        } else {
          return {_St_ref::_None};
        }
      }

      template <class _G, class _Alloc>
      static constexpr _Choice_t<_St_ref> _Choice_ref = _Choose_ref<_G, _Alloc>();

      template <class _G, class _EVF, class _Alloc>
      [[nodiscard]] static consteval _Choice_t<_St_ref_evf> _Choose_ref_evf() noexcept {
        //static_assert(is_lvalue_reference_v<_G>);
        if constexpr (_Has_ref_evf_ADL<_G, _EVF, _Alloc>) {
          return {_St_ref_evf::_Non_member, noexcept(_Fake_copy_init(sourced_edges_depth_first_search(
                                                  declval<_G>(), declval<vertex_id_t<_G>>(), declval<_EVF>(),
                                                  declval<_Alloc>())))}; // intentional ADL
        } else if constexpr (_Can_ref_evf_eval<_G, _EVF, _Alloc>) {
          return {_St_ref_evf::_Auto_eval,
                  noexcept(_Fake_copy_init(edges_depth_first_search_view<_G, _EVF, true>(
                        declval<_G>(), declval<vertex_id_t<_G>>(), declval<_EVF>(), declval<_Alloc>())))};
        } else {
          return {_St_ref_evf::_None};
        }
      }

      template <class _G, class _EVF, class _Alloc>
      static constexpr _Choice_t<_St_ref_evf> _Choice_ref_evf = _Choose_ref_evf<_G, _EVF, _Alloc>();

    public:
      /**
       * @brief Single Source, Breadth First Search for source edges.
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam Alloc The allocator type.
       * 
       * @param g      A graph instance.
       * @param seed   The vertex id to start the search.
       * 
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref<_G&, _Alloc>._Strategy != _St_ref::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const vertex_id_t<_G>& seed, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref<_G&, _Alloc>._No_throw) {
        constexpr _St_ref _Strat_ref = _Choice_ref<_G&, _Alloc>._Strategy;

        if constexpr (_Strat_ref == _St_ref::_Non_member) {
          return sourced_edges_depth_first_search(__g, seed, alloc); // intentional ADL
        } else if constexpr (_Strat_ref == _St_ref::_Auto_eval) {
          return edges_depth_first_search_view<_G, void, true>(__g, seed, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "sourced_edges_depth_first_search(g,seed,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }


      /**
       * @brief Single Source, Breadth First Search for edges with EVF
       * 
       * Complexity: O(V + E)
       * 
       * @tparam G     The graph type.
       * @tparam EVF   The vertex value function type.
       * @tparam Alloc The allocator type.
       * 
       * @param g      A graph instance.
       * @param evf    The vertex value function.
       * @param seed   The vertex id to start the search.
       * 
       * @return A forward range for the breadth first search.
      */
      template <class _G, class _EVF, class _Alloc = std::allocator<bool>>
      requires(_Choice_ref_evf<_G&, _EVF, _Alloc>._Strategy != _St_ref_evf::_None)
      [[nodiscard]] constexpr auto
      operator()(_G&& __g, const vertex_id_t<_G>& seed, _EVF&& evf, _Alloc alloc = _Alloc()) const
            noexcept(_Choice_ref_evf<_G&, _EVF, _Alloc>._No_throw) {
        constexpr _St_ref_evf _Strat_ref_evf = _Choice_ref_evf<_G&, _EVF, _Alloc>._Strategy;

        if constexpr (_Strat_ref_evf == _St_ref_evf::_Non_member) {
          return sourced_edges_depth_first_search(__g, seed, alloc); // intentional ADL
        } else if constexpr (_Strat_ref_evf == _St_ref_evf::_Auto_eval) {
          return edges_depth_first_search_view<_G, _EVF, true>(__g, seed, evf, alloc); // default impl
        } else {
          static_assert(_AlwaysFalse<_G>, "The default implementation of "
                                           "sourced_edges_depth_first_search(g,seed,evf,alloc) cannot be evaluated and "
                                           "there is no override defined for the graph.");
        }
      }
    };
  } // namespace _Sourced_Edges_DFS

  inline namespace _Cpos {
    inline constexpr _Sourced_Edges_DFS::_Cpo sourced_edges_depth_first_search;
  }

} // namespace views
} // namespace graph


#endif // GRAPH_DFS_HPP
