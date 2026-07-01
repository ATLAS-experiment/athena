/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/ITkPixFieldMapsAlg_test.cxx
 * @author Shaun Roe
 * @date June 2026
 * @brief Some tests for ITkPixFieldMapsAlg in the Boost framework
 * To see the full output:
 * $TestArea/InnerDetector/InDetConditions/PixelConditionsAlgorithms/test-bin/ITkPixFieldMapsAlg_test.exe -l all
 * 
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

#include "src/ITkPixFieldMapsAlg.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "ITkPixFieldMapsAlg_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="PixelConditionsAlgorithms/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;


//from EventIDBase
typedef unsigned int number_type;
typedef uint64_t     event_number_t;


bool
canRetrieveITkFieldData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<ITkPixFieldMaps> * cc{};
  if (not conditionStore->retrieve (cc, "ITkPixFieldMaps").isSuccess()){
    return false;
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(ITkPixFieldMapsAlgTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  BOOST_AUTO_TEST_CASE(Initialise){
    ITkPixFieldMapsAlg a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("ITkPixFieldMapsKey","ITkPixFieldMaps").isSuccess());
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "ITkPixFieldMaps");
    BOOST_TEST( canRetrieveITkFieldData(conditionStore));
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  //need test for execute, but thats more work... to be done!
   
 
 
BOOST_AUTO_TEST_SUITE_END();
