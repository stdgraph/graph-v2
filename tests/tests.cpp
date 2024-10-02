#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <deque>
#include <map>

using std::conditional_t;
using std::ranges::contiguous_range;
using std::ranges::random_access_range;
using std::ranges::bidirectional_range;
using std::ranges::forward_range;
using std::ranges::range_value_t;
using std::ranges::range_size_t;
using std::ranges::iterator_t;


unsigned int Factorial(unsigned int number) // NOLINT(misc-no-recursion)
{
  return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}


template <typename T>
struct identifier_value {
  using type = T;
};
template <typename T, typename U>
struct identifier_value<std::pair<T, U>> {
  using type = U;
};
template <typename T>
using identifier_value_t = typename identifier_value<T>::type;


template <forward_range C>
class IdentifierContainer {
public:
  using size_type       = range_size_t<C>;
  using value_type      = identifier_value_t<range_value_t<C>>;
  using id_type         = size_type; // e.g. vertex_id_t
  using identifier_type = conditional_t<contiguous_range<C>, range_size_t<C>, iterator_t<C>>;

  class iterator {
  public:
    explicit iterator(identifier_type i) : i_(i) {}

    //
    // dereference
    //
    //const size_type& operator*() const { return i_; }
    identifier_type operator*() const { return i_; }

    identifier_type operator->() const { return i_; }

    //
    // +
    //
    iterator& operator++() {
      ++i_;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++i_;
      return tmp;
    }
    iterator& operator+=(int rhs)
    requires random_access_range<C>
    {
      i_ += rhs;
      return *this;
    }
    iterator operator+(int rhs)
    requires random_access_range<C>
    {
      iterator tmp = *this;
      tmp += rhs;
      return tmp;
    }

    //
    // -
    //
    iterator& operator--()
    requires bidirectional_range<C>
    {
      --i_;
      return *this;
    }
    iterator operator--(int)
    requires bidirectional_range<C>
    {
      iterator tmp = *this;
      --i_;
      return tmp;
    }
    iterator& operator-=(int rhs)
    requires random_access_range<C>
    {
      i_ -= rhs;
      return *this;
    }
    iterator operator-(int rhs)
    requires random_access_range<C>
    {
      iterator tmp = *this;
      tmp -= rhs;
      return tmp;
    }

    //
    // ==, <=>
    //
    auto operator==(const iterator& rhs) const { return i_ == rhs.i_; }

    auto operator<=>(const iterator& rhs) const
    requires random_access_range<C>
    {
      return i_ <=> rhs.i_;
    }

  private:
    identifier_type i_ = identifier_type(); // integral or iterator value, depending on container type
  };

  explicit IdentifierContainer(C& c) : c_(c) {}

  size_type size() const { return c_.size(); }

  value_type& operator[](identifier_type i) {
    if constexpr (contiguous_range<C>) {
      return c_[i];
    } else if constexpr (random_access_range<C>) {
      return *i;
    } else {
      return i->second; // map
    }
  }
  const value_type& operator[](identifier_type i) const
  requires contiguous_range<C>
  {
    if constexpr (contiguous_range<C>) {
      return c_[i];
    } else if constexpr (random_access_range<C>) {
      return *i;
    } else {
      return i->second; // map
    }
  }

  iterator begin() const noexcept {
    if constexpr (contiguous_range<C>) {
      return iterator(0);
    } else {
      return iterator(c_.begin());
    }
  }
  iterator end() const {
    if constexpr (contiguous_range<C>) {
      return iterator(c_.size());
    } else {
      return iterator(c_.end());
    }
  };

  id_type id(identifier_type ident) const {
    if constexpr (contiguous_range<C>) {
      return ident;
    } else if constexpr (random_access_range<C>) {
      return std::distance(c_.begin(), ident);
    } else {
      return ident->first; // map
    }
  }

  iterator find(id_type id) const {
    if constexpr (contiguous_range<C>) {
      return iterator(id);
    } else if constexpr (random_access_range<C>) {
      return iterator(c_.begin() + id);
    } else {
      return iterator(c_.find(id));
    }
  }

private:
  C& c_;
};


TEST_CASE("IdentifierContainer comparison") {
  SECTION("Index range") {
    using DataVector = std::vector<size_t>;
    DataVector v     = {1, 2, 3, 4, 5};
    size_t     i     = 0;

    IdentifierContainer ic(v);

    SECTION("std for") {
      for (auto it = ic.begin(); it != ic.end(); ++it) {
        auto identifier = *it;
        static_assert(std::integral<decltype(identifier)>);
        REQUIRE(ic[identifier] == identifier + 1);
        ++i;
      }
    }

    SECTION("range for") {
      for (size_t identifier : ic) {
        REQUIRE(ic[identifier] == i + 1);
        ++i;
      }
    }
  }

  SECTION("random-access iterator range") {
    using DataDeque = std::deque<size_t>;
    DataDeque v     = {1, 2, 3, 4, 5};
    size_t    i     = 0;

    IdentifierContainer<DataDeque> nic(v);

    SECTION("std for") {
      for (auto it = nic.begin(); it != nic.end(); ++it) {
        auto identifier = *it;
        static_assert(std::random_access_iterator<decltype(identifier)>);
        REQUIRE(nic[identifier] == i + 1);
        ++i;
      }
    }

    SECTION("range for") {
      for (auto identifier : nic) {
        REQUIRE(nic[identifier] == i + 1);
        ++i;
      }
    }
  }

  SECTION("bidirectional iterator range") {
    using DataMap = std::map<size_t, size_t>;
    DataMap m     = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    size_t  i     = 0;

    IdentifierContainer<DataMap> nic(m);

    SECTION("std for") {
      for (auto it = nic.begin(); it != nic.end(); ++it) {
        auto identifier = *it;
        static_assert(std::bidirectional_iterator<decltype(identifier)>);
        REQUIRE(nic[identifier] == i + 1);
        ++i;
      }
    }

    SECTION("range for") {
      for (auto identifier : nic) {
        REQUIRE(nic[identifier] == i + 1);
        ++i;
      }
    }
  }
}
