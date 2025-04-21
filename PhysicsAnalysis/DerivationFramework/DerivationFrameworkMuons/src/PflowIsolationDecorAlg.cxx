/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PflowIsolationDecorAlg.h"
#include "DerivationFrameworkMuons/Utils.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>

//**********************************************************************
namespace DerivationFramework {
PflowIsolationDecorAlg::PflowIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}
//**********************************************************************

StatusCode PflowIsolationDecorAlg::initialize() {
    ATH_CHECK(m_isoTool.retrieve());

    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));
    // isolation types to run. The ptcones each also imply the respective ptvarcone.

    ATH_CHECK(m_trk_key.initialize());
    for (const std::string& decor : m_trkSel_Decors) m_trkSel_keys.emplace_back(m_trk_key, decor);
    ATH_CHECK(m_trkSel_keys.initialize());
    m_neflowCone20_key = std::string{"neflowisol20"} + (m_customName.empty() ? "" : "_") + m_customName;
    m_neflowCone30_key = std::string{"neflowisol30"} + (m_customName.empty() ? "" : "_") + m_customName;
    m_neflowCone40_key = std::string{"neflowisol40"} + (m_customName.empty() ? "" : "_") + m_customName;
    ATH_CHECK(m_neflowCone20_key.initialize());
    ATH_CHECK(m_neflowCone30_key.initialize());
    ATH_CHECK(m_neflowCone40_key.initialize());

    ATH_MSG_DEBUG("Decorate " << m_trk_key.fullKey() << " using '" << m_customName << "' as suffix.");
    return StatusCode::SUCCESS;
}

//**********************************************************************

StatusCode PflowIsolationDecorAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trk_key, ctx};
    if (!tracks.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve track collection " << m_trk_key.fullKey());
        return StatusCode::FAILURE;
    }
    

    using FloatDecor = SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>;
    using SelDecorator = SG::ReadDecorHandle<xAOD::TrackParticleContainer, bool>;
    
    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>& key : m_trkSel_keys){
        selDecors.emplace_back(key, ctx);
    }
    FloatDecor neflowCone40_dec{makeHandle<float>(ctx,m_neflowCone40_key, -Gaudi::Units::GeV)};
    FloatDecor neflowCone30_dec{makeHandle<float>(ctx,m_neflowCone30_key, -Gaudi::Units::GeV)};
    FloatDecor neflowCone20_dec{makeHandle<float>(ctx,m_neflowCone20_key, -Gaudi::Units::GeV)};
    

    for (const xAOD::TrackParticle* trk : *tracks) {
        if (trk->pt() < m_pt_min) continue;
        if (!selDecors.empty() && std::find_if(selDecors.begin(), selDecors.end(), [trk](const SelDecorator& dec){
                return dec(*trk);
        }) == selDecors.end()) continue;
        ATH_MSG_DEBUG("Recomputing isolation by hand");
        xAOD::CaloIsolation resultCalo;
        if (!m_isoTool->neutralEflowIsolation(resultCalo, *trk, m_pflow_isos, m_calo_corr)) {
            ATH_MSG_ERROR("Failed to compute calorimeter isolation");
            return StatusCode::FAILURE;
        }
        neflowCone40_dec(*trk) = resultCalo.etcones[0];
        neflowCone30_dec(*trk) = resultCalo.etcones[1];
        neflowCone20_dec(*trk) = resultCalo.etcones[2];        
    }
    return StatusCode::SUCCESS;
}
}
//**********************************************************************
