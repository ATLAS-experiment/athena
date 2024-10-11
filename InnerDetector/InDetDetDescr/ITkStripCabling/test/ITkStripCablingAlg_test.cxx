/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "src/ITkStripCablingAlg.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <sstream>      // std::ostringstream
#include <memory>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "ITkStripCablingAlg_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="ITkStripCabling/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;

static const std::string itkDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK_HGTD_23.xml"};

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

BOOST_AUTO_TEST_SUITE(ITkStripCablingAlgTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  
  //https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetDetDescr/InDetIdentifier/test/ITkStripID_test.cxx
  BOOST_AUTO_TEST_CASE(ExecuteOptions){
    {//This is just to setup the ITkStripID with a valid set of identifiers
      const ServiceLocatorHelper helper{*(g.svcLoc), "HELPER"};
      IService* iSvc{helper.service("StoreGateSvc/DetectorStore", true /*quiet*/ , true /*createIf*/)};
      StoreGateSvc* detStore{dynamic_cast<StoreGateSvc*>(iSvc)};
      IdDictParser parser;
      parser.register_external_entity("InnerDetector", itkDictFilename);
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      auto pITkId=std::make_unique<SCT_ID>();
      BOOST_TEST(pITkId->initialize_from_dictionary(idd)==0);
      std::ostringstream os;
      ExpandedIdentifier e{};
      for(auto i = pITkId->wafer_begin();i!=pITkId->wafer_end();++i){
        pITkId->get_expanded_id(*i,e);
        os<<*i<<" "<<e<<"\n";
      }
      BOOST_TEST_MESSAGE("Wafer Identifiers and Expanded Identifiers {2/2/Bec/LayerDisk/Phi/Eta/Side/0/0}:");
      BOOST_TEST_MESSAGE(os.str());
      BOOST_TEST(detStore->record(std::move(pITkId), "SCT_ID").isSuccess());
    }//Now the ITkStripID is in StoreGate, ready to be used by the cabling
    ITkStripCablingAlg a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("DataSource","ITkStripCabling.dat").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "ITkStripCablingData");
    CondCont<ITkStripCablingData> * cc{};
    BOOST_TEST( canRetrieveITkStripCablingData(conditionStore));
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

