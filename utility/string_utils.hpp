/*

Copyright (c) 2024 lemon19900815@buerjia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef __UTILITY_STRING_UTILS_HPP__
#define __UTILITY_STRING_UTILS_HPP__

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace utility {

namespace string_utils {

// test orignal string is start with prefix.
static bool is_startwith(const std::string& s, const std::string& prefix);

// test orignal string is end with suffix.
static bool is_endwith(const std::string& s, const std::string& suffix);

// test orignal string contain sub-string or not.
static bool contains(const std::string& s, const std::string& sub);

// split string by delimiter to vector.
static std::vector<std::string> split(const std::string& s,
                                      const std::string delimiter);

// use string new replace all old on orignal string
static size_t replace_all(std::string& str, const std::string& old_pattern,
                          const std::string& new_pattern);

// use string new replace one old on orignal string
static size_t replace(std::string& str, const std::string& old_pattern,
                      const std::string& new_pattern);

// join vector tokens to a string：e.g. { "a", "b" }-> [a, b]
static std::string join(std::vector<std::string>& tokens,
                        const std::string& delimiter = ",");

}  // namespace string_utils

////////////////////////////////////////////////////////////////////////////////

namespace string_utils {

static bool is_startwith(const std::string& s, const std::string& prefix) {
  return s.find(prefix) == 0u;
}

static bool is_endwith(const std::string& s, const std::string& suffix) {
  return s.size() >= suffix.size() &&
         s.rfind(suffix) == s.size() - suffix.size();
}

static bool contains(const std::string& s, const std::string& sub) {
  return s.find(sub, 0) != std::string::npos;
}

static std::vector<std::string> split(const std::string& s,
                                      const std::string delimiter) {
  std::vector<std::string> tokens;

  std::string::size_type lastPos = s.find_first_not_of(delimiter, 0);
  std::string::size_type pos = s.find_first_of(delimiter, lastPos);
  while ((std::string::npos != pos) || (std::string::npos != lastPos)) {
    tokens.push_back(s.substr(lastPos, pos - lastPos));
    lastPos = s.find_first_not_of(delimiter, pos);
    pos = s.find_first_of(delimiter, lastPos);
  }

  return tokens;
}

static size_t replace_all(std::string& str, const std::string& old_pattern,
                          const std::string& new_pattern) {
  size_t count = 0u;
  const size_t nsize = new_pattern.size();
  const size_t psize = old_pattern.size();

  size_t pos = str.find(old_pattern, 0);
  while (pos != std::string::npos) {
    str.replace(pos, psize, new_pattern);

    count++;
    pos = str.find(old_pattern, pos + nsize);
  }

  return count;
}

static size_t replace(std::string& str, const std::string& old_pattern,
                      const std::string& new_pattern) {
  size_t count = 0u;
  const size_t nsize = new_pattern.size();
  const size_t psize = old_pattern.size();

  size_t pos = str.find(old_pattern, 0);
  if (pos != std::string::npos) {
    str.replace(pos, psize, new_pattern);
    count++;
  }

  return count;
}

static std::string join(std::vector<std::string>& tokens,
                        const std::string& delimiter) {
  std::stringstream ss;
  ss << "[";

  std::string join_delimiter = "";
  for (auto& token : tokens) {
    ss << join_delimiter << token;
    join_delimiter = delimiter;
  }

  ss << "]";
  return ss.str();
}

}  // namespace string_utils

}  // namespace utility

#endif  // __UTILITY_STRING_UTILS_HPP__
