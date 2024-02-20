/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelRpcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"


namespace MuonGM {

GeoModelRpcTest::GeoModelRpcTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelRpcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
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
   m_chamberDesign = readoutEle->getStationType();

   m_numStripsEta = readoutEle->Nstrips(false);
   m_numStripsPhi = readoutEle->Nstrips(true);
   m_numRpcLayers = readoutEle->numberOfLayers();
   m_numGasGapsPhi = readoutEle->NgasGaps(true);
   m_numPhiPanels = readoutEle->NphiStripPanels();   

   m_stripEtaPitch = readoutEle->StripPitch(false);
   m_stripPhiPitch = readoutEle->StripPitch(true);
   m_stripEtaWidth = readoutEle->StripWidth(false);
   m_stripPhiWidth = readoutEle->StripWidth(true);
   m_stripEtaLength = readoutEle->StripLength(false);
   m_stripPhiLength = readoutEle->StripLength(true);

  
   const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};

   const Amg::Transform3D& trans{readoutEle->transform()};
   m_readoutTransform = trans;
   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
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
                    m_stripPos.push_back(readoutEle->stripPos(stripID));
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
