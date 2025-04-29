// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

#include "Identifier/Range.h"
#include "Identifier/ExpandedIdentifier.h"
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
BOOST_AUTO_TEST_CASE(RangeConstructors){
  BOOST_CHECK_NO_THROW(Range());
  Range r1;
  BOOST_CHECK_NO_THROW(Range r2(r1));
  BOOST_CHECK_NO_THROW(Range r3(std::move(r1)));
  ExpandedIdentifier e;
  BOOST_CHECK_NO_THROW(Range r4(e));
  BOOST_CHECK_NO_THROW(Range r5("-6:-1/1:6"));
}

BOOST_AUTO_TEST_CASE(RangeAssignment){
  ExpandedIdentifier e;
  Range r1(e);
  Range r2;
  Range r3;
  BOOST_CHECK_NO_THROW(r2 = r1);
  BOOST_CHECK_NO_THROW(r3 = std::move(r2));
}

BOOST_AUTO_TEST_CASE(RangeStreamExtraction){
  Range r;
  const std::string larExample="4/1/-1,2/3/0/0:19/0:255";
  std::istringstream in(larExample);
  BOOST_CHECK_NO_THROW(in>>r);
  BOOST_TEST(larExample == std::string(r));
  Range r2;
  const std::string larExample2="4/1/-1,1/3/0/0:9/0:63";
  std::istringstream in2(larExample);
  BOOST_CHECK_NO_THROW(in2>>r2);
}

BOOST_AUTO_TEST_CASE(RangeConstructFromPart){
  Range r1;
  const std::string larExample="4/1/-1,2/3/0/0:19/0:255";
  r1.build(larExample);
  Range r2(r1,2);
  const std::string truncated="-1,2/3/0/0:19/0:255";
  Range r3;
  r3.build(truncated);
  //also testing equality
  BOOST_TEST( r2 == r3, "Construction from part of another range");
}

BOOST_AUTO_TEST_CASE(RangeAccessors){
  Range r1;
  const std::string larExample="4/1/-1,2/3/0/0:19/0:255";
  r1.build(larExample);
  IdentifierField f(0,19);
  BOOST_TEST( r1[5] == f, "r[5] gives IdentifierField '0:19'");
  BOOST_TEST(r1.fields() == 7);
  BOOST_TEST(r1.is_empty() == false);
  Range r2;
  BOOST_TEST(r2.is_empty() == true);
  ExpandedIdentifier min{"4/1/-1/3/0/0/0"};
  ExpandedIdentifier max{"4/1/2/3/0/19/255"};
  BOOST_CHECK_NO_THROW(min = r1.minimum());
  BOOST_CHECK_NO_THROW(max = r1.maximum());
  //expanded multiplication by level for clarity
  BOOST_TEST(r1.cardinality() = 1*1*2*1*1*20*256);
  ExpandedIdentifier upto{"4/1/2/3/0"};
  BOOST_TEST(r1.cardinalityUpTo(upto) = 1*1*2*1*1);
  const std::string overlapping="4/1/-1,2/3/0/2/0:255";
  r2.build(overlapping);
  BOOST_TEST(r1.overlaps_with(r2) == true);
  const std::string nonOverlapping="4/1/4,5/3/0/2/0:255";
  r2.build(nonOverlapping);
  BOOST_TEST(r1.overlaps_with(r2) == false);
}


BOOST_AUTO_TEST_CASE(RangeBuildFromExpandedIdentifier){
  ExpandedIdentifier id{"4/1/1/3/0/0/26"};
  Range r1;
  BOOST_CHECK_NO_THROW(r1.build(id));
  BOOST_TEST(r1[3] == 3);
}



BOOST_AUTO_TEST_CASE(RangeBuildFromText){
  const std::string sctExample="-6:-1/1:6";//SCT barrel has eta indices -1->-5 and 1->6
  Range r0(sctExample);
  BOOST_TEST(r0.is_empty() == false);
  Range r1;
  BOOST_CHECK_NO_THROW(r1.build(sctExample));
  const std::string wildCard="*";
  BOOST_CHECK_NO_THROW(r1.build(wildCard));
  const std::string enumerated="1,2,3,4,10";
  BOOST_CHECK_NO_THROW(r1.build(enumerated));
  const std::string empty;
  BOOST_CHECK_NO_THROW(r1.build(empty)); //do nothing
  const std::string nonsense="hgfclsdvoiwe";
  BOOST_CHECK_THROW(r1.build(nonsense), std::invalid_argument); //throw exception
  const std::string larExample="4/1/-1,1/3/0/0:19/0:255";
  BOOST_CHECK_NO_THROW(r1.build(larExample));
}


BOOST_AUTO_TEST_CASE(RangeMatch){
  Range r1, r2;
  const std::string larExample="4/1/-1,1/3/0/0:19/0:255";
  BOOST_CHECK_NO_THROW(r1.build(larExample));
  ExpandedIdentifier id{"4/1/1/3/0/0/26"};
  BOOST_TEST(r1.match(id) == true);
  //4/1/-1,1/3/0/0:9/0:63
  const std::string larExample2="4/1/-1,1/3/0/0:9/0:63";
  BOOST_CHECK_NO_THROW(r2.build(larExample2));
  BOOST_TEST(r2.match(id) == true);
}

BOOST_AUTO_TEST_CASE(RangeRepresentation){
  Range r1;
  const std::string larExample="4/1/-1,2/3/0/0:19/0:255";
  r1.build(larExample);
  std::ostringstream os;
  r1.show(os);
  BOOST_TEST(os.str() == "4/1/-1,2/3/0/0:19/0:255 (1+1+1+1+1+5+8=18) ");
  BOOST_TEST(std::string(r1) == larExample);
}
BOOST_AUTO_TEST_SUITE_END()

