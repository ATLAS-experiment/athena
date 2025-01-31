
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>

#include <fstream>

using namespace ActsTrk;

namespace MuonGMR4{


StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){
        
        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 6) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 6 characters");
                continue;
            }
            /// Example string BIL1A3
            const std::string statName = token.substr(0, 3);
            const unsigned statEta = std::atoi(token.substr(3, 1).c_str()) * (token[4] == 'A' ? 1 : -1);
            const unsigned statPhi = std::atoi(token.substr(5, 1).c_str());
            bool isValid{false};
            const Identifier eleId = idHelper.elementID(statName, statEta, statPhi, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Failed to deduce a station name for " << token);
                continue;
            }
            transcriptedIds.insert(eleId);
            const Identifier secMlId = idHelper.multilayerID(eleId, 2, isValid);
            if (isValid){
                transcriptedIds.insert(secMlId);
            }
        }
        return transcriptedIds;
    };

    std::vector <std::string>& selectedSt = m_selectStat.value();
    const std::vector <std::string>& excludedSt = m_excludeStat.value();
    selectedSt.erase(std::remove_if(selectedSt.begin(), selectedSt.end(),
                     [&excludedSt](const std::string& token){
                        return std::ranges::find(excludedSt, token) != excludedSt.end();
                     }), selectedSt.end());

    if (selectedSt.size()) {
        m_testStations = translateTokenList(selectedSt);
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations) {
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    } else {
        const std::set<Identifier> excluded = translateTokenList(excludedSt);
        /// Add stations for testing
        for(auto itr = idHelper.detectorElement_begin();
                 itr!= idHelper.detectorElement_end();++itr){
            if (!excluded.count(*itr)) {
               m_testStations.insert(*itr);
            }
        }
        /// Report what stations are excluded
        if (!excluded.empty()) {
            std::stringstream excluded_report{};
            for (const Identifier& id : excluded){
                excluded_report << " *** " << m_idHelperSvc->toStringDetEl(id) << std::endl;
            }
            ATH_MSG_INFO("Test all station except the following excluded ones " << std::endl << excluded_report.str());
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
    SG::ReadHandle geoContextHandle{m_geoCtxKey, ctx};
    ATH_CHECK(geoContextHandle.isPresent());

    const ActsGeometryContext& gctx{*geoContextHandle};

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
      const Amg::Transform3D globToLocal{reElement->globalToLocalTrans(gctx)};
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
   m_chamberDesign = readoutEle->chamberDesign();
   
   m_numLayers = readoutEle->numLayers();
   m_numTubes = readoutEle->numTubesInLay();
   
   m_tubeRad = readoutEle->innerTubeRadius();
   m_tubePitch = readoutEle->tubePitch();

   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform {readoutEle->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   m_alignableNode  = readoutEle->alignableTransform()->getDefTransform();
   
   /// Loop over the tubes
   for (unsigned int lay = 1; lay <= readoutEle->numLayers(); ++lay) {
      for (unsigned int tube = 1; tube <= readoutEle->numTubesInLay(); ++tube) {
         const IdentifierHash measHash{readoutEle->measurementHash(lay,tube)};
         if (!readoutEle->isValid(measHash)) continue;
         const Amg::Transform3D& tubeTransform{readoutEle->localToGlobalTrans(gctx,measHash)};
         m_tubeLay.push_back(lay);
         m_tubeNum.push_back(tube);         
         m_tubeTransform.push_back(tubeTransform);
         m_tubePosInCh.push_back(readoutEle->msSector()->globalToLocalTrans(gctx) * 
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

