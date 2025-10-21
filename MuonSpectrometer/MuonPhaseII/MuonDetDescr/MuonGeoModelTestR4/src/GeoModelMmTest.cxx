
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMmTest.h"
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;

namespace MuonGMR4{

StatusCode GeoModelMmTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    /// Prepare the TTree dump
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
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    const ActsTrk::GeometryContext* geoContextHandle{nullptr};
    ATH_CHECK(SG::get(geoContextHandle, m_geoCtxKey, ctx));
    const ActsTrk::GeometryContext& gctx{*geoContextHandle};

    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_DEBUG("Test retrieval of Mm detector element "<<m_idHelperSvc->toStringDetEl(test_me));
        const MmReadoutElement* reElement = m_detMgr->getMmReadoutElement(test_me);
        if (!reElement) {
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                        <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }      
        const Amg::Transform3D globToLocal{reElement->globalToLocalTrans(gctx)};
        const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
        /// Closure test that the transformations actually close
        if (!Amg::doesNotDeform(globToLocal * localToGlob)) {
                ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)
                            <<" "<<Amg::toString(globToLocal * localToGlob));
                return StatusCode::FAILURE;
        }
        const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
        for (unsigned int layer = 1; layer <= reElement->nGasGaps(); ++layer) {
            const IdentifierHash layerHash{MuonGMR4::MmReadoutElement::createHash(layer, 0)};
            const int numStrips = reElement->numStrips(layerHash);
            const int fStrip = reElement->firstStrip(layerHash);
            const int lStrip = fStrip+numStrips-1;
            
            for (int strip = fStrip; strip < lStrip; ++strip) {
                bool isValid{false};
                
                const Identifier chId = id_helper.channelID(reElement->identify(),
                                                            reElement->multilayer(),
                                                            layer, strip, isValid);
                if (!isValid) {
                    continue;
                }

                /// Test the back and forth conversion of the Identifier
                const IdentifierHash channelHash = reElement->measurementHash(chId);
                const IdentifierHash layHash = reElement->layerHash(chId);
                const Identifier backCnv = reElement->measurementId(channelHash);
                if (backCnv != chId) {
                    ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chId)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (layHash != reElement->layerHash(channelHash)) {
                    ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                m_idHelperSvc->toString(chId)<<" leads to different layer hashes "<<
                                layHash<<" vs. "<< reElement->layerHash(channelHash));
                    return StatusCode::FAILURE;
                }
                const MuonGMR4::StripDesign& design{reElement->stripLayer(layHash).design()};
                const Amg::Vector3D stripPos = reElement->stripPosition(gctx, channelHash);
                const Amg::Vector3D locStripPos = reElement->globalToLocalTrans(gctx, layHash) * stripPos;
                const Amg::Vector2D stripPos2D{locStripPos.block<2,1>(0,0)};
                const double stripLen{design.stripLength(strip)};
                if (stripLen && (design.stripNumber(stripPos2D) != strip ||
                    design.stripNumber(stripPos2D - 0.49 * stripLen * Amg::Vector2D::UnitY()) != strip ||
                    design.stripNumber(stripPos2D + 0.49 * stripLen * Amg::Vector2D::UnitY()) != strip)) {
                    ATH_MSG_FATAL("Conversion channel -> strip -> channel failed for "
                        <<m_idHelperSvc->toString(chId)<<" "<<Amg::toString(stripPos)<<", local: "
                        <<Amg::toString(locStripPos)<<" got "<<design.stripNumber(stripPos2D)
                        <<", first strip: "<<fStrip<<std::endl<<design);
                    return StatusCode::FAILURE;
                }
                ATH_MSG_VERBOSE("first strip "<<fStrip<<", numStrips "<< numStrips << ", channel "
                              << m_idHelperSvc->toString(chId) <<", strip position " << Amg::toString(stripPos));
            }
        }
        ATH_CHECK(dumpToTree(ctx,gctx,reElement));
    }   

   return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::dumpToTree(const EventContext& ctx,
                                       const ActsTrk::GeometryContext& gctx, 
                                       const MmReadoutElement* reElement) {



    m_stIndex    = reElement->stationName();
    m_stEta      = reElement->stationEta();
    m_stPhi      = reElement->stationPhi();
    m_stML       = reElement->multilayer();
    m_chamberDesign = reElement->chamberDesign();
    m_stStripPitch = reElement->stripLayer(MuonGMR4::MmReadoutElement::createHash(1,1)).design().stripPitch();
    ///
    /// Dump the local to global transformation of the readout element
    const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
    m_readoutTransform = transform;
    m_alignableNode  = reElement->alignableTransform()->getDefTransform();

    ///
    m_moduleHeight = reElement->moduleHeight();
    m_moduleWidthS = reElement->moduleWidthS();
    m_moduleWidthL = reElement->moduleWidthL();

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
    for (unsigned int layer = 1; layer <= reElement->nGasGaps(); ++layer) {

        const IdentifierHash layHash{MuonGMR4::MmReadoutElement::createHash(layer, 0)};
        unsigned int numStrips = reElement->numStrips(layHash);
        unsigned int fStrip = reElement->firstStrip(layHash);
        unsigned int lStrip = fStrip+numStrips-1;

        for (unsigned int strip = fStrip; strip <= lStrip ; ++strip) {
            bool isValid{false};
            const Identifier chId = id_helper.channelID(reElement->identify(),
                                                            reElement->multilayer(),
                                                            layer, strip, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                <<" layer: "<<layer<<" strip: "<<strip);
                continue;
            }
            const IdentifierHash measHash{reElement->measurementHash(chId)};

            const MuonGMR4::StripDesign& design{reElement->stripLayer(measHash).design()};
            if (strip == fStrip) {
                const Amg::Transform3D stripLocalToGlob = reElement->localToGlobalTrans(gctx, chId);
                ATH_MSG_VERBOSE(m_idHelperSvc->toStringGasGap(chId)<<" "<< "transform: " 
                            << Amg::toString(stripLocalToGlob)<<", perp: "<<stripLocalToGlob.translation().perp());
                m_stripRot.push_back(stripLocalToGlob);
                m_stripRotGasGap.push_back(layer);
                m_firstStripPos.push_back(design.firstStripPos());
                m_readoutSide.push_back(reElement->readoutSide(measHash));

                m_ActiveWidthS = reElement->gapLengthS(measHash);
                m_ActiveWidthL = reElement->gapLengthL(measHash);
                m_ActiveHeightR = reElement->gapHeight(measHash);
                m_firstStrip.push_back(design.firstStripNumber());
                m_nStrips.push_back(design.numStrips());
            }
            CheckVector2D center = design.center(strip);
            if (!center) {
                ATH_MSG_WARNING("Strip "<<m_idHelperSvc->toString(chId)<<" is outside bounds "<<design);
                continue;
            }
            m_stripLength.push_back(reElement->stripLength(measHash));
            m_locStripCenter.push_back(center.value());
            m_isStereo.push_back(design.hasStereoAngle());
            m_stripCenter.push_back(reElement->stripPosition(gctx, measHash));
            m_stripLeftEdge.push_back(reElement->leftStripEdge(gctx,measHash));
            m_stripRightEdge.push_back(reElement->rightStripEdge(gctx,measHash));            
            m_gasGap.push_back(layer);
            m_channel.push_back(strip);
        }
    }
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

