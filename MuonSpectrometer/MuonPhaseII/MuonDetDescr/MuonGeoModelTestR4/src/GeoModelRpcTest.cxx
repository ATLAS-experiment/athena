
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelRpcTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;
namespace MuonGMR4{

GeoModelRpcTest::GeoModelRpcTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelRpcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    
    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
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
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, 1, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.stationName(id) == id_helper.stationName(eleId) &&
                                   id_helper.stationEta(id) == id_helper.stationEta(eleId) &&
                                   id_helper.stationPhi(id) == id_helper.stationPhi(eleId);
                        });
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()){
        std::copy(id_helper.detectorElement_begin(), 
                  id_helper.detectorElement_end(), 
                  std::inserter(m_testStations, m_testStations.end()));
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations){
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    // const ActsGeometryContext& gctx{**geoContextHandle};
    ActsGeometryContext gctx{};
    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of Rpc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const RpcReadoutElement* reElement = m_detMgr->getRpcReadoutElement(test_me);
      if (!reElement) {         
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                      <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      if (!Amg::doesNotDeform(transClosure)) {
        ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)
                    <<". Ended up with "<< Amg::toString(transClosure) );
        return StatusCode::FAILURE;                  
      }
      const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
      for (unsigned int gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (int doubPhi = reElement->doubletPhi(); doubPhi <= reElement->doubletPhiMax(); ++doubPhi) {
            for (bool measPhi: {false, true}) {
                unsigned int numStrip =  (measPhi ? reElement->nPhiStrips() :
                                                    reElement->nEtaStrips());
                for (unsigned int strip = 1; strip < numStrip ; ++strip) {
                    bool isValid{false};
                    const Identifier chId = id_helper.channelID(reElement->identify(),
                                                                reElement->doubletZ(),
                                                                doubPhi, gasGap, measPhi, strip, isValid);
                    if (!isValid) {
                        continue;
                    }
                    /// Test the back and forth conversion of the Identifier
                    const IdentifierHash measHash = reElement->measurementHash(chId);
                    const IdentifierHash layHash = reElement->layerHash(chId);
                    ATH_MSG_VERBOSE("gasGap: "<<gasGap<<", doubletPhi: "<<doubPhi<<", measPhi: "<<measPhi
                               <<" --> layerHash: "<<static_cast<unsigned>(layHash));
                    const Identifier backCnv = reElement->measurementId(measHash);
                    if (backCnv != chId) {
                        ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chId)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                        return StatusCode::FAILURE;
                    }
                    if (layHash != reElement->layerHash(measHash)) {
                        ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                    m_idHelperSvc->toString(chId)<<" leadds to different layer hashes "<<
                                    layHash<<" vs. "<< reElement->layerHash(measHash));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chId)<<" strip position "
                                            <<Amg::toString(reElement->stripPosition(gctx, measHash)));
                }                
            }
        }
    }   
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const RpcReadoutElement* reElement){
   
   m_stIndex    = reElement->stationName();
   m_stEta      = reElement->stationEta();
   m_stPhi      = reElement->stationPhi();
   m_doubletR   = reElement->doubletR();
   m_doubletZ   = reElement->doubletZ();
   m_doubletPhi = reElement->doubletPhi();
   m_chamberDesign = reElement->chamberDesign();
   
   m_numRpcLayers = reElement->nGasGaps();

   m_numGasGapsPhi = reElement->nPhiPanels();
   m_numPhiPanels = reElement->nPhiPanels();

   ///
   m_numStripsEta = reElement->nEtaStrips();
   m_numStripsPhi = reElement->nPhiStrips();
   
   m_stripEtaPitch = reElement->stripEtaPitch();
   m_stripPhiPitch = reElement->stripPhiPitch();
   m_stripEtaWidth = reElement->stripEtaWidth();
   m_stripPhiWidth = reElement->stripPhiWidth();
   m_stripEtaLength = reElement->stripEtaLength(); 
   m_stripPhiLength = reElement->stripPhiLength();     
 
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
      
   for (unsigned int gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (int doubPhi = reElement->doubletPhi(); doubPhi <= reElement->doubletPhiMax(); ++doubPhi) {
            for (bool measPhi: {false, true}) {
                unsigned int numStrip =  (measPhi ? reElement->nPhiStrips() :
                                                    reElement->nEtaStrips());
                for (unsigned int strip = 1; strip <= numStrip ; ++strip) {

                    bool isValid{false};
                    const Identifier stripID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->doubletZ(),
                                                                   doubPhi, gasGap, measPhi, strip, isValid);
                    if (!isValid) {
                        ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                       <<" gap: "<<gasGap<<" strip: "<<strip<<" meas phi: "<<measPhi);
                        continue;
                    }
                    m_stripPos.push_back(reElement->stripPosition(gctx, stripID));
                    m_stripPosGasGap.push_back(gasGap);
                    m_stripPosMeasPhi.push_back(measPhi);
                    m_stripPosNum.push_back(strip);
                    m_stripDblPhi.push_back(doubPhi);

                    if (strip != 1) continue;
                    const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, stripID);
                    m_stripRot.push_back(locToGlob);
                    m_stripRotGasGap.push_back(gasGap);
                    m_stripRotMeasPhi.push_back(measPhi);
                    m_stripRotDblPhi.push_back(doubPhi);                
                }
            }
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

