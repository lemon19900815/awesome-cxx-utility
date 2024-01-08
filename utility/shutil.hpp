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

#ifndef __UTILITY_SHUTIL_HPP__
#define __UTILITY_SHUTIL_HPP__

#if __cplusplus >= 201703L
// msvc: add "/Zc:__cplusplus" to C/C++ cmdline.
#include <filesystem>
namespace fs = std::filesystem;
using error_code = std::error_code;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
using error_code = boost::system::error_code;
#endif

#include <cstring>
#include <regex>
#include <string>
#include <vector>

namespace utility {

// like python shutil module.
namespace shutil {

using string_list = std::vector<std::string>;

// test target(file|directory) exist or not.
static bool is_exist(const std::string &p);

// get the target(file|directory) absolute path. refer os.path.abspath
static std::string abspath(const std::string &p);

// normalize target(file|directory) path.
// e.g. windows: a/b\\c->a\\b\\c; linux: a/b\\c->a/b/c
static std::string normalize(std::string p);

// refer os.path.join
static std::string path_join(std::string p1, std::string p2);

// get target(file|directory) size
static uint64_t get_file_size(std::string p);

// target is directory or not.
static bool is_dir(std::string p);

// target is normal file or not
static bool is_normal_file(std::string p);

// make a new directory, if it exists, return true.
static bool make_dir(const std::string &p);

// copy src_file->dest_dir/filename(src_file) or dest_dir/dest_file
static bool copy_file(const std::string &src_file, const std::string &dest_dir,
                      const std::string &dest_file = "");

static bool skip(const std::string &check_item, const string_list &skip_items);

// copy dir：src_dir/*->dest_dir/*；skip filename match any one of skip_items
static bool copy_dir(const std::string &src_dir, const std::string &dest_dir,
                     const string_list &skip_items);

// remove directory: if not exist, return true
static bool remove_dir(const std::string &dir);

// remove file：if not exist, return true
static bool remove_file(const std::string &file);

// scan files and directories under the target dir.
// refer python's glob module, not support regex.
static string_list glob(const std::string &dir, bool recursive = false);

}  // namespace shutil

////////////////////////////////////////////////////////////////////////////////

namespace shutil {

static bool is_exist(const std::string &p) {
  error_code ec;
  return fs::exists(fs::path{p}, ec);
}

static std::string abspath(const std::string &p) {
  error_code ec;
  return fs::weakly_canonical(fs::absolute(fs::path{p}, ec), ec).string();
}

static std::string normalize(std::string p) {
  p = fs::path{p}.string();
#ifdef _WIN32
  std::replace(p.begin(), p.end(), '/', '\\');
#else
  std::replace(p.begin(), p.end(), '\\', '/');
#endif  // _WIN32
  return p;
}

static std::string path_join(std::string p1, std::string p2) {
  return (fs::path{p1} / p2).string();
}

static uint64_t get_file_size(std::string p) {
  if (is_normal_file(p)) {
    error_code ec;
    auto total_size = fs::file_size(fs::path{p}, ec);

    if (!ec)
      return total_size;

    return 0;
  }

  uint64_t total_size = 0;

  for (auto &file : glob(p)) {
    total_size += get_file_size(file);
  }

  return total_size;
}

static bool is_dir(std::string p) {
  error_code ec;
  return fs::is_directory(fs::path{p}, ec) && !ec;
}

static bool is_normal_file(std::string p) {
  error_code ec;
  return fs::is_regular_file(fs::path{p}, ec) && !ec;
}

static bool make_dir(const std::string &p) {
  if (is_exist(p))
    return true;

  error_code ec;
  fs::create_directories(fs::path{p}, ec);
  return !ec;
}

static bool copy_file(const std::string &src_file, const std::string &dest_dir,
                      const std::string &dest_file) {
  auto src_path = fs::path{src_file};
  auto dest_path = fs::path{dest_dir};

  if (!is_exist(dest_dir) && !make_dir(dest_dir))
    return false;

  if (!dest_file.empty())
    dest_path /= dest_file;
  else
    dest_path /= src_path.filename();

  error_code ec;

  auto opt = fs::copy_options::overwrite_existing;
  return fs::copy_file(src_path, dest_path, opt, ec);
}

// static bool copy_dir(const std::string &src_dir, const std::string &dest_dir)
// {
//   error_code ec;
//
//   auto opt = fs::copy_options::overwrite_existing |
//   fs::copy_options::recursive; fs::copy(fs::path{src_dir},
//   fs::path{dest_dir}, opt, ec);
//
//   return !ec;
// }

static bool skip(const std::string &check_item, const string_list &skip_items) {
  for (auto &skip_item : skip_items) {
    if (check_item == skip_item)
      return true;
  }

  return false;
}

static bool copy_dir(const std::string &src_dir, const std::string &dest_dir,
                     const string_list &skip_items) {
  auto dest_path = fs::path{dest_dir};
  for (auto &dir_entry : fs::directory_iterator(fs::path{src_dir})) {
    auto dir_path = dir_entry.path();
    auto filename = dir_path.filename().string();
    if (filename == "." || filename == "..")
      continue;

    if (skip(filename, skip_items))
      continue;

    auto result = false;
    if (fs::is_directory(dir_path)) {
      auto dest_file = (dest_path / filename).string();
      result = copy_dir(dir_path.string(), dest_file, skip_items);
    }
    else
      result = copy_file(dir_path.string(), dest_dir);

    if (!result)
      return false;
  }

  return true;
}

static bool remove_dir(const std::string &dir) {
  if (!is_exist(dir))
    return true;

  if (!is_dir(dir))
    return false;

  error_code ec;
  fs::remove_all(fs::path{dir}, ec);
  return !ec;
}

static bool remove_file(const std::string &file) {
  error_code ec;
  fs::remove(fs::path{file}, ec);
  return !ec;
}

static string_list glob(const std::string &dir, bool recursive) {
  string_list files;

  if (!is_exist(dir))
    return files;

  // recursive: not use fs::recursive_directory_iterator
  for (auto &dir_entry : fs::directory_iterator(fs::path{dir})) {
    auto filename = dir_entry.path().filename().string();
    if (filename == "." || filename == "..")
      continue;

    files.emplace_back(dir_entry.path().string());

    // recursive expand sub directory.
    if (recursive && fs::is_directory(dir_entry.path())) {
      auto tmp = glob(dir_entry.path().string(), recursive);
      files.insert(files.end(), tmp.begin(), tmp.end());
    }
  }

  return files;
}

}  // namespace shutil

}  // namespace utility

#endif  // __UTILITY_SHUTIL_HPP__
