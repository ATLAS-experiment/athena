/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_HGTDMapping

#include <boost/test/unit_test.hpp>

#include "HGTDMapping/HGTDMappingData.h"
#include <sstream>
#include <string>
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(HGTDMappingTest)

  BOOST_AUTO_TEST_CASE(HGTDMappingConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] HGTDMappingData s);
  }

  BOOST_AUTO_TEST_CASE(HGTDMappingDataMethods){
    //default constructed Id should be invalid
    HGTDMappingData s;
    const HGTDOnlineID invalid;
    BOOST_CHECK(s.empty());
    BOOST_CHECK(s.onlineId(Identifier(0)) == invalid);
  }
  
  BOOST_AUTO_TEST_CASE(HGTDMappingDataFill){
    HGTDMappingData c;
    std::string inputString="0 0\n1 2\n4 6\n";
    std::istringstream s(inputString);
    s>>c;
    BOOST_CHECK(not c.empty());
    const HGTDOnlineID two(2);
    BOOST_TEST(c.onlineId(Identifier(1)) == two);
  }
  
BOOST_AUTO_TEST_SUITE_END()
