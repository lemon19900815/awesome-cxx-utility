#include <fstream>

#include "doctest.h"
#include "shutil.hpp"
using namespace utility;

bool write_file(std::string filepath, std::string content) {
  std::ofstream stream(filepath.c_str(), std::ofstream::out);
  if (!stream.is_open())
    return false;

  stream << content;
  stream.close();

  return true;
}

static uint64_t get_file_size(std::ifstream& in) {
  // seek end for get the file size.
  in.seekg(0, std::ios::end);
  std::streampos file_size = in.tellg();
  in.seekg(0, std::ios::beg);

  return static_cast<uint64_t>(file_size);
}

std::string read_file(std::string filepath) {
  std::ifstream in(filepath.c_str(), std::ifstream::in);
  if (!in.is_open())
    return "";

  auto file_size = get_file_size(in);

  std::string content;

  content.resize(file_size, ' ');
  in.read(&content[0], file_size);

  in.close();

  return content;
}

TEST_CASE("test shutil") {
  std::string filepath = "a.txt";
  std::string content = "this is a shutil test.";

  // prepare work.
  CHECK_EQ(write_file(filepath, content), true);

  CHECK_EQ(shutil::get_file_size(filepath), content.size());

  CHECK_EQ(shutil::is_exist(filepath), true);
  CHECK_EQ(shutil::is_normal_file(filepath), true);

  std::string folder = "tmp";
  CHECK_EQ(shutil::make_dir(folder), true);
  CHECK_EQ(shutil::is_exist(folder), true);
  CHECK_EQ(shutil::is_dir(folder), true);

  CHECK_EQ(shutil::abspath("tmp/../a.txt"), shutil::abspath("a.txt"));

#ifdef _WIN32
  CHECK_EQ(shutil::normalize("a\\b/c"), "a\\b\\c");
#else
  CHECK_EQ(shutil::normalize("a\\b/c"), "a/b/c");
#endif

  CHECK_EQ(shutil::copy_file(filepath, folder), true);
  auto target = shutil::path_join(folder, filepath);

  CHECK_EQ(shutil::is_exist(target), true);

  CHECK_EQ(shutil::remove_dir(folder), true);
  CHECK_EQ(shutil::remove_dir(filepath), false);
  CHECK_EQ(shutil::remove_file(filepath), true);
}
