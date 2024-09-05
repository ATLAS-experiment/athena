// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "Identifier/Range.h"
#include "Identifier/ExpandedIdentifier.h"

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
}

BOOST_AUTO_TEST_CASE(RangeAssignment){
  ExpandedIdentifier e;
  Range r1(e);
  Range r2;
  Range r3;
  BOOST_CHECK_NO_THROW(r2 = r1);
  BOOST_CHECK_NO_THROW(r3 = std::move(r2));
}


BOOST_AUTO_TEST_CASE(RangeBuildFromText,* utf::expected_failures(2)){
  const std::string sctExample="-6:-1/1:6";//SCT barrel has eta indices -1->-5 and 1->6
  Range r1;
  BOOST_CHECK_NO_THROW(r1.build(sctExample));
  const std::string wildCard="*";
  BOOST_CHECK_NO_THROW(r1.build(wildCard));
  const std::string lowerBound="-5:";
  BOOST_CHECK_NO_THROW(r1.build(lowerBound));
  const std::string upperBound=":5";
  BOOST_CHECK_NO_THROW(r1.build(upperBound));
  const std::string enumerated="1,2,3,4,10";
  BOOST_CHECK_NO_THROW(r1.build(enumerated));
  //I think it should throw if you feed it nonsense, but it doesn't; it fails silently
  const std::string empty;
  BOOST_CHECK_THROW(r1.build(empty), std::runtime_error);
  const std::string nonsense="hgfclsdvoiwe";
  BOOST_CHECK_THROW(r1.build(nonsense), std::runtime_error);
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

