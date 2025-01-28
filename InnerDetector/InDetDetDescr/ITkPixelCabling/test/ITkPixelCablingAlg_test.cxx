/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkPixelCabling/test/ITkPixelCablingAlg_test.cxx
 * @author Shaun Roe
 * @date May 2024
 * @brief Some tests for ITkPixelCablingAlg in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkPixelCabling

#include <boost/test/unit_test.hpp>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "IdDictParser/IdDictParser.h"
#include "InDetIdentifier/PixelID.h"
#include "StoreGate/ReadHandleKey.h"
#include "TestTools/initGaudi.h"

#include "src/ITkPixelCablingAlg.h"

#include <string>
#include <memory>

namespace utf = boost::unit_test;

struct TestFixture : Athena_test::InitGaudi {
  TestFixture() :
    Athena_test::InitGaudi("ITkPixelCabling/ITkPixelCablingAlg_test.txt") {}
};


static const std::string itkDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK_HGTD_23.xml"};

std::pair <EventIDBase, EventContext>
getEvent(EventIDBase::number_type runNumber, EventIDBase::number_type timeStamp){
  EventIDBase::event_number_t eventNumber(0);
  EventIDBase eid(runNumber, eventNumber, timeStamp);
  EventContext ctx;
  ctx.setEventID (eid);
  return {eid, ctx};
}

std::pair<const ITkPixelCablingData *, CondCont<ITkPixelCablingData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<ITkPixelCablingData> * cc{};
  const ITkPixelCablingData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "ITkPixelCablingData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}

bool
canRetrieveITkPixelCablingData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<ITkPixelCablingData> * cc{};
  if (not conditionStore->retrieve (cc, "ITkPixelCablingData").isSuccess()){
    return false;
  }
  return true;
}

BOOST_FIXTURE_TEST_SUITE( ITkPixelCablingAlgTest, TestFixture )

  //https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetDetDescr/InDetIdentifier/test/ITkPixelID_test.cxx
  BOOST_AUTO_TEST_CASE(ExecuteOptions){
    {//This is just to setup the ITkPixelID with a valid set of identifiers
      ServiceHandle<StoreGateSvc> detStore("StoreGateSvc/DetectorStore", "ITkPixelCablingAlgTest");
      BOOST_TEST(detStore.retrieve().isSuccess());
      IdDictParser parser;
      parser.register_external_entity("InnerDetector", itkDictFilename);
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      auto pITkId=std::make_unique<PixelID>();
      BOOST_TEST(pITkId->initialize_from_dictionary(idd)==0);
      BOOST_TEST(detStore->record(std::move(pITkId), "PixelID").isSuccess());
    }//Now the ITkPixelID is in StoreGate, ready to be used by the cabling
    ITkPixelCablingAlg a("MyAlg", svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("DataSource","ITkPixelCabling.dat").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "ITkPixelCablingAlgTest");
    CondCont<ITkPixelCablingData> * cc{};
    BOOST_TEST( canRetrieveITkPixelCablingData(conditionStore));
    //execute for the following event:
    EventContext ctx;
    //
    EventIDBase::number_type runNumber(222222 - 100);//run 1
    EventIDBase::event_number_t eventNumber(0);
    EventIDBase::number_type timeStamp(0);
    EventIDBase eidRun1 (runNumber, eventNumber, timeStamp);
    ctx.setEventID (eidRun1);
    BOOST_TEST(a.execute(ctx).isSuccess());
     //now we have something in store to retrieve
    BOOST_TEST( conditionStore->retrieve (cc, "ITkPixelCablingData").isSuccess() );
    const ITkPixelCablingData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    BOOST_TEST (not data->empty());
    //
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  
BOOST_AUTO_TEST_SUITE_END();

