#include "doctest.h"
#include "string_utils.hpp"
using namespace utility;

TEST_CASE("test string_utils") {
  std::vector<std::string> tokens = {"a", "b", "c"};
  std::string str = "a;b;c";

  CHECK_EQ(string_utils::join(tokens, ","), "[a,b,c]");
  CHECK_EQ(string_utils::split(str, ";"), tokens);

  auto dump = str;
  CHECK_EQ(string_utils::replace(dump, ";", "X"), 1);
  CHECK_EQ(dump, "aXb;c");

  CHECK_EQ(string_utils::replace(dump, ";", "X"), 1);
  CHECK_EQ(dump, "aXbXc");

  CHECK_EQ(string_utils::replace_all(dump, "X", "yy"), 2);
  CHECK_EQ(dump, "ayybyyc");

  CHECK_EQ(string_utils::is_startwith("abcaxel;axw", "abcaxel;"), true);
  CHECK_EQ(string_utils::is_endwith("abcaxel;axw", ";axw"), true);
  CHECK_EQ(string_utils::contains("abclemonaxe;;zaew", "lemon"), true);
  CHECK_EQ(string_utils::contains("abclemonaxe;;zaew", "lemon;"), false);
}
