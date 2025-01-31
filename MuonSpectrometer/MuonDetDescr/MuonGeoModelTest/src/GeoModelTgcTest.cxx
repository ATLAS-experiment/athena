/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTgcTest.h"

#include <fstream>
#include <iostream>

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonGM {

template<typename VType> bool isEqual(const std::vector<VType>& a,
                                      const std::vector<VType>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t k =0 ; k < a.size() ; ++k) {
        if ( std::abs(a[k] - b[k]) > std::numeric_limits<VType>::epsilon()){
            return false;
        }
    }
    return true;
}
template <typename VType> std::ostream& operator<<(std::ostream& ostr, const std::vector<VType>& v){
    for (size_t k = 0 ; k <v.size(); ++k){
        ostr<<v[k];
        if ( k+1 != v.size())ostr<<",";
    } 
    return ostr;
}
template <typename VType> std::ostream& operator<<(std::ostream& ostr, const std::set<VType>& s){
    unsigned int k=1;
    for (const VType& ele : s){
        ostr<<ele;
        if (k != s.size()) ostr<<";";
        ++k;
    } 
    return ostr;
}
inline int nStrips(const MuonGM::TgcReadoutElement& readoutEle, int layer) {
    return readoutEle.nStrips(layer) > 1 ? readoutEle.nStrips(layer) : 0;
} 


struct TgcChamberLayout {
    Identifier gasGap{};
    std::string techType{};
    std::vector<double> botStripPos{};
    std::vector<double> topStripPos{};
    std::vector<int> wireGangLayout{};
    double wirePitch{0.};

    bool operator==(const TgcChamberLayout& other) const{
        return isEqual(botStripPos, other.botStripPos) &&
               isEqual(topStripPos, other.topStripPos) && 
               isEqual(wireGangLayout, other.wireGangLayout) &&
               std::abs(wirePitch - other.wirePitch) < std::numeric_limits<float>::epsilon();
    }
};
struct ChamberGrp {
    ChamberGrp(const TgcChamberLayout& grp):
            m_lay{grp} {
        m_gaps[grp.techType].insert(grp.gasGap);
    }
    bool addChamber(const TgcChamberLayout& lay){
        if (m_lay == lay) { 
            m_gaps[lay.techType].insert(lay.gasGap);
            return true;
        }
        return false;
    }
    const std::map<std::string, std::set<Identifier>>& allGaps() const{ return m_gaps; }
    const TgcChamberLayout& layout() const { return m_lay; }
    TgcChamberLayout& layout(){ return m_lay; }
    private:
        TgcChamberLayout m_lay{};
        std::map<std::string, std::set<Identifier>> m_gaps{};
    
};

StatusCode GeoModelTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){

        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 7) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 7 characters");
                continue;
            }
            /// Example string T1E4A06
            const std::string statName = token.substr(0, 3);
            const unsigned statEta = std::atoi(token.substr(3, 1).c_str()) * (token[4] == 'A' ? 1 : -1);
            const unsigned statPhi = std::atoi(token.substr(5, 2).c_str());
            bool isValid{false};
            const Identifier eleId = idHelper.elementID(statName, statEta, statPhi, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Failed to deduce a station name for " << token);
                continue;
            }
            transcriptedIds.insert(eleId);
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
StatusCode GeoModelTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    dumpReadoutXML(**detMgr);
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const TgcReadoutElement* reElement = detMgr->getTgcReadoutElement(test_me);
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
       
        const Identifier prevId = reElement->getStationPhi() > 1 ? m_idHelperSvc->tgcIdHelper().elementID(m_idHelperSvc->stationNameString(test_me),
                                                                                                          reElement->getStationEta(),
                                                                                                          reElement->getStationPhi() - 1) : test_me;
        const TgcReadoutElement* prevRE = detMgr->getTgcReadoutElement(prevId);
        const Amg::Vector3D center = reElement->center();
        ATH_MSG_DEBUG("Tgc element "<<m_idHelperSvc->toString(reElement->identify())
                        <<" position "<<Amg::toString(center, 2)
                        <<" perp: "<<center.perp()
                        <<" phi: "<<(center.phi() / Gaudi::Units::deg)
                        <<" theta: "<<(center.theta() / Gaudi::Units::deg)
                        <<" rSize: "<<reElement->getRsize()<<"/"<<reElement->getLongRsize()
                        <<" sSize: "<<reElement->getSsize()<<"/"<<reElement->getLongSsize()
                        <<" zSize: "<<reElement->getZsize()<<"/"<<reElement->getLongZsize()
                        <<" dPhi: "<<(prevRE->center().deltaPhi(center) / Gaudi::Units::deg));        
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::dumpToTree(const EventContext& ctx, const TgcReadoutElement* readoutEle) {
    m_stIndex = readoutEle->getStationIndex();
    m_stEta   = readoutEle->getStationEta();
    m_stPhi   = readoutEle->getStationPhi();
    m_nGasGaps = readoutEle->nGasGaps();
    ATH_MSG_DEBUG("Dump readout element "<<m_idHelperSvc->toString(readoutEle->identify()));

    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};

    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform = trans;
    m_shortWidth = readoutEle->getSsize();
    m_longWidth = readoutEle->getLongSsize();
    m_height = readoutEle->length();

    m_thickness = readoutEle->getZsize();
    m_stLayout = readoutEle->getTechnologyName();

    const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
    m_alignableNode = station->getGeoTransform()->getDefTransform() *
                      station->getNativeToAmdbLRS().inverse();

    if (station->hasALines()){ 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }

    for (bool isStrip : {false, true}) {        
        for (int layer = 1 ; layer <= readoutEle->numberOfLayers(isStrip); ++layer){
            const unsigned int nChan = isStrip ? nStrips(*readoutEle, layer) :
                                                 readoutEle->nWireGangs(layer);
            if (!nChan) continue;
            const Identifier layerId = idHelper.channelID(readoutEle->identify(),layer, isStrip, 1);
            m_layTans.push_back(readoutEle->surface(layerId).transform());
            m_layMeasPhi.push_back(isStrip);
            m_layNumber.push_back(layer);
            m_layHeight.push_back(readoutEle->length());
            m_layShortWidth.push_back(readoutEle->getSsize() /*- readoutEle->frameXwidth() * 2. */);
            m_layLongWidth.push_back(readoutEle->getLongSsize()   /*- readoutEle->frameXwidth() * 2. */);
            unsigned int numWires = !isStrip ? readoutEle->nWires(layer) : 0;
            m_layNumWires.push_back(numWires);
            
            if (isStrip) {
                /// The last strip is for one reason always 0
                for (int strip = 1; strip <= nStrips(*readoutEle,layer); ++strip) {
                    bool is_valid{false};
                    const Identifier stripId = idHelper.channelID(readoutEle->identify(), layer, isStrip, strip, is_valid);
                    if (!is_valid) continue;
                    
                    /// Strip center
                    const Amg::Vector3D globStripPos = readoutEle->channelPos(stripId);
                    Amg::Vector2D locStripPos{Amg::Vector2D::Zero()};
                    const Trk::Surface& surf{readoutEle->surface(stripId)};
                    if (!surf.globalToLocal(globStripPos, Amg::Vector3D::Zero(), locStripPos)){
                        ATH_MSG_FATAL("Failed to build local strip position "<<m_idHelperSvc->toString(stripId));
                        return StatusCode::FAILURE;
                    }
                    m_locStripCenter.push_back(locStripPos);
                    m_stripCenter.push_back(globStripPos);
                    /// Strip bottom & top edges
                    const double stripHalfLength = readoutEle->stripLength() / 2.;

                    const Amg::Vector2D locStripBot{readoutEle->stripPosOnShortBase(strip), -stripHalfLength};                    
                    const Amg::Vector2D locStripTop{readoutEle->stripPosOnLargeBase(strip), stripHalfLength};
                    const Amg::Vector3D globStripBot{surf.localToGlobal(locStripBot)};
                    const Amg::Vector3D globStripTop{surf.localToGlobal(locStripTop)};
                    
                    m_stripBottom.push_back(globStripBot);
                    m_stripTop.push_back(globStripTop);
    
                    m_locStripBottom.push_back(locStripBot);
                    m_locStripTop.push_back(locStripTop);
  
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                    m_stripLength.push_back(stripHalfLength * 2.);
                    m_stripPitch.push_back(readoutEle->stripPitch(layer,strip));
                    m_stripShortWidth.push_back(readoutEle->stripShortWidth(layer, strip));
                    m_stripLongWidth.push_back(readoutEle->stripLongWidth(layer, strip));
                }
            } else {
                /// The last gang is for one reason always 0
                for (int gang = 1; gang <= readoutEle->nWireGangs(layer); ++gang) {
                       const Identifier gangId{idHelper.channelID(readoutEle->identify(), layer, isStrip, gang)};
                       const Trk::Surface& surf{readoutEle->surface(gangId)};
                       const Amg::Vector3D globPos{readoutEle->wireGangPos(layer, gang)};
                       Amg::Vector2D locPos{Amg::Vector2D::Zero()};
                       if (!surf.globalToLocal(globPos,Amg::Vector3D::Zero(),locPos)) {
                           ATH_MSG_FATAL("Failed to extract local position "<<m_idHelperSvc->toString(gangId));
                           return StatusCode::FAILURE;
                       }
                       m_locGangPos.push_back(locPos);
                       m_gangCenter.push_back(globPos);
                       m_gangGasGap.push_back(layer);
                       m_gangNum.push_back(gang);
                       m_gangNumWires.push_back(readoutEle->nWires(layer, gang));
                       m_gangLength.push_back(0.5 *(readoutEle->gangShortWidth(layer, gang) + 
                                                    readoutEle->gangLongWidth(layer, gang) ));
                }
            }
        }    
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

void GeoModelTgcTest::dumpReadoutXML(const MuonGM::MuonDetectorManager& detMgr) {
    if (m_readoutXML.empty()) {
        return;
    }
    std::ofstream xmlStream{m_readoutXML};
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    std::map<Identifier, TgcChamberLayout> allLayouts{};
    for (TgcIdHelper::const_id_iterator itr = idHelper.module_begin();
                                        itr != idHelper.module_end(); ++itr) {
        const MuonGM::TgcReadoutElement* reEle = detMgr.getTgcReadoutElement(*itr);
        if (!reEle) continue;
        for (bool isStrip : {false, true}) {
            for (int layer = 1 ; layer <= reEle->numberOfLayers(isStrip); ++layer){
                const Identifier layerId = idHelper.channelID(reEle->identify(), layer, isStrip, 1);
                TgcChamberLayout& chambLayout = allLayouts[m_idHelperSvc->gasGapId(layerId)];
                chambLayout.gasGap = m_idHelperSvc->gasGapId(layerId);
                chambLayout.techType = reEle->getTechnologyName();
                if (isStrip && nStrips(*reEle, layer)) {
                   const double halfHeight = 0.5 * (reEle->getRsize() - 2. * reEle->physicalDistanceFromBase());
                   const Amg::Transform3D globToLoc{reEle->surface(layerId).transform().inverse() * reEle->absTransform()};
                   const double sign = (reEle->getStationEta()> 0. ? -1. : 1.) *( (globToLoc*Amg::Vector3D::UnitY()).x() > 0 ? 1. : -1);

                   for (int strip = 1; strip < 33; ++strip) {
                        /// Note the slight shift in the coordinate system given that the positions in the legacy
                        /// are given w.r.t. strip center while for the new geometry we need them w.r.t. strip edge
                        chambLayout.botStripPos.push_back(sign *reEle->stripLowEdgeLocX(layer,strip, -halfHeight));
                        chambLayout.topStripPos.push_back(sign *reEle->stripLowEdgeLocX(layer,strip, +halfHeight));
                        if (strip != 32) continue;
                        chambLayout.botStripPos.push_back(sign *reEle->stripHighEdgeLocX(layer,strip, -halfHeight));
                        chambLayout.topStripPos.push_back(sign *reEle->stripHighEdgeLocX(layer,strip, +halfHeight));
                   }
                } else if (!isStrip) {
                    unsigned int accumlWires{0};
                    chambLayout.wirePitch = reEle->wirePitch();
                    /// Another reason to love AMDB. Summing up the number of wires in a gang does not match the
                    /// number of wires in the gasgap, because the last gang has always 0 entries. However, the total
                    /// number of wires is used in the legacy geometry to calculate the position of the first wire. I am
                    /// amazed about the precision to get the N/2 wire right into the center of the chamber. Anyhow, let's 
                    /// insert this hack to have a proper number of wires in the last gang.
                    for (int gang = 1; gang <= reEle->nWireGangs(layer); ++gang) {
                        unsigned int nWires = reEle->nWires(layer , gang);
                        accumlWires+=nWires;
                        if (nWires) {
                            chambLayout.wireGangLayout.push_back(nWires);
                        } else {
                            chambLayout.wireGangLayout.push_back(reEle->nWires(layer) - accumlWires);
                            break;
                        }
                    }
                }
            }
        }
    }
    /// Select the set of all layouts that are belonging together
    std::vector<ChamberGrp> groupedLayouts{};
    for (const auto& lay : allLayouts) {
        bool added{false};
        for (ChamberGrp& grp : groupedLayouts) {
            if (grp.addChamber(lay.second)){
                added = true;
                break;
            }
        }
        if (!added) groupedLayouts.emplace_back(lay.second);
    }
    allLayouts.clear();
    std::stable_sort(groupedLayouts.begin(),groupedLayouts.end(), 
                    [](const ChamberGrp& a, const ChamberGrp& b){
                        return a.layout().techType < b.layout().techType;
                    });
    /// All added
    ATH_MSG_INFO("Found in total "<<groupedLayouts.size()<<" different chamber layouts");
    xmlStream<<"<Table name=\"TgcSensorLayout\">"<<std::endl;
    unsigned int counter{1};
    for (const ChamberGrp& grp : groupedLayouts) {
        for (const auto& [tech_type, gapIds]: grp.allGaps()) {
            std::set<int> gaps{};
            std::set<char> sides{};         
            for (const Identifier gapId : gapIds) {
                gaps.insert(m_idHelperSvc->gasGap(gapId));
                sides.insert(m_idHelperSvc->stationEta(gapId) > 0 ? 'A' : 'C');
            }
            xmlStream<<"    <Row ";
            xmlStream<<"TGCSENSORLAYOUT_DATA_ID=\""<<counter<<"\" ";
            xmlStream<<"technology=\""<<tech_type<<"\" ";
            xmlStream<<"gasGap=\""<<gaps<<"\" ";
            xmlStream<<"side=\""<<sides<<"\" "; 
            xmlStream<<"wirePitch=\""<<grp.layout().wirePitch<<"\" ";
            xmlStream<<"wireGangs=\""<<grp.layout().wireGangLayout<<"\" ";
            xmlStream<<"bottomStrips=\""<<grp.layout().botStripPos<<"\" ";
            xmlStream<<"topStrips=\""<<grp.layout().topStripPos<<"\" ";
            xmlStream<<" />"<<std::endl;
            ++counter;
        }
    }
    xmlStream<<"</Table> "<<std::endl;
}


}
