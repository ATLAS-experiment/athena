/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelReadoutDefinitions/test/PixelReadoutDefinitions_test.cxx
 * @author Shaun Roe
 * @date July, 2023
 * @brief Some tests for PixelReadoutDefinitions  enum2uint
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELREADOUTDEFINITIONS


#include <boost/test/unit_test.hpp>
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h"

#include <stdexcept>
#include <string>
//

using namespace InDetDD;

BOOST_AUTO_TEST_SUITE(PixelReadoutDefinitionsTest)
  BOOST_AUTO_TEST_CASE(enum2uint_test){
    BOOST_TEST(enum2uint(PixelModuleType::IBL_PLANAR) == 1);
    BOOST_TEST(enum2uint(PixelDiodeType::LONG) == 1);
    BOOST_TEST(enum2uint(PixelReadoutTechnology::FEI4) == 1);
    BOOST_CHECK_THROW(enum2uint(PixelModuleType::NONE), std::out_of_range);
  }
  
   BOOST_AUTO_TEST_CASE(TypenamesTest){
    BOOST_TEST(PixelModuleTypeName(PixelModuleType::IBL_PLANAR) == "IBL_PLANAR");
    BOOST_TEST(PixelDiodeTypeName(PixelDiodeType::LONG) == "LONG");
    BOOST_TEST(PixelReadoutTechnologyName(PixelReadoutTechnology::FEI4) == "FEI4");
    BOOST_TEST(PixelModuleTypeName(PixelModuleType::NONE) == "unknown");
  }
  
BOOST_AUTO_TEST_SUITE_END()
