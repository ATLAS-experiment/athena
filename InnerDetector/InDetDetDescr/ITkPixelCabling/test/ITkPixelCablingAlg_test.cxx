/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
@file ITkPixelCabling/test/ITkPixelCablingAlg_test.cxx
@author Shaun Roe
@date May 2024
@brief Some tests for ITkPixelCablingAlg in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkPixelCabling

#include <boost/test/unit_test.hpp>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "IdDictParser/IdDictParser.h"
#include "InDetIdentifier/PixelID.h"
#include "StoreGate/ReadHandleKey.h"
#include "TestTools/initGaudi.h"

#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "CoralBase/AttributeListSpecification.h"

#include "src/ITkPixelCablingAlg.h"

#include <memory>
#include <string>

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
    BOOST_TEST(a.sysInitialize().isSuccess() );

    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "ITkPixelCablingAlgTest");

    // 1. Get RCUSvc required to create CondCont<AthenaAttributeList>
    SmartIF<Athena::IRCUSvc> rcusvc{svcLoc->service("Athena::RCUSvc")};
    BOOST_TEST(rcusvc.isValid());

    // 2. Register CondCont<AthenaAttributeList> for key "/ITk/Pixel/Identifier"
    DataObjID idKey(ClassID_traits<AthenaAttributeList>::ID(), "/ITk/Pixel/Identifier");
    auto idCondCont = std::make_unique<CondCont<AthenaAttributeList>>(*rcusvc, idKey);
    BOOST_TEST(idCondCont != nullptr);
    BOOST_TEST(conditionStore->record(std::move(idCondCont), "/ITk/Pixel/Identifier").isSuccess());

    // 3. Set up single EventContext
    EventContext ctx;
    Atlas::setExtendedEventContext(ctx, Atlas::ExtendedEventContext());

    EventIDBase::number_type runNumber(222222 - 100); // run 1
    EventIDBase::event_number_t eventNumber(0);
    EventIDBase::number_type timeStamp(0);
    EventIDBase eidRun1 (runNumber, eventNumber, timeStamp);
    ctx.setEventID (eidRun1);

    Gaudi::Hive::setCurrentContext(ctx);

    // 4. Retrieve CondCont and insert dummy payload with 3-arg insert
    CondCont<AthenaAttributeList>* ccId = nullptr;
    BOOST_TEST(conditionStore->retrieve(ccId, "/ITk/Pixel/Identifier").isSuccess());

// Create an AthenaAttributeList with the expected "cabling" specification
    coral::AttributeListSpecification* spec = new coral::AttributeListSpecification();
    spec->extend("cabling", "string");

    auto attrList = std::make_unique<AthenaAttributeList>(*spec);
    
    // Fill "cabling" with dummy/minimal valid JSON or cabling string content expected by the algorithm
    std::string dummyCablingJson = R"([
      {
        "DetectorResourceID": "802001a",
        "TrueDetectorResourceID": "802001a",
        "SourceID": "a10510"
      },
      {
        "DetectorResourceID": "802001b",
        "TrueDetectorResourceID": "802001b",
        "SourceID": "a10511"
      }
    ])";
    (*attrList)["cabling"].setValue<std::string>(dummyCablingJson);
    // Create a valid infinite Run-Event validity range
    EventIDBase startEID(0, 0, 0); // start: run 0, event 0, timestamp 0
EventIDBase stopEID(
      EventIDBase::UNDEFNUM - 1, 
      EventIDBase::UNDEFEVT - 1, 
      EventIDBase::UNDEFNUM - 1
    );
    EventIDRange infiniteRange(startEID, stopEID);

    BOOST_TEST(ccId->insert(infiniteRange, std::move(attrList), ctx).isSuccess());

    CondCont<ITkPixelCablingData> * cc{};
    BOOST_TEST( canRetrieveITkPixelCablingData(conditionStore));

    // 5. Execute algorithm
    BOOST_TEST(a.execute(ctx).isSuccess());

    // 6. Verify retrieved data
    BOOST_TEST( conditionStore->retrieve (cc, "ITkPixelCablingData").isSuccess() );
    const ITkPixelCablingData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    BOOST_TEST (not data->empty());

    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }

BOOST_AUTO_TEST_SUITE_END();