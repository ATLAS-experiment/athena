/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelDCSHVData_test.cxx
 * @author Shaun Roe
 * @date April, 2025
 * @brief basic tests in the Boost framework for PixelDCSHVData
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include "CxxUtils/checker_macros.h"
//
#include <boost/test/unit_test.hpp>
//
#include "PixelConditionsData/PixelDCSHVData.h"

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
//
BOOST_AUTO_TEST_SUITE(PixelDCSHVDataTest)
  BOOST_AUTO_TEST_CASE( CanBeDefaultConstructed ){
    BOOST_CHECK_NO_THROW(PixelDCSHVData());
  }
  
  BOOST_AUTO_TEST_CASE(DefaultConstructedHasExpectedProperties){
    PixelDCSHVData hv;
    int chanNum{100};
    BOOST_TEST(hv.useDefault() == false);
    BOOST_TEST(hv.defaultVoltage() == 150.f);
    BOOST_TEST(hv.getBiasVoltage(chanNum) == 0.f);
  }
  BOOST_AUTO_TEST_CASE(CanSetAndRetrieveChannelVoltages){
    PixelDCSHVData hv;
    BOOST_CHECK_NO_THROW(hv.setBiasVoltage(1, 20.f));
    BOOST_CHECK_NO_THROW(hv.setChannelToDefault(2));
    BOOST_TEST(hv.getBiasVoltage(1) == 20.f);
    BOOST_TEST(hv.getBiasVoltage(2) == 150.f);
  }
  
  BOOST_AUTO_TEST_CASE(CanSetAndUseDefaultValues){
    PixelDCSHVData hv;
    int chanNum{100};
    BOOST_CHECK_NO_THROW(hv.useDefault(true));
    BOOST_TEST(hv.useDefault() == true);
    BOOST_CHECK_NO_THROW(hv.defaultVoltage(200.f));
    BOOST_TEST(hv.defaultVoltage() == 200.f);
    BOOST_TEST(hv.getBiasVoltage(chanNum) == 200.f);
  }
BOOST_AUTO_TEST_SUITE_END();
