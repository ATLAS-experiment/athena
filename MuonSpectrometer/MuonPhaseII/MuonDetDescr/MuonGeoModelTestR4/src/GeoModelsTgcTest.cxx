
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelsTgcTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;
namespace MuonGMR4{

StatusCode GeoModelsTgcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){
 
        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 6) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 6 characters");
                continue;
            }
            /// Example string STL1A2
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
StatusCode GeoModelsTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};

    SG::ReadHandle geoContextHandle{m_geoCtxKey, ctx};
    ATH_CHECK(geoContextHandle.isPresent());
    const ActsGeometryContext& gctx{*geoContextHandle};


    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of sTgc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const sTgcReadoutElement* reElement = m_detMgr->getsTgcReadoutElement(test_me);
      if (!reElement) {         
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (m_idHelperSvc->stgcIdHelper().elementID(reElement->identify()) != m_idHelperSvc->stgcIdHelper().elementID(test_me)) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toString(test_me)
                      <<". But got instead "<<m_idHelperSvc->toString(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      for (Amg::Vector3D axis :{Amg::Vector3D::UnitX(),Amg::Vector3D::UnitY(),Amg::Vector3D::UnitZ()}) {
         const double closure_mag = std::abs( (transClosure*axis).dot(axis) - 1.);
         if (closure_mag > std::numeric_limits<float>::epsilon() ) {
            ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)<<" and axis "<<Amg::toString(axis, 0)
            <<". Ended up with "<< Amg::toString(transClosure*axis) );
            return StatusCode::FAILURE;
         }         
      }
      const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
      for (unsigned int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::Pad; chType <= sTgcIdHelper::sTgcChannelTypes::Wire; ++chType) {
            bool isValidLay{false};
            const Identifier layID = id_helper.channelID(reElement->identify(),
                                                        reElement->multilayer(),
                                                        layer, chType, 1, isValidLay);
            if (!isValidLay) {
                continue;
            }
            const unsigned int numChannel = reElement->numChannels(layID);

            for (unsigned int channel = 1; channel < numChannel ; ++channel) {
                bool isValidCh{false};
                const Identifier chID = id_helper.channelID(reElement->identify(),
                                                                reElement->multilayer(),
                                                                layer, chType, channel, isValidCh);
                if (!isValidCh) {
                    continue;
                }
                /// Test the back and forth conversion of the Identifier
                const IdentifierHash measHash = reElement->measurementHash(chID);
                const IdentifierHash layHash = reElement->layerHash(chID);
                ATH_MSG_VERBOSE("layer: "<<layer<<", chType: "<<chType
                               <<" --> layerHash: "<<static_cast<unsigned>(layHash));
                const Identifier backCnv = reElement->measurementId(measHash);
                if (backCnv != chID) {
                    ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chID)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (layHash != reElement->layerHash(measHash)) {
                    ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                m_idHelperSvc->toString(chID)<<" leads to different layer hashes "<<
                                layHash<<" vs. "<< reElement->layerHash(measHash));
                    return StatusCode::FAILURE;
                }
                if (chType == sTgcIdHelper::sTgcChannelTypes::Strip) {
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chID)<<" strip position "
                                    <<Amg::toString(reElement->globalChannelPosition(gctx, measHash)));
                } 
                else if (chType == sTgcIdHelper::sTgcChannelTypes::Wire) {
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chID)<<" wireGroup position "
                                    <<Amg::toString(reElement->globalChannelPosition(gctx, measHash)));
                }
                else if (chType == sTgcIdHelper::sTgcChannelTypes::Pad) {
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chID)<<" Pad position "
                                    <<Amg::toString(reElement->globalChannelPosition(gctx, measHash)));
                }   
            }
        }
      }
    
    }   
    return StatusCode::SUCCESS;
}

StatusCode GeoModelsTgcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const sTgcReadoutElement* reElement){
    
    m_stIndex    = reElement->stationName();
    m_stEta      = reElement->stationEta();
    m_stPhi      = reElement->stationPhi();
    m_stML       = reElement->multilayer();
    m_chamberDesign = reElement->chamberDesign();
    ///
    m_numLayers = reElement->numLayers();
    m_gasTck = reElement->gasGapThickness();
    ///
    m_sChamberLength = reElement->sChamberLength();
    m_lChamberLength = reElement->lChamberLength();
    m_chamberHeight = reElement->chamberHeight();
     
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   m_alignableNode  = reElement->alignableTransform()->getDefTransform();

   const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
   for (unsigned int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::Pad; chType <= sTgcIdHelper::sTgcChannelTypes::Wire; ++chType) {
            unsigned int numWireGroup = 0;
            /// Use idHelper to get the identifier
            bool isValidLay{false};
            const Identifier layID = id_helper.channelID(reElement->identify(),
                                                        reElement->multilayer(),
                                                        layer, chType, 1, isValidLay);
            if (!isValidLay) {
                continue;
            }
            /// Gas Gap dimensions
            
            m_sGapLength = 2.*reElement->stripDesign(layID).shortHalfHeight();
            m_lGapLength = 2.*reElement->stripDesign(layID).longHalfHeight();
            m_sPadLength = 2.*reElement->padDesign(layID).shortHalfHeight();
            m_lPadLength = 2.*reElement->padDesign(layID).longHalfHeight();
            m_gapHeight =2.*reElement->stripDesign(layID).halfWidth();
            m_yCutout = reElement->stripDesign(layID).yCutout();

            switch (chType) {
                case sTgcIdHelper::sTgcChannelTypes::Pad:
                    m_numPads.push_back(reElement->numChannels(layID));
                    m_numPadEta.push_back(reElement->numPadEta(layID));
                    m_numPadPhi.push_back(reElement->numPadPhi(layID));
                    // m_firstPadHeight.push_back(reElement->firstPadHeight(layID));
                    m_padHeight.push_back(reElement->padHeight(layID));
                    m_padPhiShift.push_back(reElement->padPhiShift(layID));
                    // m_firstPadPhiDiv.push_back(reElement->firstPadPhiDiv(layID));
                    m_anglePadPhi = reElement->anglePadPhi(layID);
                    m_beamlineRadius = reElement->beamlineRadius(layID);
                    for (unsigned int pad = 1; pad <= reElement->numChannels(layID); ++pad) {
                        bool isValidPad{false};
                        const Identifier padID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->multilayer(),
                                                                    layer, chType, pad, isValidPad);
                        if (!isValidPad) {
                            ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                       <<" layer: "<<layer<<" pad: "<<pad<<" channelType: "<<chType);
                            continue;
                        }

                        Amg::Vector2D localPadPos(Amg::Vector2D::Zero());
                        std::array<Amg::Vector2D,4> localPadCorners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
                        Amg::Vector3D globalPadPos(Amg::Vector3D::Zero());
                        std::array<Amg::Vector3D,4> globalPadCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};

                        localPadPos = reElement->localChannelPosition(padID);
                        localPadCorners = reElement->localPadCorners(padID);

                        m_localPadPos.push_back(localPadPos);
                        m_localPadCornerBL.push_back(localPadCorners[0]);
                        m_localPadCornerBR.push_back(localPadCorners[1]);
                        m_localPadCornerTL.push_back(localPadCorners[2]);
                        m_localPadCornerTR.push_back(localPadCorners[3]);

                        Amg::Vector2D hitCorrection{-.1, -.1};
                        Amg::Vector2D hitPos = localPadCorners[3] + hitCorrection;
                        m_hitPosition.push_back(hitPos);
                        m_padNumber.push_back(reElement->padNumber(hitPos, padID));

                        globalPadPos = reElement->globalChannelPosition(gctx, padID);
                        globalPadCorners = reElement->globalPadCorners(gctx, padID);
                   
                        m_globalPadPos.push_back(globalPadPos);
                        m_globalPadCornerBR.push_back(globalPadCorners[0]);
                        m_globalPadCornerBL.push_back(globalPadCorners[1]);
                        m_globalPadCornerTR.push_back(globalPadCorners[2]);
                        m_globalPadCornerTL.push_back(globalPadCorners[3]);
 
                        m_padEta.push_back(reElement->padEta(padID));
                        m_padPhi.push_back(reElement->padPhi(padID));
                        m_padGasGap.push_back(layer);

                        if (pad != 1) continue;
                        const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, padID);
                        ATH_MSG_DEBUG("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                        m_padRot.push_back(locToGlob);
                        m_padRotGasGap.push_back(layer);

                    }
                    break;

                case sTgcIdHelper::sTgcChannelTypes::Strip:{
                    const StripDesign& design{reElement->stripDesign(layID)};
                    m_numStrips = design.numStrips();
                    m_stripPitch = design.stripPitch();
                    m_stripWidth = design.stripWidth(); 
                    for (unsigned int strip = 1; strip <= reElement->numChannels(layID); ++strip) {
                        bool isValidStrip{false};
                        const Identifier stripID = id_helper.channelID(reElement->identify(), 
                                                                       reElement->multilayer(),
                                                                       layer, chType, strip, isValidStrip);
                        if (!isValidStrip) {
                            ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                        <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                        <<" layer: "<<layer<<" strip: "<<strip<<" channelType: "<<chType);
                            continue;
                        }
                        m_localStripPos.push_back((reElement->localChannelPosition(stripID)).block<2,1>(0,0));
                        m_globalStripPos.push_back(reElement->globalChannelPosition(gctx, stripID));
                        m_stripGasGap.push_back(layer);
                        m_stripNum.push_back(strip);
                        m_stripLengths.push_back(reElement->stripLength(stripID));

                        if (strip != 1) continue;
                        const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, stripID);
                        ATH_MSG_DEBUG("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                        m_stripRot.push_back(locToGlob);
                        m_stripRotGasGap.push_back(layer);

                    }
                    break;
                } case sTgcIdHelper::sTgcChannelTypes::Wire: {
                    const WireGroupDesign& design{reElement->wireDesign(layID)};
                    m_wireGroupWidth = design.numWiresInGroup(2);
                    numWireGroup = design.numStrips();                    
                    m_wirePitch = design.stripPitch();
                    m_wireWidth = design.stripWidth();
                    m_numWires.push_back(design.nAllWires());
                    m_firstWireGroupWidth.push_back(design.numWiresInGroup(1));
                    m_numWireGroups.push_back(numWireGroup);
                    m_wireCutout.push_back(design.wireCutout()); 
                    std::cout << "The number of wire groups are:" << numWireGroup << std::endl;
                    for (unsigned int wireGroup = 1; wireGroup <= numWireGroup; ++wireGroup) {
                        bool isValidWire{false};
                        const Identifier wireGroupID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->multilayer(),
                                                                    layer, chType, wireGroup, isValidWire);
                        if (!isValidWire) {
                            ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                        <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                        <<" layer: "<<layer<<" wireGroup: "<<wireGroup<<" channelType: "<<chType);
                            continue;
                        }  
                        m_localWireGroupPos.push_back(reElement->localChannelPosition(wireGroupID));
                        m_globalWireGroupPos.push_back(reElement->globalChannelPosition(gctx, wireGroupID));
                        m_wireGroupGasGap.push_back(layer);
                        m_wireGroupNum.push_back(wireGroup);
                    
                        if (wireGroup != 1) continue;
                        const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, wireGroupID);
                        ATH_MSG_DEBUG("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                        m_wireGroupRot.push_back(locToGlob);
                        m_wireGroupRotGasGap.push_back(layer);
                    }
                    break;
                }
            }
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

