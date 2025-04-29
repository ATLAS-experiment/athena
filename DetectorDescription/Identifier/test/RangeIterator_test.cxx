// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

#include "Identifier/Range.h"
#include "Identifier/RangeIterator.h"
#include "Identifier/ExpandedIdentifier.h"
#include <iostream>
#include <sstream>

//ensure BOOST knows how to represent an ExpandedIdentifier
namespace boost::test_tools::tt_detail {
   template<>           
   struct print_log_value<ExpandedIdentifier> {
     void operator()( std::ostream& os, ExpandedIdentifier const& v){
       os<<std::string(v);//ExpandedIdentifier has string conversion
     }
   };                                                          
 }


BOOST_AUTO_TEST_SUITE(RangeTest)

BOOST_AUTO_TEST_CASE(RangeIteratorConstructors){
  const std::string sctExample="-6:-1/1:6";//SCT barrel has eta indices -6->-1 and 1->6
  Range f1;
  f1.build(sctExample);  
  BOOST_CHECK_NO_THROW(RangeIterator f2(f1));
}

BOOST_AUTO_TEST_CASE(ConstRangeIteratorConstructors){
const std::string sctExample="-6:-1/1:6";//SCT barrel has eta indices -6->-1 and 1->6
  Range f1;
  f1.build(sctExample);  
  BOOST_CHECK_NO_THROW(ConstRangeIterator f2(f1));
  BOOST_CHECK_NO_THROW(ConstRangeIterator f3(std::move(f1)));
}

BOOST_AUTO_TEST_CASE(RangeIteratorBasic){
  const std::string sctExample="-2:-1/1:2";
  Range r1;
  r1.build(sctExample);
  RangeIterator i(r1);
  std::vector<ExpandedIdentifier> vex;
  std::copy(i.begin(),i.end(),std::back_inserter(vex));
  using E = ExpandedIdentifier;
  const std::array<ExpandedIdentifier, 4> expected{E("-2/1"), E("-2/2"), E("-1/1"), E("-1/2")};
  BOOST_CHECK_EQUAL_COLLECTIONS(vex.begin(), vex.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(ConstRangeIteratorBasic){
  const std::string sctExample="-2:-1/1:2";
  Range r1;
  r1.build(sctExample);
  ConstRangeIterator i(r1);
  std::vector<ExpandedIdentifier> vex;
  std::copy(i.begin(),i.end(),std::back_inserter(vex));
  using E = ExpandedIdentifier;
  const std::array<ExpandedIdentifier, 4> expected{E("-2/1"), E("-2/2"), E("-1/1"), E("-1/2")};
  BOOST_CHECK_EQUAL_COLLECTIONS(vex.begin(), vex.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()
