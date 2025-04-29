/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 
/**
 * @file ITkStripCabling/test/OnlineIdGenerator_test.cxx
 * @author Shaun Roe
 * @date October 2024
 * @brief Some tests for OnlineIdGenerator
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkStripCabling

#include <boost/test/unit_test.hpp>

#include "src/OnlineIdGenerator.h"
#include "ITkStripCabling/ITkStripOnlineId.h"
#include "Identifier/Identifier.h"
#include "IdDictParser/IdDictParser.h"
#include "IdDict/IdDictDefs.h"
#include "InDetIdentifier/SCT_ID.h"


#include <sstream>
#include <cstdint>

namespace utf = boost::unit_test;
using namespace ITkStripCabling;
static const std::string defaultFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};


BOOST_AUTO_TEST_SUITE(OnlineIdGeneratorTest)

  BOOST_AUTO_TEST_CASE(OnlineIdGeneratorConstructors){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", defaultFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    auto pITkId=std::make_unique<SCT_ID>();
    BOOST_CHECK(pITkId->initialize_from_dictionary(idd) ==0);
    BOOST_CHECK_NO_THROW([[maybe_unused]] OnlineIdGenerator s(pITkId.get()));
  }

  BOOST_AUTO_TEST_CASE(OnlineIdGeneratorConversion){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", defaultFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    auto pITkId=std::make_unique<SCT_ID>();
    BOOST_CHECK(pITkId->initialize_from_dictionary(idd) ==0);
    OnlineIdGenerator s(pITkId.get());
    //endcap C, disk 0
    Identifier ec1offId(0x800380000000000ull);
    ITkStripOnlineId onId1(0x240000);
    BOOST_TEST(s(ec1offId) == onId1);
    //endcap C, disk 1
    Identifier ec2offId(0x841c60000000000ull);
    ITkStripOnlineId onId2(0x3240e01);
    BOOST_TEST(s(ec2offId) == onId2);
    //barrel C side, layer 3
    Identifier brloffId(0xad39c0000000000ull);
    ITkStripOnlineId onId3(0x1c222703);
    BOOST_TEST(s(brloffId) == onId3);
  }


BOOST_AUTO_TEST_SUITE_END()

