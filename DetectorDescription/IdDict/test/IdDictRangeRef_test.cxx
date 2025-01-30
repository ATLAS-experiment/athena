// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"


BOOST_AUTO_TEST_SUITE(IdDictRangeRefTest)
BOOST_AUTO_TEST_CASE(IdDictRangeRefConstructors){
  BOOST_CHECK_NO_THROW(IdDictRangeRef());
  IdDictRangeRef i1;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRangeRef i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRangeRef i3(std::move(i1)));
}


BOOST_AUTO_TEST_CASE(IdDictRangeRefBuildRange){
  IdDictRangeRef f;
  auto pRange = std::make_unique<IdDictRange>();
  //by minmax
  pRange->m_specification = IdDictRange::by_minmax;
  pRange->m_minvalue = 2;
  pRange->m_maxvalue = 10;
  f.m_range = pRange.get();
  BOOST_TEST(f.build_range() == Range("2:10"));
  
}

BOOST_AUTO_TEST_SUITE_END()
