/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 
/**
 * @file ITkStripCabling/test/ITkStripCablingData_test.cxx
 * @author Edson Carquin
 * @date September 2024
 * @brief Some tests for ITkStripCablingData (based on ITkPixelCabling package)
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkStripCabling

#include <boost/test/unit_test.hpp>

#include "ITkStripCabling/ITkStripCablingData.h"
#include <sstream>
#include <string>
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(ITkStripCablingTest)

  BOOST_AUTO_TEST_CASE(ITkStripCablingDataConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkStripCablingData s);
  }
  
  BOOST_AUTO_TEST_CASE(ITkStripCablingDataMethods){
    //default constructed Id should be invalid
    ITkStripCablingData s;
    const ITkStripOnlineId invalid;
    BOOST_CHECK(s.empty());
    BOOST_CHECK(s.onlineId(Identifier(0)) == invalid);
  }
  
  BOOST_AUTO_TEST_CASE(ITkStripCablingDataFill){
    ITkStripCablingData c;
    std::string inputString="0 0\n1 2\n3 5\n";
    std::istringstream s(inputString);
    s>>c;
    BOOST_CHECK(not c.empty());
    const ITkStripOnlineId two(2);
    BOOST_TEST(c.onlineId(Identifier(1)) == two);
  }
  
BOOST_AUTO_TEST_SUITE_END()
