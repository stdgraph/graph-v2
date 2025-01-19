#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <vector>       // contiguous
#include <deque>        // random access
#include <map>          // bidirectional
#include <list>         // bidirectional
#include <forward_list> // forward

#include "graph/detail/descriptor.hpp"
#include "graph/graph_utility.hpp"

#define ENABLE_DESCRIPTOR_TESTS 0

using namespace graph;

using std::move;
using std::conditional_t;
using std::is_same_v;
using std::iterator_traits;
using std::vector;
using std::deque;
using std::map;
using std::list;
using std::forward_list;
using std::iter_reference_t;
using std::is_const_v;
using std::remove_cvref_t;

// concepts
using std::ranges::contiguous_range;
using std::ranges::random_access_range;
using std::ranges::bidirectional_range;
using std::ranges::forward_range;

using std::forward_iterator;
using std::bidirectional_iterator;
using std::random_access_iterator;
using std::contiguous_iterator;

using std::sentinel_for;
using std::sized_sentinel_for;
using std::disable_sized_sentinel_for;

// ranges
//using std::ranges::begin;
//using std::ranges::end;

// type traits
using std::ranges::range_value_t;
using std::ranges::range_size_t;
using std::ranges::iterator_t;
using std::ranges::range_reference_t;
using std::declval;


// We need an advance function to return the updated iterator
template <forward_iterator I>
I advance(I it, std::iter_difference_t<I> n) {
  std::advance(it, n);
  return it;
}

TEST_CASE("Tuple tail") {
  using Tuple  = std::tuple<int, double, float>;
  using Tuple2 = std::tuple<int&, double&, float&>;
  //using Pair   = std::pair<int, double>;
  using Pair2 = std::pair<int&, double&>;

  int    a{1};
  double b{2};
  float  c{3};

  SECTION("Tuple source") {
    SECTION("nth_cdr") {
      Tuple t(a, b, c);
      auto  last2   = graph::nth_cdr<1>(t);
      get<0>(last2) = 4.0;
      REQUIRE(get<1>(t) == 2.0);
      REQUIRE(b == 2.0);
    }
    SECTION("tuple_tail") {
      Tuple t(a, b, c);
      auto  last2   = graph::tuple_tail<1>(t);
      get<0>(last2) = 5.0;
      REQUIRE(get<1>(t) == 5.0);
      REQUIRE(b == 2.0);
    }
  }

  SECTION("Tuple2 source") {
    SECTION("Copy Tuple to Tuple2") {
      Tuple t(a, b, c);
      //Tuple2 t2(t); // fails in gcc-13, succeeds in msvc
      //get<1>(t2) = 6.0;
      //REQUIRE(get<1>(t) == 6.0);
    }
    SECTION("nth_cdr_ref") {
      Tuple2 t{a, b, c};
      auto   last2  = graph::nth_cdr_ref<1>(t);
      get<0>(last2) = 6.0;
      REQUIRE(b == 6.0);
    }
    SECTION("tuple_tail") {
      Tuple2 t(a, b, c);
      auto   last2  = graph::tuple_tail<1>(t);
      get<0>(last2) = 7.0;
      REQUIRE(b == 7.0);
    }
  }

  SECTION("pair source") {
    SECTION("tuple_tail") {
      Pair2 p(a, b);
      auto  last1   = graph::tuple_tail<1>(p);
      get<0>(last1) = 7.0;
      REQUIRE(b == 7.0);
    }
  }
}


template <class _It>
concept indirectly_readable_impl =
      requires(const _It __i) {
        typename std::iter_value_t<_It>;
        typename std::iter_reference_t<_It>;
        typename std::iter_rvalue_reference_t<_It>;
        { *__i } -> same_as<std::iter_reference_t<_It>>;
        //{ _RANGES iter_move(__i) } -> same_as<std::iter_rvalue_reference_t<_It>>;
        true;
      } && std::common_reference_with<std::iter_reference_t<_It>&&, iter_value_t<_It>&> &&
      std::common_reference_with<std::iter_reference_t<_It>&&, std::iter_rvalue_reference_t<_It>&&> &&
      std::common_reference_with<std::iter_rvalue_reference_t<_It>&&, const iter_value_t<_It>&>;

template <class _It>
concept my_indirectly_readable = indirectly_readable_impl<std::remove_cvref_t<_It>>;

using Cont = vector<int>;
struct ContDesc {
  Cont::iterator it;

  int& operator*() const { return *it; }
  int* operator->() const { return &*it; }
};


TEST_CASE("Descriptor for contiguous container vector<int>", "[descriptor]") {
  SECTION("inner_range traits") {
    using Container = const vector<int>;
    Container c     = {1, 2, 3, 4, 5};
    auto&&    v     = descriptor_view(c);

    auto first = v.begin();
    auto last  = v.end();
    using I    = decltype(first);
    using S    = decltype(last);
    static_assert(is_same_v<I, S>);

    static_assert(random_access_iterator<I>);

    constexpr std::ranges::subrange_kind K = std::sized_sentinel_for<I, S> //
                                                   ? std::ranges::subrange_kind::sized
                                                   : std::ranges::subrange_kind::unsized;
    assert(K == std::ranges::subrange_kind::sized);

    // constness of container passed through to members?
    static_assert(is_const_v<remove_reference_t<decltype(*first)>>);  // descriptor
    static_assert(is_const_v<remove_reference_t<decltype(**first)>>); // inner value

    static_assert(is_const_v<remove_reference_t<decltype(*last)>>);   // descriptor
    static_assert(is_const_v<remove_reference_t<decltype(**last)>>);  // inner value
  }

  SECTION("inner_range traits") {
    using Container = vector<int>;
    Container c     = {1, 2, 3, 4, 5};
    auto&&    v     = descriptor_view(c);

    auto first = v.begin();
    auto last  = v.end();
    using I    = decltype(first);
    using S    = decltype(last);
    static_assert(is_same_v<I, S>);

    static_assert(random_access_iterator<I>);

    constexpr std::ranges::subrange_kind K = std::sized_sentinel_for<I, S> //
                                                   ? std::ranges::subrange_kind::sized
                                                   : std::ranges::subrange_kind::unsized;
    assert(K == std::ranges::subrange_kind::sized);

    // constness of container passed through to members?
    static_assert(!is_const_v<remove_reference_t<decltype(*first)>>);  // descriptor
    static_assert(!is_const_v<remove_reference_t<decltype(**first)>>); // inner value

    static_assert(!is_const_v<remove_reference_t<decltype(*last)>>);   // descriptor
    static_assert(!is_const_v<remove_reference_t<decltype(**last)>>);  // inner value
  }

  SECTION("const descriptor traits") {
    using Container = const vector<int>;
    using InnerIter = const_iterator_t<Container>;

    using View                  = descriptor_view_t<Container>;
    using Iterator              = descriptor_iterator<InnerIter>;
    using Descriptor            = descriptor<InnerIter>;
    using descriptor_value_type = typename Descriptor::value_type; // integral index or iterator

    Container c = {1, 2, 3, 4, 5};
    View      v = descriptor_view(c);

    static_assert(random_access_range<View>);

    static_assert(is_same_v<Iterator, decltype(declval<const View>().begin())>);
    static_assert(is_same_v<Iterator, decltype(declval<const View>().end())>);
    static_assert(is_same_v<Iterator, decltype(declval<View>().cbegin())>);
    static_assert(is_same_v<Iterator, decltype(declval<View>().cend())>);
    static_assert(!is_const_v<range_reference_t<Container>>);

    static_assert(forward_iterator<Iterator>);
    static_assert(is_same_v<Descriptor, Iterator::value_type>);
    static_assert(is_same_v<const Descriptor&, iter_reference_t<Iterator>>);
    static_assert(!is_const_v<iter_reference_t<Iterator::inner_iterator>>);

    static_assert(integral<Descriptor::value_type>);
    static_assert(is_const_v<remove_reference_t<decltype(*declval<Descriptor>())>>, "inner value returned is const");
    static_assert(is_same_v<InnerIter, Descriptor::inner_iterator>);

    for (auto&& desc : v) {
      //desc = 0; // fails
      //desc.value() = 0; // fails
      int        i  = *desc;
      const int* ip = &*desc;
      auto       t1 = desc.get_vertex_id();
      auto       t2 = desc.get_target_id();
      int64_t    id = desc; // implicit conversion to vertex id
    }
  }
  SECTION("descriptor traits") {
    using Container = vector<int>;
    using InnerIter = iterator_t<Container>;

    using View                  = descriptor_view_t<Container>;
    using Iterator              = descriptor_iterator<InnerIter>;
    using Descriptor            = descriptor<InnerIter>;
    using descriptor_value_type = typename Descriptor::value_type; // integral index or iterator

    Container c = {1, 2, 3, 4, 5};
    auto&&    v = descriptor_view(c);

    static_assert(random_access_range<View>);
    static_assert(!is_const_v<View>);

    static_assert(is_same_v<Iterator, decltype(declval<View>().begin())>);
    static_assert(is_same_v<Iterator, decltype(declval<View>().end())>);
    static_assert(!is_const_v<range_reference_t<Container>>);

    static_assert(forward_iterator<Iterator>);
    static_assert(is_same_v<Descriptor, Iterator::value_type>);
    static_assert(is_same_v<Descriptor&, iter_reference_t<Iterator>>);
    static_assert(!is_const_v<iter_reference_t<Iterator::inner_iterator>>);

    static_assert(integral<Descriptor::value_type>);
    static_assert(!is_const_v<remove_reference_t<decltype(*declval<Descriptor>())>>,
                  "inner value returned is non-const");
    static_assert(is_same_v<InnerIter, Descriptor::inner_iterator>);

    for (auto&& desc : v) {
      //desc = 0; // fails
      //desc.value() = 0; // fails
      int     i  = *desc;
      int*    ip = &*desc;
      auto    t1 = desc.get_vertex_id();
      auto    t2 = desc.get_target_id();
      int64_t id = desc; // implicit conversion to vertex id
    }
  }
}

TEMPLATE_TEST_CASE("Descriptor iterator for contiguous container vector<int>",
                   "[descriptor]",
                   (vector<int>),
                   (const vector<int>)) {
  using Container  = TestType;
  using View       = descriptor_view_t<Container>;
  using Iterator   = descriptor_iterator<iterator_t<Container>>;
  using Descriptor = descriptor<iterator_t<Container>>;
  Container c      = {1, 2, 3, 4, 5};
  auto&&    v      = descriptor_view(c);

  SECTION("contiguous iterator concept") {
    //static_assert(sentinel_for<Iterator, Iterator>);
    //static_assert(is_same_v<difference_type, decltype(declval<Iterator>() - declval<Iterator>())>);
    //static_assert(sized_sentinel_for<Iterator, Iterator>);
    static_assert(forward_iterator<Iterator>);
    static_assert(bidirectional_iterator<Iterator>);
    static_assert(random_access_iterator<Iterator>);
    //static_assert(contiguous_iterator<Iterator>);
  }

  SECTION("construction") {
    Iterator it;
    Iterator it0(c, 0);
    Iterator it1(c, 1);
    REQUIRE(*it == 0);
    REQUIRE(*it0 == 0);
    REQUIRE(*it1 == 1);
  }
  SECTION("copy") {
    Iterator it(c, 1);
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("move") {
    Iterator it(c, 1);
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("increment and add") {
    Iterator it(c, 1);
    REQUIRE(*it == 1);
    REQUIRE(*(it++) == 1);
    REQUIRE(*it == 2);
    REQUIRE(*(++it) == 3);
    REQUIRE(*it == 3);
    REQUIRE(*(it + 2) == 5);
    REQUIRE(*(2 + it) == 5);
    REQUIRE(*(it += 2) == 5);
    REQUIRE(*it == 5);
  }

  SECTION("decrement and subtract") {
    Iterator it(c, 5);
    REQUIRE(*it == 5);
    REQUIRE(*(it--) == 5);
    REQUIRE(*it == 4);
    REQUIRE(*(--it) == 3);
    REQUIRE(*it == 3);
    REQUIRE(*(it - 2) == 1);
    REQUIRE(*(it -= 2) == 1);
    REQUIRE(*it == 1);
  }

  SECTION("compare equality") {
    Iterator it(c, 1);
    Iterator it1(c, 1);
    Iterator it2(c, 2);
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  SECTION("compare relative") {
    Iterator it(c, 1);
    Iterator it1(c, 1);
    Iterator it2(c, 2);
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it < it2);
    REQUIRE(it <= it2);
    REQUIRE(it2 > it);
    REQUIRE(it2 >= it);
  }

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
  // huh?
}

TEMPLATE_TEST_CASE("Identifier iterator for random access container deque<int>",
                   "[descriptor]",
                   (deque<int>),
                   (const deque<int>)) {
  using Container = TestType;
  using View      = descriptor_view_t<Container>;
  Container c     = {1, 2, 3, 4, 5};
  auto&&    v     = descriptor_view(c);

  using Iterator   = decltype(v.begin()); // preserve constness of container
  using Descriptor = Iterator::value_type;

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, Descriptor>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, value_type*>);
    //static_assert(is_same_v<typename iterator_traits<Iterator>::reference, value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::random_access_iterator_tag>);
    //static_assert(is_same_v<typename Iterator::iterator_concept, typename iterator_t<Container>::iterator_concept>); // not contiguous
  }

  SECTION("random access iterator concept") {
    //static_assert(sentinel_for<Iterator, Iterator>);
    //static_assert(is_same_v<difference_type, decltype(declval<Iterator>() - declval<Iterator>())>);
    static_assert(sized_sentinel_for<Iterator, Iterator>);
    static_assert(forward_iterator<Iterator>);
    static_assert(bidirectional_iterator<Iterator>);
    static_assert(random_access_iterator<Iterator>);
    //static_assert(contiguous_iterator<Iterator>);
  }

  SECTION("construction") {
    Iterator it;
    Iterator it0(c, 0);
    Iterator it1(c, 1);
    REQUIRE(*it == 0);
    REQUIRE(*it0 == 0);
    REQUIRE(*it1 == 1);
  }

  SECTION("copy") {
    Iterator it(c, 1);
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("move") {
    Iterator it(c, 1);
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("increment and add") {
    Iterator it(c, 1);
    REQUIRE(*it == 1);
    REQUIRE(*(it++) == 1);
    REQUIRE(*it == 2);
    REQUIRE(*(++it) == 3);
    REQUIRE(*it == 3);
    REQUIRE(*(it + 2) == 5);
    REQUIRE(*(2 + it) == 5);
    REQUIRE(*(it += 2) == 5);
    REQUIRE(*it == 5);
  }

  SECTION("decrement and subtract") {
    Iterator it(c, 5);
    REQUIRE(*it == 5);
    REQUIRE(*(it--) == 5);
    REQUIRE(*it == 4);
    REQUIRE(*(--it) == 3);
    REQUIRE(*it == 3);
    REQUIRE(*(it - 2) == 1);
    REQUIRE(*(it -= 2) == 1);
    REQUIRE(*it == 1);
  }

  SECTION("compare equality") {
    Iterator it(c, 1);
    Iterator it1(c, 1);
    Iterator it2(c, 2);
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  SECTION("compare relative") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(advance(begin(v), 1));
    Iterator it2(advance(begin(v), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it < it2);
    REQUIRE(it <= it2);
    REQUIRE(it2 > it);
    REQUIRE(it2 >= it);
  }

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
  // huh?
}

TEMPLATE_TEST_CASE("Identifier iterator for bidirectional container map<int,int>",
                   "[descriptor]",
                   (map<int, int>),
                   (const map<int, int>)) {
  using Container = TestType;
  using View      = descriptor_view_t<Container>;
  Container c     = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
  auto&&    v     = descriptor_view(c);

  using Iterator   = decltype(v.begin()); // preserve constness of container
  using Descriptor = Iterator::value_type;

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, Descriptor>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, value_type*>);
    //static_assert(is_same_v<typename iterator_traits<Iterator>::reference, value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::bidirectional_iterator_tag>);
    //static_assert(is_same_v<typename Iterator::iterator_concept, typename iterator_t<Container>::iterator_concept>); // not contiguous
  }

  SECTION("forward iterator concept") {
    //static_assert(sentinel_for<Iterator, Iterator>);
    //static_assert(is_same_v<difference_type, decltype(declval<Iterator>() - declval<Iterator>())>);
    //static_assert(sized_sentinel_for<Iterator, Iterator>);
    static_assert(forward_iterator<Iterator>);
    static_assert(bidirectional_iterator<Iterator>);
    //static_assert(random_access_iterator<Iterator>);
    //static_assert(contiguous_iterator<Iterator>);
  }

  SECTION("construction") {
    Iterator it;
    Iterator it0(c, advance(begin(c), 0));
    Iterator it1(c, advance(begin(c), 1));
    //REQUIRE(*it == 0);
    REQUIRE((*it0).get_vertex_id() == 1);
    REQUIRE((*it1).get_vertex_id() == 2);
  }
  SECTION("copy") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE((*it1).get_vertex_id() == 2);
    REQUIRE((*it2).get_vertex_id() == 2);
  }

  SECTION("move") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE((*it1).get_vertex_id() == 2);
    REQUIRE((*it2).get_vertex_id() == 2);
  }

  SECTION("increment and add") {
    Iterator it(c, advance(begin(c), 1));
    REQUIRE((*it).get_vertex_id() == 2);
    REQUIRE((*it++).get_vertex_id() == 2);
    REQUIRE((*it).get_vertex_id() == 3);
    REQUIRE((*++it).get_vertex_id() == 4);
    REQUIRE((*it).get_vertex_id() == 4);
  }

  SECTION("decrement and subtract") {
    Iterator it(c, advance(begin(c), 5));
    REQUIRE(*it == advance(begin(c), 5));
    REQUIRE(*(it--) == advance(begin(c), 5));
    REQUIRE(*it == advance(begin(c), 4));
    REQUIRE(*(--it) == advance(begin(c), 3));
    REQUIRE(*it == advance(begin(c), 3));
  }

  SECTION("compare equality") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(c, advance(begin(c), 1));
    Iterator it2(c, advance(begin(c), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
  // huh?
}

TEMPLATE_TEST_CASE("Identifier iterator for bidirectional container list<int>",
                   "[descriptor]",
                   (list<int>),
                   (const list<int>)) {
  using Container = TestType;
  using View      = descriptor_view_t<Container>;
  Container c     = {1, 2, 3, 4, 5};
  auto&&    v     = descriptor_view(c);

  using Iterator   = decltype(begin(v)); // preserve constness of container
  using Descriptor = Iterator::value_type;

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, Descriptor>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, value_type*>);
    //static_assert(is_same_v<typename iterator_traits<Iterator>::reference, value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::bidirectional_iterator_tag>);
    //static_assert(is_same_v<typename Iterator::iterator_concept, typename iterator_t<Container>::iterator_concept>); // not contiguous
  }

  SECTION("forward iterator concept") {
    //static_assert(sentinel_for<Iterator, Iterator>);
    //static_assert(is_same_v<difference_type, decltype(declval<Iterator>() - declval<Iterator>())>);
    //static_assert(sized_sentinel_for<Iterator, Iterator>);
    static_assert(forward_iterator<Iterator>);
    //static_assert(bidirectional_iterator<Iterator>);
    //static_assert(random_access_iterator<Iterator>);
    //static_assert(contiguous_iterator<Iterator>);
  }

  SECTION("construction") {
    Iterator it;
    Iterator it0(c, advance(begin(c), 0));
    Iterator it1(c, advance(begin(c), 1));
    //REQUIRE(*it == 0);
    REQUIRE((*it0).get_target_id() == 1);
    REQUIRE((*it1).get_target_id() == 2);
  }
  SECTION("copy") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE((*it1).get_target_id() == 2);
    REQUIRE((*it2).get_target_id() == 2);
  }

  SECTION("move") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE((*it1).get_target_id() == 2);
    REQUIRE((*it2).get_target_id() == 2);
  }

  SECTION("increment and add") {
    Iterator it(c, advance(begin(c), 1));
    REQUIRE((*it).get_target_id() == 2);
    REQUIRE((*it++).get_target_id() == 2);
    REQUIRE((*it).get_target_id() == 3);
    REQUIRE((*++it).get_target_id() == 4);
    REQUIRE((*it).get_target_id() == 4);
  }

  SECTION("decrement and subtract") {
    Iterator it(c, advance(begin(c), 5));
    REQUIRE(*it == advance(begin(c), 5));
    REQUIRE(*(it--) == advance(begin(c), 5));
    REQUIRE(*it == advance(begin(c), 4));
    REQUIRE(*(--it) == advance(begin(c), 3));
    REQUIRE(*it == advance(begin(c), 3));
  }

  SECTION("compare equality") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(c, advance(begin(c), 1));
    Iterator it2(c, advance(begin(c), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(c), 1));
  //  Iterator it1(advance(begin(c), 1));
  //  Iterator it2(advance(begin(c), 2));
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}

TEMPLATE_TEST_CASE("Identifier iterator for bidirectional container",
                   "[descriptor]",
                   (forward_list<int>),
                   (const forward_list<int>)) {
  using Container = TestType;
  using View      = descriptor_view_t<Container>;
  Container c     = {5, 4, 3, 2, 1}; // reverse order b/c forward_list adds to the front
  auto&&    v     = descriptor_view(c);

  using Iterator   = decltype(begin(v)); // preserve constness of container
  using Descriptor = Iterator::value_type;

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, Descriptor>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, value_type*>);
    //static_assert(is_same_v<typename iterator_traits<Iterator>::reference, value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category,
                            typename iterator_traits<iterator_t<Container>>::iterator_category>);
    //static_assert(is_same_v<typename Iterator::iterator_concept, typename iterator_t<Container>::iterator_concept>); // not contiguous
  }

  SECTION("forward iterator concept") {
    //static_assert(sentinel_for<Iterator, Iterator>);
    //static_assert(is_same_v<difference_type, decltype(declval<Iterator>() - declval<Iterator>())>);
    //static_assert(sized_sentinel_for<Iterator, Iterator>);
    static_assert(forward_iterator<Iterator>);
    //static_assert(bidirectional_iterator<Iterator>);
    //static_assert(random_access_iterator<Iterator>);
    //static_assert(contiguous_iterator<Iterator>);
  }

  SECTION("construction") {
    Iterator it;
    Iterator it0(c, advance(begin(c), 0));
    Iterator it1(c, advance(begin(c), 1));
    //REQUIRE(*it == 0);
    REQUIRE((*it0).get_target_id() == 5);
    REQUIRE((*it1).get_target_id() == 4);
  }
  SECTION("copy") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE((*it1).get_target_id() == 4);
    REQUIRE((*it2).get_target_id() == 4);
  }

  SECTION("move") {
    Iterator it(c, advance(begin(c), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE((*it1).get_target_id() == 4);
    REQUIRE((*it2).get_target_id() == 4);
  }

  SECTION("increment and add") {
    Iterator it(c, advance(begin(c), 1));
    REQUIRE((*it).get_target_id() == 4);
    REQUIRE((*it++).get_target_id() == 4);
    REQUIRE((*it).get_target_id() == 3);
    REQUIRE((*++it).get_target_id() == 2);
    REQUIRE((*it).get_target_id() == 2);
    //REQUIRE(*(it + 2) == advance(begin(c), 5));
    //REQUIRE(*(2 + it) == advance(begin(c), 5));
    //REQUIRE(*(it += 2) == advance(begin(c), 5));
    //REQUIRE(*it == advance(begin(c), 5));
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(c, advance(begin(c), 5));
  //  REQUIRE(*it == advance(begin(c), 5));
  //  REQUIRE(*(it--) == advance(begin(c), 5));
  //  REQUIRE(*it == advance(begin(c), 4));
  //  REQUIRE(*(--it) == advance(begin(c), 3));
  //  REQUIRE(*it == advance(begin(c), 3));
  //  REQUIRE(*(it - 2) == advance(begin(c), 1));
  //  REQUIRE(*(it -= 2) == advance(begin(c), 1));
  //  REQUIRE(*it == advance(begin(c), 1));
  //}

  SECTION("compare equality") {
    Iterator it(c, 1);
    Iterator it1(c, 1);
    Iterator it2(c, 2);
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(c), 1));
  //  Iterator it1(advance(begin(c), 1));
  //  Iterator it2(advance(begin(c), 2));
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}


#if ENABLE_DESCRIPTOR_TESTS
TEMPLATE_TEST_CASE("continuous descriptor range vector<int>", "[descriptor]", (vector<int>), (const vector<int>)) {
  using Container = TestType;
  Container c     = {1, 2, 3, 4, 5};
  int       i     = 0;

  SECTION("descriptor_view") {
    auto descriptors = descriptor_view(c);

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        auto& desc = *it;
        REQUIRE(desc == i);
        //REQUIRE(descriptors[i] == i + 1);
        REQUIRE(desc.get_vertex_id()  == i);
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto& desc : descriptors) {
        REQUIRE(desc.get_vertex_id() == i);
        //REQUIRE(descriptors[i] == i + 1);
        ++i;
      }
    }
  }

  SECTION("descriptor_subrange_view") {
    auto descriptors = descriptor_subrange_view(c);

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        auto& desc = *it;
        REQUIRE(desc.get_vertex_id() == i);
        //REQUIRE(descriptors[i] == i + 1);
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto&& desc : descriptors) {
        REQUIRE(desc.get_vertex_id() == i);
        //REQUIRE(descriptors[i] == i + 1);
        ++i;
      }
    }
  }
}

TEMPLATE_TEST_CASE("bidirectional descriptor range list<int>", "[descriptor]", (list<int>), (const list<int>)) {
  using Container = TestType;
  Container c     = {1, 2, 3, 4, 5};

  SECTION("descriptor_view") {
    using View            = descriptor_view<Container>;
    using Iterator        = iterator_t<View>;
    using Descriptor      = Iterator::value_type;
    using difference_type = iter_difference_t<Iterator>;
    View            v(c);
    difference_type i = 0;

    auto descriptors = descriptor_view(c);

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        auto descriptor = *it;
        //const int& value      = descriptors[descriptor];
        REQUIRE(descriptors.get_target_id(descriptor) == i + 1);
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto descriptor : descriptors) {
        REQUIRE(descriptors.get_target_id(descriptor) == i + 1);
        ++i;
      }
    }
  }

  SECTION("descriptor_subrange_view") {
    using View            = descriptor_subrange_view<Container>;
    using Iterator        = descriptor_iterator<iterator_t<Container>>;
    using Descriptor      = descriptor<iterator_t<Container>>;
    using difference_type = range_difference_t<Container>;
    difference_type i     = 0;

    descriptor_subrange_view<Container, int64_t> descriptors(c);

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        auto descriptor = *it;
        //const int& value      = descriptors[descriptor];
        REQUIRE(descriptors.get_target_id(descriptor) == i + 1);
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto descriptor : descriptors) {
        REQUIRE(descriptors.get_target_id(descriptor) == i + 1);
        ++i;
      }
    }
  }
}

TEMPLATE_TEST_CASE("All simple values",
                   "[descriptor]",
                   (vector<int>),
                   (const vector<int>),
                   (deque<int>),
                   (const deque<int>),
                   (list<int>),
                   (const list<int>)) {
  using Container       = TestType;
  using difference_type = range_difference_t<Container>;
  Container       c     = {1, 2, 3, 4, 5};
  difference_type i     = 0;

  SECTION("descriptor_view") {
    auto descriptors = descriptor_view(c);
    using desc_view  = decltype(descriptors);
    using desc_type  = range_value_t<desc_view>;
    static_assert(forward_range<desc_view>, "descriptor_view is a forward_range");

    SECTION("descriptor std for") {
      // for(auto it = begin(vertices(g)); it != end(vertices(g)); ++it)
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        const desc_type& descriptor = *it; // descriptor is integral or iterator
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i); // e.g. vertex_id(descriptor)
        }
        ++i;
      }
    }

    SECTION("descriptor range for") {
      // for(auto descriptor : vertices(g)) // descriptor is integral or iterator
      for (auto&& descriptor : descriptors) {
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i); // e.g. vertex_id(descriptor) == i
        }
        ++i;
      }
    }
  }

  SECTION("descriptor_subrange_view") {
    auto descriptors = descriptor_subrange_view(c);
    using desc_view  = decltype(descriptors);
    using desc_type  = range_value_t<desc_view>;
    static_assert(forward_range<desc_view>, "descriptor_subrange_view is a forward_range");

    SECTION("descriptor std for") {
      // for(auto it = begin(vertices(g)); it != end(vertices(g)); ++it)
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        const desc_type& descriptor = *it; // descriptor is integral or iterator
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i); // e.g. vertex_id(descriptor)
        }
        ++i;
      }
    }

    SECTION("descriptor range for") {
      // for(auto descriptor : vertices(g)) // descriptor is integral or iterator
      for (auto&& descriptor : descriptors) {
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i); // e.g. vertex_id(descriptor) == i
        }
        ++i;
      }
    }
  }
}

TEMPLATE_TEST_CASE("All map-like containers", "[descriptor]", (map<int, int>), (const map<int, int>)) {
  using Container       = TestType;
  using difference_type = range_difference_t<Container>;
  Container       c     = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
  difference_type i     = 0;


  SECTION("descriptor_view") {
    auto descriptors = descriptor_view(c);
    using View       = decltype(descriptors);
    using Iterator   = iterator_t<View>;
    static_assert(std::default_initializable<Iterator>);

    using desc_view = decltype(descriptors);
    using desc_type = range_value_t<desc_view>;
    static_assert(forward_range<desc_view>, "descriptor_view is a forward_range");

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        desc_type descriptor = *it;
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i);
        }
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto&& descriptor : descriptors) {
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i);
        }
        ++i;
      }
    }
  }
  SECTION("descriptor_subrange_view") {
    auto descriptors = descriptor_subrange_view(c);
    using desc_view  = decltype(descriptors);
    using desc_type  = range_value_t<desc_view>;
    static_assert(forward_range<desc_view>, "descriptor_subrange_view is a forward_range");

    SECTION("descriptor std for") {
      for (auto it = begin(descriptors); it != end(descriptors); ++it) {
        desc_type descriptor = *it;
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i);
        }
        ++i;
      }
    }

    SECTION("descriptor range for") {
      for (auto&& descriptor : descriptors) {
        //REQUIRE(descriptors[descriptor] == i + 1);
        if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
          REQUIRE(descriptors.get_vertex_id(descriptor) == i);
        }
        ++i;
      }
    }
  }
}

TEMPLATE_TEST_CASE("All simple values",
                   "[descriptor]",
                   (vector<vector<size_t>>),
                   (const vector<vector<int>>),
                   (deque<deque<int>>),
                   (const deque<deque<int>>),
                   (vector<list<int>>),
                   (const vector<list<int>>)) {
  using G = TestType;
  //using Iterator        = descriptor_iterator<Container>;
  //using difference_type = typename iterator_traits<Iterator>::difference_type;
  G g = {{1, 2}, {3, 4}, {5}};
  //difference_type i     = 0;
  static_assert(graph::basic_targeted_edge<G>); //<<<<<
  static_assert(graph::targeted_edge<G>);
  static_assert(graph::adjacency_list<G>);

  SECTION("descriptor_view") {
    auto                      ee    = edges(g, 0);
    vertex_edge_iterator_t<G> first = std::ranges::begin(ee);
    vertex_edge_iterator_t<G> last  = std::ranges::end(ee);

    edge_reference_t<G> first_ref = *first;

    auto  vid = target_id(g, first_ref);
    auto& c   = *find_vertex(g, vid);

    //auto descriptors = descriptor_view(c);
    //using desc_view  = decltype(descriptors);
    //using desc_type  = range_value_t<desc_view>;
    //static_assert(forward_range<desc_view>, "descriptor_view is a forward_range");

    SECTION("descriptor std for") {}

    SECTION("descriptor range for") {}
  }

  //SECTION("descriptor_subrange_view") {
  //  auto descriptors = descriptor_subrange_view(c);
  //  using desc_view  = decltype(descriptors);
  //  using desc_type  = range_value_t<desc_view>;
  //  static_assert(forward_range<desc_view>, "descriptor_subrange_view is a forward_range");

  //  SECTION("descriptor std for") {
  //  }

  //  SECTION("descriptor range for") {
  //  }
  //}
}

TEMPLATE_TEST_CASE("All simple values", "[descriptor]", (forward_list<int>), (const forward_list<int>)) {
  using Container             = TestType;
  using view_type             = descriptor_subrange_view<Container, int64_t>;
  using Iterator              = descriptor_iterator<view_type, iterator_t<Container>, int64_t>;
  using Descriptor            = typename view_type::value_type; // integral or iterator
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       c           = {5, 4, 3, 2, 1};
  auto            descriptors = descriptor_subrange_view(c);
  difference_type i           = 5;

  SECTION("descriptor std for") {
    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
      const Descriptor& descriptor = *it;
      REQUIRE(descriptors.get_target_id(descriptor) == i);
      if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.id(descriptor) == i);
      }
      --i;
    }
  }

  SECTION("descriptor range for") {
    for (auto&& descriptor : descriptors) {
      REQUIRE(descriptors.get_target_id(descriptor) == i);
      if constexpr (random_access_range<Container> || _is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.get_vertex_id(descriptor) == i);
      }
      --i;
    }
  }
}


#  if 0
TEST_CASE("example") {
  using G  = vector<int>;
  G    g   = {1, 2, 3, 4, 5};
  auto V = descriptor_subrange_view(g);

  for (auto&& uid : V) {
    auto id = vertex_id(g, uid);
    for (auto&& uv : edges(g,uid)) {
    }
  }
}
auto a = std::vector<std::vector<int>>;
auto b = std::map<int,std::vector<int>>;

for (auto u = begin(a); u != end(a); ++u) {
  // u is an vector<vector<int>>::iterator
  // *u is a vector<int>
  for (auto c = begin(*u); c != end(*u); ++c) {
    // c is a vector<int>::iterator
    // *c is an int
  }
}

for (auto&& u : vertices(a)) {
  // u is a vector<int>&
  for (auto&& c : edges(g,u)) {
    // c is an int&
  }
}

// for(auto&& u : descriptor_subrange_view(a))
for (auto&& u : vertices(a)) {
  // u is a vertex descriptor/descriptor = index/iterator = graph_traits<G>::vertex_info
  for (auto&& uv : edges(a,u)) {
    // uv is an edge descriptor/descriptor = index/iterator = graph_traits<G>::edge_info
    auto&& uu = source(g, uv);
    //uu is graph_traits<G>::vertex_info
  }
}

template <typename G>
auto vertices(const G& g) {
  return descriptor_subrange_view(g);
}


#  endif // 0

#endif   // ENABLE_DESCRIPTOR_TESTS
