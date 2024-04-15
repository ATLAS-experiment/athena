/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelCalibAlgs/test/PixelMapping_test.cxx
 * @author Shaun Roe
 * @date March 2023
 * @brief Some tests for PixelMapping in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE PixelCalibAlgsTest

#include <boost/test/unit_test.hpp>


#include "PathResolver/PathResolver.h"
#include "ChargeCalibration/common/PixelMapping.h"

using namespace pix;

BOOST_AUTO_TEST_SUITE(PixelMappingTest)
  BOOST_AUTO_TEST_CASE(PixelMappingConstructor){
     const std::string fname("PixelCalibAlgs/mapping.csv");
     std::string file = PathResolver::find_file (fname, "DATAPATH");
     BOOST_TEST_MESSAGE("Now opening mapping file: ");
     BOOST_TEST_MESSAGE(file);
     BOOST_CHECK_NO_THROW(PixelMapping m(file));
     BOOST_CHECK_THROW(PixelMapping m("I_DontExist.csv"), std::runtime_error);     
  }
  
  BOOST_AUTO_TEST_CASE(PixelMappingMethod){
    const std::string fname("PixelCalibAlgs/mapping.csv");
    std::string file = PathResolver::find_file (fname, "DATAPATH");
    //LI_S15_A_34_M3_A7, 2036, 4, 0, 0, 0
    const std::string name{"LI_S15_A_34_M3_A7"};
    PixelMapping pm(file);
    BOOST_TEST(pm.nModules() == 2048);
    int hashID{};
    int bec{};
    int layer{};
    int phimod{};
    int etamod{};
    pm.mapping(name, &hashID, &bec, &layer, &phimod, &etamod);
    BOOST_TEST(hashID == 2036);
  }
BOOST_AUTO_TEST_SUITE_END()
