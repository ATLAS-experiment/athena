
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelRpcTest.h"
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>
#include <fstream>

namespace {
// Splitting this out into a separate function avoids a false positive
// out-of-bounds array access warning from gcc14 in the archflag build.
// (It should also be addressed in versions of Eigen after 3.4.)
std::string stripPosToString (const MuonGMR4::RpcReadoutElement* reElement,
                              const ActsTrk::GeometryContext& gctx,
                              const IdentifierHash measHash)
{
  Amg::Vector3D v = reElement->stripPosition(gctx, measHash);
  return Amg::toString(v);
}
}

using namespace ActsTrk;
namespace MuonGMR4{


StatusCode GeoModelRpcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    if (m_visualStrips) {
        m_clientToken.preFixName="GeoModelRpcTest";
        m_clientToken.subDirectory = "RpcPlots";
        m_clientToken.canvasLimit = -1;
        m_clientToken.drawSqrtS = false;
        ATH_CHECK(m_visualSvc.retrieve());
        ATH_CHECK(m_visualSvc->registerClient(m_clientToken));
    }
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){
        
        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 6) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 6 characters");
                continue;
            }
            /// Example string BML1A3
            const std::string statName = token.substr(0, 3);
            const unsigned statEta = std::atoi(token.substr(3, 1).c_str()) * (token[4] == 'A' ? 1 : -1);
            const unsigned statPhi = std::atoi(token.substr(5, 1).c_str());
            bool isValid{false};
            const Identifier eleId = idHelper.elementID(statName, statEta, statPhi, 1, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Failed to deduce a station name for " << token);
                continue;
            }
            transcriptedIds.insert(eleId);
            std::copy_if(idHelper.detectorElement_begin(), idHelper.detectorElement_end(), 
                           std::inserter(transcriptedIds, transcriptedIds.end()), 
                           [&eleId, &idHelper](const Identifier& copyMe){ 
                            return idHelper.stationName(copyMe) == idHelper.stationName(eleId) &&
                                   idHelper.stationEta(copyMe) == idHelper.stationEta(eleId) &&
                                   idHelper.stationPhi(copyMe) == idHelper.stationPhi(eleId);
            });
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
StatusCode GeoModelRpcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
void GeoModelRpcTest::visualizeStripPanel(const EventContext& ctx,
                                          const StripDesignPtr& design,
                                          const Identifier& detId,
                                          const bool measPhi) const {
    if (!design  || !m_visualStrips) {
        return;
    }
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};          
    const std::string chName = std::format("{:}{:}{:}{:}R{:}Z{:}P{:}{:}",
                                           m_idHelperSvc->stationNameString(detId),
                                           std::abs(m_idHelperSvc->stationEta(detId)),
                                           m_idHelperSvc->stationEta(detId) > 0 ? 'A' : 'C',
                                           m_idHelperSvc->stationPhi(detId),
                                           idHelper.doubletR(detId), idHelper.doubletZ(detId),
                                           idHelper.doubletPhi(detId), measPhi ? "Phi" : "Eta");
    auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, chName);
    canvas->expandPad(-design->halfWidth(), -design->shortHalfHeight());
    canvas->expandPad(design->halfWidth(), design->shortHalfHeight());
    canvas->setAxisTitles("x [mm]", "y [mm]");
    canvas->setRangeScale(1.1);
    using namespace MuonValR4;
    canvas->add(drawBox(Amg::Vector3D::Zero(),
                        2.*design->halfWidth(),
                        2.*design->shortHalfHeight(),
                        kBlack, hollowFilling));
    constexpr int oddFill = 3315;
    constexpr int evenFill = 3351;
    for (int strip = design->firstStripNumber(); strip <= design->numStrips(); ++strip) {
        const Amg::Vector2D stripPos = design->center(strip).value_or(Amg::Vector2D::Zero());
        canvas->add(drawBox(Amg::Vector3D{0., stripPos.x(), stripPos.y()},
                            design->stripWidth(),
                              2.*design->shortHalfHeight(), strip % 2 ? kBlue: kRed, 
                              strip % 2 ? oddFill : evenFill));
    }
    canvas->add(drawLabel(std::format("{:}, #{:}-panel", 
                                      m_idHelperSvc->toStringDetEl(detId),
                                      measPhi ? "phi" : "eta"), 0.1, 0.05));
    canvas->add(drawLabel(std::format("Dimensions: {:.1f}X{:.1f} [mm]", 
                                      2.*design->halfWidth(),
                                      2.*design->shortHalfHeight()), 0.1, 0.015));
}
StatusCode GeoModelRpcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};

    const ActsTrk::GeometryContext* geoContextHandle{nullptr};
    ATH_CHECK(SG::get(geoContextHandle, m_geoCtxKey, ctx));
    const ActsTrk::GeometryContext& gctx{*geoContextHandle};

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
      ATH_CHECK(dumpToTree(ctx, gctx, reElement));
      visualizeStripPanel(ctx, reElement->getParameters().etaDesign, reElement->identify(), false);
      visualizeStripPanel(ctx, reElement->getParameters().phiDesign, reElement->identify(), true);
      
      const Amg::Transform3D globToLocal{reElement->globalToLocalTransform(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTransform(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      if (!Amg::doesNotDeform(transClosure)) {
        ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)
                    <<". Ended up with "<< Amg::toString(transClosure) );
        return StatusCode::FAILURE;                  
      }
      const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
      for (unsigned gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (int doubPhi = reElement->doubletPhi(); doubPhi <= reElement->doubletPhiMax(); ++doubPhi) {
            for (bool measPhi: {false, true}) {
                unsigned numStrip =  (measPhi ? reElement->nPhiStrips() :
                                                reElement->nEtaStrips());
                for (unsigned strip = 1; strip < numStrip ; ++strip) {
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
                                    <<stripPosToString(reElement, gctx, measHash));
                }                
            }
        }
    }   
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::dumpToTree(const EventContext& ctx,
                                       const ActsTrk::GeometryContext& gctx, 
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

   m_envelopeHeight = 2.*reElement->getParameters().halfThickness;
   m_envelopeWidth  = 2.*reElement->getParameters().halfWidth;
   m_envelopeLength = 2.*reElement->getParameters().halfLength;
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTransform(gctx)};
   m_readoutTransform = transform;
   m_alignableNode  = reElement->alignableTransform()->getDefTransform();

   const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
      
   for (unsigned gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (int doubPhi = reElement->doubletPhi(); doubPhi <= reElement->doubletPhiMax(); ++doubPhi) {
            for (bool measPhi: {false, true}) {
                unsigned numStrip =  (measPhi ? reElement->nPhiStrips() :
                                                    reElement->nEtaStrips());
                for (unsigned strip = 1; strip <= numStrip ; ++strip) {

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
                    const IdentifierHash measHash = reElement->measurementHash(stripID);
                    const IdentifierHash layHash = reElement->layerHash(measHash);
                    const Amg::Vector3D stripPos = reElement->stripPosition(gctx, measHash);
                    m_stripPos.push_back(stripPos);
                    m_locStripPos.push_back((reElement->globalToLocalTransform(gctx, layHash) * stripPos).block<2,1>(0,0));
                    m_stripPosGasGap.push_back(gasGap);
                    m_stripPosMeasPhi.push_back(measPhi);
                    m_stripPosNum.push_back(strip);
                    m_stripDblPhi.push_back(doubPhi);

                    if (strip != 1) continue;
                    const Amg::Transform3D locToGlob = reElement->localToGlobalTransform(gctx, layHash)
                                                      * Amg::getRotateZ3D(90.*Gaudi::Units::deg * measPhi);
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

