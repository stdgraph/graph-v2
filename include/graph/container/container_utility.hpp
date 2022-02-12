#pragma once

#ifndef CONTAINER_UTILITY_HPP
#  define CONTAINER_UTILITY_HPP

namespace std::graph::container {

// Requirements for extracting edge values from external sources for graph construction
// ERng is a forward_range because it is traversed twice; once to get the max vertex_key
// and a second time to load the edges.
template <class ERng, class EKeyFnc, class EValueFnc>
concept edge_value_extractor = ranges::forward_range<ERng> && invocable<EKeyFnc, typename ERng::value_type> &&
      invocable<EValueFnc, typename ERng::value_type>;

namespace detail {
  //--------------------------------------------------------------------------------------
  // graph_value<> - wraps scaler, union & reference user values for graph, vertex & edge
  //
  template <class T>
  struct graph_value_wrapper {
    constexpr graph_value_wrapper()                 = default;
    graph_value_wrapper(const graph_value_wrapper&) = default;
    graph_value_wrapper& operator=(const graph_value_wrapper&) = default;
    graph_value_wrapper(graph_value_wrapper&& v) : value(move(v.value)) {}
    graph_value_wrapper(const T& v) : value(v) {}
    graph_value_wrapper(T&& v) : value(move(v)) {}

    T value = T();
  };

  template <class T>
  struct graph_value_needs_wrap
        : integral_constant<bool, is_scalar<T>::value || is_array<T>::value || is_union<T>::value ||
                                        is_reference<T>::value> {};

  template <class T>
  constexpr auto user_value(T & v)->T& {
    return v;
  }
  template <class T>
  constexpr auto user_value(const T& v)->const T& {
    return v;
  }
} // namespace detail

//
// Common Property Values
//
struct empty_value {}; // empty graph|vertex|edge value

struct weight_value {
  int weight = 0;

  constexpr weight_value()          = default;
  weight_value(const weight_value&) = default;
  weight_value& operator=(const weight_value&) = default;
  weight_value(const int& w) : weight(w) {}
};

struct name_value {
  string name;

  name_value()                  = default;
  name_value(const name_value&) = default;
  name_value& operator=(const name_value&) = default;
  name_value(const string& s) : name(s) {}
  name_value(string&& s) : name(move(s)) {}
};

} // namespace std::graph::container

#endif //CONTAINER_UTILITY_HPP
