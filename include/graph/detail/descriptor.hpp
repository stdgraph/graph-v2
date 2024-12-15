#pragma once

#include <graph/detail/graph_using.hpp>

#ifndef GRAPH_DESCRIPTOR_HPP
#  define GRAPH_DESCRIPTOR_HPP

namespace graph {
// Descriptor Definition
//
// A descriptor is an abstract reference to a vertex or edge in a graph container.
//
// Practically, a descriptor is an integral index for contiguous and random-access containers and an iterator for other containers.
//

// Descriptor Change Notes
//
// Implications in the change for descriptors:
// 1. The vertex functions in the Graph Container Interface will replace vertex id and vertex reference with a vertex descriptor,
//    and edge functions will replace edge reference with vertex descriptor.
// 2. The second implication is that the number of concepts, views and functions will shrink because of this.
// 3. It will become easier to write generic algorithms without specializations for vertices in random-access vs. bidirectional
//    containers.
// 4. It becomes much harder to detect if uv.target_id(g) is defined, so member functions may not be auto-detected by CPOs.
//

// Questions
// 1. How to provide the raw vertex & edge ranges? (non-descriptor ranges)
//     a. special ranges? (e.g. raw_vertices_range(g), raw_edges_range(g,u))
//

// "Concepts" for descriptors
// 1. descriptor_iterator
// 2. descriptor_value: index or iterator
//
// 3. descriptor_view | descriptor_subrange_view
//     a. size()
//     b. begin(), end()
//     c. id(descriptor) -> vertex_id_t
//     d. find(id) ->
//
// 5. inner_iterator: iterator of the underlying container
// 6. inner_value_type: vertex or edge value
// 7. inner_id_type: index or key

// This is a limited form of tuple-like described in https://en.cppreference.com/w/cpp/utility/tuple/tuple-like.
// It only detects pair<T0,T1> and tuple<T0,T1,...>. It doesn't handle array or subrange.
//
template <typename T>
struct _is_tuple_like : std::false_type {};
template <class T, class U>
struct _is_tuple_like<pair<T, U>> : public std::true_type {};
template <class... Args>
struct _is_tuple_like<tuple<Args...>> : public std::true_type {};

template <typename... Args>
inline constexpr bool _is_tuple_like_v = _is_tuple_like<Args...>::value;

template<forward_iterator I, class IdT = iter_difference_t<I>>
using iter_descriptor_t = conditional_t<random_access_iterator<I>, IdT, I>;

template <forward_range R, class IdT = range_difference_t<R>>
using range_descriptor_t = iter_descriptor_t<iterator_t<R>, IdT>;


// vertex range type                          desc value_type   vertex_id_type    inner_value_type      target_id_type        edge_value_type
// =========================================  ================  ================  ====================  ====================  ================================
// vector<vector<int>>                        VId               VId               vector<int>           int                   void
// vector<vector<tuple<int, double, float>>>  VId               VId               vector<int>           int                   double (not tuple<double,float>)
// vector<map<int, double>>                   VId               VId               map<int, double>      int                   double
// vector<set<int>>                           VId               VId               set<int>              int                   void
// deque<deque<int>>                          VId               VId               deque<int>            int                   void
// deque<map<int, double>>                    VId               VId               map<int, double>      int                   double
// map<int,vector<int>>                       iterator          int               vector<int>           int                   void
// vertex<int>                                VId               VId               int                   n/a				      n/a    (e.g. CSR)
//
// edge range type                            desc value_type   inner_id_type     inner_value_type            target_id_type        edge_value_type
// =========================================  ================  ================  ==========================  ====================  ================================
// vector<int>                                VId               VId               int                         int                   void
// vector<tuple<int, double, float>>          VId               VId               tuple<int, double, float>   int                   double (not tuple<double,float>)
// map<int, double>                           iterator          int               pair<int, double>           int                   double
// set<int>                                   iterator          int               int                         int                   void
//
// inner_id_type is only useful for the edge range for adjacency_matrix
//
// Because the vertex range can be a simple vertex<int> for CSR we can't assume that the edge range is part of the
// vertex range. However, that is a common and useful use-case and can be used as a default.
//


/**
 * @brief An iterator that uses a descriptor (integral index or iterator) for a container.
 * 
 * An integral index is used for random-access containers, while an iterator is used for other containers.
 * 
 * A descriptor enables an abstraction that allows the same code to be used for different types of containers
 * without loss of efficiency from the underlying container. For instance, if an integral index is used
 * for a contiguous container, the code will be as efficient as if the index were used directly.
 * If it's used for a map (bidirectional container), the iterator will be dereferenced to get the associated
 * value, avoiding a log(n) lookup if the id were to be used.
 * 
 * A forward iterator is used because there is no requirement for random access when iterating through vertices
 * or edges.
 * 
 * @tparam I Iterator type of the underlying container.
 */
template <forward_iterator I, class VId = iter_difference_t<I>>
class _descriptor_iterator {
public:
  using this_type = _descriptor_iterator<I, VId>;

  using inner_iterator   = I;
  using difference_type  = iter_difference_t<inner_iterator>;
  using inner_value_type = iter_value_t<I>;

  using vertex_id_type = VId;
  using value_type     = iter_descriptor_t<I, vertex_id_type>;

  using pointer           = std::add_pointer_t<std::add_const_t<value_type>>;
  using reference         = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
  using iterator_category = std::forward_iterator_tag;
  using iterator_concept  = iterator_category;

  _descriptor_iterator() = default;
  explicit _descriptor_iterator(value_type descriptor) : descriptor_(descriptor) {}
  // copy and move constructors & assignment operators are default

  template <forward_range R>
  requires convertible_to<iterator_t<R>, inner_iterator>
  explicit _descriptor_iterator(R& r, inner_iterator iter) {
    if constexpr (integral<value_type>) {
      descriptor_ = static_cast<difference_type>(std::distance(std::ranges::begin(r), iter));
    } else {
      descriptor_ = iter;
    }
  }

  //
  // dereference
  //
  reference operator*() const noexcept { return descriptor_; }
  pointer   operator->() const noexcept { return &descriptor_; }

  //
  // operators ++
  //
  _descriptor_iterator& operator++() {
    ++descriptor_;
    return *this;
  }
  _descriptor_iterator operator++(int) {
    _descriptor_iterator tmp = *this;
    ++descriptor_;
    return tmp;
  }

  //
  // operators ==, !=
  //
  auto operator==(const _descriptor_iterator& rhs) const { return descriptor_ == rhs.descriptor_; }

private:
  value_type descriptor_ = value_type(); // integral index or iterator, depending on container type
};


template <forward_range R, class VId = range_difference_t<R>>
class descriptor_view : public std::ranges::view_interface<descriptor_view<R, VId>> {
public:
  //using size_type       = range_size_t<R>;
  using inner_iterator = iterator_t<R>;                             // iterator of the underlying container
  using iterator       = _descriptor_iterator<inner_iterator, VId>; //

  using value_type      = iter_value_t<iterator>;                   // descriptor value type
  using difference_type = iter_difference_t<iterator>;
  using id_type         = iterator::vertex_id_type;                 // e.g. vertex_id_t
  //using descriptor_type = iter_value_t<iterator>; // integral index or iterator, depending on container type

  descriptor_view() = default;
  constexpr descriptor_view(R& r) : r_(r) {}

  auto size() const
  requires sized_range<R>
  {
    return std::ranges::size(r_.get());
  }

  iterator begin() const {
    if constexpr (integral<value_type>) {
      return iterator(static_cast<id_type>(0));
    } else {
      return iterator(std::ranges::begin(r_.get()));
    }
  }
  iterator end() const {
    if constexpr (integral<value_type>) {
      return iterator(static_cast<id_type>(std::ranges::size(r_.get())));
    } else {
      return iterator(std::ranges::end(r_.get()));
    }
  }

  /**
   * @brief Get an iterator to an element in the underlying container.
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return An iterator to the underlying container.
   */
  inner_iterator inner_value(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return std::ranges::begin(r_.get(desc)) + desc;
    } else {
      return desc;
    }
  }

  /**
   * @brief Get the vertex id for a descriptor on an outer range.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return vertex id.
   */
  id_type get_vertex_id(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return desc;
    } else if constexpr (random_access_range<R>) {
      return static_cast<id_type>(std::distance(r_.begin(), desc));
    } else if constexpr (_is_tuple_like_v<range_value_t<R>>) {
      return std::get<0>(*desc); // e.g., pair::first used for map
    } else {
      static_assert(
            random_access_range<R>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  /**
   * @brief Get the target id for a descriptor in the inner range.
   * 
   * If the inner value is a tuple or pair, the first element is used as the target id. Examples:
   *    vector<pair<int,double>>
   *    list<tuple<int,double>>
   *    map<int,double>
   * Otherwise, the value itself is used as the target id. In some cases this is OK (e.g. set<int>).
   * In more complex cases, the target id may be a struct and the caller needs to determine what to do
   * with it.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return target id.
   */
  auto& get_target_id(const value_type& desc) const {
    if constexpr (_is_tuple_like_v<range_value_t<R>>) {
      return std::get<0>(*inner_value(desc)); // e.g., pair::first used for map, or vector<tuple<int,double>>
    } else {
      *inner_value(desc);                     // default to the value itself, e.g. set<int>
    }
  }

  /**
   * @brief Find an element in the descriptor container, given a descriptor.
   * 
   * This assumes that the full range of id's in the container is [0, size(r_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param desc The descriptor to search for, either an integral index or an iterator.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return iterator(desc);
    } else if constexpr (random_access_range<R>) {
      return iterator(r_.begin() + get_vertex_id(desc));
    } else if constexpr (bidirectional_range<R>) {
      return iterator(r_.find(get_vertex_id(desc))); // map or set
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return end();
  }

private:
  reference_wrapper<R> r_;
};


template <forward_range R, class VId = range_difference_t<R>>
class descriptor_subrange_view : public std::ranges::view_interface<descriptor_subrange_view<R, VId>> {
public:
  //using size_type       = range_size_t<R>;
  using inner_iterator = iterator_t<R>;                             // iterator of the underlying container
  using iterator       = _descriptor_iterator<inner_iterator, VId>; //

  using value_type      = iter_value_t<iterator>;                   // descriptor value type
  using difference_type = iter_difference_t<iterator>;
  using id_type         = iterator::vertex_id_type;                 // e.g. vertex_id_t
  //using descriptor_type = iter_value_t<iterator>; // integral index or iterator, depending on container type

  descriptor_subrange_view() = default;
  constexpr explicit descriptor_subrange_view(R& r)
        : r_(r), first_(r, std::ranges::begin(r)), last_(r, std::ranges::end(r)) {}
  constexpr descriptor_subrange_view(R& r, inner_iterator first, inner_iterator last)
        : r_(r), first_(r, first), last_(last) {}

  auto size() const
  requires sized_range<R>
  {
    return std::ranges::size(r_.get());
  }

  iterator begin() const { return first_; }
  iterator end() const { return last_; }

  /**
   * @brief Get an iterator to an element in the underlying container.
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return An iterator to the underlying container.
   */
  inner_iterator inner_value(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return std::ranges::begin(r_.get(desc)) + desc;
    } else {
      return desc;
    }
  }

  /**
   * @brief Get the vertex id for a descriptor on an outer range.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return vertex id.
   */
  id_type get_vertex_id(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return desc;
    } else if constexpr (random_access_range<R>) {
      return static_cast<id_type>(std::distance(r_.get().begin(), desc));
    } else if constexpr (_is_tuple_like_v<range_value_t<R>>) {
      return std::get<0>(*desc); // e.g., pair::first used for map
    } else {
      static_assert(
            random_access_range<R>,
            "id cannot be determined for a forward range or a bidirectional range without a tuple-like value type");
      return id_type();
    }
  }

  /**
   * @brief Get the target id for a descriptor in the inner range.
   * 
   * If the inner value is a tuple or pair, the first element is used as the target id. Examples:
   *    vector<pair<int,double>>
   *    list<tuple<int,double>>
   *    map<int,double>
   * Otherwise, the value itself is used as the target id. In some cases this is OK (e.g. set<int>).
   * In more complex cases, the target id may be a struct and the caller needs to determine what to do
   * with it.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return target id.
   */
  auto& get_target_id(const value_type& desc) const {
    if constexpr (_is_tuple_like_v<range_value_t<R>>) {
      return std::get<0>(*inner_value(desc)); // e.g., pair::first used for map, or vector<tuple<int,double>>
    } else {
      *inner_value(desc);                     // default to the value itself, e.g. set<int>
    }
  }

  /**
   * @brief Find an element in the descriptor container, given a descriptor.
   * 
   * This assumes that the full range of id's in the container is [0, size(r_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param id The id to search for.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(const value_type& desc) const {
    if constexpr (integral<value_type>) {
      return iterator(desc);
    } else if constexpr (random_access_range<R>) {
      return iterator(r_.get().begin() + get_vertex_id(desc));
    } else if constexpr (bidirectional_range<R>) {
      return iterator(r_.get().find(get_vertex_id(desc))); // map or set
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return last_;
  }

  /**
   * @brief Find an element in a container, given an id, constrained to the range [first_, last_).
   * 
   * The id must be in the range [first_, last_) of the container. If it isn't, the iterator will be equal to end()
   * which is also last_.
   * 
   * Note: The first/last constraint is really for edges in a CSR. Vertices in a CSR and edges in vertex<vertex<int>>
   * will include all elements in the container. Specialization for different conditions could reduce the number of
   * constraints.
   * 
   * @param id The id to search for. It must be in the range [first_, last_) of the container.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator subrange_find(const id_type& id) const {
    if constexpr (integral<value_type>) {
      assert(id >= *first_ && id < *last_);
      return iterator(r_, r_.begin() + id);
    } else if constexpr (random_access_range<R>) {
      assert((id >= *first_ - r_.begin()) && (id < *last_ - r_.begin()));
      return iterator(r_.begin() + id);
    } else if constexpr (bidirectional_range<R>) {
      auto it = r_.find(id);
      if (it != r_.end()) {
        return iterator(it);
      }
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return last_;
  }

private:
  reference_wrapper<R> r_;

  // first_ and last_ may be a sub-range of the container (e.g. for a CSR).
  iterator first_;
  iterator last_;
};

template <forward_range R, class VId = range_difference_t<R>>
auto to_descriptor_view(R&& r) {
  return descriptor_view<R, VId>(r);
}


} // namespace graph

#endif // GRAPH_DESCRIPTOR_HPP
