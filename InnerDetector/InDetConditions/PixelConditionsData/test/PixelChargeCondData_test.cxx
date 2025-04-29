/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelChargeCondData_test.cxx
 * @author Shaun Roe
 * @date June, 2023
 * @brief basic tests in the Boost framework for PixelChargeCondData
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include "CxxUtils/checker_macros.h"
//
#include <boost/test/unit_test.hpp>
//
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/ChargeCalibParameters.h"
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
//
BOOST_AUTO_TEST_SUITE(PixelChargeCalibCondDataTest)
  BOOST_AUTO_TEST_CASE( Construction ){
    BOOST_CHECK_NO_THROW(PixelChargeCalibCondData());
    BOOST_CHECK_NO_THROW(PixelChargeCalibCondData(100));
  }
  
  BOOST_AUTO_TEST_CASE( SetAndGet , * utf::tolerance(0.001)){
    size_t maxModuleHash(100); //normally given by the maximum hash
    unsigned int hashTooBig=200;
    PixelChargeCalibCondData calib(maxModuleHash);
    //arguments to set and get I will use:
    unsigned int moduleHash=4;
    unsigned int frontEndIdx=1;
    unsigned int frontEndIdxTooBig=15;
    InDetDD::PixelDiodeType type = InDetDD::PixelDiodeType::NORMAL;
    //
    //
    PixelChargeCalib::Thresholds oneValue{0,3,6,9};// thresh, noise, sigma, intime
    std::vector<PixelChargeCalib::Thresholds> allThresholds(5, oneValue);
    BOOST_CHECK_NO_THROW(calib.setThresholds(type,moduleHash, allThresholds));
    unsigned int moduleHashBiggerThanVector = 10;
    BOOST_CHECK_NO_THROW(calib.setThresholds(type,moduleHashBiggerThanVector, allThresholds));
    BOOST_CHECK_THROW(calib.setThresholds(type,hashTooBig, allThresholds), std::out_of_range);
    //return struct in new method
    BOOST_CHECK(calib.getThresholds(type, moduleHash, frontEndIdx).sigma == 3);
    BOOST_CHECK(calib.getThresholds(type, moduleHash, frontEndIdx).noise == 6);
    BOOST_CHECK_THROW(calib.getThresholds(type,hashTooBig,frontEndIdx), std::out_of_range);
    BOOST_CHECK_THROW(calib.getThresholds(type,moduleHash,frontEndIdxTooBig), std::out_of_range);

    //
    PixelChargeCalib::LegacyFitParameters oneFit{1.f, 2.f, 3.f, 32.f};
    std::vector< PixelChargeCalib::LegacyFitParameters > allParameters(4, oneFit);
    BOOST_CHECK_NO_THROW(calib.setLegacyFitParameters(type,moduleHash, allParameters));
    BOOST_CHECK_THROW(calib.setLegacyFitParameters(type,hashTooBig, allParameters), std::out_of_range);
    BOOST_TEST(calib.getLegacyFitParameters(type, moduleHash, frontEndIdx).C == 3.f);
    BOOST_CHECK_THROW(calib.getLegacyFitParameters(type,hashTooBig,frontEndIdx), std::out_of_range);
    BOOST_CHECK_THROW(calib.getLegacyFitParameters(type,moduleHash,frontEndIdxTooBig), std::out_of_range);
    //
    PixelChargeCalib::LinearFitParameters linFit{4.f, 5.f};
    std::vector< PixelChargeCalib::LinearFitParameters > linParameters(4, linFit);
    BOOST_CHECK_NO_THROW(calib.setLinearFitParameters(type,moduleHash, linParameters));
    BOOST_CHECK_THROW(calib.setLinearFitParameters(type,hashTooBig, linParameters), std::out_of_range);
    BOOST_TEST(calib.getLinearFitParameters(type, moduleHash, frontEndIdx).G ==  5.f);
    BOOST_CHECK_THROW(calib.getLinearFitParameters(type,hashTooBig,frontEndIdx), std::out_of_range);
    BOOST_CHECK_THROW(calib.getLinearFitParameters(type,moduleHash,frontEndIdxTooBig), std::out_of_range);
    //
    float Q=5.f;
    PixelChargeCalib::Resolutions oneResolution{20.f,30.f};
    std::vector< PixelChargeCalib::Resolutions > resolutions(4, oneResolution);
    BOOST_CHECK_NO_THROW(calib.setTotResolutions(moduleHash, resolutions));
    BOOST_CHECK_THROW(calib.setTotResolutions(hashTooBig, resolutions), std::out_of_range);
    BOOST_TEST(calib.getTotRes(moduleHash, frontEndIdx, Q) == (20.f + 5.f*30.f));
    //
    float expectedResult = 30.f*Q + 20.f;
    BOOST_TEST(calib.getTotRes(moduleHash, frontEndIdx, Q) == expectedResult);
    //round trip calculation
    float expectedTot = 0.875f;
    BOOST_TEST(calib.getToT(type, moduleHash, frontEndIdx, Q) == expectedTot);
    BOOST_TEST(calib.getCharge(type, moduleHash, frontEndIdx, expectedTot) == Q);
    //
    PixelChargeCalibCondData::CalibrationStrategy strategy = PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX;
    BOOST_CHECK_NO_THROW(calib.setCalibrationStrategy(moduleHash, strategy));
    //this should throw, but does not
    BOOST_CHECK_THROW(calib.setCalibrationStrategy(hashTooBig, strategy), std::out_of_range);
    //IBLModule is a vector of array<float,16>
    std::array<float,16> x;
    std::iota(x.begin(), x.end(), 5.f);
    PixelChargeCalibCondData::IBLModule charges{x,x};
    BOOST_CHECK_NO_THROW(calib.setTot2Charges(moduleHash, charges));
    //this should throw, but does not:
    BOOST_CHECK_THROW(calib.setTot2Charges(hashTooBig, charges), std::out_of_range);
    //
    BOOST_TEST((calib.getCalibrationStrategy(moduleHash) == PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX));
    BOOST_CHECK_THROW(calib.getCalibrationStrategy(hashTooBig), std::out_of_range); //should throw; does not
   
    
  }
  BOOST_AUTO_TEST_CASE( CalculatedQuantities, * utf::tolerance(0.00001) ){
    //preamble
    size_t maxModuleHash(100);
    unsigned int hashTooBig=200;
    unsigned int moduleHash=3;
     InDetDD::PixelDiodeType type = InDetDD::PixelDiodeType::NORMAL;
    PixelChargeCalibCondData calib(maxModuleHash);
    unsigned int frontEndIdx=1;//so only need two elements in the vectors
    const PixelChargeCalib::Thresholds thr{1,1,1,1};
    calib.setThresholds(type,moduleHash, std::vector<PixelChargeCalib::Thresholds>(2, thr));
    
    PixelChargeCalib::LegacyFitParameters oneFit{1.f, 2.f, 3.f, 32.f};
    std::vector< PixelChargeCalib::LegacyFitParameters > allParameters(4, oneFit);
    calib.setLegacyFitParameters(type,moduleHash, allParameters);
    
    PixelChargeCalib::LinearFitParameters oneLinFit{3.f, 4.f};
    std::vector< PixelChargeCalib::LinearFitParameters > allLinParameters(4, oneLinFit);
    calib.setLinearFitParameters(type,moduleHash, allLinParameters);
    
    PixelChargeCalib::Resolutions oneRes{5.f, 6.f};
    std::vector< PixelChargeCalib::Resolutions > allRes(4, oneRes);
    calib.setTotResolutions(moduleHash, allRes);
    //
    float Q=5.f;
    float expectedResult = 6.f*Q + 5.f;
    BOOST_TEST(calib.getTotRes(moduleHash, frontEndIdx, Q) == expectedResult);
    //round trip calculation
    float expectedTot = 0.875f;
    BOOST_TEST(calib.getToT(type, moduleHash, frontEndIdx, Q) == expectedTot);
    BOOST_TEST(calib.getCharge(type, moduleHash, frontEndIdx, expectedTot) == Q);
    //
    std::array<float,16> x;
    std::iota(x.begin(), x.end(), 5.f);
    PixelChargeCalibCondData::IBLModule charges{x,x};
    calib.setTot2Charges(moduleHash, charges);
    unsigned int ToT =1;
    //
    //ToT indices start from 1, not zero, and have 1 subtracted when indexing into the array
    BOOST_TEST(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, ToT) == 5.f);
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(hashTooBig, frontEndIdx, ToT), std::out_of_range);
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 0), std::out_of_range);//too small
    BOOST_TEST(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 16) == 20.f);//16 is OK
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 17), std::out_of_range);//too big
    //
    BOOST_TEST(calib.getToTLUTFEI4(moduleHash, frontEndIdx, 20.f) == 16.f);
    BOOST_CHECK_THROW(calib.getToTLUTFEI4(hashTooBig, frontEndIdx, 20.f), std::out_of_range);
    //method 'clear()' is declared, never defined
  }
BOOST_AUTO_TEST_SUITE_END();
