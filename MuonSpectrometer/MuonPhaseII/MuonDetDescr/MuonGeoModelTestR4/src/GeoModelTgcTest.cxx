
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTgcTest.h"
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;

namespace MuonGMR4{

StatusCode GeoModelTgcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    /// Prepare the TTree dump
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
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};

    const ActsTrk::GeometryContext* geoContextHandle{nullptr};
    ATH_CHECK(SG::get(geoContextHandle, m_geoCtxKey, ctx));
    const ActsTrk::GeometryContext& gctx{*geoContextHandle};

    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of Tgc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const TgcReadoutElement* reElement = m_detMgr->getTgcReadoutElement(test_me);
      if (!reElement) {
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                      <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
         return StatusCode::FAILURE;
      }
      const Amg::Transform3D globToLocal{reElement->globalToLocalTransform(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTransform(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      if (!Amg::doesNotDeform(transClosure)) {
            ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)
                        <<". Ended up with "<< Amg::toString(transClosure) );
            return StatusCode::FAILURE;                  
      }
      const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
      for (unsigned gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (bool isStrip : {false, true}) {
            const IdentifierHash layHash = reElement->constructHash(0, gasGap, isStrip);
            const unsigned nChan = reElement->numChannels(layHash);
            for (unsigned chan = 1; chan <= nChan ; ++chan) {
                bool isValid{false};
                const Identifier channelId = id_helper.channelID(reElement->identify(),
                                                                 gasGap, isStrip, chan, isValid);
                if (!isValid) {
                    ATH_MSG_DEBUG("No valid Identifier constructed from the fields "
                                <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                <<"isStrip: "<<(isStrip ? "yay" : "nay")<<" gasGap: "<<gasGap<<
                                " channel: "<<chan);
                    continue;
                }
                const IdentifierHash measHash{reElement->measurementHash(channelId)};
                const Identifier backCnv = reElement->measurementId(measHash);
                if (backCnv != channelId) {
                    ATH_MSG_FATAL("Forward-backward conversion of the Identifier "<<m_idHelperSvc->toString(channelId)
                                <<"failed. Got instead "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (reElement->layerHash(channelId) != reElement->layerHash(measHash)) {
                    ATH_MSG_FATAL("The cosntruction of the layer hash from the Identifier "<<m_idHelperSvc->toString(channelId)
                    <<" gave something else than doing it from the measurement hash "<<measHash<<". "<<
                    reElement->layerHash(channelId)<<" vs. "<<reElement->layerHash(measHash));
                }
            }
        }        
    } 
    ATH_CHECK(dumpToTree(ctx, gctx, reElement));  
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::dumpToTree(const EventContext& ctx,
                                       const ActsTrk::GeometryContext& gctx, 
                                       const TgcReadoutElement* reElement) {
   
   m_stIndex    = reElement->stationName();
   m_stEta      = reElement->stationEta();
   m_stPhi      = reElement->stationPhi();
   m_stLayout   = reElement->chamberDesign();
   m_nGasGaps   = reElement->nGasGaps();
   m_readoutTransform = reElement->localToGlobalTransform(gctx);

   m_alignableNode  = reElement->alignableTransform()->getDefTransform();

   m_shortWidth = reElement->moduleWidthS();
   m_longWidth = reElement->moduleWidthL();
   m_height = reElement->moduleHeight();
   m_thickness = reElement->moduleThickness();

   for (unsigned gap = 1; gap <= reElement->nGasGaps(); ++gap) {
        const IdentifierHash layHash = reElement->constructHash(0, gap, true);
        /// Loop over all strips dump their respective information
        for (unsigned strip = 1 ; strip <= reElement->numStrips(layHash); ++strip) {
            const IdentifierHash measHash = reElement->constructHash(strip, gap, true);
            const RadialStripDesign& layout{reElement->stripLayout(measHash)};

            const Amg::Transform3D localToGlobal{reElement->localToGlobalTransform(gctx , 
                                                                                reElement->layerHash(measHash)) *
                                                (Amg::getRotateZ3D(-90.*Gaudi::Units::deg))};
            if (strip == 1) {
                m_layTans.push_back(localToGlobal);
                m_layMeasPhi.push_back(true);
                m_layNumber.push_back(gap);
                m_layShortWidth.push_back(2.*layout.shortHalfHeight());
                m_layLongWidth.push_back(2.*layout.longHalfHeight());
                m_layHeight.push_back(2.*layout.halfWidth());
                m_layNumWires.push_back(0);
            }
            m_stripGasGap.push_back(gap);
            m_stripNum.push_back(strip);
            m_stripCenter.push_back(reElement->channelPosition(gctx, measHash));
            const auto sensor = reElement->sensorLayout(measHash);
            const Amg::Vector2D locTop2D{layout.leftEdge(strip).value_or(Amg::Vector2D::Zero())};
            const Amg::Vector2D locBot2D{layout.rightEdge(strip).value_or(Amg::Vector2D::Zero())};
            const Amg::Vector3D globTop{localToGlobal * sensor->to3D(layout.leftEdge(strip), true)};
            const Amg::Vector3D globBot{localToGlobal * sensor->to3D(layout.rightEdge(strip), true)};
            m_stripBottom.push_back(globBot);
            m_stripTop.push_back(globTop);
            m_locStripTop.push_back(locTop2D);
            m_locStripCenter.push_back(layout.center(strip).value_or(Amg::Vector2D::Zero()));
            m_locStripBottom.push_back(locBot2D);

        }
        /// Loop over all wire gangs dump their respective information
        for (unsigned gang = 1; gang <= reElement->numWireGangs(layHash); ++gang) {
            const IdentifierHash measHash = reElement->constructHash(gang, gap, false);
            const WireGroupDesign& layout{reElement->wireGangLayout(measHash)};
            if (gang == 1) {
                m_layTans.push_back(reElement->localToGlobalTransform(gctx, reElement->layerHash(measHash)));
                m_layMeasPhi.push_back(false);
                m_layNumber.push_back(gap);
                m_layShortWidth.push_back(2.*layout.shortHalfHeight());
                m_layLongWidth.push_back(2.*layout.longHalfHeight());
                m_layHeight.push_back(2.*layout.halfWidth());
                m_layNumWires.push_back(layout.nAllWires());
            }
            m_gangNum.push_back(gang);
            m_gangGasGap.push_back(gap);
            m_gangCenter.push_back(reElement->channelPosition(gctx, measHash));
            m_gangNumWires.push_back(layout.numWiresInGroup(gang));
            m_locGangPos.push_back(layout.center(gang).value_or(Amg::Vector2D::Zero()));
            m_gangLength.push_back(layout.stripLength(gang));
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

