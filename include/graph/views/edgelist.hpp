#pragma once
#include "graph/graph.hpp"

//
// edgelist(g)     -> edge_descriptor<VId,true,E,void> -> {source_id, target_id, edge&}
// edgelist(g,evf) -> edge_descriptor<VId,true,E,EV>   -> {source_id, target_id, edge&, value}
//
// edgelist(g,uid)     -> edge_descriptor<VId,true,E,void> -> {source_id, target_id, edge&}
// edgelist(g,uid,evf) -> edge_descriptor<VId,true,E,EV>   -> {source_id, target_id, edge&, value}
//
// edgelist(elr,proj)  -> edge_descriptor<VId,true,E,void>  -> {source_id, target_id, edge&},        where VId is defined by proj and E is range_value_t<ELR>&
// edgelist(elr,proj)  -> edge_descriptor<VId,true,E,Value> -> {source_id, target_id, edge&, Value}, where VId and Value defined by proj and E is range_value_t<ELR>&
// Note: proj(range_value_t<ELR>&) is a projection and determines whether to return a value member or not
//
// basic_edgelist(g) -> edge_descriptor<VId,true,void,void> -> {source_id, target_id}
//
// basic_edgelist(g,uid) -> edge_descriptor<VId,true,void,void> -> {source_id, target_id}
//
// basic_edgelist(elr,proj)  -> edge_descriptor<VId,true,void,void>  -> {source_id, target_id},      where VId is defined by proj
// Note: proj(range_value_t<ELR>&) is a projection and determines whether to return a value member or not
//
// given:    auto evf = [&g](edge_reference_t<G> uv) { return edge_value(uv); }
//
//           vertex_id<G> first_id = ..., last_id = ...;
//
// examples: for([uid, vid, uv]        : edgelist(g))
//           for([uid, vid, uv, value] : edgelist(g,evf))
//
//           for([uid, vid, uv]        : edgelist(g,first_id,last_id))
//           for([uid, vid, uv, value] : edgelist(g,first_id,last_id,evf))
//
//           for([uid, vid]        : basic_edgelist(g))
//
//           for([uid, vid]        : basic_edgelist(g,uid))
//

namespace std::graph {

template <adjacency_list G, class EVF = void>
class edgelist_iterator;

#ifdef ENABLE_EDGELIST_RANGE
//template <edgelist_range ELR, class Proj = identity>
//requires requires(ranges::iterator_t<ELR> i, Proj&& proj) {
//  { proj(*i) };
//}
//class edgelist_range_iterator;
#endif

/**
 * @brief Common functionality for edgelist_iterator<G>
 * @tparam G Graph
*/
template <adjacency_list G>
class edgelist_iterator_base {
  using vertex_iterator = vertex_iterator_t<G>;
  using edge_iterator   = vertex_edge_iterator_t<G>;

protected:
  /**
   * If the current vertex is non-empty then uvi is set to begin(edges(g,*ui)).
   * Otherwise, skip past vertices until we find one with edges, and set uvi to the first edge.
   * If no vertices with edges are found, ui = end(vertices(g)).
   *
   * @param g   Graph instance
   * @param ui  Current vertex
   * @param uvi Current edge
  */
  constexpr void find_non_empty_vertex(G& g, vertex_iterator& ui, edge_iterator& uvi) noexcept {
    for (; ui != ranges::end(vertices(g)); ++ui) {
      if (!ranges::empty(edges(g, *ui))) {
        uvi = ranges::begin(edges(g, *ui));
        return;
      }
    }
  }

  /**
   * @brief Find the next edge. 
   *
   * Assumes current vertex & edge iterators point to valid objects.
   * 
   * @param g   Graph instance
   * @param ui  Current vertex
   * @param uvi Current edge
  */
  constexpr void find_next_edge(G& g, vertex_iterator& ui, edge_iterator& uvi) noexcept {
    assert(ui != ranges::end(vertices(g)));
    assert(uvi != ranges::end(edges(g, *ui)));
    if (++uvi != ranges::end(edges(g, *ui)))
      return;
    ++ui;
    find_non_empty_vertex(g, ui, uvi);
  }
};


/**
 * @brief Iterator for an edgelist range of edges for a vertex.
 *
 * @tparam G    Graph type
 * @tparam EVF  Edge Value Function type
*/
template <adjacency_list G, class EVF>
class edgelist_iterator : public edgelist_iterator_base<G> {
public:
  using base_type = edgelist_iterator_base<G>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<G>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_descriptor<const vertex_id_type, true, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  edgelist_iterator(graph_type& g, vertex_iterator ui, const EVF& value_fn)
        : base_type(), g_(g), ui_(ui), uvi_(), value_fn_(&value_fn) {}
  edgelist_iterator(graph_type& g, const EVF& value_fn) : edgelist_iterator(g, ranges::begin(vertices(g)), value_fn) {
    this->find_non_empty_vertex(g_, ui_, uvi_);
  }

  constexpr edgelist_iterator()                         = default;
  constexpr edgelist_iterator(const edgelist_iterator&) = default;
  constexpr edgelist_iterator(edgelist_iterator&&)      = default;
  constexpr ~edgelist_iterator()                        = default;

  constexpr edgelist_iterator& operator=(const edgelist_iterator&) = default;
  constexpr edgelist_iterator& operator=(edgelist_iterator&&)      = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type = remove_reference_t<edge_reference_type>;
  using shadow_value_type =
        edge_descriptor<vertex_id_type, true, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    internal_value() : shadow_{} {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  constexpr reference operator*() const {
    if constexpr (unordered_edge<G, edge_type>) {
      if (target_id(g_, *uvi_) != vertex_id(g_, ui_)) {
        value_.shadow_.source_id = source_id(g_, *uvi_);
        value_.shadow_.target_id = target_id(g_, *uvi_);
      } else {
        value_.shadow_.source_id = target_id(g_, *uvi_);
        value_.shadow_.target_id = source_id(g_, *uvi_);
      }
      value_.shadow_.edge  = &*uvi_;
      value_.shadow_.value = invoke(*value_fn_, *uvi_);
    } else {
      value_.shadow_ = {vertex_id(g_, ui_), target_id(g_, *uvi_), &*uvi_, invoke(*value_fn_, *uvi_)};
    }
    return value_.value_;
  }

  constexpr edgelist_iterator& operator++() {
    this->find_next_edge(g_, ui_, uvi_);
    return *this;
  }
  constexpr edgelist_iterator operator++(int) const {
    edgelist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs.uvi_; }
  //constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs; }

private: // member variables
  mutable internal_value           value_;
  _detail::ref_to_ptr<graph_type&> g_;
  vertex_iterator                  ui_;
  edge_iterator                    uvi_;
  const EVF*                       value_fn_ = nullptr;

  friend bool operator==(const vertex_iterator& lhs, const edgelist_iterator& rhs) { return lhs == rhs.ui_; }
};


template <adjacency_list G>
class edgelist_iterator<G, void> : public edgelist_iterator_base<G> {
public:
  using base_type = edgelist_iterator_base<G>;

  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_id_type  = vertex_id_t<graph_type>;
  using vertex_iterator = vertex_iterator_t<G>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_descriptor<const vertex_id_type, true, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type  = remove_reference_t<edge_reference_type>;
  using shadow_value_type = edge_descriptor<vertex_id_type, true, shadow_edge_type*, edge_value_type>;

  union internal_value {
    value_type        value_;
    shadow_value_type shadow_;

    internal_value(const internal_value& rhs) : shadow_(rhs.shadow_) {}
    internal_value() : shadow_{} {}
    ~internal_value() {}
    internal_value& operator=(const internal_value& rhs) { value_.shadow = rhs.value_.shadow; }
  };

public:
  edgelist_iterator(graph_type& g, vertex_iterator ui) : base_type(), g_(g), ui_(ui), uvi_() {
    this->find_non_empty_vertex(g_, ui_, uvi_);
  }
  edgelist_iterator(graph_type& g) : edgelist_iterator(g, ranges::begin(vertices(g))) {}

  constexpr edgelist_iterator()                         = default;
  constexpr edgelist_iterator(const edgelist_iterator&) = default;
  constexpr edgelist_iterator(edgelist_iterator&&)      = default;
  constexpr ~edgelist_iterator()                        = default;

  constexpr edgelist_iterator& operator=(const edgelist_iterator&) = default;
  constexpr edgelist_iterator& operator=(edgelist_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    if constexpr (unordered_edge<G, edge_type>) {
      if (target_id(g_, *uvi_) != vertex_id(g_, ui_)) {
        value_.shadow_.source_id = source_id(g_, *uvi_);
        value_.shadow_.target_id = target_id(g_, *uvi_);
      } else {
        value_.shadow_.source_id = target_id(g_, *uvi_);
        value_.shadow_.target_id = source_id(g_, *uvi_);
      }
      value_.shadow_.edge = &*uvi_;
    } else {
      value_.shadow_ = {vertex_id(g_, ui_), target_id(g_, *uvi_), &*uvi_};
    }
    return value_.value_;
  }

  constexpr edgelist_iterator& operator++() {
    this->find_next_edge(g_, ui_, uvi_);
    return *this;
  }
  constexpr edgelist_iterator operator++(int) const {
    edgelist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs.uvi_; }
  //constexpr bool operator==(const edgelist_iterator& rhs) const { return uvi_ == rhs; }

private: // member variables
  mutable internal_value           value_;
  _detail::ref_to_ptr<graph_type&> g_;
  vertex_iterator                  ui_;
  edge_iterator                    uvi_;

  friend bool operator==(const vertex_iterator& lhs, const edgelist_iterator& rhs) { return lhs == rhs.ui_; }
};


/**
 * @brief Iterator for a range with values that can be projected to a edge_descriptor.
 *
 * @tparam ELR    Graph type
 * @tparam Proj  Edge Value Function type
*/
#if 0
template <edgelist_range ELR, class Proj>
//requires requires(ranges::iterator_t<ELR> i, Proj&& proj) {
//  { proj(*i)->target_id };
//}
class edgelist_range_iterator {
public:
  using edge_range          = ELR;
  using edge_iterator       = ranges::iterator_t<edge_range>;
  using edge_type           = ranges::range_value_t<edge_range>;
  using edge_reference_type = ranges::range_reference_t<edge_range>;
  using value_type          = invoke_result_t<Proj, edge_reference_type>;

  using vertex_id_type =
        typename value_type::target_id_type; // If this generates an error then value_type is not an edge_descriptor

  using iterator_category = forward_iterator_tag;

  using difference_type  = ranges::range_difference_t<edge_range>;
  using pointer          = value_type*;
  using const_pointer    = const value_type*;
  using reference        = value_type&;
  using const_reference  = const value_type&;
  using rvalue_reference = value_type&&;

public:
  edgelist_range_iterator(edge_range& elr, edge_iterator uvi, Proj&& proj_fn) //
        : uvi_(uvi), proj_fn_(proj_fn) {}
  edgelist_range_iterator(edge_range& elr, Proj&& proj_fn)
        : edgelist_range_iterator(elr, ranges::begin(vertices(elr)), forward<Proj>(proj_fn)) {}

  constexpr edgelist_range_iterator()                               = default;
  constexpr edgelist_range_iterator(const edgelist_range_iterator&) = default;
  constexpr edgelist_range_iterator(edgelist_range_iterator&&)      = default;
  constexpr ~edgelist_range_iterator()                              = default;

  constexpr edgelist_range_iterator& operator=(const edgelist_range_iterator&) = default;
  constexpr edgelist_range_iterator& operator=(edgelist_range_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    new (&value_) value_type(invoke(*proj_fn_, *uvi_));
    return value_;
  }

  constexpr edgelist_range_iterator& operator++() {
    ++uvi_;
    return *this;
  }
  constexpr edgelist_range_iterator operator++(int) const {
    edgelist_range_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const edgelist_range_iterator& rhs) const { return uvi_ == rhs.uvi_; }

private: // member variables
  mutable value_type value_;
  edge_iterator      uvi_;
  Proj&&             proj_fn_;
};
#endif


template <adjacency_list G, class EVF>
using edgelist_view = ranges::subrange<edgelist_iterator<G, EVF>, vertex_iterator_t<G>>;

//template <edgelist_range ELR, class Proj>
//using edgelist_view = ranges::subrange<edgelist_iterator<ELR, Proj>, vertex_iterator_t<ELR>>;

namespace views {

  // edgelist(g)               -> edges[uid,vid,uv]
  // edgelist(g,fn)            -> edges[uid,vid,uv,value]
  // edgelist(g,uid,vid)       -> edges[uid,vid,uv]
  // edgelist(g,uid,vid,fn)    -> edges[uid,vid,uv,value]
  //
  // edgelist(elr,proj)        -> edges[uid,vid,uv]         ; proj determines whether value is included or not
  // edgelist(elr,proj)        -> edges[uid,vid,uv,value]
  namespace _Edgelist {
#if defined(__clang__) || defined(__EDG__) // TRANSITION, VSO-1681199
    void edgelist() = delete;              // Block unqualified name lookup
#else                                      // ^^^ no workaround / workaround vvv
    void edgelist();
#endif                                     // ^^^ workaround ^^^

    template <class _G, class _UnCV>
    concept _Has_adjlist_all_ADL = _Has_class_or_enum_type<_G> //
                                   && adjacency_list<_G>       //
                                   && requires(_G&& __g) {
                                        { _Fake_copy_init(edgelist(__g)) }; // intentional ADL
                                      };
    template <class _G, class _UnCV>
    concept _Can_adjlist_all_eval = _Has_class_or_enum_type<_G> //
                                    && adjacency_list<_G>       //
                                    && requires(_G&& __g, vertex_reference_t<_G> u) {
                                         { _Fake_copy_init(vertices(__g)) };
                                         { _Fake_copy_init(edges(__g, u)) };
                                       };

    template <class _G, class _UnCV, class EVF>
    concept _Has_adjlist_all_evf_ADL = _Has_class_or_enum_type<_G>             //
                                       && adjacency_list<_G>                   //
                                       && invocable<EVF, edge_reference_t<_G>> //
                                       && requires(_G&& __g, EVF evf) {
                                            { _Fake_copy_init(edgelist(__g, evf)) }; // intentional ADL
                                          };
    template <class _G, class _UnCV, class EVF>
    concept _Can_adjlist_all_evf_eval = _Has_class_or_enum_type<_G>             //
                                        && adjacency_list<_G>                   //
                                        && invocable<EVF, edge_reference_t<_G>> //
                                        && requires(_G&& __g, vertex_reference_t<_G> u) {
                                             { _Fake_copy_init(vertices(__g)) };
                                             { _Fake_copy_init(edges(__g, u)) };
                                           };


    template <class _G, class _UnCV>
    concept _Has_adjlist_idrng_ADL = _Has_class_or_enum_type<_G> //
                                     && adjacency_list<_G>       //
                                     && requires(_G&& __g, vertex_id_t<_G> uid, vertex_id_t<_G> vid) {
                                          { _Fake_copy_init(edgelist(__g, uid, vid)) }; // intentional ADL
                                        };
    template <class _G, class _UnCV>
    concept _Can_adjlist_idrng_eval = _Has_class_or_enum_type<_G> //
                                      && adjacency_list<_G>       //
                                      && requires(_G&& __g, vertex_id_t<_G> uid) {
                                           { _Fake_copy_init(vertices(__g)) };
                                           { _Fake_copy_init(edges(__g, uid)) };
                                         };

    template <class _G, class _UnCV, class EVF>
    concept _Has_adjlist_idrng_evf_ADL = _Has_class_or_enum_type<_G>             //
                                         && adjacency_list<_G>                   //
                                         && invocable<EVF, edge_reference_t<_G>> //
                                         && requires(_G&& __g, vertex_id_t<_G> uid, vertex_id_t<_G> vid, EVF evf) {
                                              { _Fake_copy_init(edgelist(__g, uid, vid, evf)) }; // intentional ADL
                                            };
    template <class _G, class _UnCV, class EVF>
    concept _Can_adjlist_idrng_evf_eval = _Has_class_or_enum_type<_G>             //
                                          && adjacency_list<_G>                   //
                                          && invocable<EVF, edge_reference_t<_G>> //
                                          && requires(_G&& __g, vertex_id_t<_G> uid, EVF evf) {
                                               { _Fake_copy_init(vertices(__g)) };
                                               { _Fake_copy_init(edges(__g, uid)) };
                                             };

#ifdef ENABLE_EDGELIST_RANGE
    template <class ELR, class _UnCV, class Proj>
    concept _Has_edgelist_all_proj_ADL = edgelist_range<ELR> //
                                         && invocable<Proj, ranges::range_value_t<ELR>>;
    template <class ELR, class _UnCV, class Proj>
    concept _Can_edgelist_all_proj_eval = edgelist_range<ELR> //
                                          && invocable<Proj, ranges::range_value_t<ELR>>;
#endif

    class _Cpo {
    private:
      enum class _St_adjlist_all { _None, _Non_member, _Auto_eval };
      enum class _St_adjlist_idrng { _None, _Non_member, _Auto_eval };
      enum class _St_edgelist_all { _None, _Non_member, _Auto_eval };

      // edgelist(g)
      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_adjlist_all> _Choose_all() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        using _UnCV = remove_cvref_t<_G>;

        if constexpr (_Has_adjlist_all_ADL<_G, _UnCV>) {
          return {_St_adjlist_all::_Non_member, noexcept(_Fake_copy_init(edgelist(declval<_G>())))}; // intentional ADL
        } else if constexpr (_Can_adjlist_all_eval<_G, _UnCV>) {
          return {_St_adjlist_all::_Auto_eval, noexcept(true)}; // default impl (revisit)
        } else {
          return {_St_adjlist_all::_None};
        }
      }

      template <class _G>
      static constexpr _Choice_t<_St_adjlist_all> _Choice_all = _Choose_all<_G>();

      // edgelist(g,evf)
      template <class _G, class EVF>
      [[nodiscard]] static consteval _Choice_t<_St_adjlist_all> _Choose_all_evf() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        using _UnCV = remove_cvref_t<_G>;

        if constexpr (_Has_adjlist_all_evf_ADL<_G, _UnCV, EVF>) {
          return {_St_adjlist_all::_Non_member,
                  noexcept(_Fake_copy_init(edgelist(declval<_G>(), declval<EVF>())))}; // intentional ADL
        } else if constexpr (_Can_adjlist_all_evf_eval<_G, _UnCV, EVF>) {
          return {_St_adjlist_all::_Auto_eval, noexcept(true)}; // default impl (revisit)
        } else {
          return {_St_adjlist_all::_None};
        }
      }

      template <class _G, class EVF>
      static constexpr _Choice_t<_St_adjlist_all> _Choice_all_evf = _Choose_all_evf<_G, EVF>();


      // edgelist(g,uid,vid)
      template <class _G>
      [[nodiscard]] static consteval _Choice_t<_St_adjlist_idrng> _Choose_idrng() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        using _UnCV = remove_cvref_t<_G>;

        if constexpr (_Has_adjlist_idrng_ADL<_G, _UnCV>) {
          return {_St_adjlist_idrng::_Non_member,
                  noexcept(_Fake_copy_init(edgelist(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                    declval<vertex_id_t<_G>>())))}; // intentional ADL
        } else if constexpr (_Can_adjlist_idrng_eval<_G, _UnCV>) {
          return {_St_adjlist_idrng::_Auto_eval, noexcept(true)}; // default impl (revisit)
        } else {
          return {_St_adjlist_idrng::_None};
        }
      }

      template <class _G>
      static constexpr _Choice_t<_St_adjlist_idrng> _Choice_idrng = _Choose_idrng<_G>();

      // edgelist(g,uid,vid,evf)
      template <class _G, class EVF>
      [[nodiscard]] static consteval _Choice_t<_St_adjlist_idrng> _Choose_idrng_evf() noexcept {
        static_assert(is_lvalue_reference_v<_G>);
        using _UnCV = remove_cvref_t<_G>;

        if constexpr (_Has_adjlist_idrng_evf_ADL<_G, _UnCV, EVF>) {
          return {_St_adjlist_idrng::_Non_member,
                  noexcept(_Fake_copy_init(edgelist(declval<_G>(), declval<vertex_id_t<_G>>(),
                                                    declval<vertex_id_t<_G>>(), declval<EVF>())))}; // intentional ADL
        } else if constexpr (_Can_adjlist_idrng_evf_eval<_G, _UnCV, EVF>) {
          return {_St_adjlist_idrng::_Auto_eval, noexcept(true)}; // default impl (revisit)
        } else {
          return {_St_adjlist_idrng::_None};
        }
      }

      template <class _G, class EVF>
      static constexpr _Choice_t<_St_adjlist_idrng> _Choice_idrng_evf = _Choose_idrng_evf<_G, EVF>();

#ifdef ENABLE_EDGELIST_RANGE
      // edgelist(elr,proj)
      template <class ELR, class Proj>
      [[nodiscard]] static consteval _Choice_t<_St_edgelist_all> _Choose_elr_proj() noexcept {
        static_assert(is_lvalue_reference_v<ELR>);
        using _UnCV = remove_cvref_t<ELR>;

        if constexpr (_Has_edgelist_all_proj_ADL<ELR, _UnCV, Proj>) {
          return {_St_edgelist_all::_Non_member,
                  noexcept(_Fake_copy_init(edgelist(declval<ELR>(), declval<Proj>())))}; // intentional ADL
        } else if constexpr (_Can_edgelist_all_proj_eval<ELR, _UnCV, Proj>) {
          return {_St_edgelist_all::_Auto_eval, noexcept(true)}; // default impl (revisit)
        } else {
          return {_St_edgelist_all::_None};
        }
      }

      template <class ELR, class Proj>
      static constexpr _Choice_t<_St_edgelist_all> _Choice_elr_proj = _Choose_elr_proj<ELR, Proj>();
#endif

    public:
      // edgelist(g)
      /**
     * @brief Get the edgelist of all edges in a graph.
     * 
     * Complexity: O(E)
     * 
     * Default implementation: 
     *  using iterator_type = edgelist_iterator<G, void>;
     *  edgelist_view<G, void>(iterator_type(g), ranges::end(vertices(g)));
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @return A range of edges in graph g where the range value_type is 
     *         edge_descriptor<vertex_id_t<G>,false,vertex_reference_t<G>,void>
    */
      template <class _G>
      requires(_Choice_all<_G&>._Strategy != _St_adjlist_all::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g) const noexcept(_Choice_all<_G&>._No_throw) {
        constexpr _St_adjlist_all _Strat_ref = _Choice_all<_G&>._Strategy;

        if constexpr (_Strat_ref == _St_adjlist_all::_Non_member) {
          return edgelist(__g); // intentional ADL
        } else if constexpr (_Strat_ref == _St_adjlist_all::_Auto_eval) {
          using iterator_type = edgelist_iterator<_G, void>;
          return edgelist_view<_G, void>(iterator_type(__g), ranges::end(vertices(__g))); // default impl
        } else {
          static_assert(_Always_false<_G>,
                        "edgelist(g) is not defined and the default implementation cannot be evaluated");
        }
      }

      // edgelist(g,evf)
      /**
     * @brief Get the edgelist of all edges in a graph, with edge values.
     * 
     * Complexity: O(E)
     * 
     * Default implementation: 
     *  using iterator_type = edgelist_iterator<G, void>;
     *  edgelist_view<G, void>(iterator_type(g), ranges::end(vertices(g)));
     * 
     * @tparam G The graph type.
     * @param g A graph instance.
     * @return A range of edges in graph g where the range value_type is 
     *         edge_descriptor<vertex_id_t<G>,false,vertex_reference_t<G>,void>
    */
      template <class _G, class EVF>
      requires(_Choice_all_evf<_G&, EVF>._Strategy != _St_adjlist_all::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, const EVF& evf) const
            noexcept(_Choice_all_evf<_G&, EVF>._No_throw) {
        constexpr _St_adjlist_all _Strat_ref = _Choice_all_evf<_G&, EVF>._Strategy;

        if constexpr (_Strat_ref == _St_adjlist_all::_Non_member) {
          return edgelist(__g); // intentional ADL
        } else if constexpr (_Strat_ref == _St_adjlist_all::_Auto_eval) {
          using iterator_type = edgelist_iterator<_G, EVF>;
          return edgelist_view<_G, EVF>(iterator_type(__g, evf), ranges::end(vertices(__g)));
        } else {
          static_assert(_Always_false<_G>,
                        "edgelist(g,evf) is not defined and the default implementation cannot be evaluated");
        }
      }

      // edgelist(__g,uid,vid)
      /**
     * @brief Get the edgelist of all edges in a graph.
     * 
     * Complexity: O(E)
     * 
     * Default implementation: 
     *  using iterator_type = edgelist_iterator<G, void>;
     *  edgelist_view<G, void>(iterator_type(__g), ranges::end(vertices(__g)));
     * 
     * @tparam G The graph type.
     * @param __g A graph instance.
     * @return A range of edges in graph __g where the range value_type is 
     *         edge_descriptor<vertex_id_t<G>,false,vertex_reference_t<G>,void>
    */
      template <class _G>
      requires(_Choice_idrng<_G&>._Strategy != _St_adjlist_idrng::_None)
      [[nodiscard]] constexpr auto operator()(_G&& __g, vertex_id_t<_G> first, vertex_id_t<_G> last) const
            noexcept(_Choice_idrng<_G&>._No_throw) {
        constexpr _St_adjlist_idrng _Strat_ref = _Choice_idrng<_G&>._Strategy;

        if constexpr (_Strat_ref == _St_adjlist_idrng::_Non_member) {
          return edgelist(__g); // intentional ADL
        } else if constexpr (_Strat_ref == _St_adjlist_idrng::_Auto_eval) {
          using iterator_type = edgelist_iterator<_G, void>;
          return edgelist_view<_G, void>(iterator_type(__g, find_vertex(__g, first)), find_vertex(__g, last));
        } else {
          static_assert(_Always_false<_G>,
                        "edgelist(g,uid,vid) is not defined and the default implementation cannot be evaluated");
        }
      }

      // edgelist(g,uid,vid,evf)
      /**
     * @brief Get the edgelist of all edges in a graph, with edge values.
     * 
     * Complexity: O(E)
     * 
     * Default implementation: 
     *  using iterator_type = edgelist_iterator<_G, void>;
     *  edgelist_view<_G, void>(iterator_type(g), ranges::end(vertices(g)));
     * 
     * @tparam _G The graph type.
     * @param g A graph instance.
     * @return A range of edges in graph g where the range value_type is 
     *         edge_descriptor<vertex_id_t<_G>,false,vertex_reference_t<_G>,void>
    */
      template <class _G, class EVF>
      requires(_Choice_idrng_evf<_G&, EVF>._Strategy != _St_adjlist_idrng::_None)
      [[nodiscard]] constexpr auto
      operator()(_G&& __g, vertex_id_t<_G> first, vertex_id_t<_G> last, const EVF& evf) const
            noexcept(_Choice_idrng<_G&>._No_throw) {
        constexpr _St_adjlist_idrng _Strat_ref = _Choice_idrng_evf<_G&, EVF>._Strategy;

        if constexpr (_Strat_ref == _St_adjlist_idrng::_Non_member) {
          return edgelist(__g); // intentional ADL
        } else if constexpr (_Strat_ref == _St_adjlist_idrng::_Auto_eval) {
          using iterator_type = edgelist_iterator<_G, EVF>;
          return edgelist_view<_G, void>(iterator_type(__g, evf), ranges::end(vertices(__g))); // default impl
        } else {
          static_assert(_Always_false<_G>,
                        "edgelist(g,uid,vid,evf) is not defined and the default implementation cannot be evaluated");
        }
      }


#ifdef ENABLE_EDGELIST_RANGE
      // edgelist(elr,proj)
      /**
     * @brief Create an edgelist from an arbitrary range using a projection.
     * 
     * The projection must return a value of one of the following types:
     *      edge_descriptor<Id, true, range_value_t<ELR>&, void>
     *      edge_descriptor<Id, true, range_value_t<ELR>&, Value>
     * where Id is an integral type defined by proj, and Value is also a type defined by proj.
     * 
     * Complexity: O(n)
     * 
     * Default implementation: 
     *  ???
     * 
     * @tparam ELR The Edge List Range type. This can be any forward_range.
     * @tparam Proj The projection function type that converts a range_value_t<ELR> to an edge_descriptor.
     * @param elr The Edge List Range instance.
     * @param proj The projection instance that converts a range_value_t<ELR> to an edge_descriptor. If
     *             the range_value_t<ELR> is already a valid edge_descriptor then identity can be used.
     * @return A range of edge_descriptors projected from elr. The value member type can be void or non-void.
     *         If it is void, the value member will not exist in the returned edge_descriptor.
    */
      template <class ELR, class Proj>
      requires(_Choice_elr_proj<ELR&, Proj>._Strategy != _St_adjlist_all::_None)
      [[nodiscard]] constexpr auto operator()(ELR&& elr, const Proj& proj = identity()) const
            noexcept(_Choice_elr_proj<ELR&, Proj>._No_throw) {
        constexpr _St_adjlist_all _Strat_ref = _Choice_elr_proj<ELR&, Proj>._Strategy;

        if constexpr (_Strat_ref == _St_adjlist_all::_Non_member) {
          return edgelist(elr); // intentional ADL
        } else if constexpr (_Strat_ref == _St_adjlist_all::_Auto_eval) {
          //using iterator_type = edgelist_iterator<ELR, Proj>;
          //return edgelist_view<ELR, Proj>(iterator_type(elr, proj), ranges::end(vertices(elr)));
          return 1; // bogus; must implement
        } else {
          static_assert(_Always_false<ELR>,
                        "edgelist(elr,proj) is not defined and the default implementation cannot be evaluated");
        }
      }
#endif //ENABLE_EDGELIST_RANGE

    }; // class _Cpo
  }    // namespace _Edgelist

  inline namespace _Cpos {
    inline constexpr _Edgelist::_Cpo edgelist;
  }

} // namespace views
} // namespace std::graph
