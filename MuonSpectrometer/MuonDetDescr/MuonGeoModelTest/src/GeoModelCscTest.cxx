/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelCscTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"


namespace MuonGM {

GeoModelCscTest::GeoModelCscTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelCscTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelCscTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const CscIdHelper& id_helper{m_idHelperSvc->cscIdHelper()};
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
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
            m_testStations.insert(*itr);
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelCscTest::execute() {
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
        const CscReadoutElement* reElement = detMgr->getCscReadoutElement(test_me);
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
StatusCode GeoModelCscTest::dumpToTree(const EventContext& ctx, const CscReadoutElement* readoutEle) {
   m_stIndex    = readoutEle->getStationIndex();
   m_stEta      = readoutEle->getStationEta();
   m_stPhi      = readoutEle->getStationPhi();
   m_stMultiLayer = readoutEle->ChamberLayer();

   const CscIdHelper& idHelper{m_idHelperSvc->cscIdHelper()};

    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform = trans;

   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
   if (station->hasALines()){ 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }
    for (bool measPhi : {false, true}) {
        for (int layer = 1 ; layer <= readoutEle->numberOfLayers(measPhi); ++layer){
            const Identifier id = idHelper.channelID(readoutEle->identify(),readoutEle->ChamberLayer(),layer, measPhi,1);
            m_layerTrans.push_back(readoutEle->localToGlobalTransf(id));            
            m_layMeasPhi.push_back(measPhi);
            m_layNumber.push_back(layer);
        }    
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}


}
