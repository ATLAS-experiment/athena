/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelRpcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonGM {

StatusCode GeoModelRpcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){
        
        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 6) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 6 characters");
                continue;
            }
            /// Example string MMS4A2
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
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const RpcReadoutElement* reElement = detMgr->getRpcReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << m_idHelperSvc->toStringDetEl(test_me) << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }   
        ATH_CHECK(dumpToTree(ctx, reElement));
        if (m_idHelperSvc->stationNameString(reElement->identify()) == "BIS") continue;
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        for (int gasGap = 1 ; gasGap <= reElement->numberOfLayers(); ++gasGap) {
            for (int doubPhi = reElement->getDoubletPhi(); doubPhi <= reElement->NphiStripPanels(); ++doubPhi) {
                for (bool measPhi: {false, true}) {
                    for (int strip = 1 ; strip < reElement->Nstrips(measPhi); ++strip) {
                        bool isValid{false};
                        const Identifier stripId  = idHelper.channelID(test_me, 
                                                                       idHelper.doubletZ(test_me),
                                                                       doubPhi,
                                                                       gasGap, measPhi, strip, isValid);
                        if (!isValid) {
                            ATH_MSG_DEBUG("Could not construct Identifier from "
                                        <<m_idHelperSvc->toStringChamber(test_me)
                                        <<", gasGap:"<<gasGap<<", doubletPhi: "<<doubPhi
                                        <<", measurePhi "<<measPhi<<", strip: "<<strip);
                            continue;
                        }
                        Amg::Vector2D locPos2D{Amg::Vector2D::Zero()};
                        if (!reElement->stripPosition(stripId, locPos2D)) {
                            ATH_MSG_FATAL("Could not retrieve the local strip position for "<<m_idHelperSvc->toString(stripId));
                            return StatusCode::FAILURE;
                        }
                        const Trk::Surface& planeSurf{reElement->surface(stripId)};
                        const Amg::Vector3D globPos3D = planeSurf.localToGlobal(locPos2D);
                        /// Check the stripPos reference 
                        const Amg::Vector3D stripPos = reElement->stripPos(stripId);
                        if ( (stripPos - globPos3D).mag() > std::numeric_limits<float>::epsilon()) {
                            ATH_MSG_FATAL("Retrieving the strip position in two different ways leads to two distinct results "
                                        <<Amg::toString(stripPos)<<" as reference. Second path "<<Amg::toString(globPos3D));
                            return StatusCode::FAILURE;
                        }
                        const Trk::Surface& laySurf{reElement->surface(stripId)};
                        const double stripLen = 0.5 *reElement->StripLength(measPhi) - 1. * Gaudi::Units::cm;
                        if (!laySurf.insideBounds(locPos2D)){
                            ATH_MSG_FATAL("The strip center "<<Amg::toString(locPos2D)<<" of "<<m_idHelperSvc->toString(stripId)
                            <<" is outside bounds "<<laySurf.bounds());
                            return StatusCode::FAILURE;
                        }
                        if (!laySurf.insideBounds(locPos2D + stripLen * Amg::Vector2D::UnitY())){
                            ATH_MSG_FATAL("The right strip edge "<<Amg::toString(locPos2D + stripLen * Amg::Vector2D::UnitY())<<
                                        " of "<<m_idHelperSvc->toString(stripId)<<" is outside bounds "<<laySurf.bounds());
                            return StatusCode::FAILURE;
                        }if (!laySurf.insideBounds(locPos2D - stripLen * Amg::Vector2D::UnitY())){
                            ATH_MSG_FATAL("The left strip edge "<<Amg::toString(locPos2D  - stripLen * Amg::Vector2D::UnitY())
                                         <<" of "<<m_idHelperSvc->toString(stripId)<<" is outside bounds "<<laySurf.bounds());
                            return StatusCode::FAILURE;
                        }
                    }
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::dumpToTree(const EventContext& ctx, const RpcReadoutElement* readoutEle) {

   m_stIndex    = readoutEle->getStationIndex();
   m_stEta      = readoutEle->getStationEta();
   m_stPhi      = readoutEle->getStationPhi();
   m_doubletR   = readoutEle->getDoubletR();
   m_doubletZ   = readoutEle->getDoubletZ();
   m_doubletPhi = readoutEle->getDoubletPhi();
   m_chamberDesign = readoutEle->getTechnologyName();

   m_numStripsEta = readoutEle->Nstrips(false);
   m_numStripsPhi = readoutEle->Nstrips(true);
   m_numRpcLayers = readoutEle->numberOfLayers();
   m_numPhiPanels = readoutEle->NphiStripPanels();   

   m_stripEtaPitch = readoutEle->StripPitch(false);
   m_stripPhiPitch = readoutEle->StripPitch(true);
   m_stripEtaWidth = readoutEle->StripWidth(false);
   m_stripPhiWidth = readoutEle->StripWidth(true);
   m_stripEtaLength = readoutEle->StripLength(false);
   m_stripPhiLength = readoutEle->StripLength(true);

  
   const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};

   const Amg::Transform3D& trans{readoutEle->absTransform()};
   m_readoutTransform = trans;
   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
   m_alignableNode = station->getGeoTransform()->getDefTransform() *
                     station->getNativeToAmdbLRS().inverse();
   if (station->hasALines()) { 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }
    const int maxDoubPhi = std::max(readoutEle->getDoubletPhi(), readoutEle->NphiStripPanels());
    for (int doubPhi = readoutEle->getDoubletPhi(); doubPhi <= maxDoubPhi; ++doubPhi) {
        for (int gap = 1; gap <= readoutEle->numberOfLayers(); ++gap) {   
            for (bool measPhi : {false, true}) {
                unsigned int numStrip = readoutEle->Nstrips(measPhi);
                for (unsigned int strip = 1; strip <= numStrip ; ++strip) {
                    bool isValid{false};
                    const Identifier stripID = idHelper.channelID(readoutEle->identify(), 
                                                                  readoutEle->getDoubletZ(),
                                                                  doubPhi, gap, measPhi, strip, isValid);
                    if (!isValid) {
                        ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(readoutEle->identify())
                                       <<" gap: "<<gap<<" strip: "<<strip<<" meas phi: "<<measPhi);
                        continue;
                    }
                    Amg::Vector2D lStripPos{Amg::Vector2D::Zero()};
                    if (!readoutEle->stripPosition(stripID, lStripPos)){
                        ATH_MSG_FATAL("Failed to obtain strip position for "<<m_idHelperSvc->toString(stripID));
                        return StatusCode::FAILURE;
                    }

                    m_stripPos.push_back(readoutEle->stripPos(stripID));
                    m_locPos.push_back(lStripPos);
                    m_stripPosGasGap.push_back(gap);
                    m_stripPosMeasPhi.push_back(measPhi);
                    m_stripPosNum.push_back(strip);
                    m_stripDblPhi.push_back(doubPhi);

                    if (strip != 1) continue;
                    const Amg::Transform3D locToGlob = readoutEle->transform(stripID);
                    m_stripRot.push_back(locToGlob);                    
                    m_stripRotGasGap.push_back(gap);
                    m_stripRotMeasPhi.push_back(measPhi);
                    m_stripRotDblPhi.push_back(doubPhi); 
                    
                }
            }
        }
    }
  
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}


}
