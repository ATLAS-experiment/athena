/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripCabling/test/ITkStripCablingAlg_test.cxx
 * @author Edson Carquin
 * @date September 2024
 * @brief Some tests for ITkStripCablingAlg in the Boost framework (based on ITkPixelCabling package)
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITkStripCabling

#include <boost/test/unit_test.hpp>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/SCT_ID.h"
#include "StoreGate/ReadHandleKey.h"
#include "TestTools/initGaudi.h"

#include "src/ITkStripCablingAlg.h"
#include "src/OnlineIdGenerator.h"

#include <string>
#include <sstream>      // std::ostringstream
#include <memory>
#include <set>

namespace utf = boost::unit_test;


struct TestFixture : Athena_test::InitGaudi {
  TestFixture() :
    Athena_test::InitGaudi("ITkStripCabling/ITkStripCablingAlg_test.txt") {}
};

static const std::string itkDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};


std::pair <EventIDBase, EventContext>
getEvent(EventIDBase::number_type runNumber, EventIDBase::number_type timeStamp){
  EventIDBase::event_number_t eventNumber(0);
  EventIDBase eid(runNumber, eventNumber, timeStamp);
  EventContext ctx;
  ctx.setEventID (eid);
  return {eid, ctx};
}

std::pair<const ITkStripCablingData *, CondCont<ITkStripCablingData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<ITkStripCablingData> * cc{};
  const ITkStripCablingData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "ITkStripCablingData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}

bool
canRetrieveITkStripCablingData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<ITkStripCablingData> * cc{};
  if (not conditionStore->retrieve (cc, "ITkStripCablingData").isSuccess()){
    return false;
  }
  return true;
}

BOOST_FIXTURE_TEST_SUITE( ITkStripCablingAlgTest, TestFixture )

  //https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetDetDescr/InDetIdentifier/test/ITkStripID_test.cxx
  BOOST_AUTO_TEST_CASE(ExecuteOptions){
    {//This is just to setup the ITkStripID with a valid set of identifiers
      ServiceHandle<StoreGateSvc> detStore("StoreGateSvc/DetectorStore", "ITkStripCablingAlgTest");
      BOOST_TEST(detStore.retrieve().isSuccess());
      IdDictParser parser;
      parser.register_external_entity("InnerDetector", itkDictFilename);
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      auto pITkId=std::make_unique<SCT_ID>();
      BOOST_TEST(pITkId->initialize_from_dictionary(idd)==0);
      ITkStripCabling::OnlineIdGenerator gen(pITkId.get());
      std::ostringstream os;
      ExpandedIdentifier e{};
      struct Numerology{
        int barrelA{};
        int barrelC{};
        int endcapA{};
        int endcapC{};
      } num;
      std::set<ITkStripOnlineId> onlineIds;
      std::set<std::uint32_t> rodIds;
      for(auto i = pITkId->wafer_begin();i!=pITkId->wafer_end();++i){
        if (pITkId->is_barrel(*i)){
          if (pITkId->eta_module(*i) > 0) ++num.barrelA;
          else ++num.barrelC;
        } else {
          if (pITkId->barrel_ec(*i) > 0) ++num.endcapA;
          else ++num.endcapC;
        }
        pITkId->get_expanded_id(*i,e);
        onlineIds.insert(gen(*i));
        rodIds.insert(gen.rod(*i));
        os<<*i<<" "<<e<<", "<<gen(*i)<<"\n";
      }
      BOOST_TEST_MESSAGE("Wafer Identifiers and Expanded Identifiers {2/2/Bec/LayerDisk/Phi/Eta/Side/0/0}:");
      BOOST_TEST_MESSAGE(os.str());
      std::string stats = "nBarrel A: "+std::to_string(num.barrelA)+"\n";
      stats+= "nBarrel C : "+std::to_string(num.barrelC)+"\n";
      stats+= "nEndcap A : "+std::to_string(num.endcapA)+"\n";
      stats+= "nEndcap C : "+std::to_string(num.endcapC)+"\n";
      stats+= "n onlineId: "+std::to_string(onlineIds.size())+"\n";
      stats+= "n robs    : "+std::to_string(rodIds.size())+"\n";
      BOOST_TEST_MESSAGE(stats);
      BOOST_TEST(detStore->record(std::move(pITkId), "SCT_ID").isSuccess());
    }//Now the ITkStripID is in StoreGate, ready to be used by the cabling
    ITkStripCablingAlg a("MyAlg", svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("DataSource","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/ITkStripCabling/ITkStripCabling.dat").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "ITkStripCablingData");
    CondCont<ITkStripCablingData> * cc{};
    BOOST_TEST( canRetrieveITkStripCablingData(conditionStore));
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
    BOOST_TEST( conditionStore->retrieve (cc, "ITkStripCablingData").isSuccess() );
    const ITkStripCablingData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    BOOST_TEST (not data->empty());
    std::ostringstream s;
    s<<*data<<std::endl;
    BOOST_TEST_MESSAGE("Cabling entries: ");
    BOOST_TEST_MESSAGE(s.str());
    //
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  
BOOST_AUTO_TEST_SUITE_END();

