/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_Cabling/test/SCT_CablingCondAlgFromText_test.cxx
 * @author Shaun Roe
 * @date May 2024
 * @brief Some tests for SCT_CablingCondAlgFromText in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_SCT_Cabling

#include <boost/test/unit_test.hpp>
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
//
#include "CxxUtils/checker_macros.h"

#include "TestTools/initGaudi.h"
#include "TInterpreter.h"
#include "CxxUtils/ubsan_suppress.h"
#include "CxxUtils/checker_macros.h"

#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/SCT_ID.h"
#include "src/SCT_CablingCondAlgFromText.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <memory>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "SCT_CablingCondAlgFromText_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="SCT_Cabling/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;

static const std::string sctDictFilename{"InDetIdDictFiles/IdDictInnerDetector_IBL3D25-03.xml"};

//from EventIDBase
typedef unsigned int number_type;
typedef uint64_t     event_number_t;

std::pair <EventIDBase, EventContext>
getEvent(number_type runNumber, number_type timeStamp){
  event_number_t eventNumber(0);
  EventIDBase eid(runNumber, eventNumber, timeStamp);
  EventContext ctx;
  ctx.setEventID (eid);
  return {eid, ctx};
}

std::pair<const SCT_CablingData *, CondCont<SCT_CablingData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<SCT_CablingData> * cc{};
  const SCT_CablingData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "SCT_CablingData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}

bool
canRetrieveSCT_CablingData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<SCT_CablingData> * cc{};
  if (not conditionStore->retrieve (cc, "SCT_CablingData").isSuccess()){
    return false;
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(SCT_CablingCondAlgFromTextTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  
  //https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetDetDescr/InDetIdentifier/test/SCT_ID_test.cxx
  BOOST_AUTO_TEST_CASE(ExecuteOptions){
    {//This is just to setup the SCT_ID with a valid set of identifiers
      const ServiceLocatorHelper helper{*(g.svcLoc), "HELPER"};
      IService* iSvc{helper.service("StoreGateSvc/DetectorStore", true /*quiet*/ , true /*createIf*/)};
      StoreGateSvc* detStore{dynamic_cast<StoreGateSvc*>(iSvc)};
      IdDictParser parser;
      parser.register_external_entity("InnerDetector", sctDictFilename);
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      auto pSctId=std::make_unique<SCT_ID>();
      BOOST_TEST(pSctId->initialize_from_dictionary(idd)==0);
      BOOST_TEST(detStore->record(std::move(pSctId), "SCT_ID").isSuccess());
    }//Now the SCT_ID is in StoreGate, ready to be used by the cabling
    SCT_CablingCondAlgFromText a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("DataSource","SCT_Cabling_svc.dat").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "SCT_CablingData");
    CondCont<SCT_CablingData> * cc{};
    BOOST_TEST( canRetrieveSCT_CablingData(conditionStore));
    //execute for the following event:
    EventContext ctx;
    //
    number_type runNumber(222222 - 100);//run 1
    event_number_t eventNumber(0);
    number_type timeStamp(0);
    EventIDBase eidRun1 (runNumber, eventNumber, timeStamp);
    ctx.setEventID (eidRun1);
    BOOST_TEST(a.execute(ctx).isSuccess());
     //now we have something in store to retrieve
    BOOST_TEST( conditionStore->retrieve (cc, "SCT_CablingData").isSuccess() );
    const SCT_CablingData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    //
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  
BOOST_AUTO_TEST_SUITE_END();
