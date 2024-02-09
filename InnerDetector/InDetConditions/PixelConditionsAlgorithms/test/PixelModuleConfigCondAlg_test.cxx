/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/PixelModuleConfigCondAlg_test.cxx
 * @author Shaun Roe
 * @date Aug 2023
 * @brief Some tests for PixelModuleConfigCondAlg in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSALGORITHMS

#include <boost/test/unit_test.hpp>
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
//
#include "CxxUtils/checker_macros.h"

#include "TestTools/initGaudi.h"
#include "TInterpreter.h"
#include "CxxUtils/ubsan_suppress.h"
#include "CxxUtils/checker_macros.h"

#include "src/PixelModuleConfigCondAlg.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "PixelModuleConfigCondAlg_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="PixelConditionsAlgorithms/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;

constexpr double operator"" _m ( long double b ){
    return b*0.001;
} 

BOOST_AUTO_TEST_SUITE(PixelModuleConfigCondAlgTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  BOOST_AUTO_TEST_CASE(Execute, *utf::tolerance( 1e-7 )){
    PixelModuleConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("BarrelCrossTalk","{0.1,0.2,0.3,0.4}").isSuccess());
    BOOST_TEST(a.setProperty("BarrelToTThreshold","{100,200,300,400}").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    CondCont<PixelModuleData> * cc{};
    const PixelModuleData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST( conditionStore->retrieve (cc, "PixelModuleData").isSuccess() );
    //execute for the following event:
    EventContext ctx;
    EventIDBase eid (1, 0, 0, 0, 0);
    ctx.setEventID (eid);
    BOOST_TEST(a.execute(ctx).isSuccess());
    //now we have something in store to retrieve
    BOOST_TEST( conditionStore->retrieve (cc, "PixelModuleData").isSuccess() );
    //and can extract the actual "PixelModuleData"..
    BOOST_TEST (cc->find (eid, data, &range2p));
   
    
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getDefaultAnalogThreshold(barrel_ec, layer) == -1); //all -1 by default
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35); //these vary
    BOOST_TEST(data->getDefaultAnalogThresholdNoise(barrel_ec, layer) == 150); //these vary
    BOOST_TEST(data->getDefaultInTimeThreshold(barrel_ec, layer) == 5000); //these vary
    BOOST_TEST(data->getToTThreshold(barrel_ec, layer) == 200); //empty by default
    BOOST_TEST(data->getCrossTalk(barrel_ec, layer) == 0.2); //empty by default
    //following should be "std::out_of_range"; will fix later
    BOOST_CHECK_THROW(data->getThermalNoise(barrel_ec, layer), std::range_error); //empty by default
    //
    //The following methods would similarly throw, and are not tested here:
    //getNoiseOccupancy, getDisableProbability, getNoiseShape, getFEI3Latency
    BOOST_TEST(data->getDefaultQ2TotA() == 70.2f);
    BOOST_TEST(data->getDefaultQ2TotE() == -3561.25f);
    BOOST_TEST(data->getDefaultQ2TotC() == 26000.0f);
    BOOST_TEST(data->getPIXLinearExtrapolation() == false);//in c'tor of PixelModuleData
    BOOST_TEST(data->getDefaultBiasVoltage() == 150.f);
    //These methods would throw, and are not tested here:
    //getFluenceLayer() , getRadSimFluenceMapList(), getRadSimFluenceMapList3D()
    BOOST_TEST(data->getCablingMapToFile() == false);
    BOOST_TEST(data->getCablingMapFileName() == "PixelCabling/Pixels_Atlas_IdMapping_2016.dat");//why?
   
  }

BOOST_AUTO_TEST_SUITE_END()