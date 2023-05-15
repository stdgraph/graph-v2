#pragma once

#include "detail/graph_cpo.hpp"

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

namespace std::graph::edgelist {

template <class E>
concept edgelist_range = ranges::forward_range<edgelist_range_t<E>>;

template <class E>
concept edgelist = edgelist_range<E>;
} // namespace std::graph::edgelist

#endif