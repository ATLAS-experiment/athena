/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"

#include <fstream>
#include <iostream>

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
namespace MuonGM {

std::ostream& operator<<(std::ostream& ostr, const Amg::Transform3D& trans){
    ostr<<Amg::toString(trans, 3);
    return ostr;
}  
StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_cablingKey.initialize(!m_cablingKey.empty()));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));

    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    auto translateTokenList = [this, &idHelper](const std::vector<std::string>& chNames){

        std::set<Identifier> transcriptedIds{};
        for (const std::string& token : chNames) { 
            if (token.size() != 6) {
                ATH_MSG_WARNING("Wrong format given for "<<token<<". Expecting 6 characters");
                continue;
            }
            /// Example string BIL1A3
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
StatusCode GeoModelMdtTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    for (const Identifier& test_me : m_testStations) {
        const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " << detStr);
        const MdtReadoutElement* reElement = detMgr->getMdtReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << detStr << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::dumpToTree(const EventContext& ctx, const MdtReadoutElement* readoutEle) {
    m_stIndex = readoutEle->getStationIndex();
    m_stEta = readoutEle->getStationEta();
    m_stPhi = readoutEle->getStationPhi();
    m_stML = readoutEle->getMultilayer();
    m_chamberDesign = readoutEle->getTechnologyName();

    m_numTubes = readoutEle->getNtubesperlayer();
    m_numLayers = readoutEle->getNLayers();

    m_tubeRad = readoutEle->innerTubeRadius();
    m_tubePitch = readoutEle->tubePitch();
    
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
    const BLinePar* bline = readoutEle->getBLinePar();
    if (bline) {
        using Parameter = BLinePar::Parameter;
        m_BLineBz = bline->getParameter(Parameter::bz);
        m_BLineBp = bline->getParameter(Parameter::bp);
        m_BLineBn = bline->getParameter(Parameter::bn);
        m_BLineSp = bline->getParameter(Parameter::sp);
        m_BLineSn = bline->getParameter(Parameter::sn);
        m_BLineTw = bline->getParameter(Parameter::tw);
        m_BLinePg = bline->getParameter(Parameter::pg);
        m_BLineTr = bline->getParameter(Parameter::tr);
        m_BLineEg = bline->getParameter(Parameter::eg);
        m_BLineEp = bline->getParameter(Parameter::ep);
        m_BLineEn = bline->getParameter(Parameter::en);
    }
    
    if (station->hasMdtAsBuiltParams()) {
        const MdtAsBuiltPar* asBuilt = station->getMdtAsBuiltParams();
        using multilayer_t = MdtAsBuiltPar::multilayer_t;
        using tubeSide_t   = MdtAsBuiltPar::tubeSide_t;
        const multilayer_t asBuiltMl = readoutEle->getMultilayer() == 1 ? multilayer_t::ML1  : multilayer_t::ML2;
        m_asBuiltPosY0 = asBuilt->y0(asBuiltMl, tubeSide_t::POS);
        m_asBuiltPosZ0 = asBuilt->z0(asBuiltMl, tubeSide_t::POS);
        m_asBuiltPosAlpha = asBuilt->alpha (asBuiltMl, tubeSide_t::POS);
        m_asBuiltPosPitchY = asBuilt->ypitch(asBuiltMl, tubeSide_t::POS);
        m_asBuiltPosPitchZ = asBuilt->zpitch(asBuiltMl, tubeSide_t::POS);
        m_asBuiltPosStagg = asBuilt->stagg (asBuiltMl, tubeSide_t::POS);

        m_asBuiltNegY0 = asBuilt->y0(asBuiltMl, tubeSide_t::NEG);
        m_asBuiltNegZ0 = asBuilt->z0(asBuiltMl, tubeSide_t::NEG);
        m_asBuiltNegAlpha = asBuilt->alpha (asBuiltMl, tubeSide_t::NEG);
        m_asBuiltNegPitchY = asBuilt->ypitch(asBuiltMl, tubeSide_t::NEG);
        m_asBuiltNegPitchZ = asBuilt->zpitch(asBuiltMl, tubeSide_t::NEG);
        m_asBuiltNegStagg = asBuilt->stagg (asBuiltMl, tubeSide_t::NEG);
    }

    const MuonMDT_CablingMap* cabling{nullptr};
    if (!m_cablingKey.empty()){
         SG::ReadCondHandle<MuonMDT_CablingMap> cablingHandle{m_cablingKey, ctx};
         ATH_CHECK(cablingHandle.isValid());
         cabling = cablingHandle.cptr();
    }

    const Amg::Transform3D trans{readoutEle->getMaterialGeom()->getAbsoluteTransform()};
    m_readoutTransform = readoutEle->getMaterialGeom()->getAbsoluteTransform();
    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    
    for (int lay = 1; lay <= readoutEle->getNLayers(); ++lay) {
        for (int tube = 1; tube <= readoutEle->getNtubesperlayer(); ++tube) {
            bool is_valid{false};
            const Identifier tube_id =id_helper.channelID(readoutEle->identify(), 
                                                          readoutEle->getMultilayer(), 
                                                          lay, tube, is_valid);
            if (!is_valid) continue;
            if (m_idHelperSvc->stationNameString(tube_id) == "BMG") {
                try{
                    readoutEle->transform(tube_id);
                } catch (const std::runtime_error& err ){
                    ATH_MSG_VERBOSE("Tube does not exist "<<err.what());
                    continue;
                }
            }            
            const Amg::Transform3D layTransf{readoutEle->transform(tube_id)};
            m_tubeLay.push_back(lay);
            m_tubeNum.push_back(tube);

            m_tubeTransform.push_back(layTransf);

            const Amg::Vector3D roPos = readoutEle->ROPos(tube_id);
            m_roPos.push_back(roPos);
            m_activeTubeLength.push_back(readoutEle->getActiveTubeLength(lay,tube));
            m_tubeLength.push_back(readoutEle->tubeLength(tube_id));
            m_wireLength.push_back(readoutEle->getWireLength(lay, tube));
            if (cabling) {
                MdtCablingData translation{};
                if (!cabling->convert(tube_id, translation) ||
                    !cabling->getOnlineId(translation, msgStream())){
                    ATH_MSG_FATAL("Cabling translation failed");
                    return StatusCode::FAILURE;
                }
                m_cablingCSM.push_back(translation.csm);
                m_cablingMROD.push_back(translation.mrod);
                m_cablingTdcId.push_back(translation.tdcId);
                m_cablingTdcCh.push_back(translation.channelId);
            }
        }
    }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
}  // namespace MuonGM
