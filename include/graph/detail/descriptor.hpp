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

template <class I, class VId = iter_difference_t<I>>
struct _descriptor_traits {
  using inner_iterator   = I;
  using difference_type  = iter_difference_t<inner_iterator>;
  using vertex_id_type   = VId;
  using value_type       = conditional_t<random_access_iterator<inner_iterator>, VId, inner_iterator>;
  using inner_value_type = iter_value_t<I>;
};


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
  using value_type     = conditional_t<random_access_iterator<inner_iterator>, vertex_id_type, inner_iterator>;

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


template <class T>
struct descriptor_inner_value {
  using type = T;
};
template <class T, class U>
struct descriptor_inner_value<std::pair<T, U>> {
  using type = decltype(tuple_tail(1, std::declval<std::pair<T, U>>()));
};
template <class... Args>
struct descriptor_inner_value<std::tuple<Args...>> {
  using type = decltype(tuple_tail(1, std::declval<std::tuple<Args...>>()));
};
template <class T>
using descriptor_inner_value_t = typename descriptor_inner_value<T>::type;


#  if 1
template <forward_range R, class VId = range_difference_t<R>>
class descriptor_view : public std::ranges::view_interface<descriptor_view<R>> {
public:
  //using size_type       = range_size_t<R>;
  using inner_iterator = iterator_t<R>;                             // iterator of the underlying container
  using iterator       = _descriptor_iterator<inner_iterator, VId>; //

  using value_type      = iter_value_t<iterator>;                   // descriptor value type
  using difference_type = iter_difference_t<iterator>;
  using id_type         = iterator::vertex_id_type;                 // e.g. vertex_id_t
  //using descriptor_type = iter_value_t<iterator>; // integral index or iterator, depending on container type

  descriptor_view() = default;
  constexpr explicit descriptor_view(R&& r) : r_(r) {}

  auto size() const
  requires sized_range<R>
  {
    return std::ranges::size(r_);
  }

  iterator begin() const { return std::ranges::begin(r_); }
  iterator end() const { return std::ranges::end(r_); }

  /**
   * @brief Get the vertex id for a descriptor.
   * 
   * @param desc The descriptor. This must refer to a valid element in the container.
   * @return vertex id.
   */
  id_type id(value_type& desc) const {
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
   * @brief Find an element in the container, given an id.
   * 
   * This assumes that the full range of id's in the container is [0, size(r_)). If a subrange is needed, use 
   * subrange_find.
   * 
   * @param id The id to search for.
   * @return Descriptor iterator to the element. If the element is not found, the iterator is equal to end().
   */
  iterator find(id_type id) const {
    if constexpr (integral<value_type>) {
      return iterator(id);
    } else if constexpr (random_access_range<R>) {
      return iterator(r_.begin() + id);
    } else if constexpr (bidirectional_range<R>) {
      return iterator(r_.find(id)); // map or set
    } else {
      static_assert(random_access_range<R>,
                    "find(id) cannot be evaluated for a forward range because there is no id/key in the container");
    }
    return end();
  }

private:
  reference_wrapper<R> r_;
};
#  endif //0

} // namespace graph

#endif // GRAPH_DESCRIPTOR_HPP
