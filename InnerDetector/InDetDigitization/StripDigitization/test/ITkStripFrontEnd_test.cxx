/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file ITkStripFrontEnd/test/ITkStripFrontEnd_test.cxx
 * @author Shaun Roe
 * @date Oct, 2024
 * @brief Some tests for ITkStripFrontEnd algtool in the Boost framework
 */
 
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE StripDigitizationTest
#include <boost/test/unit_test.hpp>

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY; // This test uses global svcLoc.


#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"

#include "TInterpreter.h"
//
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/ubsan_suppress.h"
#include "IdDictParser/IdDictParser.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "src/ITkStripFrontEnd.h"

#include "src/ITkStripFrontEnd.h"

static const std::string sctDictFilename{"InDetIdDictFiles/IdDictInnerDetector_IBL3D25-03.xml"};

// Gaudi fixture
class GaudiFixture {
 public:
   ISvcLocator * 
   svcLoc(){
     return m_svcLoc;
   }
   
   IToolSvc *
   toolSvc(){
     return m_toolSvc;
   }
   
   StoreGateSvc* 
   storeGateSvc(){
    return m_sg;
   }
   StoreGateSvc* 
   detStore(){
    return m_detStore;
   }

   explicit GaudiFixture(const std::string & joPath = "ITkStripFrontEnd/ITkStripFrontEnd_test.txt") {
     setUpGaudi(joPath);
   }
 
   ~GaudiFixture() {
     tearDownGaudi();
   }
   
 private:
   void 
   setUpGaudi(const std::string & joPath) {
     CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
     m_appMgr = Gaudi::createApplicationMgr();
     m_svcLoc = m_appMgr;
     m_svcMgr = m_appMgr;
     m_propMgr = m_appMgr;
     m_propMgr->setProperty( "EvtSel",         "NONE" ).ignore() ;
     m_propMgr->setProperty( "JobOptionsType", "FILE" ).ignore();
     m_propMgr->setProperty( "JobOptionsPath", joPath ).ignore();
     m_toolSvc = m_svcLoc->service("ToolSvc");
     m_appMgr->configure().ignore();
     m_appMgr->initialize().ignore();
     m_sg = m_svcLoc->service ("StoreGateSvc");
     m_detStore = m_svcLoc->service ("StoreGateSvc/DetectorStore");
   }
 
   void 
   tearDownGaudi() {
     m_svcMgr->finalize().ignore();
     m_appMgr->finalize().ignore();
     m_appMgr->terminate().ignore();
     m_svcLoc->release();
     m_svcMgr->release();
     Gaudi::setInstance( static_cast<IAppMgrUI*>(nullptr) );
   }
 
   StoreGateSvc* 
   evtStore(){
     return m_sg;
   }
 
   //member variables for Core Gaudi components
   IAppMgrUI*               m_appMgr{nullptr};
   SmartIF<ISvcLocator>     m_svcLoc;
   SmartIF<ISvcManager>     m_svcMgr;
   SmartIF<IToolSvc>        m_toolSvc;
   SmartIF<IProperty>       m_propMgr;
   SmartIF<StoreGateSvc>    m_sg;
   SmartIF<StoreGateSvc>    m_detStore;
 };

BOOST_AUTO_TEST_SUITE(TEST_ITkStripFrontEnd)
  GaudiFixture g("StripDigitization/ITkStripFrontEnd_test.txt");
  auto  pSvcLoc=g.svcLoc();
  auto  pToolSvc=g.toolSvc();
  auto  pDetStore=g.detStore();
  IAlgTool* pToolInterface{};
  IAlgTool* pToolAmp{nullptr};
  
  BOOST_AUTO_TEST_CASE( sanityCheck ){
    const bool svcLocatorIsOk=(pSvcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
    const bool toolSvcIsOk = ( pToolSvc != nullptr);
    BOOST_TEST(toolSvcIsOk);
    const bool detStoreIsOk = (pDetStore != nullptr);
    BOOST_TEST(detStoreIsOk);
  }
  
  BOOST_AUTO_TEST_CASE(retrieveTool){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", sctDictFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    auto sctId = std::make_unique<SCT_ID>();
    BOOST_TEST(sctId->initialize_from_dictionary (idd) == 0);
    //put the SCT_ID helper in Storegate, ready for the class to pick up
    BOOST_TEST ( pDetStore->record (std::move (sctId), "SCT_ID").isSuccess() );
    auto sctMgr = std::make_unique<InDetDD::SCT_DetectorManager>(pDetStore);
    //put the SCT_DetectorManager in Storegate, ready for the class to pick up
    BOOST_TEST ( pDetStore->record (std::move (sctMgr), "SCT").isSuccess() );
    BOOST_TEST ( pToolSvc->retrieveTool("ITkStripFrontEnd", pToolInterface).isSuccess());
    BOOST_TEST(pToolInterface -> initialize());
  }

  BOOST_AUTO_TEST_CASE(process){

  ITkStripFrontEndData data;

  // Check number of strips in design and from manager(max number of strips on any module)
  // The design value should always be equal or lower than the manager one
  // However, no resising is now done in case of a lower value
  // const int strip_max = p_design->cells();
  const int strip_max = 8;
  // Init vectors
  auto pToolFrontEnd = dynamic_cast<ITkStripFrontEnd*>(pToolInterface);
  pToolFrontEnd->initVectors(strip_max, data);

  // Contains strip hit info, reset to 0 for each wafer processed
  data.m_StripHitsOnWafer.assign(strip_max, 0);

  // Containes the charge for each bin on each hit strip

  for (int i = 0; i < strip_max; ++i) {
      data.m_Analogue[0][i] = 0.0;
      data.m_Analogue[1][i] = 0.0;
      data.m_Analogue[2][i] = 0.0;
    }

  }
  
BOOST_AUTO_TEST_SUITE_END()

