#pragma once

#ifndef CONTAINER_UTILITY_HPP
#  define CONTAINER_UTILITY_HPP

namespace std::graph::container {

// Requirements for extracting edge values from external sources for graph construction
// ERng is a forward_range because it is traversed twice; once to get the max vertex_key
// and a second time to load the edges.
template <typename ERng, typename EKeyFnc, typename EValueFnc>
concept edge_value_extractor = ranges::forward_range<ERng> && invocable<EKeyFnc, typename ERng::value_type> &&
      invocable<EValueFnc, typename ERng::value_type>;


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
