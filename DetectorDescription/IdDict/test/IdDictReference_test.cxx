// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"


BOOST_AUTO_TEST_SUITE(IdDictReferenceTest)
BOOST_AUTO_TEST_CASE(IdDictReferenceConstructors){
  BOOST_CHECK_NO_THROW(IdDictReference(""));
  IdDictReference i1("");
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictReference i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictReference i3(std::move(i1)));
}


BOOST_AUTO_TEST_CASE(IdDictReferenceBuildRange){
  IdDictSubRegion sr ("", "", "");
  IdDictReference f (&sr);
  BOOST_TEST(f.build_range() == Range());
  
}

BOOST_AUTO_TEST_SUITE_END()
