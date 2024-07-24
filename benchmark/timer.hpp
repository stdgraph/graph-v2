#pragma once

#include <chrono>
#include <string>

class timer {
  std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
  std::string                           name_;
  double                                count_ = 0;
  std::string                           count_name_; // e.g., "rows"
  bool                                  _include_start = false;

public:
  explicit timer(const std::string& name, bool include_start = false);
  ~timer();

  double elapsed() const; // seconds

  void reset();
  void set_count(int64_t count, const std::string_view& desc);
  void output_ellapsed();
};
