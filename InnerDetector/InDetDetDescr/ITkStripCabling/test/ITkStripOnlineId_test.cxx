/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 
/**
 * @file ITkStripCabling/test/ITkStripOnlineId_test.cxx
 * @author Edson Carquin
 * @date September 2024
 * @brief Some tests for ITkStripOnlineId (based on ITkPixelCabling package)
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkStripCabling

#include <boost/test/unit_test.hpp>

#include "ITkStripCabling/ITkStripOnlineId.h"
#include <sstream>
#include <cstdint>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(ITkStripOnlineIdTest)

 
  BOOST_AUTO_TEST_CASE(ITkStripOnlineIdConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkStripOnlineId s);
    std::uint32_t onlineId{1};
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkStripOnlineId s(onlineId));
    std::uint32_t rodId{1};
    std::uint32_t fibre{2};
    BOOST_CHECK_NO_THROW([[maybe_unused]] ITkStripOnlineId s(rodId, fibre));
  }
  
  BOOST_AUTO_TEST_CASE(ITkStripOnlineIdDefaultMethods){
    //default constructed Id should be invalid
    ITkStripOnlineId s;
    BOOST_CHECK(not s.isValid());
    BOOST_CHECK(s.rod() == ITkStripOnlineId::INVALID_ROD);
    BOOST_CHECK(s.fibre() == ITkStripOnlineId::INVALID_FIBRE);
    BOOST_CHECK(unsigned(s) == ITkStripOnlineId::INVALID_ONLINE_ID);    
  }
  
  BOOST_AUTO_TEST_CASE(ITkStripOnlineIdValidlyConstructedMethods){
    //construct with valid rod id and fibre number
    ITkStripOnlineId s(0x210000,1);
    BOOST_CHECK(s.isValid());
    BOOST_CHECK(s.rod() == 0x210000);
    BOOST_CHECK(s.fibre() == 1);
    //construct from an unsigned int
    ITkStripOnlineId t(18939904);
    //equality operator
    BOOST_CHECK(s == t);
    //test representation (stream insertion). Uses hex representation
    std::stringstream os;
    os<<s;
    BOOST_TEST (os.str() == "0x1210000");
  }
  
BOOST_AUTO_TEST_SUITE_END()
