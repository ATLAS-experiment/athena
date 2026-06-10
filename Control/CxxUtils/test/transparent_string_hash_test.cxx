/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#define BOOST_TEST_MODULE CxxUtils
#include <boost/test/unit_test.hpp>

#include "CxxUtils/transparent_string_hash.h"

#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>


using CxxUtils::TransparentStringHash;
using CLID = unsigned int;

using NameMap = std::unordered_map< std::string,CLID,TransparentStringHash,std::equal_to<>>;

BOOST_AUTO_TEST_SUITE(transparent_string_hash)
BOOST_AUTO_TEST_CASE(test_transparent_string_hash){
  static_assert(std::is_same_v<TransparentStringHash::is_transparent, void>);

  const TransparentStringHash hash;

  const std::string s = "Foo";
  const std::string_view sv = "Foo";
  const char* cstr = "Foo";

  BOOST_TEST(hash(s) == hash(sv));
  BOOST_TEST(hash(cstr) == hash(sv));
}


BOOST_AUTO_TEST_CASE(test_transparent_string_equal){
  const std::string s = "Foo";
  const std::string_view sv = "Foo";
  const char* cstr = "Foo";

  BOOST_TEST(std::equal_to<>{}(s, sv));
  BOOST_TEST(std::equal_to<>{}(sv, s));
  BOOST_TEST(std::equal_to<>{}(cstr, sv));
  BOOST_TEST(std::equal_to<>{}(sv, cstr));
  //avoid pointer comparison if using string literals with std::equal_to
  BOOST_TEST(!std::equal_to<>{}(std::string_view{"Foo"}, std::string_view{"Bar"}));
}

BOOST_AUTO_TEST_CASE(test_unordered_map_transparent_lookup){
  NameMap nameMap;
  nameMap.emplace("Foo", 123);
  nameMap.emplace("Bar", 456);

  const std::string s = "Foo";
  const std::string_view sv = "Foo";
  const char* cstr = "Foo";

  {
    const auto itr = nameMap.find(s);
    BOOST_REQUIRE(static_cast<bool>(itr != nameMap.end()));
    BOOST_TEST(itr->second == 123);
  }

  {
    const auto itr = nameMap.find(sv);
    BOOST_REQUIRE(static_cast<bool>(itr != nameMap.end()));
    BOOST_TEST(itr->second == 123);
  }

  {
    const auto itr = nameMap.find(cstr);
    BOOST_REQUIRE(static_cast<bool>(itr != nameMap.end()));
    BOOST_TEST(itr->second == 123);
  }

  BOOST_TEST(nameMap.count(std::string_view{"Bar"}) == 1);
  BOOST_TEST(nameMap.count("Missing") == 0);

  BOOST_TEST(nameMap.contains(std::string_view{"Foo"}));
  BOOST_TEST(nameMap.contains("Bar"));
  BOOST_TEST(!nameMap.contains(std::string_view{"Missing"}));
}
BOOST_AUTO_TEST_SUITE_END()


