// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "Identifier/Range.h"


BOOST_AUTO_TEST_SUITE(RangeTest)
BOOST_AUTO_TEST_CASE(RangeConstructors){
  BOOST_CHECK_NO_THROW(Range());
  Range r1;
  BOOST_CHECK_NO_THROW(Range r2(r1));
  BOOST_CHECK_NO_THROW(Range r3(std::move(r1)));
  ExpandedIdentifier e;
  BOOST_CHECK_NO_THROW(Range r4(e));
}


//Range::identifier_factory is a publicly accessible class defined in the Range class
BOOST_AUTO_TEST_CASE(RangeIdentifier_factoryConstructors){
  BOOST_CHECK_NO_THROW(Range::identifier_factory());
  Range::identifier_factory f1;
  BOOST_CHECK_NO_THROW(Range::identifier_factory f2(f1));
  BOOST_CHECK_NO_THROW(Range::identifier_factory f3(std::move(f1)));
  Range r1;
  BOOST_CHECK_NO_THROW(Range::identifier_factory f4(r1));
}

//const_identifier_factory
//Range::const_identifier_factory is a publicly accessible class defined in the Range class
BOOST_AUTO_TEST_CASE(RangeConst_identifier_factoryConstructors){
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory());
  Range::const_identifier_factory f1;
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory f2(f1));
  Range r1;
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory f3(r1));
}

BOOST_AUTO_TEST_SUITE_END()

