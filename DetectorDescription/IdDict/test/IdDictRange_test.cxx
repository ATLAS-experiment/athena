// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictRange.h"
#include "Identifier/Range.h"


BOOST_AUTO_TEST_SUITE(IdDictRangeTest)
BOOST_AUTO_TEST_CASE(IdDictRangeConstructors){
  BOOST_CHECK_NO_THROW(IdDictRange(""));
  IdDictRange i1("");
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRange i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRange i3(std::move(i1)));
}

BOOST_AUTO_TEST_CASE(EmptyIdDictRangeAccessors){
  IdDictRange f("");
  BOOST_TEST(f.specification() == IdDictRange::unknown);
}

BOOST_AUTO_TEST_CASE(IdDictRangeBuildRange){
  //This is the main workhorse, to build a range according to inputs
  //In practice only two basic types of range are built: enumerated and "both bounded"
  //by value or label (single-valued)
  IdDictRange f1 ("", 455);
  BOOST_TEST(f1.build_range() == Range("455"));
  //by minmax
  IdDictRange f2 ("", -1, 5);
  BOOST_TEST(f2.build_range() == Range("-1:5"));
  //enumerated
  IdDictRange f3 ("", std::vector<int> { 0, 1, 2, 4, 5});
  //note: consecutive value _might_ be optimised to min/max
  BOOST_TEST(f3.build_range() == Range("0,1,2,4,5"));
}

BOOST_AUTO_TEST_SUITE_END()
