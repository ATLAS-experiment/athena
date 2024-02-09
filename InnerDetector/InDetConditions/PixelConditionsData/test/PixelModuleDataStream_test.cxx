/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/PixelModuleDataStream_test.cxx
 * @author Shaun Roe
 * @date Aug 2023
 * @brief Some tests for PixelModuleDataStream in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include <boost/test/unit_test.hpp>
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
//
#include "CxxUtils/checker_macros.h"

#include "CxxUtils/checker_macros.h"

#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelModuleDataStream.h"
#include <string>
#include <sstream>
#include <fstream>
#include "PathResolver/PathResolver.h"

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

BOOST_AUTO_TEST_SUITE(PixelModuleDataStreamTest )

  BOOST_AUTO_TEST_CASE( PixelModuleDataStreamInsertionTest ){
    PixelModuleData pmd; //all default values
    //the following is a raw string literal, will (necessarily) preserve formatting "as-is"
    const std::string expected=R"(#defaultBarrelAnalogThreshold
-1 -1 -1 -1 
#defaultEndcapAnalogThreshold
-1 -1 -1 
#defaultDBMAnalogThreshold
-1 -1 -1 
#defaultBarrelAnalogThresholdSigma
45 35 30 30 
#defaultEndcapAnalogThresholdSigma
30 30 30 
#defaultDBMAnalogThresholdSigma
70 70 70 
#defaultBarrelAnalogThresholdNoise
130 150 160 160 
#defaultEndcapAnalogThresholdNoise
150 150 150 
#defaultDBMAnalogThresholdNoise
190 190 190 
#defaultBarrelInTimeThreshold
2000 5000 5000 5000 
#defaultEndcapInTimeThreshold
5000 5000 5000 
#defaultDBMInTimeThreshold
1200 1200 1200 
#barrelToTThreshold
-1 5 5 5 
#endcapToTThreshold
5 5 5 
#DBMToTThreshold
-1 -1 -1 
#barrelCrossTalk
0.3 0.12 0.12 0.12 
#endcapCrossTalk
0.06 0.06 0.06 
#DBMCrossTalk
0.06 0.06 0.06 
#barrelThermalNoise
160 160 160 160 
#endcapThermalNoise
160 160 160 
#DBMThermalNoise
160 160 160 
#barrelNoiseOccupancy
5e-08 5e-08 5e-08 5e-08 
#endcapNoiseOccupancy
5e-08 5e-08 5e-08 
#DBMNoiseOccupancy
5e-08 5e-08 5e-08 
#barrelDisableProbability
0.009 0.009 0.009 0.009 
#endcapDisableProbability
0.009 0.009 0.009 
#DBMDisableProbability
0.009 0.009 0.009 
#barrelNoiseShape
0 0 0 0 0.2204 0.5311 0.7493 0.8954 0.998 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
#endcapNoiseShape
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
#DBMNoiseShape
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
#FEI3BarrelLatency
0 151 256 256 
#FEI3EndcapLatency
256 256 256 
#FEI3BarrelTimingSimTune
-1 2015 2015 2015 
#FEI3EndcapTimingSimTune
2015 2015 2015 
#paramA
70.2
#paramE
-3561.25
#paramC
26000
#doLinearExtrapolation
0
#biasVoltage
150
#fluenceLayer
8e+13 1.61e+14 7.1e+13 4.8e+13 
#radSimFluenceMapList
PixelDigitization/maps_IBL_PL_80V_fl0_8e14.root PixelDigitization/maps_PIX_350V_fl1_61e14.root PixelDigitization/maps_PIX_200V_fl0_71e14.root PixelDigitization/maps_PIX_150V_fl0_48e14.root 
#fluenceLayer3D
5e+15 
#radSimFluenceMapList3D
PixelDigitization/TCAD_IBL_3Dsensors_efields/phi_5e15_160V.root 
#cablingMapToFile
0
#cablingMapFileName
PixelCabling/Pixels_Atlas_IdMapping_2016.dat)";
    std::stringstream s;
    s << pmd;
    BOOST_TEST (s.str() == expected);
  }
  BOOST_AUTO_TEST_CASE( PixelModuleDataStreamExtractionTest, *utf::tolerance( 1e-7 ) ){
    PixelModuleData pmd; 
    const std::string fname="testInput.txt";
    std::string fullName = PathResolver::find_file (fname, "DATAPATH");
    BOOST_TEST_MESSAGE("Reading file "+fullName);
    std::ifstream iss(fullName);
    BOOST_TEST_REQUIRE(iss.good());
    iss>>pmd;
    const int bec = 0;
    const int layer = 1;
    BOOST_TEST(pmd.getDefaultAnalogThreshold(bec,layer) == 1);
    BOOST_TEST(pmd.getDefaultAnalogThresholdSigma(bec, layer) == 100);
    BOOST_TEST(pmd.getDefaultAnalogThresholdNoise(bec, layer) == 1000);
    BOOST_TEST(pmd.getDefaultInTimeThreshold(bec, layer) == 4);
    BOOST_TEST(pmd.getToTThreshold(bec, layer) == 5);
    BOOST_TEST(pmd.getCrossTalk(bec, layer) == 0.10);
    BOOST_TEST(pmd.getThermalNoise(bec, layer) == 200);
    BOOST_TEST(pmd.getNoiseOccupancy(bec, layer) ==  1000);
    BOOST_TEST(pmd.getDisableProbability(bec, layer)  == 0.9);
    const std::vector<float> expected{1, 0, 0, 0, 0, 0, 0.2418, 0.4397, 0.5858, 0.6949, 0.7737, 0.8414, 0.8959, 0.9414, 0.9828, 1};
    BOOST_TEST(pmd.getNoiseShape(bec, layer) == expected, boost::test_tools::per_element());
    BOOST_TEST(pmd.getFEI3Latency(bec, layer) == 200);
    BOOST_TEST(pmd.getFEI3TimingSimTune(bec, layer) == 2023);
    BOOST_TEST(pmd.getDefaultQ2TotA() == 100.);
    BOOST_TEST(pmd.getDefaultQ2TotE() == 200.);
    BOOST_TEST(pmd.getDefaultQ2TotC() == 300.);
    BOOST_TEST(pmd.getPIXLinearExtrapolation() == false);
    BOOST_TEST(pmd.getPIXLinearExtrapolation() == false);
    //
    BOOST_TEST(pmd.getFluenceLayer()[0] == 7e+13);
    //     
  }
BOOST_AUTO_TEST_SUITE_END();



