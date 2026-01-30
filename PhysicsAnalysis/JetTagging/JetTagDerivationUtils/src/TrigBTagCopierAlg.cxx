/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBTagCopierAlg.h"

namespace ftag {

    TrigBTagCopierAlg::TrigBTagCopierAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode TrigBTagCopierAlg::initialize() {
        if (m_trigger_edm_version == 3) {
            ATH_MSG_INFO("Using Run 3 EDM version for TrigBTagCopierAlg");
            ATH_MSG_INFO("For Run 3, we do index matching for data and link navigation for MC.");
            for (const auto& [btagKey, jetKey] : m_conts_pair_run3) {
                SG::ReadHandleKey<xAOD::BTaggingContainer> btagRHK { btagKey };
                SG::ReadHandleKey<xAOD::JetContainer>      jetRHK  { jetKey  };
                ATH_CHECK(btagRHK.initialize());
                ATH_CHECK(jetRHK.initialize());
                std::vector<SG::ReadDecorHandleKey<xAOD::BTaggingContainer>> decorReadKeys;
                std::vector<SG::WriteDecorHandleKey<xAOD::JetContainer>>     decorWriteKeys;
                for (const std::string &var : m_vars_run3) {
                    decorReadKeys.emplace_back(btagKey + "." + var);
                    decorWriteKeys.emplace_back(jetKey + "." + var);
                    ATH_CHECK(decorReadKeys.back().initialize());
                    ATH_CHECK(decorWriteKeys.back().initialize());
                }
                m_decorKeys.emplace_back(btagRHK, jetRHK, decorReadKeys, decorWriteKeys);
            }
        } 
        return StatusCode::SUCCESS;
    }

    StatusCode TrigBTagCopierAlg::execute(const EventContext& ctx) const {
        if (m_trigger_edm_version != 3){
            ATH_MSG_DEBUG("Run 2 TrigBTagCopierAlg does not copy anything to keep consistency between data and MC");
            ATH_MSG_DEBUG("Please see https://its.cern.ch/jira/browse/ATR-26904 for more details.");
            ATH_MSG_DEBUG("Run 4 onward doesn't need this.");
            return StatusCode::SUCCESS;
        }

        for (const auto& [btagRHK, jetRHK, decorReadKeys, decorWriteKeys] : m_decorKeys) {
            // retrieve containers
            SG::ReadHandle<xAOD::BTaggingContainer> btaggingContHandle(btagRHK, ctx);
            if (!btaggingContHandle.isValid()) {
                ATH_MSG_DEBUG (btagRHK.key() << " is not available in this event.");
                continue;
            }
            SG::ReadHandle<xAOD::JetContainer> jetContHandle(jetRHK, ctx);
            if (!jetContHandle.isValid()) {
                ATH_MSG_ERROR (jetRHK.key() << " is not available in this event but btagging is. Cannot proceed.");
                return StatusCode::FAILURE;
            }
            const xAOD::BTaggingContainer* btaggingCont = btaggingContHandle.cptr();
            const xAOD::JetContainer* jetCont = jetContHandle.cptr();

            //prepare decorators and accessors
            std::vector<std::tuple<
                std::string, // variable name
                SG::ReadDecorHandle <xAOD::BTaggingContainer, float>, // accessor
                SG::WriteDecorHandle<xAOD::JetContainer,      float> // decorator
            >> decoMap;
            for (size_t j = 0; j < decorReadKeys.size(); ++j) {
                std::string var = decorReadKeys[j].key();
                SG::ReadDecorHandle <xAOD::BTaggingContainer, float> acc(decorReadKeys[j],  ctx);
                SG::WriteDecorHandle<xAOD::JetContainer,      float> dec(decorWriteKeys[j], ctx);
                decoMap.emplace_back(std::move(var), std::move(acc), std::move(dec));
            }
            // loop over btagging objects and copy decorations
            for (size_t i = 0; i < btaggingCont->size(); ++i) {
                const xAOD::BTagging* btagging = btaggingCont->at(i);
                const xAOD::Jet* jet = jetCont->at(i);
                ATH_MSG_DEBUG("Processing btagging object " << i << " btag: " << btagging << " and jet: " << jet);
                for (auto& [var, acc, dec] : decoMap) {
                    if (acc.isAvailable()) {
                        dec(*jet) = acc(*btagging);
                        ATH_MSG_DEBUG("Decorator " << var << " was copied");
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode TrigBTagCopierAlg::finalize () {
        return StatusCode::SUCCESS;
    }

}// end namespace ftag
