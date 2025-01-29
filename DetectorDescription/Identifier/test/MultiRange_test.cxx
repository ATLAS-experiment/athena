// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

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

BOOST_AUTO_TEST_CASE(MultiRangeRepresentation){
  Range p1;
  p1.build("-1:5");
  Range p2;
  p2.build("4:10");
  MultiRange m(p1,p2);
  std::ostringstream os;
  BOOST_CHECK_NO_THROW(m.show(os));
  BOOST_TEST_MESSAGE(os.str());
  BOOST_TEST(os.str() == "-1:5 (3=3) \n4:10 (3=3) ");
  std::string s;
  BOOST_CHECK_NO_THROW(s = std::string(m));
  BOOST_TEST_MESSAGE(s);
  BOOST_TEST(s == "-1:5 | 4:10");
}




BOOST_AUTO_TEST_CASE(MultiRangeModifiers){
  Range p1;
  p1.build("-1:5");
  Range p2;
  p2.build("4:10");
  MultiRange m(p1,p2);
  BOOST_TEST(m.size() == 2,"Size is 2 before adding a list"); //initially 2 elements
  Range p3;
  p3.build("-1,1");
  //add a range
  BOOST_CHECK_NO_THROW(m.add(p3));
  BOOST_TEST(m.size() == 3, "Size is 3 after adding a list"); //three elements now
  Range p4;
  p4.build("20:30");
  BOOST_CHECK_NO_THROW(m.add(std::move(p4)));
  BOOST_TEST(m.size() == 4, "Size is 4 after adding a move(bounds)"); //three elements now
  ExpandedIdentifier i("-3/-2/-1/1/2/2");
  BOOST_CHECK_NO_THROW(m.add(i));
  BOOST_TEST(m.size() == 5, "Size is 5 after adding an ExpandedIdentifier");
  //what does that look like??
  BOOST_TEST_MESSAGE(std::string(m));//-1:5 | 4:10 | -1,1 | 20:30 | -3/-2/-1/1/2/2
  BOOST_CHECK_NO_THROW(m.remove_range(i));
  BOOST_TEST(m.size() == 4, "Size is 4 after removing an ExpandedIdentifier");
  p4.build("20:30");//rebuild, because it was moved
  BOOST_TEST(m.back() == p4);
  BOOST_CHECK_NO_THROW(m.clear());
  BOOST_TEST(m.size() == 0, "Size is zero after clear()");
}

BOOST_AUTO_TEST_CASE(MultiRangeMatch){
  MultiRange m;
  ExpandedIdentifier i("-3/-2/-1/1/2/2");
  ExpandedIdentifier j("-3/-2/-1/1/2/3");
  m.add(i);
  BOOST_TEST(m.match(i) == true);
  ExpandedIdentifier k("-3/-2/-1/1/2/1");
  BOOST_TEST(m.match(k) == false);
}

BOOST_AUTO_TEST_CASE(MultiRangeIteratorConstructor){
  using MultiRangeIterator = MultiRange::identifier_factory;
  BOOST_CHECK_NO_THROW(MultiRangeIterator m1);
  MultiRange m;
  ExpandedIdentifier i("-3/-2/-1/1/2/2");
  m.add(i);
  BOOST_CHECK_NO_THROW(MultiRangeIterator m2(m));
}

BOOST_AUTO_TEST_CASE(MultiRangeIteratorBasic){
  using MultiRangeIterator = MultiRange::identifier_factory;
  MultiRange m;
  ExpandedIdentifier i("-3/-2/-1/1/2/2");
  m.add(i);
  ExpandedIdentifier j("-3/-2/-1/1/2/3");
  m.add(j);
  BOOST_CHECK_NO_THROW(MultiRangeIterator m2(m));
  auto b = m.begin();
  //derefence
  BOOST_TEST(*b == i);
  BOOST_CHECK_NO_THROW(++b);
  BOOST_TEST(*b == j);
  b =  m.begin();
  auto e = m.end();
  std::array<ExpandedIdentifier, 2> expected = {i,j};
  BOOST_CHECK_EQUAL_COLLECTIONS(b, e, expected.begin(), expected.end());
  for (const auto & ex:m){
    BOOST_TEST_MESSAGE(std::string(ex));
  }
}



BOOST_AUTO_TEST_SUITE_END()
