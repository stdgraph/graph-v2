#include "timer.hpp"
#include <iostream>
#include <fmt/format.h>

using std::cout;
using std::endl;

double simple_timer::elapsed() const {
  return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start_time_)
        .count();
}

timer::timer(const std::string& name, bool include_start) : name_(name), _include_start(include_start) {
  if (_include_start)
    fmt::print("{} started...", name_);
  reset();
}
timer ::~timer() { output_ellapsed(); }

void timer::reset() { start_time_ = std::chrono::steady_clock::now(); }

double timer::elapsed() const {
  return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start_time_)
        .count();
}

void timer::set_count(int64_t count, const std::string_view& desc) {
  count_      = static_cast<double>(count);
  count_name_ = desc;
}

// write the elapsed time to the console, including minutes and seconds, and total seconds
// the seconds should be displayed with 3 decimal places
void timer::output_ellapsed() {
  double seconds = elapsed();
  double hr      = trunc(seconds / 3600.);
  double min     = trunc((seconds - hr * 3600.) / 60.);
  double sec     = seconds - min * 60.;

  if (!_include_start)
    fmt::print("{}", name_);
  fmt::print(" took {}h{}m{:.0f}s ({:.3f})", hr, min, sec, seconds);
  if (count_ > 0)
    fmt::print(", {:.0Lf} {} at {:.0Lf} {}/sec", count_, count_name_, (count_ / seconds), count_name_);
  fmt::print("\n");
}
