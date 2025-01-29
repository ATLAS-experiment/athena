/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 
/**
 * @file ITkPixelCabling/test/ITkPixelOnlineId_test.cxx
 * @author Shaun Roe
 * @date June 2024
 * @brief Some tests for ITkPixelOnlineId 
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkPixelCabling

#include <boost/test/unit_test.hpp>

#include "ITkPixelCabling/ITkPixelOnlineId.h"
#include <sstream>
#include <cstdint>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(ITkPixelOnlineIdTest)

 
  BOOST_AUTO_TEST_CASE(ITkPixelOnlineIdConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkPixelOnlineId s);
    std::uint32_t onlineId{1};
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkPixelOnlineId s(onlineId));
    std::uint32_t rodId{1};
    std::uint32_t fibre{2};
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkPixelOnlineId s(rodId, fibre));
  }
  
  BOOST_AUTO_TEST_CASE(ITkPixelOnlineIdDefaultMethods){
    //default constructed Id should be invalid
    ITkPixelOnlineId s;
    BOOST_CHECK(not s.isValid());
    BOOST_CHECK(s.rod() == ITkPixelOnlineId::INVALID_ROD);
    BOOST_CHECK(s.fibre() == ITkPixelOnlineId::INVALID_FIBRE);
    BOOST_CHECK(unsigned(s) == ITkPixelOnlineId::INVALID_ONLINE_ID);    
  }
  
  BOOST_AUTO_TEST_CASE(ITkPixelOnlineIdValidlyConstructedMethods){
    //construct with valid rod id and fibre number
    ITkPixelOnlineId s(0x210000,1);
    BOOST_CHECK(s.isValid());
    BOOST_CHECK(s.rod() == 0x210000);
    BOOST_CHECK(s.fibre() == 1);
    //construct from an unsigned int
    ITkPixelOnlineId t(18939904);
    //equality operator
    BOOST_CHECK(s == t);
    //test representation (stream insertion). Uses hex representation
    std::stringstream os;
    os<<s;
    BOOST_TEST (os.str() == "0x1210000");
  }
  
BOOST_AUTO_TEST_SUITE_END()
