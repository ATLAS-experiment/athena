/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkPixSimulationParameters_test.cxx
 * @author Shaun Roe
 * @date April, 2025
 * @brief basic tests in the Boost framework for ITkPixSimulationParameters
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include "CxxUtils/checker_macros.h"
//
#include <boost/test/unit_test.hpp>
//
#include "PixelConditionsData/ITkPixSimulationParameters.h"

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
//
BOOST_AUTO_TEST_SUITE(ITkPixSimulationParametersTest)
  BOOST_AUTO_TEST_CASE( CanBeDefaultConstructed ){
    BOOST_CHECK_NO_THROW(ITkPixSimulationParameters());
  }
  
  BOOST_AUTO_TEST_CASE(DefaultConstructedHasExpectedProperties, * utf::tolerance(0.00001)){
    ITkPixSimulationParameters chip;
    BOOST_TEST(chip.noiseShape().size() == 2);
    BOOST_TEST(chip.totThreshold() == -1);
    BOOST_TEST(chip.crossTalk() == 0.06);
    BOOST_TEST(chip.disableProbability() == 9e-3);
    BOOST_TEST(chip.noiseOccupancy() == 5e-8);
    //Stream output:
    //ToT Threshold = -1; XTalk = 0.06; p(Disable) = 0.009; noiseOcc. = 5e-08; Noise shape: 0 1 
    BOOST_TEST_MESSAGE(chip);
  }
 
BOOST_AUTO_TEST_SUITE_END();
