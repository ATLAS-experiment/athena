/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_HGTDMapping

#include <boost/test/unit_test.hpp>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "IdDictParser/IdDictParser.h"  
#include "HGTD_Identifier/HGTD_ID.h"
#include "StoreGate/ReadHandleKey.h"
#include "TestTools/initGaudi.h"

#include "src/HGTDMappingAlg.h"


#include <string>
#include <sstream>      // std::ostringstream
#include <memory>
#include <set>

namespace utf = boost::unit_test;

struct TestFixture : Athena_test::InitGaudi {
  TestFixture() :
    Athena_test::InitGaudi("HGTDMapping/HGTDMappingAlg_test.txt") {}
};

static const std::string DictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK_HGTD_23.xml"};

std::pair <EventIDBase, EventContext>
getEvent(EventIDBase::number_type runNumber, EventIDBase::number_type timeStamp){
  EventIDBase::event_number_t eventNumber(0);
  EventIDBase eid(runNumber, eventNumber, timeStamp);
  EventContext ctx;
  ctx.setEventID (eid);
  return {eid, ctx};
}

std::pair<const HGTDMappingData *, CondCont<HGTDMappingData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<HGTDMappingData> * cc{};
  const HGTDMappingData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "HGTDMappingData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}


bool
canRetrieveHGTDMappingData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<HGTDMappingData> * cc{};
  if (not conditionStore->retrieve (cc, "HGTDMappingData").isSuccess()){
    return false;
  }
  return true;
}


BOOST_FIXTURE_TEST_SUITE( HGTDMappingAlgTest, TestFixture )

  BOOST_AUTO_TEST_CASE(ExecuteOptions){
    {//This is just to setup the HGTD_ID with a valid set of identifiers
      ServiceHandle<StoreGateSvc> detStore("StoreGateSvc/DetectorStore", "HGTDMappingAlgTest");
      BOOST_TEST(detStore.retrieve().isSuccess());
      IdDictParser parser;
      parser.register_external_entity("InnerDetector", DictFilename);
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      auto pId=std::make_unique<HGTD_ID>();
      BOOST_TEST(pId->initialize_from_dictionary(idd)==0);
      BOOST_TEST(detStore->record(std::move(pId), "HGTD_ID").isSuccess());
    }//Now the HGTD_ID is in StoreGate, ready to be used by the cabling
    HGTDMappingAlg a("MyAlg", svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("DataSource","HGTDData.dat").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "HGTDMappingAlgTest");
    CondCont<HGTDMappingData> * cc{};
    BOOST_TEST( canRetrieveHGTDMappingData(conditionStore));
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
    BOOST_TEST( conditionStore->retrieve (cc, "HGTDMappingData").isSuccess() );
    const HGTDMappingData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    BOOST_TEST (not data->empty());
    //
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  
BOOST_AUTO_TEST_SUITE_END();
