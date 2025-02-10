/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMmTest.h"

#include <fstream>
#include <iostream>


#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

namespace MuonGM {

StatusCode GeoModelMmTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));

        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
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
StatusCode GeoModelMmTest::execute() {

    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
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
        for (int gasGap = 1; gasGap <= 4; ++gasGap) {
             const Identifier layerId = id_helper.channelID(test_me, id_helper.multilayer(test_me), gasGap, 1024); 
             int fStrip = reElement->numberOfMissingBottomStrips(layerId) + 1;
             int lStrip = id_helper.channelMax(layerId)-reElement->numberOfMissingTopStrips(layerId);
             for (int channel=fStrip; channel<=lStrip; ++channel) {

                bool is_valid{false};
                const Identifier strip_id = id_helper.channelID(test_me, id_helper.multilayer(test_me), 
                                                                gasGap, channel, is_valid);
                if (!is_valid) {
                    continue;
                }
                Amg::Vector3D globStripPos{Amg::Vector3D::Zero()};
                reElement->stripGlobalPosition(strip_id, globStripPos);
                Amg::Vector2D locPos{Amg::Vector2D::Zero()};
                reElement->surface(strip_id).globalToLocal(globStripPos, Amg::Vector3D::Zero(), locPos);
                const MuonGM::MuonChannelDesign& design{*reElement->getDesign(strip_id)};
                const double stripLength = 0.49 * reElement->stripLength(strip_id);
                if (design.channelNumber(locPos) != channel || 
                    design.channelNumber(locPos + stripLength *Amg::Vector2D::UnitY()) != channel ||
                    design.channelNumber(locPos - stripLength *Amg::Vector2D::UnitY()) != channel ){
                    ATH_MSG_FATAL("Conversion of channel -> strip -> channel failed for "
                            <<m_idHelperSvc->toString(strip_id)<<", global pos:"
                            <<Amg::toString(globStripPos)<<", locPos: "<<Amg::toString(locPos)
                            <<", backward channel: "<<design.channelNumber(locPos));
                    return StatusCode::FAILURE;
                }                
            }
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
    m_stStripPitch = roEl->getDesign(detElId)->inputPitch;
    const Amg::Transform3D permute{GeoTrf::GeoRotation{90.*Gaudi::Units::deg,90.*Gaudi::Units::deg, 0.}};
    m_alignableNode = roEl->AmdbLRSToGlobalTransform() * roEl->getDelta().inverse()*permute;

    /// Transformation of the readout element (Translation, ColX, ColY, ColZ) 

    m_readoutTransform = roEl->transform();
    for (int gasgap = 1; gasgap <= 4; ++gasgap) {

        const Identifier layerId = id_helper.channelID(detElId, multilayer, gasgap, 1024);
        const int fStrip = roEl->numberOfMissingBottomStrips(layerId)+1;
        const int lStrip = id_helper.channelMax(layerId)-roEl->numberOfMissingTopStrips(layerId);
        
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

            const MuonGM::MuonChannelDesign& design{*roEl->getDesign(strip_id)};

            design.leftEdge(channel, l_left);
            design.center(channel, l_cen);
            design.rightEdge(channel, l_right);

            roEl->surface(strip_id).localToGlobal(l_left, Amg::Vector3D::Zero(), strip_leftEdge);
            roEl->surface(strip_id).localToGlobal(l_cen, Amg::Vector3D::Zero(), strip_center);
            roEl->surface(strip_id).localToGlobal(l_right, Amg::Vector3D::Zero(), strip_rightEdge);

            m_locStripCenter.push_back(l_cen);
            m_isStereo.push_back(design.hasStereoAngle());           
            m_gasGap.push_back(id_helper.gasGap(strip_id));
            m_channel.push_back(id_helper.channel(strip_id));
            m_stripCenter.push_back(strip_center);
            m_stripLeftEdge.push_back(strip_leftEdge);
            m_stripRightEdge.push_back(strip_rightEdge);
            m_stripLength.push_back(roEl->stripLength(strip_id));
            m_stripActiveLength.push_back(roEl->stripActiveLength(strip_id));
            m_stripActiveLengthLeft.push_back(roEl->stripActiveLengthLeft(strip_id));
            m_stripActiveLengthRight.push_back(roEl->stripActiveLengthRight(strip_id));

            m_ActiveHeightR = design.xSize();  
            m_ActiveWidthL =  design.maxYSize();            
            m_ActiveWidthS =  design.minYSize();

            if (channel != fStrip) continue;

            m_stripRot.push_back(roEl->transform(strip_id));
            m_stripRotGasGap.push_back(gasgap);
            m_firstStripPos.push_back(design.firstPos() * Amg::Vector2D::UnitX()); 
            m_readoutFirstStrip.push_back(design.numberOfMissingBottomStrips() + 1);
            m_readoutSide.push_back(roEl->getReadoutSide()[gasgap -1]);    
        }
    }
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}



}