/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_HGTDMapping

#include <boost/test/unit_test.hpp>

#include "HGTDMapping/HGTDOnlineID.h"
#include <sstream>
#include <cstdint>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(HGTDOnlineIDTest)

 
  BOOST_AUTO_TEST_CASE(HGTDOnlineIDConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] HGTDOnlineID s);
    std::uint32_t onlineId{1};
    BOOST_CHECK_NO_THROW([[maybe_unused]] HGTDOnlineID s(onlineId));
    std::uint32_t rodId{1};
    std::uint32_t elink{2};
    BOOST_CHECK_NO_THROW([[maybe_unused]] HGTDOnlineID s(rodId, elink));
  }
  
  BOOST_AUTO_TEST_CASE(HGTDOnlineIDDefaultMethods){
    //default constructed Id should be invalid
    HGTDOnlineID s;
    BOOST_CHECK(not s.isValid());
    BOOST_CHECK(s.rod() == HGTDOnlineID::INVALID_ROD);
    BOOST_CHECK(s.elink() == HGTDOnlineID::INVALID_ELINK);
    //BOOST_CHECK(s.value()  == HGTDOnlineID::INVALID_ONLINE_ID);   //  not working  
  }
  
  BOOST_AUTO_TEST_CASE(HGTDOnlineIDValidlyConstructedMethods){
    //construct with valid rod id and fibre number
    HGTDOnlineID s(0x210000,1);
    BOOST_CHECK(s.isValid());
    BOOST_CHECK(s.rod() == 0x210000);
    BOOST_CHECK(s.elink() == 1);
    //construct from an unsigned int
    HGTDOnlineID t(18939904);
    //equality operator
    BOOST_CHECK(s == t);
    //test representation (stream insertion). Uses hex representation
    std::stringstream os;
    os<<s;
    BOOST_TEST (os.str() == "0x1210000");
  }
  
BOOST_AUTO_TEST_SUITE_END()

