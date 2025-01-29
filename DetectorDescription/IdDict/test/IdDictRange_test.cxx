// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"


BOOST_AUTO_TEST_SUITE(IdDictRangeTest)
BOOST_AUTO_TEST_CASE(IdDictRangeConstructors){
  BOOST_CHECK_NO_THROW(IdDictRange());
  IdDictRange i1;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRange i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRange i3(std::move(i1)));
}

BOOST_AUTO_TEST_CASE(EmptyIdDictRangeAccessors){
  IdDictRange f;
  BOOST_TEST(f.m_specification == IdDictRange::unknown);
}

BOOST_AUTO_TEST_CASE(IdDictRangeBuildRange){
  //This is the main workhorse, to build a range according to inputs
  //In practice only two basic types of range are built: enumerated and "both bounded"
  IdDictRange f;
  //by value or label (single-valued)
  f.m_specification = IdDictRange::by_value;
  f.m_value = 455;
  BOOST_TEST(f.build_range() == Range("455"));
  //by minmax
  f.m_specification = IdDictRange::by_minmax;
  f.m_minvalue = -1;
  f.m_maxvalue = 5;
  BOOST_TEST(f.build_range() == Range("-1:5"));
  //enumerated
  f.m_specification = IdDictRange::by_values;
  //note: consecutive value _might_ be optimised to min/max
  f.m_values = {0,1,2,4,5};
  BOOST_TEST(f.build_range() == Range("0,1,2,4,5"));
}

BOOST_AUTO_TEST_SUITE_END()
