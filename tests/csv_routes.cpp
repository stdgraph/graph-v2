#include "csv_routes.hpp"

void utf8_append(std::string& out, const char ch) {
  if ((ch & 0x80) == 0)
    out += ch;
  else {
    const char* digits = "0123456789abcdef";
    out += "\\x";
    int lsb = ch & 0x0f;
    int msb = (ch >> 4) & 0xf;
    out += digits[msb];
    out += digits[lsb];
  }
}

// create a string that can be pasted into the source code
std::string quoted_utf8(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (auto ch : s)
    utf8_append(out, ch);
  return out;
}

std::string quoted_utf8(const std::string_view& s) {
  std::string out;
  out.reserve(s.size());
  for (auto ch : s)
    utf8_append(out, ch);
  return out;
}

std::string quoted_utf8(const char* s) {
  std::string out;
  for (const char* ch = s; ch && *ch; ++ch)
    utf8_append(out, *ch);
  return out;
}
