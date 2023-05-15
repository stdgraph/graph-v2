#pragma once

#include "graph/edgelist.hpp"

namespace std::graph::container{

template<typename VSourceId, typename VTargetId, typename EV>
class utility_edgelist {
public:
  using value_type = std::tuple<VSourceId, VTargetId, EV>;
  using storage_type = std::vector<value_type>;
  using storage_iterator_type = storage_type::iterator;

  utility_edgelist() : source_max_(0), target_max_(0) { }
  utility_edgelist( const utility_edgelist &list ) :
    storage_(list.storage_),
    source_max_(list.source_max_),
    target_max_(list.target_max_),
    bipartite_(list.bipartite_),
    sorted_source_(list.sourced_source_),
    sorted_target_(list.sorted_target_),
    directed_(list.directed_) 
  {
  }

  void push_back( value_type edge ) {
    if ( std::get<0>(edge) > source_max_ ) {
        source_max_ = std::get<0>(edge);
    }
    if ( std::get<1>(edge) > target_max_ ) {
        target_max_ = std::get<1>(edge);
    }
    storage_.push_back( edge );
  }

  auto erase( storage_type::iterator first, storage_type::iterator second) {
    return storage_.erase(first, second);
  }

  auto begin() { return storage_.begin(); }
  auto end() { return storage_.end(); }
  auto size() { return storage_.size(); }

  //std::is_same_v<std::common_type_t<int, double, double>;
  template<std::integral T, std::integral U>
  struct VLargeId {
    //using type = std::conditional_t<(std::numeric_limits<T>::max() > std::numeric_limits<U>::max()), T, U>::type;
    using type = std::conditional_t<(std::numeric_limits<T>::max() > std::numeric_limits<U>::max()), T, U>;
  };

  //VSourceId max_vid() { return std::max(source_max_, target_max_); }
  VLargeId<VSourceId, VTargetId>::type max_vid() { return std::max(source_max_, target_max_); }
  VSourceId max_source() { return source_max_; }
  VTargetId max_target() { return target_max_; }

  void sort_by_source() {
    std::sort(storage_.begin(), storage_.end(),
    [](auto&& e1, auto&& e2){ return std::get<0>(e1) < std::get<0>(e2); });
    sorted_source_ = true;
    sorted_target_ = false;
  }

  void sort_by_target() {
    std::sort(storage_.begin(), storage_.end(),
    [](auto&& e1, auto&& e2){ return std::get<1>(e1) < std::get<1>(e2);});
    sorted_target_ = true;
    sorted_source_ = false;
  }

  void set_bipartite( bool flag ) { bipartite_ = flag; }
  void set_sorted_source( bool flag ) { sorted_source_ = flag; }
  void set_sorted_target( bool flag ) { sorted_target_ = flag; }
  void set_directed( bool flag ) { directed_ = flag; }

  bool is_bipartite() { return bipartite_; }
  bool is_sorted_source() { return sorted_source_; }
  bool is_sorted_target() { return sorted_target_; }
  bool is_directed() { return directed_; }

private:
  friend constexpr storage_type& tag_invoke(::std::graph::edgelist::tag_invoke::edges_fn_t, utility_edgelist& el) {
    return el.storage_;
  }

  friend constexpr VSourceId& tag_invoke(::std::graph::edgelist::tag_invoke::vertex_id_source_fn_t,
                                                                          utility_edgelist&     el,
                                                                          storage_iterator_type& e) {
    return std::get<0>(*e);
  }

  friend constexpr EV& tag_invoke(::std::graph::edgelist::tag_invoke::edge_value_fn_t,
                                                             utility_edgelist&     el,
                                                             storage_iterator_type& e) {
    return std::get<2>(*e);
  }

  storage_type storage_;
  VSourceId    source_max_;
  VTargetId    target_max_;
  bool         bipartite_;
  bool         sorted_source_;
  bool         sorted_target_;
  bool         directed_;
};

} // namespace std::graph::container