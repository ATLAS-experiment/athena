
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;
namespace MuonGMR4{

GeoModelMdtTest::GeoModelMdtTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string BIL1A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId =
            id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
        /// Add the second multilayer if possible
        const Identifier secMl = id_helper.multilayerID(eleId, 2, is_valid);
        if (is_valid)
            m_testStations.insert(secMl);
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
           m_testStations.insert(*itr);
        }
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    const ActsGeometryContext& gctx{**geoContextHandle};

    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    for (const Identifier& test_me : m_testStations) {
      const int ml = id_helper.multilayer(test_me);
      const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
      ATH_MSG_DEBUG("Test retrieval of Mdt detector element "<<detStr);
      const MdtReadoutElement* reElement = m_detMgr->getMdtReadoutElement(test_me);
      if (!reElement) {
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<detStr<<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      if (!Amg::doesNotDeform(transClosure)) {
            ATH_MSG_FATAL("Closure test failed for "<<detStr<<". Ended up with "<< Amg::toString(transClosure) );
            return StatusCode::FAILURE;         
      }
      for (unsigned int lay = 1 ; lay <= reElement->numLayers() ; ++lay ) {
         for (unsigned int tube = 1; tube <=reElement->numTubesInLay(); ++tube ){
            const Identifier tube_id = id_helper.channelID(test_me,ml,lay,tube);                 
            /// Test the forward -> backward conversion
            const IdentifierHash measHash = reElement->measurementHash(tube_id);
            const Identifier cnv_tube_id  = reElement->measurementId(measHash);
            if (tube_id != cnv_tube_id) {
               ATH_MSG_FATAL("Failed to convert "<<m_idHelperSvc->toString(tube_id)<<" back and forth "<<m_idHelperSvc->toString(cnv_tube_id));
               return StatusCode::FAILURE;
            }
         }
      }      
   }
   dumpReadoutSideXML();
   return StatusCode::SUCCESS;
}
void GeoModelMdtTest::dumpReadoutSideXML() const {
   if (m_swapRead.empty()) return;
   std::ofstream swapReadXML{m_swapRead};
   if (!swapReadXML.good()) {
      ATH_MSG_ERROR("Failed to create "<<m_swapRead);
      return;
   }
   std::set<Identifier> chamberIDs{};
   const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
   swapReadXML<<"<Table name=\"MdtTubeROSides\">"<<std::endl;
   unsigned int counter{1};
   for (MdtIdHelper::const_id_iterator itr = idHelper.detectorElement_begin();
                                       itr != idHelper.detectorElement_end(); 
                                       ++itr){
      const Identifier swap{*itr};
      const MdtReadoutElement* readoutEle = m_detMgr->getMdtReadoutElement(swap);
      if(!readoutEle) continue;
      if (!chamberIDs.insert(idHelper.elementID(swap)).second) continue;
      const int side = readoutEle->getParameters().readoutSide;
      swapReadXML<<"    <Row   ";
      swapReadXML<<"MDTTUBEROSIDES_DATA_ID=\""<<counter<<"\" ";
      swapReadXML<<"stationName=\""<<m_idHelperSvc->stationNameString(swap)<<"\" ";
      swapReadXML<<"stationEta=\""<<m_idHelperSvc->stationEta(swap)<<"\" ";
      swapReadXML<<"stationPhi=\""<<m_idHelperSvc->stationPhi(swap)<<"\" ";
      swapReadXML<<"side=\""<<side<<"\" ";
      swapReadXML<<"/>"<<std::endl;
      ++counter;
   }
   swapReadXML<<"</Table>"<<std::endl;

}
StatusCode GeoModelMdtTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const MdtReadoutElement* readoutEle) {

   m_stIndex = readoutEle->stationName();
   m_stEta = readoutEle->stationEta();
   m_stPhi = readoutEle->stationPhi();
   m_stML = readoutEle->multilayer();
   
   m_numLayers = readoutEle->numLayers();
   m_numTubes = readoutEle->numTubesInLay();
   
   m_tubeRad = readoutEle->innerTubeRadius();
   m_tubePitch = readoutEle->tubePitch();

   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform {readoutEle->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   
   /// Loop over the tubes
   for (unsigned int lay = 1; lay <= readoutEle->numLayers(); ++lay) {
      for (unsigned int tube = 1; tube <= readoutEle->numTubesInLay(); ++tube) {
         const IdentifierHash measHash{readoutEle->measurementHash(lay,tube)};
         if (!readoutEle->isValid(measHash)) continue;
         const Amg::Transform3D& tubeTransform{readoutEle->localToGlobalTrans(gctx,measHash)};
         m_tubeLay.push_back(lay);
         m_tubeNum.push_back(tube);         
         m_tubeTransform.push_back(tubeTransform);
         m_tubePosInCh.push_back(m_surfaceProvTool->globalToChambCenter(gctx, readoutEle->identify()) * 
                                 readoutEle->center(gctx, measHash));
         m_roPos.push_back(readoutEle->readOutPos(gctx, measHash));
         m_tubeLength.push_back(readoutEle->tubeLength(measHash));
         m_activeTubeLength.push_back(readoutEle->activeTubeLength(measHash));
         m_wireLength.push_back(readoutEle->wireLength(measHash));
      }
   }

   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

