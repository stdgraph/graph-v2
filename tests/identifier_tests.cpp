#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <vector>       // contiguous
#include <deque>        // random access
#include <map>          // bidirectional
#include <list>         // bidirectional
#include <forward_list> // forward

#include "graph/detail/graph_identifier.hpp"

// Review
// - identifier_iterator
// - identifier_view
//   - identifier_type  - integral index or iterator (contiguous, random_access, bidirectional, forward)
//   - identifier_value - value type of underlying container
//   - id(identifier) -> index of entry, e.g. vertex_id_t
//     - n/a for forward_list, set
//   - operator[identifiler] -> value_type of underlying container
//     - .second for pair<U, V>
//     - value_type for tuple<Ts...>
//   - find(id_type) -> identifier_type
//   - edge cases
//     - operator[] with invalid behavior for integral index (contiguous_range)
//     - forward_list (no size)
// - REVIEW exmaple
// - customizability? e.g. non-integral and non-iterator identifier_type

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
                   "[identifier]",
                   (vector<int>),
                   (const vector<int>)) {
  using Container       = TestType;
  using Iterator        = identifier_iterator<iterator_t<Container>>;
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
                   "[identifier]",
                   (deque<int>),
                   (const deque<int>)) {
  using Container       = TestType;
  using Iterator        = identifier_iterator<iterator_t<Container>>;
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
                   "[identifier]",
                   (map<int, int>),
                   (const map<int, int>)) {
  using Container       = TestType;
  using Iterator        = identifier_iterator<iterator_t<Container>>;
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
                   "[identifier]",
                   (list<int>),
                   (const list<int>)) {
  using Container       = TestType;
  using Iterator        = identifier_iterator<iterator_t<Container>>;
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
                   "[identifier]",
                   (forward_list<int>),
                   (const forward_list<int>)) {
  using Container       = TestType;
  using Iterator        = identifier_iterator<iterator_t<Container>>;
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


TEMPLATE_TEST_CASE("continuous identifier range vector<int>", "[identifier]", (vector<int>), (const vector<int>)) {
  using Container             = TestType;
  using Iterator              = identifier_iterator<iterator_t<Container>>;
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            identifiers = identifier_view(v);
  difference_type i           = 0;

  SECTION("identifier std for") {
    for (auto it = begin(identifiers); it != end(identifiers); ++it) {
      difference_type identifier = *it;
      REQUIRE(identifier == i);
      //REQUIRE(identifiers[identifier] == identifier + 1);
      REQUIRE(identifiers.id(identifier) == i);
      ++i;
    }
  }

  SECTION("identifier range for") {
    for (difference_type identifier : identifiers) {
      REQUIRE(identifier == i);
      //REQUIRE(identifiers[identifier] == identifier + 1);
      ++i;
    }
  }
}

TEMPLATE_TEST_CASE("bidirectional identifier range list<int>", "[identifier]", (list<int>), (const list<int>)) {
  using Container             = TestType;
  using Iterator              = identifier_iterator<iterator_t<Container>>;
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            identifiers = identifier_view(v);
  difference_type i           = 0;

  SECTION("identifier std for") {
    for (auto it = begin(identifiers); it != end(identifiers); ++it) {
      auto identifier = *it;
      //const int& value      = identifiers[identifier];
      //REQUIRE(identifiers[identifier] == i + 1);
      ++i;
    }
  }

  SECTION("identifier range for") {
    for (auto identifier : identifiers) {
      //REQUIRE(identifiers[identifier] == i + 1);
      ++i;
    }
  }
}

// REVIEW
TEMPLATE_TEST_CASE("All simple values",
                   "[identifier]",
                   (vector<int>),
                   (const vector<int>),
                   (deque<int>),
                   (const deque<int>),
                   (list<int>),
                   (const list<int>)) {
  using Container             = TestType;
  using IdentifierView        = identifier_view<Container>;
  using Iterator              = identifier_iterator<iterator_t<Container>>;
  using identifier_type       = typename IdentifierView::identifier_type; // integral or iterator
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {1, 2, 3, 4, 5};
  auto            identifiers = identifier_view(v);
  difference_type i           = 0;

  SECTION("identifier std for") {
    // for(auto it = begin(vertices(g)); it != end(vertices(g)); ++it)
    for (auto it = begin(identifiers); it != end(identifiers); ++it) {
      identifier_type identifier = *it; // identifier is integral or iterator
      //REQUIRE(identifiers[identifier] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(identifiers.id(identifier) == i); // e.g. vertex_id(identifier)
      }
      ++i;
    }
  }

  SECTION("identifier range for") {
    // for(auto identifier : vertices(g)) // identifier is integral or iterator
    for (identifier_type identifier : identifiers) {
      //REQUIRE(identifiers[identifier] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(identifiers.id(identifier) == i); // e.g. vertex_id(identifier) == i
      }
      ++i;
    }
  }
}

TEMPLATE_TEST_CASE("All map-like containers", "[identifier]", (map<int, int>), (const map<int, int>)) {
  using Container             = TestType;
  using IdentifierView        = identifier_view<Container>;
  using Iterator              = identifier_iterator<iterator_t<Container>>;
  using identifier_type       = typename IdentifierView::identifier_type; // integral or iterator
  using difference_type       = typename iterator_traits<Iterator>::difference_type;
  Container       v           = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
  auto            identifiers = identifier_view(v);
  difference_type i           = 0;

  SECTION("identifier std for") {
    for (auto it = begin(identifiers); it != end(identifiers); ++it) {
      identifier_type identifier = *it;
      //REQUIRE(identifiers[identifier] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(identifiers.id(identifier) == i);
      }
      ++i;
    }
  }

  SECTION("identifier range for") {
    for (identifier_type identifier : identifiers) {
      //REQUIRE(identifiers[identifier] == i + 1);
      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
        REQUIRE(identifiers.id(identifier) == i);
      }
      ++i;
    }
  }
}

//TEMPLATE_TEST_CASE("All simple values", "[identifier]", (forward_list<int>), (const forward_list<int>)) {
//  using Container             = TestType;
//  using IdentifierView        = identifier_view<Container>;
//  using Iterator              = identifier_iterator<iterator_t<Container>>;
//  using identifier_type       = typename IdentifierView::identifier_type; // integral or iterator
//  using difference_type       = typename iterator_traits<Iterator>::difference_type;
//  Container       v           = {5, 4, 3, 2, 1};
//  auto            identifiers = identifier_view(v);
//  difference_type i           = 0;
//
//  SECTION("identifier std for") {
//    for (auto it = begin(identifiers); it != end(identifiers); ++it) {
//      identifier_type identifier = *it;
//      REQUIRE(identifiers[identifier] == i + 1);
//      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
//        REQUIRE(identifiers.id(identifier) == i);
//      }
//      ++i;
//    }
//  }
//
//  SECTION("identifier range for") {
//    for (identifier_type identifier : identifiers) {
//      REQUIRE(identifiers[identifier] == i + 1);
//      if constexpr (random_access_range<Container> || is_tuple_like_v<range_value_t<Container>>) {
//        REQUIRE(identifiers.id(identifier) == i);
//      }
//      ++i;
//    }
//  }
//}


#if 0
TEST_CASE("example") {
  using G  = vector<int>;
  G    g   = {1, 2, 3, 4, 5};
  auto V = identifier_view(g);

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

// for(auto&& u : identifier_view(a))
for (auto&& u : vertices(a)) {
  // u is a vertex identifier/descriptor = index/iterator = graph_traits<G>::vertex_info
  for (auto&& uv : edges(a,u)) {
    // uv is an edge identifier/descriptor = index/iterator = graph_traits<G>::edge_info
    auto&& uu = source(g, uv);
    //uu is graph_traits<G>::vertex_info
  }
}

template <typename G>
auto vertices(const G& g) {
  return identifier_view(g);
}


#endif // 0
