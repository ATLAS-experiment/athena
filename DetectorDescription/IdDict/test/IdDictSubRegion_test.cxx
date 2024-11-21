// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "IdDict/IdDictDefs.h"
#include "Identifier/Range.h" 

BOOST_AUTO_TEST_SUITE(IdDictSubRegionTest)
BOOST_AUTO_TEST_CASE(IdDictSubRegionConstructors){
  BOOST_CHECK_NO_THROW(IdDictSubRegion());
  IdDictSubRegion i1;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictSubRegion i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictSubRegion i3(std::move(i1)));
}



BOOST_AUTO_TEST_SUITE_END()
