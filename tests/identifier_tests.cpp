#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <vector>       // contiguous
#include <deque>        // random access
#include <map>          // bidirectional
#include <list>         // bidirectional
#include <forward_list> // forward

#include "graph/detail/graph_descriptor.hpp"

using namespace graph;
using namespace graph::detail;

using std::move;
using std::conditional_t;
using std::is_same_v;
using std::iterator_traits;
using std::vector;
using std::deque;
using std::map;
using std::list;
using std::forward_list;

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
using std::declval;

// We need an advance function to return the updated iterator
template <forward_iterator I>
I advance(I it, std::iter_difference_t<I> n) {
  std::advance(it, n);
  return it;
}


TEMPLATE_TEST_CASE("Identifier iterator for contiguous container vector<int>",
                   "[descriptor]",
                   (vector<int>),
                   (const vector<int>)) {
  using Container       = TestType;
  using Iterator        = descriptor_iterator<iterator_t<Container>>;
  using difference_type = typename iterator_traits<Iterator>::difference_type;
  Container v           = {1, 2, 3, 4, 5};

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>); // integral index
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, const value_type*>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::reference, const value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>);
    static_assert(is_same_v<typename Iterator::iterator_concept, std::forward_iterator_tag>);
  }

  SECTION("contiguous iterator concept") {
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
    Iterator it0(0);
    Iterator it1(1);
    REQUIRE(*it == 0);
    REQUIRE(*it0 == 0);
    REQUIRE(*it1 == 1);
  }
  SECTION("copy") {
    Iterator it(1);
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("move") {
    Iterator it(1);
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 1);
  }

  SECTION("increment and add") {
    Iterator it(1);
    REQUIRE(*it == 1);
    REQUIRE(*(it++) == 1);
    REQUIRE(*it == 2);
    REQUIRE(*(++it) == 3);
    REQUIRE(*it == 3);
    //REQUIRE(*(it + 2) == 5);
    //REQUIRE(*(2 + it) == 5);
    //REQUIRE(*(it += 2) == 5);
    //REQUIRE(*it == 5);
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(5);
  //  REQUIRE(*it == 5);
  //  REQUIRE(*(it--) == 5);
  //  REQUIRE(*it == 4);
  //  REQUIRE(*(--it) == 3);
  //  REQUIRE(*it == 3);
  //  REQUIRE(*(it - 2) == 1);
  //  REQUIRE(*(it -= 2) == 1);
  //  REQUIRE(*it == 1);
  //}

  SECTION("compare equality") {
    Iterator it(1);
    Iterator it1(1);
    Iterator it2(2);
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(1);
  //  Iterator it1(1);
  //  Iterator it2(2);
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}

TEMPLATE_TEST_CASE("Identifier iterator for random access container deque<int>",
                   "[descriptor]",
                   (deque<int>),
                   (const deque<int>)) {
  using Container       = TestType;
  using Iterator        = descriptor_iterator<iterator_t<Container>>;
  using difference_type = typename iterator_traits<Iterator>::difference_type;
  Container v           = {1, 2, 3, 4, 5};

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, iterator_t<Container>>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, const value_type*>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::reference, const value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>);
    //static_assert(is_same_v<typename Iterator::iterator_concept, typename iterator_t<Container>::iterator_concept>); // not contiguous
  }

  SECTION("random access iterator concept") {
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
    Iterator it0(advance(begin(v), 0));
    Iterator it1(advance(begin(v), 1));
    //REQUIRE(*it == 0);
    REQUIRE(*it0 == advance(begin(v), 0));
    REQUIRE(*it1 == advance(begin(v), 1));
  }
  SECTION("copy") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("move") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("increment and add") {
    Iterator it(advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 1));
    REQUIRE(*(it++) == advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 2));
    REQUIRE(*(++it) == advance(begin(v), 3));
    REQUIRE(*it == advance(begin(v), 3));
    //REQUIRE(*(it + 2) == advance(begin(v), 5));
    //REQUIRE(*(2 + it) == advance(begin(v), 5));
    //REQUIRE(*(it += 2) == advance(begin(v), 5));
    //REQUIRE(*it == advance(begin(v), 5));
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 5));
  //  REQUIRE(*(it--) == advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 4));
  //  REQUIRE(*(--it) == advance(begin(v), 3));
  //  REQUIRE(*it == advance(begin(v), 3));
  //  REQUIRE(*(it - 2) == advance(begin(v), 1));
  //  REQUIRE(*(it -= 2) == advance(begin(v), 1));
  //  REQUIRE(*it == advance(begin(v), 1));
  //}

  SECTION("compare equality") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(advance(begin(v), 1));
    Iterator it2(advance(begin(v), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(v), 1));
  //  Iterator it1(advance(begin(v), 1));
  //  Iterator it2(advance(begin(v), 2));
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}

TEMPLATE_TEST_CASE("Identifier iterator for bidirectional container map<int,int>",
                   "[descriptor]",
                   (map<int, int>),
                   (const map<int, int>)) {
  using Container       = TestType;
  using Iterator        = descriptor_iterator<iterator_t<Container>>;
  using difference_type = typename iterator_traits<Iterator>::difference_type;
  Container v           = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, iterator_t<Container>>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, const value_type*>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::reference, const value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>);
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
    Iterator it0(advance(begin(v), 0));
    Iterator it1(advance(begin(v), 1));
    //REQUIRE(*it == 0);
    REQUIRE(*it0 == advance(begin(v), 0));
    REQUIRE(*it1 == advance(begin(v), 1));
  }
  SECTION("copy") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("move") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("increment and add") {
    Iterator it(advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 1));
    REQUIRE(*(it++) == advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 2));
    REQUIRE(*(++it) == advance(begin(v), 3));
    REQUIRE(*it == advance(begin(v), 3));
    //REQUIRE(*(it + 2) == advance(begin(v), 5));
    //REQUIRE(*(2 + it) == advance(begin(v), 5));
    //REQUIRE(*(it += 2) == advance(begin(v), 5));
    //REQUIRE(*it == advance(begin(v), 5));
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 5));
  //  REQUIRE(*(it--) == advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 4));
  //  REQUIRE(*(--it) == advance(begin(v), 3));
  //  REQUIRE(*it == advance(begin(v), 3));
  //  //REQUIRE(*(it - 2) == advance(begin(v), 1));
  //  //REQUIRE(*(it -= 2) == advance(begin(v), 1));
  //  //REQUIRE(*it == advance(begin(v), 1));
  //}

  SECTION("compare equality") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(advance(begin(v), 1));
    Iterator it2(advance(begin(v), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(v), 1));
  //  Iterator it1(advance(begin(v), 1));
  //  Iterator it2(advance(begin(v), 2));
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}

TEMPLATE_TEST_CASE("Identifier iterator for bidirectional container list<int>",
                   "[descriptor]",
                   (list<int>),
                   (const list<int>)) {
  using Container       = TestType;
  using Iterator        = descriptor_iterator<iterator_t<Container>>;
  using difference_type = typename iterator_traits<Iterator>::difference_type;
  Container v           = {1, 2, 3, 4, 5};

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, iterator_t<Container>>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, const value_type*>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::reference, const value_type&>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>);
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
    Iterator it0(advance(begin(v), 0));
    Iterator it1(advance(begin(v), 1));
    //REQUIRE(*it == 0);
    REQUIRE(*it0 == advance(begin(v), 0));
    REQUIRE(*it1 == advance(begin(v), 1));
  }
  SECTION("copy") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("move") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("increment and add") {
    Iterator it(advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 1));
    REQUIRE(*(it++) == advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 2));
    REQUIRE(*(++it) == advance(begin(v), 3));
    REQUIRE(*it == advance(begin(v), 3));
    //REQUIRE(*(it + 2) == advance(begin(v), 5));
    //REQUIRE(*(2 + it) == advance(begin(v), 5));
    //REQUIRE(*(it += 2) == advance(begin(v), 5));
    //REQUIRE(*it == advance(begin(v), 5));
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 5));
  //  REQUIRE(*(it--) == advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 4));
  //  REQUIRE(*(--it) == advance(begin(v), 3));
  //  REQUIRE(*it == advance(begin(v), 3));
  //  //REQUIRE(*(it - 2) == advance(begin(v), 1));
  //  //REQUIRE(*(it -= 2) == advance(begin(v), 1));
  //  //REQUIRE(*it == advance(begin(v), 1));
  //}

  SECTION("compare equality") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(advance(begin(v), 1));
    Iterator it2(advance(begin(v), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(v), 1));
  //  Iterator it1(advance(begin(v), 1));
  //  Iterator it2(advance(begin(v), 2));
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
  using Container       = TestType;
  using Iterator        = descriptor_iterator<iterator_t<Container>>;
  using difference_type = typename iterator_traits<Iterator>::difference_type;
  Container v           = {5, 4, 3, 2, 1}; // reverse order b/c forward_list adds to the front

  SECTION("iterator traits") {
    using value_type = typename iterator_traits<Iterator>::value_type;

    static_assert(is_same_v<typename iterator_traits<Iterator>::difference_type,
                            typename iterator_traits<iterator_t<Container>>::difference_type>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::value_type, iterator_t<Container>>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::pointer, const value_type*>);
    static_assert(is_same_v<typename iterator_traits<Iterator>::reference, const value_type&>);
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
    Iterator it0(advance(begin(v), 0));
    Iterator it1(advance(begin(v), 1));
    //REQUIRE(*it == 0);
    REQUIRE(*it0 == advance(begin(v), 0));
    REQUIRE(*it1 == advance(begin(v), 1));
  }
  SECTION("copy") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(it);
    Iterator it2;
    it2 = it1;
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("move") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(move(it));
    Iterator it2;
    it2 = move(it1);
    REQUIRE(*it1 == advance(begin(v), 1));
    REQUIRE(*it2 == advance(begin(v), 1));
  }

  SECTION("increment and add") {
    Iterator it(advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 1));
    REQUIRE(*(it++) == advance(begin(v), 1));
    REQUIRE(*it == advance(begin(v), 2));
    REQUIRE(*(++it) == advance(begin(v), 3));
    REQUIRE(*it == advance(begin(v), 3));
    //REQUIRE(*(it + 2) == advance(begin(v), 5));
    //REQUIRE(*(2 + it) == advance(begin(v), 5));
    //REQUIRE(*(it += 2) == advance(begin(v), 5));
    //REQUIRE(*it == advance(begin(v), 5));
  }

  //SECTION("decrement and subtract") {
  //  Iterator it(advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 5));
  //  REQUIRE(*(it--) == advance(begin(v), 5));
  //  REQUIRE(*it == advance(begin(v), 4));
  //  REQUIRE(*(--it) == advance(begin(v), 3));
  //  REQUIRE(*it == advance(begin(v), 3));
  //  REQUIRE(*(it - 2) == advance(begin(v), 1));
  //  REQUIRE(*(it -= 2) == advance(begin(v), 1));
  //  REQUIRE(*it == advance(begin(v), 1));
  //}

  SECTION("compare equality") {
    Iterator it(advance(begin(v), 1));
    Iterator it1(advance(begin(v), 1));
    Iterator it2(advance(begin(v), 2));
    REQUIRE(it == it1);
    REQUIRE(it != it2);
    REQUIRE(it1 == it);
    REQUIRE(it1 != it2);
    REQUIRE(it2 != it);
    REQUIRE(it2 != it1);
  }

  //SECTION("compare relative") {
  //  Iterator it(advance(begin(v), 1));
  //  Iterator it1(advance(begin(v), 1));
  //  Iterator it2(advance(begin(v), 2));
  //  REQUIRE(it == it1);
  //  REQUIRE(it != it2);
  //  REQUIRE(it < it2);
  //  REQUIRE(it <= it2);
  //  REQUIRE(it2 > it);
  //  REQUIRE(it2 >= it);
  //}

  // operator[] is not tested because it will return a reference to a non-existing element for contiguous_iterator
}


TEMPLATE_TEST_CASE("continuous descriptor range vector<int>", "[descriptor]", (vector<int>), (const vector<int>)) {
  using Container             = TestType;
  using Iterator              = descriptor_iterator<iterator_t<Container>>;
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            descriptors = descriptor_view(v);
  difference_type i           = 0;

  SECTION("descriptor std for") {
    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
      difference_type descriptor = *it;
      REQUIRE(descriptor == i);
      //REQUIRE(descriptors[descriptor] == descriptor + 1);
      REQUIRE(descriptors.id(descriptor) == i);
      ++i;
    }
  }

  SECTION("descriptor range for") {
    for (difference_type descriptor : descriptors) {
      REQUIRE(descriptor == i);
      //REQUIRE(descriptors[descriptor] == descriptor + 1);
      ++i;
    }
  }
}

TEMPLATE_TEST_CASE("bidirectional descriptor range list<int>", "[descriptor]", (list<int>), (const list<int>)) {
  using Container             = TestType;
  using Iterator              = descriptor_iterator<iterator_t<Container>>;
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            descriptors = descriptor_view(v);
  difference_type i           = 0;

  SECTION("descriptor std for") {
    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
      auto descriptor = *it;
      //const int& value      = descriptors[descriptor];
      //REQUIRE(descriptors[descriptor] == i + 1);
      ++i;
    }
  }

  SECTION("descriptor range for") {
    for (auto descriptor : descriptors) {
      //REQUIRE(descriptors[descriptor] == i + 1);
      ++i;
    }
  }
}

// REVIEW
TEMPLATE_TEST_CASE("All simple values",
                   "[descriptor]",
                   (vector<int>),
                   (const vector<int>),
                   (deque<int>),
                   (const deque<int>),
                   (list<int>),
                   (const list<int>)) {
  using Container             = TestType;
  using IdentifierView        = descriptor_view<Container>;
  using Iterator              = descriptor_iterator<iterator_t<Container>>;
  using descriptor_type       = typename IdentifierView::descriptor_type; // integral or iterator
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            descriptors = descriptor_view(v);
  difference_type i           = 0;

  SECTION("descriptor std for") {
    // for(auto it = begin(vertices(g)); it != end(vertices(g)); ++it)
    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
      descriptor_type descriptor = *it; // descriptor is integral or iterator
      //REQUIRE(descriptors[descriptor] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.id(descriptor) == i); // e.g. vertex_id(descriptor)
      }
      ++i;
    }
  }

  SECTION("descriptor range for") {
    // for(auto descriptor : vertices(g)) // descriptor is integral or iterator
    for (descriptor_type descriptor : descriptors) {
      //REQUIRE(descriptors[descriptor] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.id(descriptor) == i); // e.g. vertex_id(descriptor) == i
      }
      ++i;
    }
  }
}

TEMPLATE_TEST_CASE("All map-like containers", "[descriptor]", (map<int, int>), (const map<int, int>)) {
  using Container             = TestType;
  using IdentifierView        = descriptor_view<Container>;
  using Iterator              = descriptor_iterator<iterator_t<Container>>;
  using descriptor_type       = typename IdentifierView::descriptor_type; // integral or iterator
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
  auto            descriptors = descriptor_view(v);
  difference_type i           = 0;

  SECTION("descriptor std for") {
    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
      descriptor_type descriptor = *it;
      //REQUIRE(descriptors[descriptor] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.id(descriptor) == i);
      }
      ++i;
    }
  }

  SECTION("descriptor range for") {
    for (descriptor_type descriptor : descriptors) {
      //REQUIRE(descriptors[descriptor] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(descriptors.id(descriptor) == i);
      }
      ++i;
    }
  }
}

//TEMPLATE_TEST_CASE("All simple values", "[descriptor]", (forward_list<int>), (const forward_list<int>)) {
//  using Container             = TestType;
//  using IdentifierView        = descriptor_view<Container>;
//  using Iterator              = descriptor_iterator<iterator_t<Container>>;
//  using descriptor_type       = typename IdentifierView::descriptor_type; // integral or iterator
//  using difference_type       = typename iterator_traits<Iterator>::difference_type;
//  Container       v           = {5, 4, 3, 2, 1};
//  auto            descriptors = descriptor_view(v);
//  difference_type i           = 0;
//
//  SECTION("descriptor std for") {
//    for (auto it = begin(descriptors); it != end(descriptors); ++it) {
//      descriptor_type descriptor = *it;
//      REQUIRE(descriptors[descriptor] == i + 1);
//      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
//        REQUIRE(descriptors.id(descriptor) == i);
//      }
//      ++i;
//    }
//  }
//
//  SECTION("descriptor range for") {
//    for (descriptor_type descriptor : descriptors) {
//      REQUIRE(descriptors[descriptor] == i + 1);
//      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
//        REQUIRE(descriptors.id(descriptor) == i);
//      }
//      ++i;
//    }
//  }
//}


#if 0
TEST_CASE("example") {
  using G  = vector<int>;
  G    g   = {1, 2, 3, 4, 5};
  auto V = descriptor_view(g);

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
  for (auto v = begin(*u); v != end(*u); ++v) {
    // v is a vector<int>::iterator
    // *v is an int
  }
}

for (auto&& u : vertices(a)) {
  // u is a vector<int>&
  for (auto&& v : edges(g,u)) {
    // v is an int&
  }
}

// for(auto&& u : descriptor_view(a))
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
  return descriptor_view(g);
}


#endif // 0
