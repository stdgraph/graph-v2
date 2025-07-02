#pragma once

#ifndef CONTAINER_UTILITY_HPP
#  define CONTAINER_UTILITY_HPP

namespace graph::container {


//--------------------------------------------------------------------------------------------------
// utility functions
//

template <class C>
concept reservable = requires(C& container, typename C::size_type n) {
  { container.reserve(n) };
};
template <class C>
concept resizable = requires(C& container, typename C::size_type n) {
  { container.resize(n) };
};

template <class C>
concept has_emplace_back = requires(C& container, typename C::value_type&& value) {
  { container.emplace_back(move(value)) };
};
template <class C>
concept has_push_back = requires(C& container, std::ranges::range_reference_t<C> val) {
  { container.push_back(val) };
};
template <class C>
concept has_emplace_front = requires(C& container, typename C::value_type&& value) {
  { container.emplace_front(move(value)) };
};
template <class C>
concept has_push_front = requires(C& container, const typename C::value_type& value) {
  { container.push_front(value) };
};
template <class C>
concept has_emplace = requires(C& container, typename C::value_type&& value) {
  { container.emplace(move(value)) };
};
template <class C>
concept has_insert = requires(C& container, const typename C::value_type& value) {
  { container.insert(value) };
};

template <class C, class Idx>
concept has_array_operator = requires(C&& container, Idx idx) {
  { container[idx] }; //->is_lvalue_reference_v;
};

// return a lambda to push/insert/emplace an element in a container
template <class C>
constexpr auto push_or_insert(C& container) {
  // favor pushing to the back over the front for things like list & deque
  if constexpr (has_emplace_back<C>)
    return [&container](typename C::value_type&& value) { container.emplace_back(std::move(value)); };
  else if constexpr (has_push_back<C>) {
    return [&container](const typename C::value_type& value) { container.push_back(value); };
  } else if constexpr (has_emplace_front<C>)
    return [&container](typename C::value_type&& value) { container.emplace_front(std::move(value)); };
  else if constexpr (has_push_front<C>) {
    return [&container](const typename C::value_type& value) { container.push_front(value); };
  } else if constexpr (has_emplace<C>)
    return [&container](typename C::value_type&& value) { container.emplace(std::move(value)); };
  else if constexpr (has_insert<C>) {
    return [&container](const typename C::value_type& value) { container.insert(value); };
  }
#  ifdef _MSC_VER
  // This didn't assert if a previous if was true until MSVC 1931; gcc has always asserted
  // We need the ability to put constexpr on an else
  //else {
  //    static_assert(false,
  //              "The container doesn't have emplace_back, push_back, emplace_front, push_front, emplace or insert");
  //}
#  endif
}

// return a lambda to assign/insert an element in a container
// assignment applies to random_access containers that have elements pre-allocated
// insert     applies to other containers that can insert elements (e.g. map, unordered_map, ...)
template <class C, class K>
requires has_array_operator<C, K>
constexpr auto assign_or_insert(C& container) {
  if constexpr (::std::ranges::random_access_range<C>) {
    static_assert(::std::ranges::sized_range<C>, "random_access container is assumed to have size()");
    return [&container](const K& id, typename C::value_type&& value) {
      typename C::size_type k = static_cast<typename C::size_type>(id);
      assert(k < container.size());
      container[k] = move(value);
    };
  } else if constexpr (has_array_operator<C, K>) {
    return [&container](const K& id, typename C::value_type&& value) { container[id] = move(value); };
  }
}


// Requirements for extracting edge values from external sources for graph construction
// ERng is a forward_range because it is traversed twice; once to get the max vertex_id
// and a second time to load the edges.
template <class ERng, class EIdFnc, class EValueFnc>
concept edge_value_extractor = std::ranges::forward_range<ERng> && ::std::invocable<EIdFnc, typename ERng::value_type> &&
                               ::std::invocable<EValueFnc, typename ERng::value_type>;

namespace detail {
  //--------------------------------------------------------------------------------------
  // graph_value<> - wraps scaler, union & reference user values for graph, vertex & edge
  //
  template <class T>
  struct graph_value_wrapper {
    constexpr graph_value_wrapper()                            = default;
    graph_value_wrapper(const graph_value_wrapper&)            = default;
    graph_value_wrapper& operator=(const graph_value_wrapper&) = default;
    graph_value_wrapper(graph_value_wrapper&& v) : value(move(v.value)) {}
    graph_value_wrapper(const T& v) : value(v) {}
    graph_value_wrapper(T&& v) : value(move(v)) {}

    T value = T();
  };

  template <class T>
  struct graph_value_needs_wrap
        : std::integral_constant<bool,
                                 std::is_scalar<T>::value || std::is_array<T>::value || std::is_union<T>::value ||
                                       std::is_reference<T>::value> {};

  template <class T>
  constexpr auto user_value(T& v) -> T& {
    return v;
  }
  template <class T>
  constexpr auto user_value(const T& v) -> const T& {
    return v;
  }
} // namespace detail

//
// Common Property Values
//
struct empty_value {}; // empty graph|vertex|edge value

struct weight_value {
  int weight = 0;

  constexpr weight_value()                     = default;
  weight_value(const weight_value&)            = default;
  weight_value& operator=(const weight_value&) = default;
  weight_value(const int& w) : weight(w) {}
};

struct name_value {
  std::string name;

  name_value()                             = default;
  name_value(const name_value&)            = default;
  name_value& operator=(const name_value&) = default;
  name_value(const std::string& s) : name(s) {}
  name_value(std::string&& s) : name(std::move(s)) {}
};

} // namespace graph::container

#endif //CONTAINER_UTILITY_HPP
