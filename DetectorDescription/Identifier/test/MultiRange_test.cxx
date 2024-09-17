// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "Identifier/MultiRange.h"
#include "Identifier/Range.h"


BOOST_AUTO_TEST_SUITE(MultiRangeTest)
BOOST_AUTO_TEST_CASE(MultiRangeConstructors){
  BOOST_CHECK_NO_THROW(MultiRange());
  MultiRange r1;
  BOOST_CHECK_NO_THROW(MultiRange r2(r1));
  BOOST_CHECK_NO_THROW(MultiRange r3 = r1);
  BOOST_CHECK_NO_THROW(MultiRange r4(std::move(r1)));
  Range p1;
  p1.build("-1:5");
  Range p2;
  p2.build("4:10");
  BOOST_CHECK_NO_THROW(MultiRange r4(p1,p2));
}

BOOST_AUTO_TEST_CASE(MultiRangeAccessors){
  Range p1;
  p1.build("-1:5");
  Range p2;
  p2.build("4:10");
  MultiRange m(p1,p2);
  BOOST_TEST(m.back() == p2, "Last added range is given by back()");
  BOOST_TEST(m[0] == p1, "Index accessor");
  BOOST_TEST(m.size() == 2, "size()");
  BOOST_TEST(*(m.begin()) == p1,"dereference MultiRange::begin()");
  BOOST_TEST(m.has_overlap() == true, "has_overlap returns true for overlapping ranges");
  Range p3;
  p3.build("7:20");
  MultiRange m2(p1,p3);
  BOOST_TEST(m2.has_overlap() == false, "has_overlap returns false for disjoint ranges");
}

BOOST_AUTO_TEST_SUITE_END()
