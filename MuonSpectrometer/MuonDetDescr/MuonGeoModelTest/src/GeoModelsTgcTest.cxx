/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelsTgcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

namespace MuonGM {


StatusCode GeoModelsTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
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
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of sTgc detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const sTgcReadoutElement* reElement = detMgr->getsTgcReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (m_idHelperSvc->toStringDetEl(reElement->identify()) != m_idHelperSvc->toStringDetEl(test_me)) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << m_idHelperSvc->toStringDetEl(test_me) << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::dumpToTree(const EventContext& ctx, const sTgcReadoutElement* roEl) {
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};

    const Amg::Transform3D permute{GeoTrf::GeoRotation{90.*Gaudi::Units::deg,90.*Gaudi::Units::deg, 0.}};
    m_alignableNode = roEl->AmdbLRSToGlobalTransform() * roEl->getDelta().inverse()*permute;
//// Identifier of the readout element
    int stIndex    = roEl->getStationIndex();
    int stEta      = roEl->getStationEta();
    int stPhi      = roEl->getStationPhi();
    int stML      = id_helper.multilayer(roEl->identify());
    std::string chamberDesign = roEl->getStationType();

    m_stIndex = stIndex;
    m_stEta = stEta;
    m_stPhi = stPhi;
    m_stML = stML;
    m_chamberDesign = chamberDesign;

    const Identifier genStripID =id_helper.channelID(stIndex, stEta, stPhi, 
                    stML, 1, sTgcIdHelper::sTgcChannelTypes::Strip, 1);
    const Identifier genPadID =id_helper.channelID(stIndex, stEta, stPhi, 
                    stML, 1, sTgcIdHelper::sTgcChannelTypes::Pad, 1);

//// Chamber Details from sTGCDetectorDescription 
    int numLayers = roEl->numberOfLayers(true); 
    double yCutout = roEl->getDesign(genStripID)->yCutout();
    double gasTck = roEl->getDesign(genStripID)->thickness;

    m_numLayers = numLayers;
    m_yCutout = yCutout;
    m_gasTck = gasTck;
//// Gas Gap lengths for debug
    double sGapLength = roEl->getDesign(genStripID)->minYSize();
    double lGapLength = roEl->getDesign(genStripID)->maxYSize();
    double gapHeight = roEl->getDesign(genStripID)->xSize();

    double sPadLength = roEl->getPadDesign(genPadID)->sPadWidth;
    double lPadLength = roEl->getPadDesign(genPadID)->lPadWidth;

    m_sGapLength = sGapLength;
    m_lGapLength = lGapLength;
    m_gapHeight = gapHeight;

    m_sPadLength = sPadLength;
    m_lPadLength = lPadLength;
//// Chamber lengths for debug
    double sChamberLength = roEl->getPadDesign(genPadID)->sWidth;
    double lChamberLength = roEl->getPadDesign(genPadID)->lWidth;
    double chamberHeight = roEl->getPadDesign(genPadID)->Length;

    m_sChamberLength = sChamberLength;
    m_lChamberLength = lChamberLength;
    m_chamberHeight = chamberHeight;

/// Transformation of the readout element (Translation, ColX, ColY, ColZ) 
    const Amg::Transform3D& trans{roEl->transform()};
    m_readoutTransform = trans;

//// All the Vectors
    for (int lay = 1; lay <= numLayers; ++lay) {
        const Identifier layWireID =id_helper.channelID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Wire, 1);
        const Identifier layStripID =id_helper.channelID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Strip, 1);
        const Identifier layPadID =id_helper.padID(stIndex, stEta, stPhi, 
                        stML, lay, sTgcIdHelper::sTgcChannelTypes::Pad, 1, 1);

//// Wire Dimensions
        unsigned int numWires = roEl->numberOfWires(layWireID); 
        unsigned int firstWireGroupWidth = roEl->getDesign(layWireID)->firstPitch;
        int numWireGroups = roEl->getDesign(layWireID)->nGroups;
        double wireCutout = roEl->getDesign(layWireID)->wireCutout;
        double wirePitch = roEl->wirePitch(); 
        double wireWidth = roEl->getDesign(layWireID)->inputWidth;
        double wireGroupWidth = roEl->getDesign(layWireID)->groupWidth;

        m_numWires.push_back(numWires);
        m_firstWireGroupWidth.push_back(firstWireGroupWidth);
        m_numWireGroups.push_back(numWireGroups);
        m_wireCutout.push_back(wireCutout);
        m_wirePitch = wirePitch;
        m_wireWidth = wireWidth;
        m_wireGroupWidth = wireGroupWidth;

//// Global transformations for wires
        for (int wireGroupIndex = 1; wireGroupIndex <= numWireGroups; ++wireGroupIndex) {
            bool isValid = false;
            const Identifier wireGroupID =id_helper.channelID(stIndex, stEta, stPhi, stML, lay, 
                            sTgcIdHelper::sTgcChannelTypes::Wire, wireGroupIndex, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("The following wire group ID is not valid: " << wireGroupID);
            }
            Amg::Vector3D wireGroupPos(Amg::Vector3D::Zero());
            Amg::Vector2D localWireGroupPos(Amg::Vector2D::Zero());
            roEl->stripPosition(wireGroupID, localWireGroupPos);
            m_localWireGroupPos.push_back(localWireGroupPos);            
            roEl->stripGlobalPosition(wireGroupID, wireGroupPos);
            m_globalWireGroupPos.push_back(wireGroupPos);
            m_wireGroupNum.push_back(wireGroupIndex);
            m_wireGroupGasGap.push_back(lay);

            if (wireGroupIndex != 1) continue;
            const Amg::Transform3D locToGlob = roEl->transform(wireGroupID);
            m_wireGroupRot.push_back(locToGlob);                    
            m_wireGroupRotGasGap.push_back(lay);
        }

////Strip Dimensions
        int numStrips = roEl->getDesign(layStripID)->nch;
        double stripPitch = roEl->channelPitch(layStripID);
        double stripWidth = roEl->getDesign(layStripID)->inputWidth;
        double firstStripPitch = roEl->getDesign(layStripID)->firstPitch;
        
        m_numStrips = numStrips;
        m_stripPitch = stripPitch;
        m_stripWidth = stripWidth;
        m_firstStripPitch.push_back(firstStripPitch);

//// Global transformations for strips
        for (int stripIndex = 1; stripIndex <= numStrips; ++stripIndex) {
            bool isValid = false;
            const Identifier stripID =id_helper.channelID(stIndex, stEta, stPhi, stML, lay, 
                            sTgcIdHelper::sTgcChannelTypes::Strip, stripIndex, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("The following strip ID is not valid: " << stripID);
            }
            double stripLength = roEl->getDesign(stripID)->channelLength(stripIndex);
            Amg::Vector3D globalStripPos(Amg::Vector3D::Zero());
            Amg::Vector2D localStripPos(Amg::Vector2D::Zero());
            roEl->stripPosition(stripID, localStripPos);
            m_localStripPos.push_back(localStripPos);
            roEl->stripGlobalPosition(stripID, globalStripPos);
            m_globalStripPos.push_back(globalStripPos);
            m_stripNum.push_back(stripIndex);
            m_stripGasGap.push_back(lay);
            m_stripLengths.push_back(stripLength);

            if (stripIndex != 1) continue;
            const Amg::Transform3D locToGlob = roEl->transform(stripID);
            m_stripRot.push_back(locToGlob);                    
            m_stripRotGasGap.push_back(lay);
        }

////Pad Dimensions
        unsigned int numPads = roEl->numberOfPads(layPadID);
        int numPadEta = roEl->getPadDesign(layPadID)->nPadH;
        int numPadPhi = roEl->getPadDesign(layPadID)->nPadColumns;
        double firstPadHeight = roEl->getPadDesign(layPadID)->firstRowPos;
        double padHeight = roEl->getPadDesign(layPadID)->inputRowPitch;
        double padPhiShift = roEl->getPadDesign(layPadID)->PadPhiShift;
        double firstPadPhiDiv = roEl->getPadDesign(layPadID)->firstPhiPos;
        double anglePadPhi = roEl->getPadDesign(layPadID)->inputPhiPitch;
        double beamlineRadius = roEl->getPadDesign(layPadID)->radialDistance;

        m_numPads.push_back(numPads);
        m_numPadEta.push_back(numPadEta);
        m_numPadPhi.push_back(numPadPhi);
        m_firstPadHeight.push_back(firstPadHeight);
        m_padHeight.push_back(padHeight);
        m_padPhiShift.push_back(padPhiShift);
        m_firstPadPhiDiv.push_back(firstPadPhiDiv);
        m_anglePadPhi = anglePadPhi;
        m_beamlineRadius = beamlineRadius;

//// Global transformations for pads
        for (int phiIndex = 1; phiIndex <= numPadPhi; ++phiIndex) {
            for(int etaIndex = 1; etaIndex <= numPadEta; ++etaIndex) {
                bool isValid = false;
                const Identifier padID =id_helper.padID(stIndex, stEta, stPhi, stML, lay, 
                    sTgcIdHelper::sTgcChannelTypes::Pad, etaIndex, phiIndex, isValid);
                if(!isValid) {
                    ATH_MSG_WARNING("The following pad ID is not valid: " << padID);
                }
                Amg::Vector2D localPadPos(Amg::Vector2D::Zero());
                Amg::Vector3D globalPadPos(Amg::Vector3D::Zero());
                std::array<Amg::Vector2D,4> localPadCorners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
                std::array<Amg::Vector3D,4> globalPadCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};

                roEl->padPosition(padID, localPadPos);
                roEl->padGlobalPosition(padID, globalPadPos);
                roEl->padCorners(padID, localPadCorners);
                roEl->padGlobalCorners(padID, globalPadCorners);

                m_localPadPos.push_back(localPadPos);
                m_localPadCornerBL.push_back(localPadCorners[0]);
                m_localPadCornerBR.push_back(localPadCorners[1]);
                m_localPadCornerTL.push_back(localPadCorners[2]);
                m_localPadCornerTR.push_back(localPadCorners[3]);

                Amg::Vector2D hitCorrection{-.1, -.1};
                Amg::Vector2D hitPos = localPadCorners[3] + hitCorrection;
                m_hitPosition.push_back(hitPos);
                m_padNumber.push_back(roEl->padNumber(hitPos, padID));

                m_globalPadPos.push_back(globalPadPos);
                m_globalPadCornerBR.push_back(globalPadCorners[0]);
                m_globalPadCornerBL.push_back(globalPadCorners[1]);
                m_globalPadCornerTR.push_back(globalPadCorners[2]);
                m_globalPadCornerTL.push_back(globalPadCorners[3]);

                m_padEta.push_back(etaIndex);
                m_padPhi.push_back(phiIndex);
                m_padGasGap.push_back(lay);

                if (etaIndex != 1 || phiIndex != 1) continue;
                const Amg::Transform3D locToGlob = roEl->transform(padID);
                m_padRot.push_back(locToGlob);                    
                m_padRotGasGap.push_back(lay);
            }
        }
    }

    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}
