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


} // namespace std::graph::views
