#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "graph/edgelist.hpp"
#include "graph/graph_descriptors.hpp"
#include <vector>
#include <tuple>

using std::declval;
using std::vector;
using std::tuple;
using std::is_same_v;
using std::same_as;
using std::graph::edge_descriptor;
using namespace std::graph::edgelist;

TEST_CASE("edgelist tuple test", "[edgelist][tuple]") {
  using EL = vector<tuple<int, int>>;
  using E  = std::ranges::range_value_t<EL>;
  _Target_id::_Cpo cpo;
  E                e;
  static_assert(same_as<E, tuple<int, int>>);

  static_assert(!std::ranges::forward_range<E>);
  //static_assert(_el_value<E>);
  static_assert(_el_tuple_edge<E>);
  static_assert(_Target_id::_is_tuple_edge<E>);
  //static_assert(_Target_id::_Cpo::_Choice_edgl_ref<E>._Strategy == _Target_id::_Cpo::_St_ref::_Tuple_id);
  static_assert(same_as<decltype(cpo(declval<E>())), int>);
  static_assert(same_as<decltype(target_id(e)), int>);

  static_assert(!std::ranges::forward_range<E>);
  //static_assert(_el_value<E>);
  static_assert(_el_tuple_edge<E>);
  static_assert(_Source_id::_is_tuple_edge<E>);
  //static_assert(_Source_id::_Cpo::_Choice_edgl_ref<E>._Strategy == _Source_id::_Cpo::_St_ref::_Tuple_id);
  static_assert(same_as<decltype(cpo(declval<E>())), int>);
  static_assert(same_as<decltype(source_id(e)), int>);

  static_assert(_source_target_id<E>);
  static_assert(basic_sourced_edgelist<EL>);
}

TEST_CASE("edgelist edge_descriptor test", "[edgelist][edge_descriptor]") {
  using EL = vector<edge_descriptor<int, true, void, void>>;
  using E  = std::ranges::range_value_t<EL>;
  _Target_id::_Cpo cpo;
  E                e;
  static_assert(same_as<E, edge_descriptor<int, true, void, void>>);

  static_assert(!std::ranges::forward_range<E>);
  //static_assert(_el_value<E>);
  static_assert(_el_basic_sourced_edge_desc<E>);
  static_assert(_Target_id::_is_edge_desc<E>);
  //static_assert(_Target_id::_Cpo::_Choice_edgl_ref<E>._Strategy == _Target_id::_Cpo::_St_ref::_Tuple_id);
  static_assert(same_as<decltype(cpo(declval<E>())), int>);
  static_assert(same_as<decltype(target_id(e)), int>);

  static_assert(!std::ranges::forward_range<E>);
  //static_assert(_el_value<E>);
  static_assert(_el_basic_sourced_edge_desc<E>);
  static_assert(_Source_id::_is_edge_desc<E>);
  //static_assert(_Source_id::_Cpo::_Choice_edgl_ref<E>._Strategy == _Source_id::_Cpo::_St_ref::_Tuple_id);
  static_assert(same_as<decltype(cpo(declval<E>())), int>);
  static_assert(same_as<decltype(source_id(e)), int>);

  static_assert(_source_target_id<E>);
  static_assert(basic_sourced_edgelist<EL>);
}
