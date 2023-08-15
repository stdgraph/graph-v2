#pragma once

#include "detail/graph_cpo.hpp"

#ifndef EDGELIST_HPP
#  define EDGELIST_HPP

namespace std::graph::edgelist {

template <class EL>
concept edgelist_range = ranges::forward_range<edgelist_range_t<EL>>;

template <class EL>
concept edgelist = edgelist_range<EL>;
} // namespace std::graph::edgelist

#endif