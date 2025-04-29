/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 
/**
 * @file ITkPixelCabling/test/ITkPixelCablingData_test.cxx
 * @author Shaun Roe
 * @date June 2024
 * @brief Some tests for ITkPixelCablingData 
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkPixelCabling

#include <boost/test/unit_test.hpp>

#include "ITkPixelCabling/ITkPixelCablingData.h"
#include <sstream>
#include <string>
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(ITkPixelCablingTest)

  BOOST_AUTO_TEST_CASE(ITkPixelCablingDataConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkPixelCablingData s);
  }
  
  BOOST_AUTO_TEST_CASE(ITkPixelCablingDataMethods){
    //default constructed Id should be invalid
    ITkPixelCablingData s;
    const ITkPixelOnlineId invalid;
    BOOST_CHECK(s.empty());
    BOOST_CHECK(s.onlineId(Identifier(0)) == invalid);
  }
  
  BOOST_AUTO_TEST_CASE(ITkPixelCablingDataFill){
    ITkPixelCablingData c;
    std::string inputString="0 0\n1 2\n3 5\n";
    std::istringstream s(inputString);
    s>>c;
    BOOST_CHECK(not c.empty());
    const ITkPixelOnlineId two(2);
    BOOST_TEST(c.onlineId(Identifier(1)) == two);
  }
  
BOOST_AUTO_TEST_SUITE_END()
