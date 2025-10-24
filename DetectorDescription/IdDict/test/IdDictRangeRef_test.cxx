// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictRangeRef.h"
#include "IdDict/IdDictRange.h"
#include "Identifier/Range.h"


BOOST_AUTO_TEST_SUITE(IdDictRangeRefTest)
BOOST_AUTO_TEST_CASE(IdDictRangeRefConstructors){
  IdDictRange r ("", 0);
  IdDictRangeRef i1(r);
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRangeRef i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRangeRef i3(std::move(i1)));
}


BOOST_AUTO_TEST_CASE(IdDictRangeRefBuildRange){
  //by minmax
  IdDictRange r ("", 2, 10);
  IdDictRangeRef f(r);
  BOOST_TEST(f.build_range() == Range("2:10"));
  
}

BOOST_AUTO_TEST_SUITE_END()
