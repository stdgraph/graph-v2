#pragma once

namespace std::graph::views {


template <class G, bool Sourced>
class source_vertex {
public:
  using vertex_key_type = vertex_key_t<G>;

  source_vertex(vertex_key_type key) : key_(key) {}

  source_vertex()                         = default;
  source_vertex(const source_vertex& rhs) = default;
  source_vertex(source_vertex&&)          = default;
  ~source_vertex()                        = default;

  source_vertex& operator=(const source_vertex&) = default;
  source_vertex& operator=(source_vertex&&) = default;

  constexpr vertex_key_type source_vertex_key() const noexcept { return key_; }

protected:
  vertex_key_type key_ = 0;
};

template <class G>
class source_vertex<G, false> {
public:
  using vertex_key_type = vertex_key_t<G>;

  source_vertex(vertex_key_type key) {}
  source_vertex() = default;
};

/// <summary>
/// ref_to_ptr changes a reference to a pointer and stores it as a pointer in value.
/// Pointers and values are stored as-is.
///
/// ref_to_ptr is similar to reference_wrapper but there are a couple of
/// differences when used in a view iterator that are important:
/// 1.  It is default constructible, as long as the value being stored is default
///     constructible.
/// 2.  It stores a copy of the value if not a reference or a pointer.
/// </summary>
/// <typeparam name="T">The type to store</typeparam>
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

} // namespace _detail


} // namespace std::graph::views
