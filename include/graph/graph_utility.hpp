#pragma once

namespace std::graph {

// Common types for DFS & BFS views
enum three_colors : int8_t { black, white, grey }; // { finished, undiscovered, discovered }
enum struct cancel_search : int8_t { continue_search, cancel_branch, cancel_all };


template <class G, bool Sourced>
class source_vertex {
public:
  using vertex_id_type = vertex_id_t<G>;

  source_vertex(vertex_id_type id) : id_(id) {}

  source_vertex()                         = default;
  source_vertex(const source_vertex& rhs) = default;
  source_vertex(source_vertex&&)          = default;
  ~source_vertex()                        = default;

  source_vertex& operator=(const source_vertex&) = default;
  source_vertex& operator=(source_vertex&&)      = default;

  constexpr vertex_id_type source_vertex_id() const noexcept { return id_; }

protected:
  vertex_id_type id_ = 0;
};

template <class G>
class source_vertex<G, false> {
public:
  using vertex_id_type = vertex_id_t<G>;

  source_vertex(vertex_id_type id) {}
  source_vertex() = default;
};

/**
 * @brief ref_to_ptr changes a reference to a pointer and stores it as a pointer in value.
 *        Pointers and values are stored as-is.
 *
 * @c ref_to_ptr is similar to @c reference_wrapper but there are a couple of
 * differences when used in a view iterator that are important:
 * 1.  It is default constructible, as long as the value being stored is default
 *     constructible.
 * 2.  It stores a copy of the value if not a reference or a pointer.
 *
 * @tparam T    The type to store
*/
namespace _detail {
  template <class T>
  class ref_to_ptr {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");

    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(const T& rhs) : value(rhs) {}
    constexpr ~ref_to_ptr() = default;

    constexpr ref_to_ptr& operator=(const T& rhs) {
      value = rhs;
      return *this;
    }

    constexpr operator bool() const noexcept { return true; }

    constexpr T*       get() noexcept { return &value; }
    constexpr const T* get() const noexcept { return &value; }

    constexpr operator T&() noexcept { return value; }
    constexpr operator const T&() const noexcept { return value; }

  private:
    T value = {};
  };

  template <class T>
  class ref_to_ptr<T&> {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");
    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(T& rhs) noexcept : value(&rhs) {}
    constexpr ~ref_to_ptr() = default;

    ref_to_ptr& operator=(T& rhs) noexcept {
      value = &rhs;
      return *this;
    }

    constexpr T*       get() noexcept { return value; }
    constexpr const T* get() const noexcept { return value; }

    constexpr operator bool() const noexcept { return value != nullptr; }

    constexpr operator T&() noexcept {
      assert(value);
      return *value;
    }
    constexpr operator const T&() const noexcept {
      assert(value);
      return *value;
    }

  private:
    T* value = nullptr;
  };

  template <class T>
  class ref_to_ptr<T*> {
  public:
    static_assert(is_object_v<T> || is_function_v<T>,
                  "ref_to_ptr<T> requires T to be an object type or a function type.");
    using type = T;

    constexpr ref_to_ptr() = default;
    constexpr ref_to_ptr(T* rhs) noexcept : value(rhs) {}
    constexpr ~ref_to_ptr() = default;

    constexpr ref_to_ptr& operator=(T* rhs) noexcept {
      value = rhs;
      return *this;
    }

    constexpr T*       get() noexcept { return value; }
    constexpr const T* get() const noexcept { return value; }

    constexpr operator bool() const noexcept { return value != nullptr; }

    constexpr operator T&() noexcept {
      assert(value);
      return *value;
    }
    constexpr operator const T&() const noexcept {
      assert(value);
      return *value;
    }

  private:
    T* value = nullptr;
  };


  template <class A>
  concept is_allocator_v = is_copy_constructible_v<A> && requires(A alloc, size_t n) {
    { alloc.allocate(n) };
  };


} // namespace _detail


} // namespace std::graph
