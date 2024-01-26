/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMmTest.h"

#include <fstream>
#include <iostream>


#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"

namespace MuonGM {

GeoModelMmTest::GeoModelMmTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelMmTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};


    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string MML1A6
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.elementID(id) == eleId;
                        });
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()) {
        m_testStations.insert(id_helper.detectorElement_begin(),
                              id_helper.detectorElement_end());
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations){
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::execute() {

    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    //Looping through the MicroMegas identifiers and pushing back the respective roe.
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of Mm detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const MuonGM::MMReadoutElement *reElement = detMgr->getMMReadoutElement(test_me);
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
        ATH_CHECK(dumpToTree(ctx,reElement));

    }
    return StatusCode::SUCCESS;
}

StatusCode GeoModelMmTest::dumpToTree(const EventContext& ctx, const MuonGM::MMReadoutElement* roEl) {

    const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
    const Identifier detElId = roEl->identify();
    //// Identifier of the readout element
    m_stationIndex    = roEl->getStationIndex();
    m_stationEta = roEl->getStationEta();
    m_stationPhi = roEl->getStationPhi();
    m_stationName = id_helper.stationName(detElId);
    const int multilayer = id_helper.multilayer(detElId);
    m_multilayer = multilayer;


    /// Transformation of the readout element (Translation, ColX, ColY, ColZ) 

    m_readoutTransform = roEl->transform();
    for (int gasgap = 1; gasgap <= 4; ++gasgap) {

        Identifier layerId = id_helper.channelID(detElId, multilayer, gasgap, 1024);
        int fStrip = roEl->numberOfMissingBottomStrips(layerId)+1;
        int lStrip = id_helper.channelMax(layerId)-roEl->numberOfMissingTopStrips(layerId);
        
        for (int channel=fStrip; channel<=lStrip; ++channel) {

            bool is_valid{false};
            const Identifier strip_id = id_helper.channelID(detElId, multilayer, 
                                                            gasgap, channel, is_valid);
            if (!is_valid) continue;

               
            //If the strip number is outside the range of valid strips, the function will return false
            //this method also assignes strip center global coordinates to the gp vector.

            //Strip global points
            Amg::Vector3D strip_center{Amg::Vector3D::Zero()},
                          strip_leftEdge{Amg::Vector3D::Zero()},
                          strip_rightEdge{Amg::Vector3D::Zero()};
          
            Amg::Vector2D l_cen{Amg::Vector2D::Zero()}, 
                          l_left{Amg::Vector2D::Zero()},
                          l_right{Amg::Vector2D::Zero()};

            const MuonGM::MuonChannelDesign* design = roEl->getDesign(strip_id);

            design->leftEdge(channel, l_left);
            design->center(channel, l_cen);
            design->rightEdge(channel, l_right);

            roEl->surface(strip_id).localToGlobal(l_left, Amg::Vector3D::Zero(), strip_leftEdge);
            roEl->surface(strip_id).localToGlobal(l_cen, Amg::Vector3D::Zero(), strip_center);
            roEl->surface(strip_id).localToGlobal(l_right, Amg::Vector3D::Zero(), strip_rightEdge);

            m_locStripCenter.push_back(l_cen);
            m_isStereo.push_back(design->hasStereoAngle());           
            m_gasGap.push_back(id_helper.gasGap(strip_id));
            m_channel.push_back(id_helper.channel(strip_id));
            m_stripCenter.push_back(strip_center);
            m_stripLeftEdge.push_back(strip_leftEdge);
            m_stripRightEdge.push_back(strip_rightEdge);
            m_stripLength.push_back(roEl->stripLength(strip_id));
            m_stripActiveLength.push_back(roEl->stripActiveLength(strip_id));
            m_stripActiveLengthLeft.push_back(roEl->stripActiveLengthLeft(strip_id));
            m_stripActiveLengthRight.push_back(roEl->stripActiveLengthRight(strip_id));

            m_ActiveHeightR = roEl->getDesign(strip_id)->xSize();  
            m_ActiveWidthL = roEl->getDesign(strip_id)->maxYSize();            
            m_ActiveWidthS = roEl->getDesign(strip_id)->minYSize();

            if (channel != fStrip) continue;
            // /// Strip center
            // const Amg::Vector3D globStripPos = roEl->globalPosition();
            // Amg::Vector2D locStripPos{Amg::Vector2D::Zero()};
            // const Trk::Surface& surf{roEl->surface(strip_id)};
            // if (!surf.globalToLocal(globStripPos, Amg::Vector3D::Zero(), locStripPos)){
            //     ATH_MSG_FATAL("Failed to build local strip position "<<m_idHelperSvc->toString(strip_id));
            //     return StatusCode::FAILURE;
            // }
            // const Amg::Transform3D locTransf{locStripPos.x,locStripPos.y,0};
            // m_locStripCenter.push_back(locStripPos);                  
            // m_stripRotGasGap.push_back(gasgap);

        }
    }
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}



}